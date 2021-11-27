#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_formats/juce_audio_formats.h>

#include "MultigrainSound.h"
#include "MultigrainVoice.h"
#include "Synthesiser.h"

class MultigrainSynth : public Synthesiser
{
public:
    MultigrainSynth (juce::AudioProcessorValueTreeState& apvts);
    ~MultigrainSynth();

    void init(MultigrainSound* sound);
private:
    juce::AudioProcessorValueTreeState& apvts;

    int numVoices = 1;
    juce::String name;

    JUCE_LEAK_DETECTOR(MultigrainSynth);
};