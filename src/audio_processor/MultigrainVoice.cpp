//
// Created by kobe on 19/11/2021.
//

#include "MultigrainVoice.h"

// MultigrainVoice
MultigrainVoice::MultigrainVoice(juce::AudioProcessorValueTreeState& apvts, MultigrainSound& sound)
        : apvts(apvts),
          samplesTillNextOnset(0),
          nextGrainToActivateIndex(0),
          sound(sound)
{
    // init grain array
    for(int i = 0; i < 8; i++)
        grains.add(new Grain(sound));
}

MultigrainVoice::~MultigrainVoice(){}

bool MultigrainVoice::canPlaySound(juce::SynthesiserSound* sound)
{
    return dynamic_cast<MultigrainSound*>(sound) != nullptr;
}

void MultigrainVoice::startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound* s, int /*currentPitchWheelPosition*/)
{
    if (auto* sound = dynamic_cast<const MultigrainSound*>(s))
    {
        deactivateGrains();
        pitchRatio = std::pow(2.0, (midiNoteNumber - sound->midiRootNote) / 12.0)
                     *sound->sourceSampleRate / getSampleRate();

        currentNoteInHertz = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
        samplesTillNextOnset = 0;
        grainSpawnPosition = apvts.getParameter("Position")->getValue()*sound->length;

        lgain = velocity;
        rgain = velocity;

        adsr.setSampleRate(getSampleRate());
        juce::ADSR::Parameters params(
                (float) apvts.getRawParameterValue("Synth Attack")->load()  / 1000.f,
                (float) apvts.getRawParameterValue("Synth Decay")->load()   / 1000.f,
                (float) apvts.getRawParameterValue("Synth Sustain")->load() / 100.f,
                (float) apvts.getRawParameterValue("Synth Release")->load() / 1000.f
        );
        adsr.setParameters(params);
        adsr.noteOn();
    }
}

void MultigrainVoice::stopNote(float /*velocity*/, bool allowTailOff)
{
    if (allowTailOff)
    {
        adsr.noteOff();
    }
    else
    {
        clearCurrentNote();
        adsr.reset();
    }
}

void MultigrainVoice::pitchWheelMoved(int) {}

void MultigrainVoice::controllerMoved(int, int) {}

void MultigrainVoice::renderNextBlock(juce::AudioSampleBuffer& outputBuffer, int startSample, int numSamples)
{
    if (!isVoiceActive())
        return;

    if (auto* playingSound = static_cast<MultigrainSound*> (getCurrentlyPlayingSound().get()))
    {
        auto grainDurationFactor = apvts.getRawParameterValue("Grain Duration")->load();
        int numGrains = apvts.getRawParameterValue("Num Grains")->load();

        auto grainDurationSamples = getSampleRate() * grainDurationFactor / currentNoteInHertz;
        auto samplesBetweenOnsets = juce::roundDoubleToInt(grainDurationSamples/(float) numGrains);

        // // Render all active grains
        // for(Grain* grain : grains)
        //     grain->renderNextBlock(outputBuffer, startSample, numSamples);

        // // Check if new grains need to be activated
        // while (samplesTillNextOnset < numSamples)
        // {
        //     Grain& grain = activateNextGrain(getNextGrainPosition(), juce::roundDoubleToInt(grainDurationSamples));
        //     grain.renderNextBlock(outputBuffer, startSample + samplesTillNextOnset, numSamples - samplesTillNextOnset);
        //     samplesTillNextOnset += samplesBetweenOnsets; // TODO allow randomness here
        //     updateGrainSpawnPosition(samplesBetweenOnsets);
        // }

        // samplesTillNextOnset -= numSamples;

        // adsr.applyEnvelopeToBuffer(outputBuffer, startSample, numSamples);
        // // outputBuffer.applyGain(1/(float) numGrains);

        float* outL = outputBuffer.getWritePointer(0, startSample);
        float* outR = outputBuffer.getNumChannels() > 1 ? outputBuffer.getWritePointer(1, startSample) : nullptr;

        while(--numSamples >= 0)
        {

            if(samplesTillNextOnset == 0)
            {
                activateNextGrain(getNextGrainPosition(), juce::roundDoubleToInt(grainDurationSamples));
                samplesTillNextOnset += samplesBetweenOnsets;
                updateGrainSpawnPosition(samplesBetweenOnsets);
            }
            for(Grain* grain : grains)
            {
                float grainSample = grain->getNextSample();
                if (outR != nullptr)
                {
                    *outL += grainSample;
                    *outR += grainSample;
                }
                else
                {
                    // is actually (left+right)/2
                    *outL += (grainSample + grainSample) * 0.5f;
                }
            }

            auto envelopeValue = adsr.getNextSample();

            *outL *= envelopeValue;
            *outR *= envelopeValue;
            outL++;
            outR++;

            if (!adsr.isActive())
            {
                clearCurrentNote();
                return;
            }

            samplesTillNextOnset--;
        }

    }
}

void MultigrainVoice::updateGrainSpawnPosition(int samplesBetweenOnsets)
{
    grainSpawnPosition += (float) samplesBetweenOnsets*apvts.getRawParameterValue("Grain Speed")->load();
    grainSpawnPosition = std::fmod(grainSpawnPosition, sound.length);
}

GrainPosition MultigrainVoice::getNextGrainPosition()
{
    auto randomRange = apvts.getParameter("Position Random")->getValue()*sound.length;
    auto randomDouble = randomGenerator.nextDouble();
    auto nextPosLeft = grainSpawnPosition + randomRange*randomDouble - randomRange/2;
    randomDouble = randomGenerator.nextDouble();
    auto nextPosRight = grainSpawnPosition + randomRange*randomDouble - randomRange/2;
    nextPosLeft = std::fmod(nextPosLeft, sound.length);
    nextPosRight = std::fmod(nextPosRight, sound.length);
    return {nextPosLeft, nextPosRight};
}

Grain& MultigrainVoice::activateNextGrain(GrainPosition grainPosition, int grainDurationInSamples)
{
    Grain* grain = grains[nextGrainToActivateIndex];
    // if (grain->isActive)
    //     jassertfalse; // grain voicestealing is happening
    grain->activate(
            grainDurationInSamples,
            grainPosition,
            pitchRatio,
            1.f // TODO allow randomization of this value
    );
    nextGrainToActivateIndex++;
    if (nextGrainToActivateIndex == grains.size())
        nextGrainToActivateIndex = 0;

    return *grain;
}

void MultigrainVoice::deactivateGrains()
{
    for(auto* grain : grains)
        grain->isActive = false;
}