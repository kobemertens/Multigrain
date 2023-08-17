#include "../GrainVisualizer.h"
#include "juce_core/system/juce_PlatformDefs.h"

namespace
{
    size_t getNumGrains(const std::vector<Silo*>& silos)
    {
        auto grainCount = size_t{0};
        for(const auto silo : silos) {
            for(int i = 0; i < 8; i++) {
                if ((*silo)[i]->isActive) grainCount += 1;
            }
        }

        return grainCount;
    }

    size_t getNumActiveVoices(const juce::Synthesiser& synth)
    {
        auto numActiveVoices = size_t{0};
        for (int i = 0; i < synth.getNumVoices(); i++)
        {
            auto voice = synth.getVoice(i);
            numActiveVoices += voice->isVoiceActive() ? 1 : 0;
        }

        return numActiveVoices;
    }
}

GrainVisualizer::GrainVisualizer(MultigrainAudioProcessor& processorRef)
    : processorRef(processorRef)
{
    setInterceptsMouseClicks(false, false);
    startTimer(60);
}

GrainVisualizer::~GrainVisualizer()
{
}

void GrainVisualizer::resized()
{
    auto bounds = getLocalBounds();
}

void GrainVisualizer::paint(juce::Graphics& g)
{
    const auto bounds = getLocalBounds();
    const auto centreY = bounds.getCentreY();
    const auto height = bounds.getHeight();
    g.setColour(juce::Colours::white);
    g.drawRect(getLocalBounds());
    for (const auto silo: processorRef.getSynthAudioSource().getSilos())
    {
        for (const auto grain : *silo)
        {
            if (grain->isActive)
            {
                const auto xPos = grain->getRelativeGrainPosition().leftPosition * bounds.getWidth();
                const auto amplitude = grain->getGrainAmplitude();
                g.drawLine(xPos, centreY - amplitude/2*height, xPos, centreY + amplitude/2*height, 2 + 3*amplitude);
            }
        }
    }
}

void GrainVisualizer::timerCallback() 
{
    repaint();
}