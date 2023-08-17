
#include "../DebugComponent.h"

DebugComponent::DebugComponent(MultigrainAudioProcessor& processorRef)
    : processorRef(processorRef)
{
    startTimer(60);
}

DebugComponent::~DebugComponent()
{
}

void DebugComponent::resized()
{
    auto bounds = getLocalBounds();
}

void DebugComponent::paint(juce::Graphics& g)
{
    g.drawText(juce::String(mGrainCount), getLocalBounds(), juce::Justification::topRight);
}

void DebugComponent::timerCallback() 
{

    mGrainCount = 0;
    const auto silos = this->processorRef.getSynthAudioSource().getSilos();
    for(const auto silo : silos) {
        for(int i = 0; i < 8; i++) {
            if ((*silo)[i]->isActive) mGrainCount += 1;
        }
    }

    repaint();
}