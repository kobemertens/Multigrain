#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "RotarySliderWithLabels.h"
#include "PluginProcessor.h"
#include "RandomizableSlider.h"

class GrainParamsComponent : public juce::Component
{
using APVTS = juce::AudioProcessorValueTreeState;
using SliderAttachment = APVTS::SliderAttachment;

public:
    GrainParamsComponent(APVTS& apvts);
    void resized() override;
private:
    RotarySliderWithLabels numGrainsSlider,
                           grainDurationSlider,
                           positionSlider,
                           randomPositionSlider;

    SliderAttachment numGrainsSliderAttachment,
                     grainDurationSliderAttachment,
                     positionSliderAttachment,
                     randomPositionSliderAttachment;

    RandomizableSlider grainSpeedSlider;

    APVTS& apvts;

    std::vector<juce::Component*> getComps();
};