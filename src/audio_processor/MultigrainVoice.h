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
 * Manages and schedules grains;
 */
class MultigrainVoice : public juce::SynthesiserVoice
{
public:
    MultigrainVoice(juce::AudioProcessorValueTreeState& apvts, MultigrainSound& sound);
    ~MultigrainVoice() override;

    bool canPlaySound(juce::SynthesiserSound* sound) override;

    void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*, int /*currentPitchWheelPosition*/) override;
    void stopNote(float /*velocity*/, bool allowTailOff) override;

    void pitchWheelMoved(int newValue) override;
    void controllerMoved(int controllerNumber, int newValue) override;

    void renderNextBlock(juce::AudioSampleBuffer& outputBuffer, int startSample, int numSamples) override;
private:
    Grain& activateNextGrain(GrainPosition grainPosition, int grainDurationInSamples);
    void updateGrainSpawnPosition(int samplesBetweenOnsets);
    GrainPosition getNextGrainPosition();
    void deactivateGrains();
    double pitchRatio = 0;
    double sourceSamplePosition = 0;
    float lgain = 0, rgain = 0;

    double currentNoteInHertz;

    int samplesTillNextOnset;
    unsigned int nextGrainToActivateIndex;
    double grainSpawnPosition;

    juce::ADSR adsr;

    juce::OwnedArray<Grain> grains;

    juce::AudioProcessorValueTreeState& apvts;
    MultigrainSound& sound;

    juce::Random randomGenerator;

    JUCE_LEAK_DETECTOR(MultigrainVoice);
};