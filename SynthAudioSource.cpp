#include "SynthAudioSource.h"

// MultigrainSound
MultigrainSound::MultigrainSound(const juce::String& soundName,
                                 juce::AudioFormatReader& source, 
                                 const juce::BigInteger& notes,
                                 int midiNoteForNormalPitch, 
                                 double attackTimeSecs, 
                                 double releaseTimeSecs, 
                                 double maxSampleLengthSeconds)
    : name(soundName),
      sourceSampleRate(source.sampleRate),
      midiNotes(notes),
      midiRootNote(midiNoteForNormalPitch)
{
    if (sourceSampleRate > 0 && source.lengthInSamples > 0)
    {
        length = juce::jmin ((int) source.lengthInSamples,
                       (int) (maxSampleLengthSeconds * sourceSampleRate));

        data.reset (new juce::AudioBuffer<float> (juce::jmin (2, (int) source.numChannels), length + 4));

        source.read (data.get(), 0, length + 4, 0, true, true);

        params.attack  = static_cast<float> (attackTimeSecs);
        params.release = static_cast<float> (releaseTimeSecs);
    }
}

MultigrainSound::~MultigrainSound()
{
}

bool MultigrainSound::appliesToNote(int /*midiNoteNumber*/)
{
    // return midiNotes[midiNoteNumber];
    return true;
}

bool MultigrainSound::appliesToChannel(int /*midiChannel*/)
{
    return true;
}

// MultigrainVoice
MultigrainVoice::MultigrainVoice(){}

MultigrainVoice::~MultigrainVoice(){}

bool MultigrainVoice::canPlaySound(juce::SynthesiserSound* sound)
{
    return dynamic_cast<MultigrainSound*>(sound) != nullptr;
}

void MultigrainVoice::startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound* s, int /*currentPitchWheelPosition*/)
{
    // do things to start the note (envelope etc)
    if (auto* sound = dynamic_cast<const MultigrainSound*>(s))
    {
        pitchRatio = std::pow(2.0, (midiNoteNumber - sound->midiRootNote) / 12.0)
            *sound->sourceSampleRate / getSampleRate();
        
        sourceSamplePosition = 0.0;
        lgain = velocity;
        rgain = velocity;

        adsr.setSampleRate(sound->sourceSampleRate);
        adsr.setParameters(sound->params);

        adsr.noteOn();
    }
}

void MultigrainVoice::stopNote(float /*velocity*/, bool allowTailOff)
{
    if (allowTailOff)
    {
        adsr.noteOff();
    }
    else
    {
        clearCurrentNote();
        adsr.reset();
    }
}

void MultigrainVoice::pitchWheelMoved(int) {}

void MultigrainVoice::controllerMoved(int, int) {}

void MultigrainVoice::renderNextBlock(juce::AudioSampleBuffer& outputBuffer, int startSample, int numSamples)
{
    if (auto* playingSound = static_cast<MultigrainSound*> (getCurrentlyPlayingSound().get()))
    {
        auto& data = *playingSound->data;
        const float* const inL = data.getReadPointer (0);
        const float* const inR = data.getNumChannels() > 1 ? data.getReadPointer (1) : nullptr;

        float* outL = outputBuffer.getWritePointer (0, startSample);
        float* outR = outputBuffer.getNumChannels() > 1 ? outputBuffer.getWritePointer (1, startSample) : nullptr;
        
        while(--numSamples >= 0)
        {
            auto pos = (int) sourceSamplePosition;
            auto alpha = (float) (sourceSamplePosition - pos);
            auto invAlpha = 1.0f - alpha;

            // just using a very simple linear interpolation here..
            float l = (inL[pos] * invAlpha + inL[pos + 1] * alpha);
            float r = (inR != nullptr) ? (inR[pos] * invAlpha + inR[pos + 1] * alpha)
                                       : l;

            auto envelopeValue = adsr.getNextSample();

            l *= lgain * envelopeValue;
            r *= rgain * envelopeValue;

            if (outR != nullptr)
            {
                *outL++ += l;
                *outR++ += r;
            }
            else
            {
                *outL++ += (l + r) * 0.5f;
            }

            sourceSamplePosition += pitchRatio;

            if (sourceSamplePosition > playingSound->length)
            {
                stopNote (0.0f, false);
                break;
            }
        }
    }
}


// SynthAudioSource
SynthAudioSource::SynthAudioSource(juce::MidiKeyboardState& keyboardState)
    : keyboardState(keyboardState)
{
    for (auto i = 0; i < 4; i++)
        synth.addVoice(new MultigrainVoice());
}

SynthAudioSource::~SynthAudioSource(){}

void SynthAudioSource::setUsingSineWaveSound()
{
    synth.clearSounds();
}

void SynthAudioSource::prepareToPlay(int /*samplesPerBlockExpected*/, double sampleRate) 
{
    synth.setCurrentPlaybackSampleRate(sampleRate);
}

void SynthAudioSource::releaseResources() {}

void SynthAudioSource::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) 
{
    juce::MidiBuffer incomingMidi;
    keyboardState.processNextMidiBuffer (incomingMidi, bufferToFill.startSample, bufferToFill.numSamples, true);
    bufferToFill.clearActiveBufferRegion();

    synth.renderNextBlock(*bufferToFill.buffer, incomingMidi, bufferToFill.startSample, bufferToFill.numSamples);
}

juce::Synthesiser& SynthAudioSource::getSynth()
{
    return synth;
}