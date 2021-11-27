//
// Created by kobe on 19/11/2021.
//

#include "Grain.h"


// Grain
Grain::Grain(MultigrainSound& sound)
        : source(sound),
          isActive(false)
{}

void Grain::activate(int durationSamples, GrainPosition grainPosition, double pitchRatio, float grainAmplitude)
{
    samplesRemaining = durationSamples;
    source.init(grainPosition, pitchRatio);
    envelope.init(durationSamples, grainAmplitude);
    isActive = true;
}

float Grain::getNextSample()
{
    if (!isActive)
        return 0.f;

    auto sourceSample = source.getNextSample();
    auto envelopeValue = envelope.getNextSample();

    samplesRemaining--;

    isActive = samplesRemaining > 0;

    return sourceSample*envelopeValue;
}

void Grain::renderNextBlock(juce::AudioSampleBuffer& outputBuffer, int startSample, int numSamples)
{
    jassertfalse; // this function should not be used for now

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

float GrainEnvelope::getNextSample()
{
    auto returnValue = amplitude;

    if (currentSample == attackSamples)
        amplitudeIncrement = -(grainAmplitude / (float) releaseSamples);
    amplitude += amplitudeIncrement;
    currentSample++;

    return returnValue;
}

void GrainEnvelope::processNextBlock(juce::AudioSampleBuffer& bufferToProcess, int startSample, int numSamples)
{
    jassertfalse; // this function should not be used for now

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

// GrainSource
GrainSource::GrainSource(MultigrainSound& sound)
        : sound(sound) {}

void GrainSource::init(GrainPosition initPosition, double pitchRatio)
{
    this->pitchRatio = pitchRatio;
    sourceSamplePosition = initPosition.fmod(sound.length);
}

float GrainSource::getNextSample()
{
    juce::AudioSampleBuffer* data = sound.getAudioData();
    const float* const inL = data->getReadPointer (0);
    const float* const inR = data->getNumChannels() > 1 ? data->getReadPointer (1) : nullptr;

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


    sourceSamplePosition = sourceSamplePosition + pitchRatio;

    sourceSamplePosition = sourceSamplePosition.fmod(sound.length);

    return l;
}

void GrainSource::processNextBlock(juce::AudioSampleBuffer& bufferToProcess, int startSample, int numSamples)
{
    jassertfalse; // this function should not be used for now

    juce::AudioSampleBuffer* data = sound.getAudioData();
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

        if (sourceSamplePosition.rightPosition >= sound.length)
            sourceSamplePosition.rightPosition -= sound.length;

        if (sourceSamplePosition.leftPosition >= sound.length)
            sourceSamplePosition.leftPosition -= sound.length;
    }
}