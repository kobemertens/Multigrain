//
// Created by kobe on 19/11/2021.
//

#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include "MultigrainSound.h"
#include "GrainPosition.h"
/**
 * Process an incoming buffer by applying the envelope.
 */
class GrainEnvelope
{
public:
    void processNextBlock(juce::AudioSampleBuffer& bufferToProcess, int startSample, int numSamples);
    void init(int durationSamples, float grainAmplitude);
    float getNextSample();
private:
    float amplitude;
    float grainAmplitude;
    int attackSamples;
    int releaseSamples;
    float amplitudeIncrement;
    int currentSample;
    int durationSamples;
};

/**
 * Write samples from sourceData to buffer according to pitch ratio.
 */
class GrainSource
{
public:
    GrainSource(MultigrainSound& sourceData, GrainEnvelope& env, juce::ADSR& globalEnvelope);
    void processNextBlock(juce::AudioSampleBuffer& bufferToProcess, int startSample, int numSamples); // write information about pitch here
    void init(GrainPosition sourceSamplePosition, double pitchRatio);

private:
    GrainEnvelope& env;
    double pitchRatio;
    GrainPosition sourceSamplePosition;
    MultigrainSound& sourceData;
    juce::ADSR& globalEnvelope;
};


/**
 * Applies envelope to source. Deactivates itself when completed.
 */
class Grain
{
public:
    Grain(MultigrainSound& sound, juce::ADSR& globalEnvelope);
    void renderNextBlock(juce::AudioSampleBuffer& outputBuffer, int startSample, int numSamples);
    void activate(int durationSamples, GrainPosition sourcePosition, double pitchRatio, float grainAmplitude);
    bool isActive;
private:
    GrainEnvelope envelope;
    GrainSource source;

    int samplesRemaining;
};