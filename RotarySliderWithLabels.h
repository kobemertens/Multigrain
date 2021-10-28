#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

#include "LookAndFeel.h"

struct RotarySliderWithLabels : juce::Slider
{
    RotarySliderWithLabels(juce::RangedAudioParameter& rap, const juce::String& unitSuffix)
        : juce::Slider(juce::Slider::SliderStyle::RotaryVerticalDrag,
                                        juce::Slider::TextEntryBoxPosition::NoTextBox),
          param(&rap),
          suffix(unitSuffix)
    {
        setLookAndFeel(&lnf);
    }

    ~RotarySliderWithLabels()
    {
        setLookAndFeel(nullptr);
    }

    void paint(juce::Graphics& g) override;
    juce::Rectangle<int> getSliderBounds() const;
    juce::Rectangle<int> getTextBoxBounds() const;
    // float getTextHeightPercentage() const { return .1f; }
    int getTextHeight() const { return 14; }
    juce::String getDisplayString() const;

private:
    LookAndFeel lnf;

    juce::RangedAudioParameter* param;
    juce::String suffix;
};