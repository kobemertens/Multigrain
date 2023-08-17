#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "../audio_processor/PluginProcessor.h"

class DebugComponent : public juce::Component,
                       public juce::Timer
{
public:
    DebugComponent(MultigrainAudioProcessor& processorRef);
    ~DebugComponent() override;
    void resized() override;
    void paint(juce::Graphics& g) override;
private:
    void timerCallback() override;
    juce::String generateDebugText();

    size_t mGrainCount = 0;
    size_t mActiveVoices = 0;
    MultigrainAudioProcessor& processorRef;
};