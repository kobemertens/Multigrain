#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p),
    grainRateSliderAttachment(processorRef.apvts, "Grain Rate", grainRateSlider),
    grainDurationSliderAttachment(processorRef.apvts, "Grain Duration", grainDurationSlider),
    positionSliderAttachment(processorRef.apvts, "Position", positionSlider),
    keyboardComponent(processorRef.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard),
    audioThumbnailCache(5),
    audioThumbnail(512, formatManager, audioThumbnailCache)
{
    for (auto* comp : getComps())
    {
        addAndMakeVisible(comp);
    }

    setSize (600, 400);

    formatManager.registerBasicFormats();
    audioThumbnail.addChangeListener(this);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
    audioThumbnail.removeChangeListener(this);
    audioThumbnail.setSource(nullptr); // No idea why this is needed but does not work otherwise
}

void AudioPluginAudioProcessorEditor::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &audioThumbnail) repaint();
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    auto bounds = getLocalBounds();
    auto thumbnailBounds = bounds.removeFromTop(bounds.getHeight() * 0.5);
    if (audioThumbnail.getNumChannels() == 0)
        paintIfNoFileLoaded(g, thumbnailBounds);
    else
        paintIfFileLoaded(g, thumbnailBounds);

    // g.setColour (juce::Colours::white);
    // g.setFont (15.0f);
    // g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void AudioPluginAudioProcessorEditor::paintIfNoFileLoaded (juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds)
{
    g.setColour(juce::Colours::darkgrey);
    g.fillRect(thumbnailBounds);
    g.setColour(juce::Colours::white);
    g.drawFittedText("Drag .wav file here", thumbnailBounds, juce::Justification::centred, 1);
}

void AudioPluginAudioProcessorEditor::paintIfFileLoaded (juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds)
{
    g.setColour(juce::Colours::white);
    g.fillRect(thumbnailBounds);
    g.setColour(juce::Colours::black);

    audioThumbnail.drawChannels(g,
                                thumbnailBounds,
                                0.0,
                                audioThumbnail.getTotalLength(),
                                1.0f);
}

void AudioPluginAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    auto waveFormArea = bounds.removeFromTop(bounds.getHeight() * 0.5);
    auto knobArea = bounds.removeFromTop(bounds.getHeight() * 0.7);
    auto keyboardArea = bounds;

    auto grainRateArea = knobArea.removeFromLeft(knobArea.getWidth() * 0.33);
    auto grainDurationArea = knobArea.removeFromLeft(knobArea.getWidth() * 0.5);
    auto durationArea = knobArea;
    
    grainRateSlider.setBounds(grainRateArea);
    grainDurationSlider.setBounds(grainDurationArea);
    positionSlider.setBounds(durationArea);
    keyboardComponent.setBounds(keyboardArea);
}

std::vector<juce::Component*> AudioPluginAudioProcessorEditor::getComps()
{
    return
    {
        &grainRateSlider,
        &grainDurationSlider,
        &positionSlider,
        &keyboardComponent
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
        auto file = juce::File(string);
        std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor(file));
        if (reader.get() != nullptr)
        {
            std::cout << "Reader created!" << std::endl;
            auto duration = (float) reader->lengthInSamples / reader->sampleRate;
            if (duration < 10)
            {
                processorRef.getSynthAudioSource().getSynth().addSound(new MultigrainSound(string, *reader, 0, 60, 0.02, 0.02, 10));
                audioThumbnail.setSource(new juce::FileInputSource(file));
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