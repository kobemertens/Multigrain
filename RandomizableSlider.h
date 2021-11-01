#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "RotarySliderWithLabels.h"

class RandomizableSlider : public juce::Component
{
using APVTS = juce::AudioProcessorValueTreeState;
using SliderAttachment = APVTS::SliderAttachment;

public:
    RandomizableSlider(APVTS& apvts, juce::String mainParamId, juce::String randomParamId);
    ~RandomizableSlider();

    void resized() override;
    void paint(juce::Graphics& g) override;

private:
    APVTS& apvts;

    RotarySliderWithLabels mainSlider,
                           randomSlider;


    SliderAttachment mainSliderAttachment,
                     randomSliderAttachment;

};