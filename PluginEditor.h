#pragma once

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_utils/juce_audio_utils.h>

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
                                         public juce::FileDragAndDropTarget,
                                         private juce::ChangeListener
                                         
{
public:
    explicit AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor&);
    ~AudioPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    // FileDragAndDropTarget functions
    bool isInterestedInFileDrag(const juce::StringArray &files) override;
    void fileDragEnter (const juce::StringArray &files, int x, int y) override;
    void fileDragMove (const juce::StringArray &files, int x, int y) override;
    void fileDragExit (const juce::StringArray &files) override;
    void filesDropped(const juce::StringArray &files, int x, int y) override;

private:
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    void paintIfNoFileLoaded (juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds);
    void paintIfFileLoaded (juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds);
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AudioPluginAudioProcessor& processorRef;
    
    // --------------------------------------------------------------
    // Components (dont forget to add to getComps)
    // --------------------------------------------------------------
    CustomRotarySlider grainRateSlider,
                       grainDurationSlider,
                       positionSlider;

    juce::MidiKeyboardComponent keyboardComponent;
    juce::AudioThumbnail audioThumbnail;

    std::vector<juce::Component*> getComps();
    // ---------------------------------------------------------------

    using APVTS = juce::AudioProcessorValueTreeState;
    using Attachment = APVTS::SliderAttachment;

    Attachment grainRateSliderAttachment,
               grainDurationSliderAttachment,
               positionSliderAttachment;
    
    juce::AudioFormatManager formatManager;

    juce::AudioThumbnailCache audioThumbnailCache;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)
};
