#include "SynthAudioSource.h"

// SynthAudioSource
SynthAudioSource::SynthAudioSource(juce::MidiKeyboardState& keyboardState, juce::AudioProcessorValueTreeState& apvts)
    : keyboardState(keyboardState),
      apvts(apvts) {}

SynthAudioSource::~SynthAudioSource(){}

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

void SynthAudioSource::init(MultigrainSound* sound)
{
    // clear all previous sounds and voices
    synth.clearSounds();
    synth.clearVoices();

    synth.addSound(sound);
    for (int i = 0; i < numVoices; i++)
        synth.addVoice(new MultigrainVoice(apvts, *sound));
}