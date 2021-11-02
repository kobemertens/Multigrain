#include "FxTabComponent.h"

FxTabComponent::FxTabComponent(APVTS& apvts)
    : apvts(apvts),
      reverbToggleButtonAttachment(apvts, "Reverb Toggle", reverbToggleButton)
{
    for(auto* comp : getComps())
        addAndMakeVisible(comp);

    reverbToggleButton.addListener(this);
}

FxTabComponent::~FxTabComponent()
{
    reverbToggleButton.removeListener(this);
}

void FxTabComponent::resized()
{
    auto bounds = getLocalBounds();
    auto reverbPart = bounds.removeFromLeft(bounds.getWidth()*0.5);
    auto delayPart = bounds;

    reverbToggleButton.setBounds(reverbPart);
}

void FxTabComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    g.setColour(juce::Colour::fromRGB(247, 108, 94));
    g.fillRect(bounds);
    auto reverbPart = bounds.removeFromLeft(bounds.getWidth()*0.5);
    auto delayPart = bounds;
    auto colour = apvts.getParameter("Reverb Toggle")->getValue() ? juce::Colours::lightsteelblue : juce::Colours::grey;
    g.setColour(colour);
    g.fillRect(reverbPart);

    colour = apvts.getParameter("Reverb Toggle")->getValue() ? juce::Colours::lightcoral : juce::Colours::grey;
    g.setColour(colour);
    g.fillRect(delayPart);
    g.setColour(juce::Colours::white);
    g.drawText("Reverb", reverbPart, juce::Justification::centred);
    g.drawText("Delay", delayPart, juce::Justification::centred);
}

void FxTabComponent::buttonClicked (juce::Button *)
{
    repaint();
}

std::vector<juce::Component*> FxTabComponent::getComps()
{
    return
    {
        &reverbToggleButton
    };
}