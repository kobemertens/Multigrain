#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_formats/juce_audio_formats.h>

#include "MultigrainSound.h"
#include "MultigrainVoice.h"

class SynthAudioSource : public juce::AudioSource
{
public:
    SynthAudioSource (
        juce::MidiKeyboardState& inMidiKeyboardState,
        juce::AudioProcessorValueTreeState& inApvts
    );
    ~SynthAudioSource() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void releaseResources() override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    
    std::vector<Silo*> getSilos() const;
    juce::Synthesiser mSynth;

    void init(MultigrainSound* sound);
private:
    juce::MidiKeyboardState& mKeyboardState;
    juce::AudioProcessorValueTreeState& mApvts;

    static int const kNumVoices = 16;

    JUCE_LEAK_DETECTOR(SynthAudioSource)
};