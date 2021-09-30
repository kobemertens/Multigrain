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

    formatManager.registerBasicFormats();
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

bool AudioPluginAudioProcessorEditor::isInterestedInFileDrag(const juce::StringArray &files)
{
    return true;
}

void AudioPluginAudioProcessorEditor::fileDragEnter (const juce::StringArray &files, int x, int y){}

void AudioPluginAudioProcessorEditor::fileDragMove (const juce::StringArray &files, int x, int y){}

void AudioPluginAudioProcessorEditor::fileDragExit (const juce::StringArray &files){}

void AudioPluginAudioProcessorEditor::filesDropped(const juce::StringArray &files, int x, int y)
{
    for (auto string : files)
    {
        std::cout << string << std::endl;
        auto file = juce::File(string);
        std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor(file));
        if (reader.get() != nullptr)
        {
            std::cout << "Reader created!" << std::endl;
            auto duration = (float) reader->lengthInSamples / reader->sampleRate;
            if (duration < 10)
            {
                auto fileBuffer = processorRef.getFileBuffer();
                fileBuffer.setSize((int) reader->numChannels, (int) reader->lengthInSamples);
                reader->read(&fileBuffer,
                             0,
                             (int) reader->lengthInSamples,
                             0,
                             true,
                             true);
            }
            else
            {
                // TODO: handle the error that the file is 10 seconds or longer..
            }
        }
        else
        {
            // TODO: display error message here
            std::cout << "No reader created!" << std::endl;
        }
    }
    
}