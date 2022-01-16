//
// Created by kobe on 19/11/2021.
//

#include "MultigrainVoice.h"

// MultigrainVoice
MultigrainVoice::MultigrainVoice(juce::AudioProcessorValueTreeState& apvts, MultigrainSound& sound)
        : apvts(apvts),
          m_grainSpawnPosition{0.},
          m_currentNoteInHertz{440.},
          m_samplesTillNextOnset(0),
          m_nextGrainToActivateIndex(0),
          m_sound(sound)
{
    // init grain array
    for(int i = 0; i < 8; i++)
        m_grains.add(new Grain{sound});
}

bool MultigrainVoice::canPlaySound(juce::SynthesiserSound* sound)
{
    return dynamic_cast<MultigrainSound*>(sound) != nullptr;
}

void MultigrainVoice::startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound* s, int /*currentPitchWheelPosition*/)
{
    if (auto* sound = dynamic_cast<const MultigrainSound*>(s))
    {
        deactivateGrains();
        m_pitchRatio = std::pow(2.0, (midiNoteNumber - sound->midiRootNote) / 12.0)
                       * sound->sourceSampleRate / getSampleRate();

        m_currentNoteInHertz = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
        m_samplesTillNextOnset = 0;
        m_grainSpawnPosition = apvts.getParameter("Position")->getValue() * (float) sound->length;

        m_lGain = velocity;
        m_rGain = velocity;

        m_adsr.setSampleRate(getSampleRate());
        juce::ADSR::Parameters params(
                (float) apvts.getRawParameterValue("Synth Attack")->load()  / 1000.f,
                (float) apvts.getRawParameterValue("Synth Decay")->load()   / 1000.f,
                (float) apvts.getRawParameterValue("Synth Sustain")->load() / 100.f,
                (float) apvts.getRawParameterValue("Synth Release")->load() / 1000.f
        );
        m_adsr.setParameters(params);
        m_adsr.noteOn();
    }
}

void MultigrainVoice::stopNote(float /*velocity*/, bool allowTailOff)
{
    if (allowTailOff)
    {
        m_adsr.noteOff();
    }
    else
    {
        clearCurrentNote();
        m_adsr.reset();
    }
}

void MultigrainVoice::pitchWheelMoved(int) {}

void MultigrainVoice::controllerMoved(int, int) {}

void MultigrainVoice::renderNextBlock(juce::AudioSampleBuffer& outputBuffer, int startSample, int numSamples)
{
    if (!isVoiceActive())
        return;

    if (auto* playingSound = dynamic_cast<MultigrainSound*> (getCurrentlyPlayingSound().get()))
    {
        auto grainDurationFactor = apvts.getRawParameterValue("Grain Duration")->load();
        int numGrains = (int) apvts.getRawParameterValue("Num Grains")->load();

        auto grainDurationSamples = getSampleRate() * grainDurationFactor / m_currentNoteInHertz;
        auto samplesBetweenOnsets = (unsigned int) juce::roundToInt(grainDurationSamples/(float) numGrains);

        float* outL = outputBuffer.getWritePointer(0, startSample);
        float* outR = outputBuffer.getNumChannels() > 1 ? outputBuffer.getWritePointer(1, startSample) : nullptr;

        while (--numSamples >= 0)
        {
            if (m_samplesTillNextOnset == 0)
            {
                activateNextGrain(getNextGrainPosition(), juce::roundToInt(grainDurationSamples));
                m_samplesTillNextOnset += samplesBetweenOnsets;
                updateGrainSpawnPosition(samplesBetweenOnsets);
            }

            float voiceOutLeft = 0.f;
            float voiceOutRight = 0.f;

            for (Grain* grain : m_grains)
            {
                float grainLeft;
                float grainRight;

                grain->getNextSample(&grainLeft, &grainRight);

                voiceOutLeft += grainLeft;
                voiceOutRight += grainRight;
            }

            auto envelopeValue = m_adsr.getNextSample();
            voiceOutLeft *= envelopeValue;
            voiceOutRight *= envelopeValue;

            *outL += voiceOutLeft;
            *outR += voiceOutRight;

            outL++;
            outR++;

            m_samplesTillNextOnset--;

            if (!m_adsr.isActive())
                clearCurrentNote();
        }
//        // Render all active m_grains
//        for(Grain* grain : m_grains)
//            grain->renderNextBlock(outputBuffer, startSample, numSamples);
//
//        // Check if new m_grains need to be activated
//        while (m_samplesTillNextOnset < numSamples)
//        {
//            Grain& grain = activateNextGrain(getNextGrainPosition(), juce::roundDoubleToInt(grainDurationSamples));
//            grain.renderNextBlock(outputBuffer, startSample + m_samplesTillNextOnset, numSamples - m_samplesTillNextOnset);
//            m_samplesTillNextOnset += samplesBetweenOnsets; // TODO allow randomness here
//            updateGrainSpawnPosition(samplesBetweenOnsets);
//        }

//        m_samplesTillNextOnset -= (unsigned int) numSamples;

//        m_adsr.applyEnvelopeToBuffer(outputBuffer, startSample, numSamples);
        // outputBuffer.applyGain(1/(float) numGrains);

//        if (!m_adsr.isActive())
//            clearCurrentNote();
    }
}

void MultigrainVoice::updateGrainSpawnPosition(unsigned int samplesBetweenOnsets)
{
    m_grainSpawnPosition += (float) samplesBetweenOnsets * apvts.getRawParameterValue("Grain Speed")->load();
    m_grainSpawnPosition = std::fmod(m_grainSpawnPosition, m_sound.length);
}

GrainPosition MultigrainVoice::getNextGrainPosition()
{
    auto randomRange = apvts.getParameter("Position Random")->getValue() * (float) m_sound.length;
    auto randomDouble = m_randomGenerator.nextDouble();
    auto nextPosLeft = m_grainSpawnPosition + randomRange * randomDouble - randomRange / 2;
    randomDouble = m_randomGenerator.nextDouble();
    auto nextPosRight = m_grainSpawnPosition + randomRange * randomDouble - randomRange / 2;
    nextPosLeft = std::fmod(nextPosLeft, m_sound.length);
    nextPosRight = std::fmod(nextPosRight, m_sound.length);
    return {nextPosLeft, nextPosRight};
}

Grain& MultigrainVoice::activateNextGrain(GrainPosition grainPosition, int grainDurationInSamples)
{
    Grain* grain = m_grains[m_nextGrainToActivateIndex];
    // if (grain->isActive)
    //     jassertfalse; // grain voicestealing is happening
    grain->activate(
            grainDurationInSamples,
            grainPosition,
            m_pitchRatio,
            1.f // TODO allow randomization of this value
    );
    m_nextGrainToActivateIndex++;
    if (m_nextGrainToActivateIndex == m_grains.size())
        m_nextGrainToActivateIndex = 0;

    return *grain;
}

void MultigrainVoice::deactivateGrains()
{
    for(auto* grain : m_grains)
        grain->isActive = false;
}