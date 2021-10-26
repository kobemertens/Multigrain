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
    g.setColour(juce::Colours::white);
    g.drawRect(bounds);
}

AdsrComponent::AdsrVisualComponent::AdsrVisualComponent(AdsrComponent::Parameters& parameters, APVTS& apvts)
    : parameters(parameters),
      apvts(apvts)
{}

void AdsrComponent::AdsrVisualComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    // auto attackMs = apvts.getRawParameterValue(parameters.attackParameter)->load();
    // auto decayMs = apvts.getRawParameterValue(parameters.decayParameter)->load();
    // auto releaseMs = apvts.getRawParameterValue(parameters.releaseParameter)->load();

    auto attackMs = 1000;
    auto decayMs = 1000;
    auto releaseMs = 1000;

    auto totalMs = attackMs + decayMs + releaseMs;

    auto attackRatio = attackMs / totalMs;
    auto decayRatio = decayMs / totalMs;
    auto releaseRatio = releaseMs / totalMs;

    g.setColour(juce::Colours::white);
    g.drawRect(bounds);

    g.setColour(juce::Colours::grey);
    g.drawLine(
        getWidth()*attackRatio,
        0,
        getWidth()*attackRatio,
        bounds.getHeight()
    );
}