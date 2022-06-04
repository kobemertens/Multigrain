//
// Created by kobe on 19/11/2021.
//

#include "../Grain.h"

// GrainEnvelope
void GrainEnvelope::init(unsigned int durationSamples, float grainAmplitude)
{
    mAmplitude = 0;
    mCurrentSample = 0;
    mGrainAmplitude = grainAmplitude;
    mAttackSamples = durationSamples / 2;
    mReleaseSamples = durationSamples - mAttackSamples - 1;
    mAmplitudeIncrement = grainAmplitude / (float) mAttackSamples;
}

//void GrainEnvelope::processNextBlock(juce::AudioSampleBuffer &bufferToProcess, int startSample, int numSamples)
//{
//    float *outL = bufferToProcess.getWritePointer(0, startSample);
//    float *outR = bufferToProcess.getNumChannels() > 1 ? bufferToProcess.getWritePointer(1, startSample) : nullptr;
//
//    while (--numSamples >= 0)
//    {
//        *outL++ *= mAmplitude;
//        if (outR != nullptr)
//        {
//            *outR++ *= getNextSample();
//        }
//    }
//}

float GrainEnvelope::getNextSample()
{
    auto returnValue = mAmplitude;

    if (mCurrentSample == mAttackSamples)
        mAmplitudeIncrement = -(mGrainAmplitude / (float) mReleaseSamples);
    mAmplitude += mAmplitudeIncrement;
    mCurrentSample++;

    return returnValue;
}

// GrainSource
GrainSource::GrainSource(const MultigrainSound &sourceData)
        : mPitchRatio{1.},
          mSourceSamplePosition{0., 0.},
          mSourceData{sourceData}
{
}

void GrainSource::init(GrainPosition initPosition, double pitchRatio)
{
    this->mPitchRatio = pitchRatio;
    if (initPosition.leftPosition >= mSourceData.length)
        initPosition.leftPosition -= mSourceData.length;

    if (initPosition.rightPosition >= mSourceData.length)
        initPosition.rightPosition -= mSourceData.length;


    if (initPosition.leftPosition < 0.)
        initPosition.leftPosition = mSourceData.length + initPosition.leftPosition;

    if (initPosition.rightPosition < 0.)
        initPosition.rightPosition = mSourceData.length + initPosition.rightPosition;

    mSourceSamplePosition = initPosition;
}

void GrainSource::getNextSample(float* outL, float* outR)
{
    juce::AudioSampleBuffer* data = mSourceData.getAudioData();
    const float* const inL = data->getReadPointer (0);
    const float* const inR = data->getNumChannels() > 1 ? data->getReadPointer (1) : nullptr;

    auto posLeft = (int) mSourceSamplePosition.leftPosition;
    auto alphaLeft = (float) (mSourceSamplePosition.leftPosition - posLeft);
    auto invAlphaLeft = 1.f - alphaLeft;

    auto posRight = (int) mSourceSamplePosition.rightPosition;
    auto alphaRight = (float) (mSourceSamplePosition.rightPosition - posRight);
    auto invAlphaRight = 1.f - alphaRight;

    // just using a very simple linear interpolation here..
    float l = (inL[posLeft] * invAlphaLeft + inL[posLeft + 1] * alphaLeft);
    float r = (inR != nullptr) ? (inR[posRight] * invAlphaRight + inR[posRight + 1] * alphaRight)
                               : (inL[posRight] * invAlphaRight + inL[posRight + 1] * alphaRight); // use the left channel if mono sample was provided

    if (outR != nullptr)
    {
        *outL += l;
        *outR += r;
    }
    else
    {
        *outL += (l + r) * 0.5f;
    }

    mSourceSamplePosition.leftPosition += mPitchRatio;
    mSourceSamplePosition.rightPosition += mPitchRatio;

     if (mSourceSamplePosition.rightPosition >= mSourceData.length)
         mSourceSamplePosition.rightPosition -= mSourceData.length;

     if (mSourceSamplePosition.leftPosition >= mSourceData.length)
         mSourceSamplePosition.leftPosition -= mSourceData.length;
    }


