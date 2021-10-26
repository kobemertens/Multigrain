#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

#include "LookAndFeel.h"

struct RotarySliderWithLabels : juce::Slider
{
    enum Type
    {
        DEFAULT,
        HIGHVALUEINT
    };

    RotarySliderWithLabels(juce::RangedAudioParameter& rap, const juce::String& unitSuffix, RotarySliderWithLabels::Type type = RotarySliderWithLabels::DEFAULT)
        : juce::Slider(juce::Slider::SliderStyle::RotaryVerticalDrag,
                                        juce::Slider::TextEntryBoxPosition::NoTextBox),
          param(&rap),
          suffix(unitSuffix),
          type(type)
    {
        setLookAndFeel(&lnf);
    }

    ~RotarySliderWithLabels()
    {
        setLookAndFeel(nullptr);
    }

    struct LabelPos
    {
        float pos;
        juce::String label;
    };

    juce::Array<LabelPos> labels;

    void paint(juce::Graphics& g) override;
    juce::Rectangle<int> getSliderBounds() const;
    int getTextHeight() const { return 14; }
    juce::String getDisplayString() const;

private:
    LookAndFeel lnf;
    RotarySliderWithLabels::Type type;

    juce::RangedAudioParameter* param;
    juce::String suffix;
};

struct RotarySlider : juce::Slider
{
    enum Type
    {
        DEFAULT,
        HIGHVALUEINT
    };

    RotarySlider(juce::RangedAudioParameter& rap, const juce::String& unitSuffix)
        : juce::Slider(juce::Slider::SliderStyle::RotaryVerticalDrag,
                                        juce::Slider::TextEntryBoxPosition::NoTextBox),
          param(&rap),
          suffix(unitSuffix)
    {
        setLookAndFeel(&lnf);
    }

    ~RotarySlider()
    {
        setLookAndFeel(nullptr);
    }

    void paint(juce::Graphics& g) override;
    juce::Rectangle<int> getSliderBounds() const;
    int getTextHeight() const { return 14; }
    juce::String getDisplayString() const;

private:
    LookAndFeel lnf;

    juce::RangedAudioParameter* param;
    juce::String suffix;
};