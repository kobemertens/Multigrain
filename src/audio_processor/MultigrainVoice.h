//
// Created by kobe on 19/11/2021.
//

#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

#include "MultigrainSound.h"
#include "Grain.h"
#include "GrainPosition.h"


/**
 * Manages and schedules mGrains;
 */
class MultigrainVoice : public juce::SynthesiserVoice
{
public:
    MultigrainVoice(juce::AudioProcessorValueTreeState &apvts, MultigrainSound &sound);
    ~MultigrainVoice() override = default;

    bool canPlaySound(juce::SynthesiserSound *sound) override;
    void startNote(
        int midiNoteNumber,
        float velocity,
        juce::SynthesiserSound *,
        int /*currentPitchWheelPosition*/
    ) override;
    void stopNote(float /*velocity*/, bool allowTailOff) override;

    void pitchWheelMoved(int newValue) override;

    void controllerMoved(int controllerNumber, int newValue) override;

    void renderNextBlock(
        juce::AudioSampleBuffer &outputBuffer,
        int startSample,
        int numSamples
    ) override;

private:
    juce::Random mRandomGenerator;
    Grain &activateNextGrain(GrainPosition grainPosition, int grainDurationInSamples);
    void updateGrainSpawnPosition(unsigned int samplesBetweenOnsets);
    GrainPosition getNextGrainPosition();
    void deactivateGrains();

    //==========================================================================================

    juce::AudioProcessorValueTreeState &mApvts;

    double mPitchRatio = 0;
    double mSourceSamplePosition = 0;
    double mGrainSpawnPosition;

    float mLGain = 0;
    float mRGain = 0;

    double mCurrentNoteInHertz;

    unsigned int mSamplesTillNextOnset;
    unsigned int mNextGrainToActivateIndex;

    juce::ADSR mAdsr;

    juce::OwnedArray<Grain> mGrains;

    MultigrainSound &mSound;

    JUCE_LEAK_DETECTOR(MultigrainVoice)
};