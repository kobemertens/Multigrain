#include "MultigrainSynth.h"

// MultigrainSynth
MultigrainSynth::MultigrainSynth(juce::AudioProcessorValueTreeState& apvts)
    : apvts(apvts) {}

MultigrainSynth::~MultigrainSynth(){}

void MultigrainSynth::init(MultigrainSound* sound)
{
    // clear all previous sounds and voices
    clearSounds();
    clearVoices();

    addSound(sound);
    for (int i = 0; i < numVoices; i++)
        addVoice(new MultigrainVoice(apvts, *sound));
}