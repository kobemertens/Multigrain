#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p),
    grainRateSliderAttachment(processorRef.apvts, "Grain Rate", grainRateSlider),
    grainDurationSliderAttachment(processorRef.apvts, "Grain Duration", grainDurationSlider),
    positionSliderAttachment(processorRef.apvts, "Position", positionSlider)
{
    juce::ignoreUnused (processorRef);
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    for (auto* comp : getComps())
    {
        addAndMakeVisible(comp);
    }

    setSize (600, 400);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void AudioPluginAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    auto bounds = getLocalBounds();
    auto waveFormArea = bounds.removeFromTop(bounds.getHeight() * 0.33);

    auto grainRateArea = bounds.removeFromLeft(bounds.getWidth() * 0.33);
    auto grainDurationArea = bounds.removeFromLeft(bounds.getWidth() * 0.5);
    auto durationArea = bounds;

    grainRateSlider.setBounds(grainRateArea);
    grainDurationSlider.setBounds(grainDurationArea);
    positionSlider.setBounds(durationArea);
}

std::vector<juce::Component*> AudioPluginAudioProcessorEditor::getComps()
{
    return
    {
        &grainRateSlider,
        &grainDurationSlider,
        &positionSlider
    };
}