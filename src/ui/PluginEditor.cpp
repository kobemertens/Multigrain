#include "../audio_processor/PluginProcessor.h"
#include "./PluginEditor.h"

MainAudioThumbnailComponent::MainAudioThumbnailComponent(MultigrainAudioProcessor& processorRef, int sourceSamplesPerThumbnailSample, juce::AudioFormatManager& formatManager, juce::AudioThumbnailCache& cacheToUse)
    : grainVisualizer(processorRef),
      audioThumbnail(sourceSamplesPerThumbnailSample, formatManager, cacheToUse),
      previewAudioThumbnailCache(1),
      previewAudioThumbnail(sourceSamplesPerThumbnailSample, formatManager, previewAudioThumbnailCache),
      formatManager(formatManager),
      processorRef(processorRef)
{
    audioThumbnail.addChangeListener(this);
    setMouseCursor(juce::MouseCursor::PointingHandCursor);

    addAndMakeVisible(grainVisualizer);
}

MainAudioThumbnailComponent::~MainAudioThumbnailComponent()
{
    audioThumbnail.removeChangeListener(this);
    audioThumbnail.setSource(nullptr); // No idea why this is needed but does not work otherwise
    previewAudioThumbnail.setSource(nullptr);
}

void MainAudioThumbnailComponent::paint(juce::Graphics& g)
{
    if (audioThumbnail.getNumChannels() == 0 && previewAudioThumbnail.getNumChannels() == 0)
        paintIfNoFileLoaded(g, getLocalBounds());
    else
        paintIfFileLoaded(g, getLocalBounds());
}

void MainAudioThumbnailComponent::resized()
{
    grainVisualizer.setBounds(getLocalBounds());
}

void MainAudioThumbnailComponent::parameterChanged (const juce::String &parameterID, float newValue)
{
    repaint();
}

void MainAudioThumbnailComponent::setSource(juce::InputSource* newSource)
{
    audioThumbnail.setSource(newSource);
}

void MainAudioThumbnailComponent::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &audioThumbnail) repaint();
}

void MainAudioThumbnailComponent::mouseDown(const juce::MouseEvent& event)
{
    if(audioThumbnail.getNumChannels() > 0)
    {
        setCursorAtPoint(event.getPosition());
    }
    else
    {
        setMouseCursor(juce::MouseCursor::WaitCursor);
        openFileChooser();
    }
}

void MainAudioThumbnailComponent::mouseDrag(const juce::MouseEvent& event)
{
    if (audioThumbnail.getNumChannels() > 0)
        setCursorAtPoint(event.getPosition());
}

void MainAudioThumbnailComponent::mouseEnter(const juce::MouseEvent& event)
{
}

void MainAudioThumbnailComponent::mouseExit(const juce::MouseEvent& event)
{
}

void MainAudioThumbnailComponent::setCursorAtPoint(const juce::Point<int>& point)
{
    auto x = point.getX();
    processorRef.apvts.getParameter("Position")->setValueNotifyingHost((float) x / (float) getWidth());
    repaint();
}

void MainAudioThumbnailComponent::paintIfNoFileLoaded (juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds)
{
    g.setColour(juce::Colours::grey);
    g.fillRect(thumbnailBounds);
    g.setColour(juce::Colours::white);
    g.drawFittedText("Drag .wav file here (or click to open file browser)", thumbnailBounds, juce::Justification::centred, 1);
}

void MainAudioThumbnailComponent::paintIfFileLoaded (juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds)
{
    auto bounds = getLocalBounds();
    g.setColour(juce::Colour::fromRGB(245, 221, 144));
    g.fillRect(thumbnailBounds);
    juce::AudioThumbnail* thumbnailToDraw;
    juce::Colour waveformColour;
    if (previewAudioThumbnail.getNumChannels() > 0)
    {
        thumbnailToDraw = &previewAudioThumbnail;
        waveformColour = juce::Colours::grey;
    }
    else
    {
        thumbnailToDraw = &audioThumbnail;
        waveformColour = juce::Colours::black;
    }

    paintRandomPositionRegion(g);

    auto grainPosition = bounds.getWidth() * processorRef.apvts.getParameter("Position")->getValue();

    g.setColour(waveformColour);

    thumbnailToDraw->drawChannel(
        g,
        thumbnailBounds,
        0.0,
        thumbnailToDraw->getTotalLength(),
        0,
        1.f
    );

    // draw position line
    g.drawVerticalLine(
        grainPosition,
        bounds.getTopLeft().getY(),
        bounds.getBottom()
    );
}

void MainAudioThumbnailComponent::paintRandomPositionRegion(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    auto grainPosition = bounds.getWidth() * processorRef.apvts.getParameter("Position")->getValue();
    auto randomRangeScaled = bounds.getWidth() * processorRef.apvts.getParameter("Position Random")->getValue();
    auto randomRangeScaledHalf = randomRangeScaled/2;
    g.setColour(juce::Colours::lightseagreen);

    if (grainPosition + randomRangeScaledHalf > getWidth())
        g.fillRect(
            0.f,
            0.f,
            (float) (grainPosition + randomRangeScaledHalf) - getWidth(),
            (float) getHeight()
        );

    if (grainPosition - randomRangeScaledHalf < 0.f)
    {
        auto remainderX = (float) getWidth() + (grainPosition - randomRangeScaledHalf);
        g.fillRect(
            remainderX,
            0.f,
            getWidth() - remainderX,
            (float) getHeight()
        );
    }
    // draw random region
    g.fillRect(
        grainPosition - randomRangeScaled/2,
        0.f,
        (float) randomRangeScaled,
        (float) getHeight()
    );
}

