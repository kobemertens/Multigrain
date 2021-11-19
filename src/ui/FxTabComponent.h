#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "RotarySliderWithLabels.h"
#include "../audio_processor/PluginProcessor.h"

class FxTabComponent : public juce::Component,
                       public juce::Button::Listener
{
using APVTS = juce::AudioProcessorValueTreeState;
using SliderAttachment = APVTS::SliderAttachment;
using ButtonAttachment = APVTS::ButtonAttachment;
public:
    FxTabComponent(APVTS& apvts);
    ~FxTabComponent();
    void resized() override;
    void paint(juce::Graphics& g) override;
    void buttonClicked (juce::Button *) override;
private:
    juce::ToggleButton reverbToggleButton;
    ButtonAttachment reverbToggleButtonAttachment;
    std::vector<juce::Component*> getComps();
    APVTS& apvts;
};