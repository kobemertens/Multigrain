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
 * Manages and schedules m_grains;
 */
class MultigrainVoice : public juce::SynthesiserVoice
{
public:
    MultigrainVoice(juce::AudioProcessorValueTreeState& apvts, MultigrainSound& sound);
    ~MultigrainVoice() override = default;

    bool canPlaySound(juce::SynthesiserSound* sound) override;

    void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*, int /*currentPitchWheelPosition*/) override;
    void stopNote(float /*velocity*/, bool allowTailOff) override;

    void pitchWheelMoved(int newValue) override;
    void controllerMoved(int controllerNumber, int newValue) override;

    void renderNextBlock(juce::AudioSampleBuffer& outputBuffer, int startSample, int numSamples) override;
private:
    juce::AudioProcessorValueTreeState& apvts;

    double m_pitchRatio = 0,
           m_sourceSamplePosition = 0,
           m_grainSpawnPosition;

    float m_lGain = 0,
          m_rGain = 0;

    double m_currentNoteInHertz;

    unsigned int m_samplesTillNextOnset,
                 m_nextGrainToActivateIndex;

    juce::ADSR m_adsr;

    juce::OwnedArray<Grain> m_grains;

    MultigrainSound& m_sound;

    juce::Random m_randomGenerator;

    Grain& activateNextGrain(GrainPosition grainPosition, int grainDurationInSamples);
    void updateGrainSpawnPosition(unsigned int samplesBetweenOnsets);
    GrainPosition getNextGrainPosition();
    void deactivateGrains();

    JUCE_LEAK_DETECTOR(MultigrainVoice)
};