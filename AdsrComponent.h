#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "PluginProcessor.h"

class AdsrComponent : public juce::Component,
                      public juce::Slider::Listener
{
public:
    AdsrComponent(AudioPluginAudioProcessor& processorRef);
    void paint(juce::Graphics& g) override;
    void sliderValueChanged(juce::Slider* slider) override;
private:
};