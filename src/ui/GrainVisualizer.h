#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "../audio_processor/PluginProcessor.h"

class GrainVisualizer : public juce::Component,
                        public juce::Timer
{
public:
    GrainVisualizer(MultigrainAudioProcessor& processorRef);
    ~GrainVisualizer() override;
    void resized() override;
    void paint(juce::Graphics& g) override;
private:
    void timerCallback() override;

    MultigrainAudioProcessor& processorRef;
};