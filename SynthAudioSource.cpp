#include "SynthAudioSource.h"


// MultigrainSound
MultigrainSound::MultigrainSound(){};

MultigrainSound::~MultigrainSound(){};

bool MultigrainSound::appliesToNote(int) {return true;}

bool MultigrainSound::appliesToChannel(int) {return true;}


//MultigrainVoice
MultigrainVoice::MultigrainVoice(){};

MultigrainVoice::~MultigrainVoice(){};

bool MultigrainVoice::canPlaySound(juce::SynthesiserSound* sound)
{
    return dynamic_cast<MultigrainSound*>(sound) != nullptr;
}

void MultigrainVoice::startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*, int /*currentPitchWheelPosition*/)
{
    // do things to start the note (envelope etc)
}

void MultigrainVoice::stopNote(float /*velocity*/, bool allowTailOff)
{
    // do things to stop note (release etc)
}

void MultigrainVoice::pitchWheelMoved(int) {}

void MultigrainVoice::controllerMoved(int, int) {}

void MultigrainVoice::renderNextBlock(juce::AudioSampleBuffer& outputBuffer, int startSample, int numSamples)
{
    // write the correct data to the buffer
}


// SynthAudioSource
SynthAudioSource::SynthAudioSource(juce::MidiKeyboardState& keyboardState)
    : keyboardState(keyboardState)
{
    for (auto i = 0; i < 4; i++)
        synth.addVoice(new MultigrainVoice());
    
    synth.addSound(new MultigrainSound());
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
    bufferToFill.clearActiveBufferRegion();

    juce::MidiBuffer incomingMidi;
    keyboardState.processNextMidiBuffer(incomingMidi, bufferToFill.startSample, bufferToFill.numSamples, true);

    synth.renderNextBlock(*bufferToFill.buffer, incomingMidi, bufferToFill.startSample, bufferToFill.numSamples);
}