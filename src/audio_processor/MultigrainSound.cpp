//
// Created by kobe on 19/11/2021.
//

#include "./MultigrainSound.h"


// MultigrainSound
MultigrainSound::MultigrainSound(
    const juce::String& soundName,
    juce::AudioFormatReader& source,
    const juce::BigInteger& notes,
    int midiNoteForNormalPitch,
    double maxSampleLengthSeconds
):
    name(soundName),
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
    }
}

MultigrainSound::~MultigrainSound() = default;

bool MultigrainSound::appliesToNote(int /*midiNoteNumber*/)
{
    // return midiNotes[midiNoteNumber];
    return true;
}

bool MultigrainSound::appliesToChannel(int /*midiChannel*/)
{
    return true;
}