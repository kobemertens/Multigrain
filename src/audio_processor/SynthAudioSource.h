#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_formats/juce_audio_formats.h>

#include "MultigrainSound.h"
#include "MultigrainVoice.h"

class SynthAudioSource : public juce::AudioSource
{
public:
    SynthAudioSource (juce::MidiKeyboardState& keyboardState, juce::AudioProcessorValueTreeState& apvts);
    ~SynthAudioSource();

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void releaseResources() override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    juce::Synthesiser& getSynth();

    void init(MultigrainSound* sound);
private:
    juce::MidiKeyboardState& keyboardState;
    juce::AudioProcessorValueTreeState& apvts;
    juce::Synthesiser synth;

    int numVoices = 1;

    JUCE_LEAK_DETECTOR(SynthAudioSource)
};