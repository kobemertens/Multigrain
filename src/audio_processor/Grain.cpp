//
// Created by kobe on 19/11/2021.
//

#include "Grain.h"


// Grain
Grain::Grain(MultigrainSound& sound, juce::ADSR& globalEnvelope)
        : source(sound, envelope, globalEnvelope),
          isActive(false)
{}

void Grain::activate(int durationSamples, GrainPosition grainPosition, double pitchRatio, float grainAmplitude)
{
    samplesRemaining = durationSamples;
    source.init(grainPosition, pitchRatio);
    envelope.init(durationSamples, grainAmplitude);
    isActive = true;
}

void Grain::renderNextBlock(juce::AudioSampleBuffer& outputBuffer, int startSample, int numSamples)
{
    if (!isActive)
        return;

    auto samplesToProcess = juce::jmin(numSamples, samplesRemaining);
    source.processNextBlock(outputBuffer, startSample, samplesToProcess);
    // envelope.processNextBlock(outputBuffer, startSample, samplesToProcess);

    samplesRemaining -= samplesToProcess;

    if(samplesRemaining == 0)
        isActive = false;

    if(samplesRemaining < 0)
        jassertfalse; // this should not happen
}

// GrainEnvelope
void GrainEnvelope::init(int durationSamples, float grainAmplitude)
{
    amplitude = 0;
    currentSample = 0;
    this->durationSamples = durationSamples;
    this->grainAmplitude = grainAmplitude;
    attackSamples = durationSamples / 2;
    releaseSamples = durationSamples - attackSamples - 1;
    amplitudeIncrement = grainAmplitude / (float) attackSamples;
}

void GrainEnvelope::processNextBlock(juce::AudioSampleBuffer& bufferToProcess, int startSample, int numSamples)
{
    float* outL = bufferToProcess.getWritePointer(0, startSample);
    float* outR = bufferToProcess.getNumChannels() > 1 ? bufferToProcess.getWritePointer(1, startSample) : nullptr;

    while(--numSamples >= 0)
    {
        *outL++ *= amplitude;
        if(outR != nullptr)
        {
            *outR++ *= getNextSample();
        }
    }
}

float GrainEnvelope::getNextSample()
{
    auto returnValue = amplitude;

    if (currentSample == attackSamples)
        amplitudeIncrement = -(grainAmplitude / (float) releaseSamples);
    amplitude += amplitudeIncrement;
    currentSample++;

    return returnValue;
}

// GrainSource
GrainSource::GrainSource(MultigrainSound& sourceData, GrainEnvelope& env, juce::ADSR& globalEnvelope)
        : sourceData(sourceData), env(env), globalEnvelope(globalEnvelope) {}

void GrainSource::init(GrainPosition initPosition, double pitchRatio)
{
    this->pitchRatio = pitchRatio;
    if (initPosition.leftPosition >= sourceData.length)
        initPosition.leftPosition -= sourceData.length;

    if (initPosition.rightPosition >= sourceData.length)
        initPosition.rightPosition -= sourceData.length;


    if (initPosition.leftPosition < 0.)
        initPosition.leftPosition = sourceData.length + initPosition.leftPosition;

    if (initPosition.rightPosition < 0.)
        initPosition.rightPosition = sourceData.length + initPosition.rightPosition;

    sourceSamplePosition = initPosition;
}

void GrainSource::processNextBlock(juce::AudioSampleBuffer& bufferToProcess, int startSample, int numSamples)
{
    juce::AudioSampleBuffer* data = sourceData.getAudioData();
    const float* const inL = data->getReadPointer (0);
    const float* const inR = data->getNumChannels() > 1 ? data->getReadPointer (1) : nullptr;

    float* outL = bufferToProcess.getWritePointer (0, startSample);
    float* outR = bufferToProcess.getNumChannels() > 1 ? bufferToProcess.getWritePointer (1, startSample) : nullptr;
    float nextEnvelopeSample;
    while(--numSamples >= 0)
    {
        auto posLeft = (int) sourceSamplePosition.leftPosition;
        auto alphaLeft = (float) (sourceSamplePosition.leftPosition - posLeft);
        auto invAlphaLeft = 1.f - alphaLeft;

        auto posRight = (int) sourceSamplePosition.rightPosition;
        auto alphaRight = (float) (sourceSamplePosition.rightPosition - posRight);
        auto invAlphaRight = 1.f - alphaRight;

        // just using a very simple linear interpolation here..
        float l = (inL[posLeft] * invAlphaLeft + inL[posLeft + 1] * alphaLeft);
        float r = (inR != nullptr) ? (inR[posRight] * invAlphaRight + inR[posRight + 1] * alphaRight)
                                   : (inL[posRight] * invAlphaRight + inL[posRight + 1] * alphaRight); // use the left channel if mono sample was provided

        nextEnvelopeSample = env.getNextSample();

        l *= nextEnvelopeSample;
        r *= nextEnvelopeSample;

        if (outR != nullptr)
        {
            *outL++ += l;
            *outR++ += r;
        }
        else
        {
            *outL++ += (l + r) * 0.5f;
        }

        sourceSamplePosition.leftPosition += pitchRatio;
        sourceSamplePosition.rightPosition += pitchRatio;

        if (sourceSamplePosition.rightPosition >= sourceData.length)
            sourceSamplePosition.rightPosition -= sourceData.length;

        if (sourceSamplePosition.leftPosition >= sourceData.length)
            sourceSamplePosition.leftPosition -= sourceData.length;
    }
}