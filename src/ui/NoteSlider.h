//
// Created by kobem on 28/05/2022.
//

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

#include "LookAndFeel.h"

class NoteSlider : public juce::Slider
{
public:
    NoteSlider(
        juce::RangedAudioParameter& rootNote,
        juce::String labelText
    );

    ~NoteSlider() override = default;

    void paint(juce::Graphics& g) override;

private:
    LookAndFeel mLnf;

    juce::String mLabelText;
    juce::RangedAudioParameter& mRootNote;

    const double kNoteNameHeight = 14;
};