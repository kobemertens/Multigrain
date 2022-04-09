//
// Created by kobe on 19/11/2021.
//

#include "../Grain.h"

// GrainEnvelope
void GrainEnvelope::init(unsigned int durationSamples, float grainAmplitude)
{
    m_amplitude = 0;
    m_currentSample = 0;
    this->m_grainAmplitude = grainAmplitude;
    m_attackSamples = durationSamples / 2;
    m_releaseSamples = durationSamples - m_attackSamples - 1;
    m_amplitudeIncrement = grainAmplitude / (float) m_attackSamples;
}

//void GrainEnvelope::processNextBlock(juce::AudioSampleBuffer &bufferToProcess, int startSample, int numSamples)
//{
//    float *outL = bufferToProcess.getWritePointer(0, startSample);
//    float *outR = bufferToProcess.getNumChannels() > 1 ? bufferToProcess.getWritePointer(1, startSample) : nullptr;
//
//    while (--numSamples >= 0)
//    {
//        *outL++ *= m_amplitude;
//        if (outR != nullptr)
//        {
//            *outR++ *= getNextSample();
//        }
//    }
//}

float GrainEnvelope::getNextSample()
{
    auto returnValue = m_amplitude;

    if (m_currentSample == m_attackSamples)
        m_amplitudeIncrement = -(m_grainAmplitude / (float) m_releaseSamples);
    m_amplitude += m_amplitudeIncrement;
    m_currentSample++;

    return returnValue;
}

// GrainSource
GrainSource::GrainSource(const MultigrainSound &sourceData)
        : m_pitchRatio{1.},
          m_sourceSamplePosition{0., 0.},
          m_sourceData{sourceData}
{
}

void GrainSource::init(GrainPosition initPosition, double pitchRatio)
{
    this->m_pitchRatio = pitchRatio;
    if (initPosition.leftPosition >= m_sourceData.length)
        initPosition.leftPosition -= m_sourceData.length;

    if (initPosition.rightPosition >= m_sourceData.length)
        initPosition.rightPosition -= m_sourceData.length;


    if (initPosition.leftPosition < 0.)
        initPosition.leftPosition = m_sourceData.length + initPosition.leftPosition;

    if (initPosition.rightPosition < 0.)
        initPosition.rightPosition = m_sourceData.length + initPosition.rightPosition;

    m_sourceSamplePosition = initPosition;
}

void GrainSource::getNextSample(float* outL, float* outR)
{
    juce::AudioSampleBuffer* data = m_sourceData.getAudioData();
    const float* const inL = data->getReadPointer (0);
    const float* const inR = data->getNumChannels() > 1 ? data->getReadPointer (1) : nullptr;

    auto posLeft = (int) m_sourceSamplePosition.leftPosition;
    auto alphaLeft = (float) (m_sourceSamplePosition.leftPosition - posLeft);
    auto invAlphaLeft = 1.f - alphaLeft;

    auto posRight = (int) m_sourceSamplePosition.rightPosition;
    auto alphaRight = (float) (m_sourceSamplePosition.rightPosition - posRight);
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

     m_sourceSamplePosition.leftPosition += m_pitchRatio;
     m_sourceSamplePosition.rightPosition += m_pitchRatio;

     if (m_sourceSamplePosition.rightPosition >= m_sourceData.length)
         m_sourceSamplePosition.rightPosition -= m_sourceData.length;

     if (m_sourceSamplePosition.leftPosition >= m_sourceData.length)
         m_sourceSamplePosition.leftPosition -= m_sourceData.length;
    }


// void GrainSource::processNextBlock(juce::AudioSampleBuffer& bufferToProcess, int startSample, int numSamples)
// {
//     juce::AudioSampleBuffer* data = m_sourceData.getAudioData();
//     const float* const inL = data->getReadPointer (0);
//     const float* const inR = data->getNumChannels() > 1 ? data->getReadPointer (1) : nullptr;

//     float* outL = bufferToProcess.getWritePointer (0, startSample);
//     float* outR = bufferToProcess.getNumChannels() > 1 ? bufferToProcess.getWritePointer (1, startSample) : nullptr;
//     float nextEnvelopeSample;
//     while(--numSamples >= 0)
//     {
//         auto posLeft = (int) m_sourceSamplePosition.leftPosition;
//         auto alphaLeft = (float) (m_sourceSamplePosition.leftPosition - posLeft);
//         auto invAlphaLeft = 1.f - alphaLeft;

//         auto posRight = (int) m_sourceSamplePosition.rightPosition;
//         auto alphaRight = (float) (m_sourceSamplePosition.rightPosition - posRight);
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

//         m_sourceSamplePosition.leftPosition += m_pitchRatio;
//         m_sourceSamplePosition.rightPosition += m_pitchRatio;

//         if (m_sourceSamplePosition.rightPosition >= m_sourceData.length)
//             m_sourceSamplePosition.rightPosition -= m_sourceData.length;

//         if (m_sourceSamplePosition.leftPosition >= m_sourceData.length)
//             m_sourceSamplePosition.leftPosition -= m_sourceData.length;
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