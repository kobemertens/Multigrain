//
// Created by kobe on 19/11/2021.
//

#pragma once

#include <atomic>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

#include "MultigrainSound.h"
#include "Grain.h"
#include "GrainPosition.h"

// Stores grains
using Silo = juce::OwnedArray<Grain>;

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

    Silo& getSilo();

private:
    juce::Random mRandomGenerator;
    Grain &activateNextGrain(GrainPosition grainPosition, int grainDurationInSamples);
    void updateGrainSpawnPosition(unsigned int samplesBetweenOnsets);
    GrainPosition getNextGrainPosition();
    void deactivateGrains();
    void killNote();

    //==========================================================================================

    double mPitchRatio = 0;
    double mSourceSamplePosition = 0;
    double mGrainSpawnPosition;

    float mLGain = 0;
    float mRGain = 0;

    double mCurrentNoteInHertz;

    unsigned int mSamplesTillNextOnset;
    unsigned int mNextGrainToActivateIndex;

    std::atomic<float>* mRootNoteNumberParam;
    std::atomic<float>* mPositionParam;
    std::atomic<float>* mGrainDurationParam;
    std::atomic<float>* mNumGrainsParam;
    std::atomic<float>* mGrainSpeedParam;
    std::atomic<float>* mPositionRandomParam;

    std::atomic<float>* mAttackParam;
    std::atomic<float>* mDecayParam;
    std::atomic<float>* mSustainParam;
    std::atomic<float>* mReleaseParam;


    juce::ADSR mAdsr;

    Silo mGrains;

    MultigrainSound &mSound;

    JUCE_LEAK_DETECTOR(MultigrainVoice)
};