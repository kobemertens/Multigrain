//
// Created by kobe on 19/11/2021.
//

#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>

/**
 * Manages sourceData buffer and channel-note-mask
 */
class MultigrainSound : public juce::SynthesiserSound
{
public:
    MultigrainSound(const juce::String& soundName,
                    juce::AudioFormatReader& source,
                    const juce::BigInteger& notes,
                    int midiNoteForNormalPitch,
                    double maxSampleLengthSecs);
    ~MultigrainSound() override;

    const juce::String& getName() const noexcept { return name; }

    juce::AudioSampleBuffer* getAudioData() const noexcept { return data.get(); }

    //==============================================================================
    bool appliesToNote (int midiNoteNumber) override;
    bool appliesToChannel (int midiChannel) override;

private:
    friend class MultigrainVoice;
    friend class GrainSource;

    juce::String name;
    std::unique_ptr<juce::AudioBuffer<float>> data;
    double sourceSampleRate;
    juce::BigInteger midiNotes;
    int length = 0, midiRootNote = 0;

    JUCE_LEAK_DETECTOR(MultigrainSound);

};