#include "PluginProcessor.h"
#include "PluginEditor.h"

void LookAndFeel::drawRotarySlider (juce::Graphics& g,
                                    int x, int y, int width, int height,
                                    float sliderPosProportional,
                                    float rotaryStartAngle,
                                    float rotaryEndAngle,
                                    juce::Slider& slider)
{
    using namespace juce;

    auto bounds = Rectangle<float>(x, y, width, height);

    g.setColour(Colours::white);
    g.fillEllipse(bounds);

    g.setColour(Colours::black);
    g.drawEllipse(bounds, 1.f);

    if(auto* rswl = dynamic_cast<RotarySliderWithLabels*>(&slider))
    {
        auto center = bounds.getCentre();
        Path p;

        Rectangle<float> r;
        r.setLeft(center.getX() - 2);
        r.setRight(center.getX() + 2);
        r.setTop(bounds.getY());
        r.setBottom(center.getY() - rswl->getTextHeight() * 1.5);
        p.addRoundedRectangle(r, 2.f);
        
        jassert(rotaryStartAngle < rotaryEndAngle);

        auto sliderAngRad = jmap(sliderPosProportional, 0.f, 1.f, rotaryStartAngle, rotaryEndAngle);
        p.applyTransform(AffineTransform().rotated(sliderAngRad, center.getX(), center.getY()));

        g.fillPath(p);

        g.setFont(rswl->getTextHeight());
        auto text = rswl->getDisplayString();
        auto strWidth = g.getCurrentFont().getStringWidth(text);
        r.setSize(strWidth + 4, rswl->getTextHeight() + 2);
        r.setCentre(bounds.getCentre());

        g.setColour(Colours::black);
        g.fillRect(r);

        g.setColour(Colours::white);
        g.drawFittedText(text, r.toNearestInt(), juce::Justification::centred, 1);
    }
}

void RotarySliderWithLabels::paint(juce::Graphics& g)
{
    using namespace juce;

    auto startAng = degreesToRadians(180.f + 45.f);
    auto endAng = degreesToRadians(180.f - 45.f) + MathConstants<float>::twoPi;

    auto range = getRange();

    auto sliderBounds = getSliderBounds();

    // g.setColour(Colours::red);
    // g.drawRect(getLocalBounds());
    // g.setColour(Colours::yellow);
    // g.drawRect(sliderBounds);

    getLookAndFeel().drawRotarySlider(
        g, 
        sliderBounds.getX(),
        sliderBounds.getY(),
        sliderBounds.getWidth(),
        sliderBounds.getHeight(),
        jmap(getValue(), range.getStart(), range.getEnd(), 0., 1.),
        startAng,
        endAng, 
        *this
    );

    auto center = sliderBounds.toFloat().getCentre();
    auto radius = sliderBounds.getWidth() * 0.5f;

    g.setColour(Colours::black);
    g.setFont(getTextBoxHeight());

    auto numChoices = labels.size();
    for (int i = 0; i < numChoices; i++)
    {
        auto pos = labels[i].pos;
        jassert(0.f <= pos);
        jassert(pos <= 1.f);

        auto ang = jmap(pos, 0.f, 1.f, startAng, endAng);

        auto c = center.getPointOnCircumference(radius + getTextHeight() * 0.5f + 1, ang);

        Rectangle<float> r;
        auto str = labels[i].label;
        r.setSize(g.getCurrentFont().getStringWidth(str), getTextHeight());
        r.setCentre(c);
        r.setY(r.getY() + getTextHeight());

        g.drawFittedText(str, r.toNearestInt(), juce::Justification::centred, 1);
    }
}

juce::Rectangle<int> RotarySliderWithLabels::getSliderBounds() const
{
    auto bounds = getLocalBounds();

    auto size = juce::jmin(bounds.getWidth(), bounds.getHeight());

    size -= getTextHeight() * 2;
    juce::Rectangle<int> r;
    r.setSize(size, size);
    r.setCentre(bounds.getCentreX(), 0);
    r.setY(2);

    return r;
}

juce::String RotarySliderWithLabels::getDisplayString() const
{
    juce::String str;
    bool isPercent = false;
    if(auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(param))
    {
        float val = getValue();
        if(floatParam->paramID == "Position")
        {
            val *= 100;
            isPercent = true;

        }
        str = juce::String(val, 0);
    }
    else
    {
        jassertfalse; // this should not happen!
    }

    if (suffix.isNotEmpty())
    {
        str << " ";
        str << suffix;
    }

    return str;
}

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p),
    grainRateSlider(*processorRef.apvts.getParameter("Grain Rate"), "Hz"),
    grainDurationSlider(*processorRef.apvts.getParameter("Grain Duration"), "s"),
    positionSlider(*processorRef.apvts.getParameter("Position"), "%"),
    grainRateSliderAttachment(processorRef.apvts, "Grain Rate", grainRateSlider),
    grainDurationSliderAttachment(processorRef.apvts, "Grain Duration", grainDurationSlider),
    positionSliderAttachment(processorRef.apvts, "Position", positionSlider),
    keyboardComponent(processorRef.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard),
    audioThumbnailCache(5),
    audioThumbnail(512, formatManager, audioThumbnailCache)
{
    grainRateSlider.labels.add({0.f, "0.1 Hz"});
    grainRateSlider.labels.add({1.f, "4 Hz"});
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

bool AudioPluginAudioProcessorEditor::isInterestedInFileDrag(const juce::StringArray &/*files*/)
{
    return true;
}

void AudioPluginAudioProcessorEditor::fileDragEnter (const juce::StringArray &/*files*/, int /*x*/, int /*y*/){}

void AudioPluginAudioProcessorEditor::fileDragMove (const juce::StringArray &/*files*/, int /*x*/, int /*y*/){}

void AudioPluginAudioProcessorEditor::fileDragExit (const juce::StringArray &/*files*/){}

void AudioPluginAudioProcessorEditor::filesDropped(const juce::StringArray &files, int /*x*/, int /*y*/)
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