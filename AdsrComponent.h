#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "PluginProcessor.h"
#include "RotarySliderWithLabels.h"

class AdsrVisualComponent;

class AdsrComponent : public juce::Component,
                      public juce::Slider::Listener
{
public:
    struct Parameters
    {
        juce::String attackParameter;
        juce::String decayParameter;
        juce::String sustainParameter;
        juce::String releaseParameter;
    };

    AdsrComponent(AudioPluginAudioProcessor& processorRef, const Parameters parameters);
    ~AdsrComponent();
    void resized() override;
    void paint(juce::Graphics& g) override;
    void sliderValueChanged(juce::Slider* slider) override;
private:
    using APVTS = juce::AudioProcessorValueTreeState;
    using SliderAttachment = APVTS::SliderAttachment;

    class AdsrVisualComponent : public juce::Component
    {
    public:
        using APVTS = juce::AudioProcessorValueTreeState;
        AdsrVisualComponent(AdsrComponent::Parameters parameters, APVTS& apvts);
        void paint(juce::Graphics& g) override;
        void drawGrid(juce::Graphics& g, float ms);
    private:
        Parameters parameters;
        APVTS& apvts;
    };

    RotarySliderWithLabels attackSlider,
                           decaySlider,
                           sustainSlider,
                           releaseSlider;

    SliderAttachment attackSliderAttachment,
                     decaySliderAttachment,
                     sustainSliderAttachment,
                     releaseSliderAttachment;

    Parameters parameters;
    AdsrVisualComponent visualComponent;

    AudioPluginAudioProcessor& processorRef;
};