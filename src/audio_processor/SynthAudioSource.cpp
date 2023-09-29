#include "./SynthAudioSource.h"

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
    mSynth.setCurrentPlaybackSampleRate(sampleRate);
}

void SynthAudioSource::releaseResources() {}

void SynthAudioSource::getNextAudioBlock(
    const juce::AudioSourceChannelInfo& bufferToFill
)
{
    auto theMidiBuffer = juce::MidiBuffer();
    mKeyboardState.processNextMidiBuffer(theMidiBuffer, 0, bufferToFill.numSamples, true);

    mSynth.renderNextBlock(*bufferToFill.buffer, theMidiBuffer, bufferToFill.startSample, bufferToFill.numSamples);
}

std::vector<Silo*> SynthAudioSource::getSilos() const
{
    auto silos = std::vector<Silo*>();
    for(int i = 0; i < this->mSynth.getNumVoices(); i++) 
    {
        auto voice = static_cast<MultigrainVoice*>(this->mSynth.getVoice(i));
        silos.push_back(&voice->getSilo());
    }

    return silos;
}

void SynthAudioSource::init(MultigrainSound* sound)
{
    // clear all previous sounds and voices
    mSynth.clearSounds();
    mSynth.clearVoices();

    mSynth.addSound(sound);
    for (int i = 0; i < kNumVoices; i++)
        mSynth.addVoice(new MultigrainVoice(mApvts, *sound));
}