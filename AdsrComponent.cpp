#include "AdsrComponent.h"

AdsrComponent::AdsrComponent(AudioPluginAudioProcessor& processorRef, Parameters parameters)
    : processorRef(processorRef),
      attackSlider(*processorRef.apvts.getParameter(parameters.attackParameter), "ms"),
      decaySlider(*processorRef.apvts.getParameter(parameters.decayParameter), "ms"),
      sustainSlider(*processorRef.apvts.getParameter(parameters.sustainParameter), "%"),
      releaseSlider(*processorRef.apvts.getParameter(parameters.releaseParameter), "ms"),

      attackSliderAttachment(processorRef.apvts, processorRef.apvts.getParameter(parameters.attackParameter)->paramID, attackSlider),
      decaySliderAttachment(processorRef.apvts, processorRef.apvts.getParameter(parameters.decayParameter)->paramID, decaySlider),
      sustainSliderAttachment(processorRef.apvts, processorRef.apvts.getParameter(parameters.sustainParameter)->paramID, sustainSlider),
      releaseSliderAttachment(processorRef.apvts, processorRef.apvts.getParameter(parameters.releaseParameter)->paramID, releaseSlider),

      parameters(parameters),
      visualComponent(parameters, processorRef.apvts)
{
    attackSlider.addListener(this);
    decaySlider.addListener(this);
    sustainSlider.addListener(this);
    releaseSlider.addListener(this);

    addAndMakeVisible(visualComponent);
    addAndMakeVisible(attackSlider);
    addAndMakeVisible(decaySlider);
    addAndMakeVisible(sustainSlider);
    addAndMakeVisible(releaseSlider);
}

AdsrComponent::~AdsrComponent()
{
    attackSlider.removeListener(this);
    decaySlider.removeListener(this);
    sustainSlider.removeListener(this);
    releaseSlider.removeListener(this);
}

void AdsrComponent::sliderValueChanged(juce::Slider* slider)
{
    visualComponent.repaint();
}

void AdsrComponent::resized()
{
    auto bounds = getLocalBounds();
    auto visualArea = bounds.removeFromTop(getHeight()*0.66);
    auto sliderArea = bounds;

    visualComponent.setBounds(visualArea);

    attackSlider.setBounds(sliderArea.removeFromLeft(sliderArea.getWidth()*.25));
    decaySlider.setBounds(sliderArea.removeFromLeft(sliderArea.getWidth()*.33));
    sustainSlider.setBounds(sliderArea.removeFromLeft(sliderArea.getWidth()*.5));
    releaseSlider.setBounds(sliderArea);
}

void AdsrComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    bounds.removeFromTop(getHeight()*0.66);
    g.setColour(juce::Colour::fromRGB(50, 67, 118));
    g.drawRect(bounds);
}

AdsrComponent::AdsrVisualComponent::AdsrVisualComponent(AdsrComponent::Parameters parameters, APVTS& apvts)
    : parameters(parameters),
      apvts(apvts)
{}

void AdsrComponent::AdsrVisualComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    g.setColour(juce::Colour::fromRGB(88, 107, 164));
    g.fillRect(bounds);

    auto attackMs = apvts.getRawParameterValue(parameters.attackParameter)->load();
    auto decayMs = apvts.getRawParameterValue(parameters.decayParameter)->load();
    auto releaseMs = apvts.getRawParameterValue(parameters.releaseParameter)->load();
    auto sustain = apvts.getParameter(parameters.sustainParameter)->getValue();

    auto totalMs = juce::jmax(attackMs + decayMs + releaseMs, 2.f);
    drawGrid(g, totalMs);

    auto attackRatio = (float) attackMs / (float) totalMs;
    auto decayRatio = (float) decayMs / (float) totalMs;
    auto releaseRatio = (float) releaseMs / (float) totalMs;

    float lineThickness = 2.f;
    float lineThicknessHalf = lineThickness/2;

    auto paddedBounds = bounds;
    paddedBounds.removeFromLeft(lineThicknessHalf);
    paddedBounds.removeFromRight(lineThicknessHalf);
    paddedBounds.removeFromBottom(lineThicknessHalf);
    paddedBounds.removeFromTop(lineThicknessHalf);

    g.setColour(juce::Colours::white);
    g.drawLine(
        paddedBounds.getX(),
        paddedBounds.getY() + paddedBounds.getHeight(),
        paddedBounds.getX() + paddedBounds.getWidth()*attackRatio,
        paddedBounds.getY(),
        lineThickness
    );

    g.drawLine(
        paddedBounds.getX() + paddedBounds.getWidth()*attackRatio,
        paddedBounds.getY(),
        paddedBounds.getX() + paddedBounds.getWidth()*(attackRatio + decayRatio),
        paddedBounds.getY() + paddedBounds.getHeight()*(1-sustain),
        lineThickness
    );

    g.drawLine(
        paddedBounds.getX() + paddedBounds.getWidth()*(attackRatio + decayRatio),
        paddedBounds.getY() + paddedBounds.getHeight()*(1-sustain),
        paddedBounds.getX() + paddedBounds.getWidth(),
        paddedBounds.getY() + paddedBounds.getHeight(),
        lineThickness
    );
}

void AdsrComponent::AdsrVisualComponent::drawGrid(juce::Graphics& g, float ms)
{
    g.setColour(juce::Colours::white);
    bool useS = ms > 1000;
    ms /= useS ? 1000 : 100 ;
    int msFloor = (int) ms;
    float ratio = (float) msFloor / ms;
    float increment = (getWidth()*ratio)/(float) msFloor;
    float xIterator = 0;
    int counter = 0;
    while(xIterator <= getWidth())
    {
        g.drawLine(
            xIterator,
            0,
            xIterator,
            getHeight()
        );
        juce::String str;
        str << (useS ? counter : counter*100) ;
        str << (useS ? " s" : " ms");
        auto labelArea = juce::Rectangle<int>(xIterator + 4, getHeight() - 10, 100, 10);
        g.drawText(
            str,
            labelArea,
            juce::Justification::centredLeft
        );
        xIterator += increment;
        counter++;
    }
}