bool MainAudioThumbnailComponent::isInterestedInFileDrag(const juce::StringArray &/*files*/)
{
    return true;
}

void MainAudioThumbnailComponent::fileDragEnter (const juce::StringArray &files, int /*x*/, int /*y*/)
{
    juce::File file(files[0]);
    previewAudioThumbnail.setSource(new juce::FileInputSource(file));
    repaint();
}

void MainAudioThumbnailComponent::fileDragMove (const juce::StringArray &/*files*/, int /*x*/, int /*y*/){}

void MainAudioThumbnailComponent::fileDragExit (const juce::StringArray &files)
{
    previewAudioThumbnail.setSource(nullptr);
    repaint();
}

void MainAudioThumbnailComponent::filesDropped(const juce::StringArray &files, int /*x*/, int /*y*/)
{
    for (auto string : files)
    {
        auto file = juce::File(string);
        setAudioSource(file);
    }
}

void MainAudioThumbnailComponent::openFileChooser()
{
    chooser = std::make_unique<juce::FileChooser> ("Select a Wave file to play...",
                                                       juce::File{},
                                                       "*.wav");
    auto chooserFlags = juce::FileBrowserComponent::openMode
                        | juce::FileBrowserComponent::canSelectFiles;

    chooser->launchAsync (chooserFlags, [this] (const juce::FileChooser& fc)
    {
        auto file = fc.getResult();

        if (file != juce::File{})
        {
            setAudioSource(file);
        }
        else
        {
            setMouseCursor(juce::MouseCursor::PointingHandCursor);
        }
    });
}

void MainAudioThumbnailComponent::setAudioSource(juce::File& file)
{
    setMouseCursor(juce::MouseCursor::IBeamCursor);
    std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor(file));
    if (reader.get() != nullptr)
    {
        std::cout << "Reader created!" << std::endl;
        auto duration = (float) reader->lengthInSamples / reader->sampleRate;
        if (duration < 10)
        {
            previewAudioThumbnail.setSource(nullptr);
            processorRef.getSynthAudioSource().init(new MultigrainSound("Default", *reader, 0, 60, 10));
            setSource(new juce::FileInputSource(file));
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

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (MultigrainAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p),
    keyboardComponent(processorRef.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard),
    mRootNoteSlider(*processorRef.apvts.getParameter("Root Note"), "Root Note"),
    mRootNoteSliderAttachment(processorRef.apvts, "Root Note", mRootNoteSlider),
    audioThumbnailCache(5),
    audioThumbnailComponent(processorRef, 512, formatManager, audioThumbnailCache),
    mainAdsrComponent(processorRef, {"Synth Attack", "Synth Decay", "Synth Sustain", "Synth Release"}),
    mainTabbedComponent(juce::TabbedButtonBar::Orientation::TabsAtTop),
    grainParamsComponent(processorRef.apvts),
    fxTabComponent(processorRef.apvts),
    masterGainSlider(*processorRef.apvts.getParameter("Master Gain"), "%"),
    masterGainSliderAttachment(processorRef.apvts, "Master Gain", masterGainSlider)
#if DEBUG
    , debugComponent(p)
#endif
{
    for (auto* comp : getComps())
    {
        addAndMakeVisible(comp);
    }

    mainTabbedComponent.addTab("ADSR", juce::Colour::fromRGB(50, 67, 118), &mainAdsrComponent, false);
    mainTabbedComponent.addTab("Grain", juce::Colour::fromRGB(246, 142, 95), &grainParamsComponent, false);
    mainTabbedComponent.addTab("Fx", juce::Colour::fromRGB(247, 108, 94), &fxTabComponent, false);

    setSize (500, 700);

    formatManager.registerBasicFormats();
    processorRef.apvts.addParameterListener("Position Random", &audioThumbnailComponent);
    processorRef.apvts.addParameterListener("Position", &audioThumbnailComponent);

    setResizable(true, false);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
    processorRef.apvts.removeParameterListener("Position Random", &audioThumbnailComponent);
    processorRef.apvts.removeParameterListener("Position", &audioThumbnailComponent);
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    // g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.fillAll(juce::Colour::fromRGB(50, 67, 118));

    // g.setColour (juce::Colours::white);
    // g.setFont (15.0f);
    // g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void AudioPluginAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();

#if DEBUG
    auto debugArea = bounds.removeFromTop(bounds.getHeight() * 0.25);
    debugComponent.setBounds(debugArea);
#endif

    waveformArea = bounds.removeFromTop(bounds.getHeight() * 0.25);
    auto generalSettings = bounds.removeFromTop(bounds.getHeight()*0.2);
    auto gainSliderArea = generalSettings.removeFromLeft(generalSettings.getWidth() * .5);
    auto rootNoteSliderArea = generalSettings;
    auto tabbedComponentArea = bounds.removeFromTop(bounds.getHeight() * 0.75);
    auto keyboardArea = bounds;
    keyboardComponent.setBounds(keyboardArea);
    audioThumbnailComponent.setBounds(waveformArea);

    mainTabbedComponent.setBounds(tabbedComponentArea);
    masterGainSlider.setBounds(gainSliderArea);
    mRootNoteSlider.setBounds(rootNoteSliderArea);
}

std::vector<juce::Component*> AudioPluginAudioProcessorEditor::getComps()
{
    return
    {
        &keyboardComponent,
        &mRootNoteSlider,
        &audioThumbnailComponent,
        &mainTabbedComponent,
        &masterGainSlider
#if DEBUG
        ,&debugComponent
#endif
    };
}