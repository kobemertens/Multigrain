#include "../SynthAudioSource.h"

// SynthAudioSource
SynthAudioSource::SynthAudioSource(
    juce::MidiKeyboardState& inKeyboardState,
    juce::AudioProcessorValueTreeState& inApvts
)
: 
    mKeyboardState(inKeyboardState),
    mApvts(inApvts)
{
}

SynthAudioSource::~SynthAudioSource() = default;

void SynthAudioSource::prepareToPlay(int /*samplesPerBlockExpected*/, double sampleRate)
{
    synth.setCurrentPlaybackSampleRate(sampleRate);
}

void SynthAudioSource::releaseResources() {}

void SynthAudioSource::getNextAudioBlock(
    const juce::AudioSourceChannelInfo& bufferToFill
)
{
    auto theMidiBuffer = juce::MidiBuffer();
    mKeyboardState.processNextMidiBuffer(theMidiBuffer, 0, bufferToFill.numSamples, true);

    synth.renderNextBlock(*bufferToFill.buffer, theMidiBuffer, bufferToFill.startSample, bufferToFill.numSamples);
}

void SynthAudioSource::init(MultigrainSound* sound)
{
    // clear all previous sounds and voices
    synth.clearSounds();
    synth.clearVoices();

    synth.addSound(sound);
    for (int i = 0; i < kNumVoices; i++)
        synth.addVoice(new MultigrainVoice(mApvts, *sound));
}