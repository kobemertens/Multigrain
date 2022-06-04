//
// Created by kobe on 19/11/2021.
//

#include "../MultigrainVoice.h"

// MultigrainVoice
MultigrainVoice::MultigrainVoice(
    juce::AudioProcessorValueTreeState& apvts, 
    MultigrainSound& sound
):
        mApvts(apvts),
        mGrainSpawnPosition{0.},
        mCurrentNoteInHertz{440.},
        mSamplesTillNextOnset(0),
        mNextGrainToActivateIndex(0),
        mSound(sound)
{
    // init grain array
    for(int i = 0; i < 8; i++)
        mGrains.add(new Grain{sound});
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

        auto theRootNoteNumber = static_cast<int>(mApvts.getRawParameterValue("Root Note")->load());
//        auto theNoteNumber = midiNoteNumber - 60 + theRootNoteNumber;

        mPitchRatio = std::pow(2.0, (midiNoteNumber - theRootNoteNumber) / 12.0)
                      * sound->sourceSampleRate / getSampleRate();

        mCurrentNoteInHertz = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
        mSamplesTillNextOnset = 0;
        mGrainSpawnPosition = mApvts.getParameter("Position")->getValue() * (float) sound->length;

        mLGain = velocity;
        mRGain = velocity;

        mAdsr.setSampleRate(getSampleRate());
        juce::ADSR::Parameters params(
                (float) mApvts.getRawParameterValue("Synth Attack")->load() / 1000.f,
                (float) mApvts.getRawParameterValue("Synth Decay")->load() / 1000.f,
                (float) mApvts.getRawParameterValue("Synth Sustain")->load() / 100.f,
                (float) mApvts.getRawParameterValue("Synth Release")->load() / 1000.f
        );
        mAdsr.setParameters(params);
        mAdsr.noteOn();
    }
}

void MultigrainVoice::stopNote(float /*velocity*/, bool allowTailOff)
{
    if (allowTailOff)
    {
        mAdsr.noteOff();
    }
    else
    {
        clearCurrentNote();
        mAdsr.reset();
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
        auto grainDurationFactor = mApvts.getRawParameterValue("Grain Duration")->load();
        int numGrains = (int) mApvts.getRawParameterValue("Num Grains")->load();

        auto grainDurationSamples = getSampleRate() * grainDurationFactor / mCurrentNoteInHertz;
        auto samplesBetweenOnsets = (unsigned int) juce::roundToInt(grainDurationSamples/(float) numGrains);

        float* outL = outputBuffer.getWritePointer(0, startSample);
        float* outR = outputBuffer.getNumChannels() > 1 ? outputBuffer.getWritePointer(1, startSample) : nullptr;

        while (--numSamples >= 0)
        {
            if (mSamplesTillNextOnset == 0)
            {
                activateNextGrain(getNextGrainPosition(), juce::roundToInt(grainDurationSamples));
                mSamplesTillNextOnset += samplesBetweenOnsets;
                updateGrainSpawnPosition(samplesBetweenOnsets);
            }

            auto voiceOutLeft = 0.f;
            auto voiceOutRight = 0.f;

            for (Grain* grain : mGrains)
            {
                auto grainLeft = 0.f;
                auto grainRight = 0.f;

                grain->getNextSample(&grainLeft, &grainRight);

                voiceOutLeft += grainLeft;
                voiceOutRight += grainRight;
            }

            auto envelopeValue = mAdsr.getNextSample();
            voiceOutLeft *= envelopeValue;
            voiceOutRight *= envelopeValue;

            *outL++ += voiceOutLeft;
            if (outR)
            {
                *outR++ += voiceOutRight;
            }
            
            mSamplesTillNextOnset--;

            if (!mAdsr.isActive())
                clearCurrentNote();
        }
//        // Render all active mGrains
//        for(Grain* grain : mGrains)
//            grain->renderNextBlock(outputBuffer, startSample, numSamples);
//
//        // Check if new mGrains need to be activated
//        while (mSamplesTillNextOnset < numSamples)
//        {
//            Grain& grain = activateNextGrain(getNextGrainPosition(), juce::roundDoubleToInt(grainDurationSamples));
//            grain.renderNextBlock(outputBuffer, startSample + mSamplesTillNextOnset, numSamples - mSamplesTillNextOnset);
//            mSamplesTillNextOnset += samplesBetweenOnsets; // TODO allow randomness here
//            updateGrainSpawnPosition(samplesBetweenOnsets);
//        }

//        mSamplesTillNextOnset -= (unsigned int) numSamples;

//        mAdsr.applyEnvelopeToBuffer(outputBuffer, startSample, numSamples);
        // outputBuffer.applyGain(1/(float) numGrains);

//        if (!mAdsr.isActive())
//            clearCurrentNote();
    }
}

void MultigrainVoice::updateGrainSpawnPosition(unsigned int samplesBetweenOnsets)
{
    mGrainSpawnPosition += (float) samplesBetweenOnsets * mApvts.getRawParameterValue("Grain Speed")->load();
    mGrainSpawnPosition = std::fmod(mGrainSpawnPosition, mSound.length);
}

GrainPosition MultigrainVoice::getNextGrainPosition()
{
    auto randomRange = mApvts.getParameter("Position Random")->getValue() * (float) mSound.length;
    auto randomDouble = mRandomGenerator.nextDouble();
    auto nextPosLeft = mGrainSpawnPosition + randomRange * randomDouble - randomRange / 2;
    randomDouble = mRandomGenerator.nextDouble();
    auto nextPosRight = mGrainSpawnPosition + randomRange * randomDouble - randomRange / 2;
    nextPosLeft = std::fmod(nextPosLeft, mSound.length);
    nextPosRight = std::fmod(nextPosRight, mSound.length);
    return {nextPosLeft, nextPosRight};
}

Grain& MultigrainVoice::activateNextGrain(GrainPosition grainPosition, int grainDurationInSamples)
{
    Grain* grain = mGrains[mNextGrainToActivateIndex];
    // if (grain->isActive)
    //     jassertfalse; // grain voicestealing is happening
    grain->activate(
            grainDurationInSamples,
            grainPosition,
            mPitchRatio,
            1.f // TODO allow randomization of this value
    );
    mNextGrainToActivateIndex++;
    if (mNextGrainToActivateIndex == mGrains.size())
        mNextGrainToActivateIndex = 0;

    return *grain;
}

void MultigrainVoice::deactivateGrains()
{
    for(auto* grain : mGrains)
        grain->isActive = false;
}