#pragma once

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_utils/juce_audio_utils.h>

#include "PluginProcessor.h"

struct LookAndFeel : juce::LookAndFeel_V4
{
    void drawRotarySlider (juce::Graphics&,
                                       int x, int y, int width, int height,
                                       float sliderPosProportional,
                                       float rotaryStartAngle,
                                       float rotaryEndAngle,
                                       juce::Slider&) override;
};

struct RotarySliderWithLabels : juce::Slider
{
    RotarySliderWithLabels(juce::RangedAudioParameter& rap, const juce::String& unitSuffix, double skewFactor=1., bool symmetricSkew=false)
        : juce::Slider(juce::Slider::SliderStyle::RotaryVerticalDrag,
                                        juce::Slider::TextEntryBoxPosition::NoTextBox),
          param(&rap),
          suffix(unitSuffix)
    {
        setLookAndFeel(&lnf);
        setSkewFactor(skewFactor, symmetricSkew);
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

    juce::RangedAudioParameter* param;
    juce::String suffix;
};

//==============================================================================
class AudioPluginAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                         public juce::FileDragAndDropTarget,
                                         private juce::ChangeListener,
                                         private juce::Slider::Listener
                                         
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

    // Slider::Listener functions
    void sliderValueChanged(juce::Slider* slider) override;

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
    RotarySliderWithLabels numGrainsSlider,
                           grainDurationSlider,
                           positionSlider;

    juce::MidiKeyboardComponent keyboardComponent;
    juce::AudioThumbnail audioThumbnail;

    std::vector<juce::Component*> getComps();
    // ---------------------------------------------------------------

    using APVTS = juce::AudioProcessorValueTreeState;
    using Attachment = APVTS::SliderAttachment;

    Attachment numGrainsSliderAttachment,
               grainDurationSliderAttachment,
               positionSliderAttachment;
    
    juce::AudioFormatManager formatManager;

    juce::AudioThumbnailCache audioThumbnailCache;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)
};
