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
    void init(int durationSamples, float grainAmplitude);
    float getNextSample();
    void processNextBlock(juce::AudioSampleBuffer& bufferToProcess, int startSample, int numSamples);
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
    GrainSource(MultigrainSound& sound);
    void init(GrainPosition sourceSamplePosition, double pitchRatio);
    float getNextSample();
    void processNextBlock(juce::AudioSampleBuffer& bufferToProcess, int startSample, int numSamples); // write information about pitch here

private:
    double pitchRatio;
    GrainPosition sourceSamplePosition;
    MultigrainSound& sound;
};


/**
 * Applies envelope to source. Deactivates itself when completed.
 */
class Grain
{
public:
    Grain(MultigrainSound& sound);
    void activate(int durationSamples, GrainPosition sourcePosition, double pitchRatio, float grainAmplitude);
    float getNextSample();
    void renderNextBlock(juce::AudioSampleBuffer& outputBuffer, int startSample, int numSamples);
    bool isActive;
private:
    GrainEnvelope envelope;
    GrainSource source;

    int samplesRemaining;
};