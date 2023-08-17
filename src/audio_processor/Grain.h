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
//    void processNextBlock(juce::AudioSampleBuffer& bufferToProcess, int startSample, int numSamples);
    void init(unsigned int durationSamples, float grainAmplitude);
    float getNextSample();
    float mAmplitude;
private:
    float mGrainAmplitude;
    float mAmplitudeIncrement;

    std::size_t mAttackSamples;
    std::size_t mReleaseSamples;
    std::size_t mCurrentSample;
};

/**
 * Write samples from mSourceData to buffer according to pitch ratio.
 */
class GrainSource // aka AudioSource
{
public:
    explicit GrainSource(const MultigrainSound& sourceData);
    // void processNextBlock(juce::AudioSampleBuffer& bufferToProcess, int startSample, int numSamples); // write information about pitch here
    void init(GrainPosition sourceSamplePosition, double pitchRatio);
    void getNextSample(float* outL, float* outR);
    GrainPosition getRelativeGrainPosition() const;

private:
    double mPitchRatio;
    GrainPosition mSourceSamplePosition;
    const MultigrainSound& mSourceData;
};


/**
 * Applies envelope to source. Deactivates itself when completed.
 */
class Grain
{
public:
    explicit Grain(MultigrainSound& sound);
    // void renderNextBlock(juce::AudioSampleBuffer& outputBuffer, int startSample, int numSamples);
    void activate(unsigned int durationSamples, GrainPosition sourcePosition, double pitchRatio, float grainAmplitude);
    void getNextSample(float* outL, float* outR);
    GrainPosition getRelativeGrainPosition() const;
    float getGrainAmplitude() const;
    bool isActive;
private:
    GrainSource source;
    GrainEnvelope envelope;

    unsigned int samplesRemaining;
};