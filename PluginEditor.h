#pragma once

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_utils/juce_audio_utils.h>

#include "PluginProcessor.h"
#include "RotarySliderWithLabels.h"
#include "AdsrComponent.h"
#include "GrainTabComponent.h"
#include "FxTabComponent.h"

class MainAudioThumbnailComponent : public juce::Component,
                                    public juce::AudioProcessorValueTreeState::Listener,
                                    public juce::ChangeListener,
                                    public juce::FileDragAndDropTarget
{
public:
    MainAudioThumbnailComponent(AudioPluginAudioProcessor& processorRef, int sourceSamplesPerThumbnailSample, juce::AudioFormatManager& formatManager, juce::AudioThumbnailCache& cacheToUse);
    ~MainAudioThumbnailComponent();
    void paint(juce::Graphics& g) override;
    void parameterChanged (const juce::String &parameterID, float newValue) override;
    void setSource(juce::InputSource* newSource);
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseEnter(const juce::MouseEvent& event) override;
    void mouseExit(const juce::MouseEvent& event) override;

    // FileDragAndDropTarget functions
    bool isInterestedInFileDrag(const juce::StringArray &files) override;
    void fileDragEnter (const juce::StringArray &files, int x, int y) override;
    void fileDragMove (const juce::StringArray &files, int x, int y) override;
    void fileDragExit (const juce::StringArray &files) override;
    void filesDropped(const juce::StringArray &files, int x, int y) override;
private:
    void paintIfNoFileLoaded (juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds);
    void paintIfFileLoaded (juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds);
    void paintRandomPositionRegion(juce::Graphics& g);
    void setCursorAtPoint(const juce::Point<int>& point);
    juce::AudioThumbnail audioThumbnail;
    juce::AudioThumbnailCache previewAudioThumbnailCache;
    juce::AudioThumbnail previewAudioThumbnail;
    juce::AudioFormatManager& formatManager;
    AudioPluginAudioProcessor& processorRef;
};

//==============================================================================
class AudioPluginAudioProcessorEditor  : public juce::AudioProcessorEditor

{
public:
    explicit AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor&);
    ~AudioPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AudioPluginAudioProcessor& processorRef;

    // --------------------------------------------------------------
    // Components (dont forget to add to getComps!)
    // --------------------------------------------------------------

    juce::MidiKeyboardComponent keyboardComponent;
    std::vector<juce::Component*> getComps();


    // ---------------------------------------------------------------
    juce::Rectangle<int> waveformArea;
    // ---------------------------------------------------------------

    using APVTS = juce::AudioProcessorValueTreeState;
    using SliderAttachment = APVTS::SliderAttachment;
    using ButtonAttachment = APVTS::ButtonAttachment;

    juce::AudioFormatManager formatManager;

    juce::AudioThumbnailCache audioThumbnailCache;
    // ---------------------------------------------------------------
    MainAudioThumbnailComponent audioThumbnailComponent;
    juce::TabbedComponent mainTabbedComponent;
    AdsrComponent mainAdsrComponent;
    GrainParamsComponent grainParamsComponent;
    FxTabComponent fxTabComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)
};
