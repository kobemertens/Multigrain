#include "./DebugComponent.h"

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
    g.setColour(juce::Colours::white);
    g.setFont(15);
    g.drawText(generateDebugText(), getLocalBounds(), juce::Justification::centred);
}

juce::String DebugComponent::generateDebugText()
{
    auto debugText = juce::String{};
    debugText += "Grain Count: ";
    debugText += (int) mGrainCount;
    debugText += "\n";
    debugText += "Voices: ";
    debugText += (int) mActiveVoices;

    return debugText;
}

void DebugComponent::timerCallback() 
{
    mGrainCount = getNumGrains(processorRef.getSynthAudioSource().getSilos());
    mActiveVoices = getNumActiveVoices(processorRef.getSynthAudioSource().mSynth);
    repaint();
}