// void GrainSource::processNextBlock(juce::AudioSampleBuffer& bufferToProcess, int startSample, int numSamples)
// {
//     juce::AudioSampleBuffer* data = mSourceData.getAudioData();
//     const float* const inL = data->getReadPointer (0);
//     const float* const inR = data->getNumChannels() > 1 ? data->getReadPointer (1) : nullptr;

//     float* outL = bufferToProcess.getWritePointer (0, startSample);
//     float* outR = bufferToProcess.getNumChannels() > 1 ? bufferToProcess.getWritePointer (1, startSample) : nullptr;
//     float nextEnvelopeSample;
//     while(--numSamples >= 0)
//     {
//         auto posLeft = (int) mSourceSamplePosition.leftPosition;
//         auto alphaLeft = (float) (mSourceSamplePosition.leftPosition - posLeft);
//         auto invAlphaLeft = 1.f - alphaLeft;

//         auto posRight = (int) mSourceSamplePosition.rightPosition;
//         auto alphaRight = (float) (mSourceSamplePosition.rightPosition - posRight);
//         auto invAlphaRight = 1.f - alphaRight;

//         // just using a very simple linear interpolation here..
//         float l = (inL[posLeft] * invAlphaLeft + inL[posLeft + 1] * alphaLeft);
//         float r = (inR != nullptr) ? (inR[posRight] * invAlphaRight + inR[posRight + 1] * alphaRight)
//                                    : (inL[posRight] * invAlphaRight + inL[posRight + 1] * alphaRight); // use the left channel if mono sample was provided

//         nextEnvelopeSample = env.getNextSample();

//         l *= nextEnvelopeSample;
//         r *= nextEnvelopeSample;

//         if (outR != nullptr)
//         {
//             *outL++ += l;
//             *outR++ += r;
//         }
//         else
//         {
//             *outL++ += (l + r) * 0.5f;
//         }

//         mSourceSamplePosition.leftPosition += mPitchRatio;
//         mSourceSamplePosition.rightPosition += mPitchRatio;

//         if (mSourceSamplePosition.rightPosition >= mSourceData.length)
//             mSourceSamplePosition.rightPosition -= mSourceData.length;

//         if (mSourceSamplePosition.leftPosition >= mSourceData.length)
//             mSourceSamplePosition.leftPosition -= mSourceData.length;
//     }
// }

// Grain
Grain::Grain(MultigrainSound &sound)
        : isActive(false),
          source(sound),
          envelope{},
          samplesRemaining{0}
{
}

void Grain::activate(unsigned int durationSamples, GrainPosition grainPosition, double pitchRatio, float grainAmplitude)
{
    samplesRemaining = durationSamples;
    source.init(grainPosition, pitchRatio);
    envelope.init(durationSamples, grainAmplitude);
    isActive = true;
}

void Grain::getNextSample(float* outL, float* outR)
{
    if (!isActive)
        return;

    source.getNextSample(outL, outR);
    auto envValue = envelope.getNextSample();

    *outL *= envValue;
    *outR *= envValue;

    samplesRemaining--;
    if (samplesRemaining == 0)
        isActive = false;
}

// void Grain::renderNextBlock(juce::AudioSampleBuffer& outputBuffer, int startSample, int numSamples)
// {
//     if (!isActive)
//         return;

//     auto samplesToProcess = juce::jmin(numSamples, samplesRemaining);
//     source.processNextBlock(outputBuffer, startSample, samplesToProcess);
//     // envelope.processNextBlock(outputBuffer, startSample, samplesToProcess);

//     samplesRemaining -= samplesToProcess;

//     if(samplesRemaining == 0)
//         isActive = false;

//     if(samplesRemaining < 0)
//         jassertfalse; // this should not happen
// }