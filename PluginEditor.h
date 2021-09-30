#pragma once

#include <juce_audio_formats/juce_audio_formats.h>

#include "PluginProcessor.h"

struct CustomRotarySlider : juce::Slider
{
    CustomRotarySlider() : juce::Slider(juce::Slider::SliderStyle::RotaryVerticalDrag,
                                        juce::Slider::TextEntryBoxPosition::NoTextBox)
    {
    }
};

//==============================================================================
class AudioPluginAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                         public juce::FileDragAndDropTarget
{
public:
    explicit AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor&);
    ~AudioPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    // FileDragAndDropTarget functions
    bool isInterestedInFileDrag(const juce::StringArray &files);
    void fileDragEnter (const juce::StringArray &files, int x, int y);
    void fileDragMove (const juce::StringArray &files, int x, int y);
    void fileDragExit (const juce::StringArray &files);
    void filesDropped(const juce::StringArray &files, int x, int y);

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AudioPluginAudioProcessor& processorRef;

    CustomRotarySlider grainRateSlider,
                       grainDurationSlider,
                       positionSlider;

    using APVTS = juce::AudioProcessorValueTreeState;
    using Attachment = APVTS::SliderAttachment;

    Attachment grainRateSliderAttachment,
               grainDurationSliderAttachment,
               positionSliderAttachment;

    std::vector<juce::Component*> getComps();

    juce::AudioFormatManager formatManager;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)
};
