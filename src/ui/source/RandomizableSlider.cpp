#include "../RandomizableSlider.h"

RandomizableSlider::RandomizableSlider(APVTS& apvts, juce::String mainParamId, juce::String randomParamId)
    : apvts(apvts),
      mainSlider(*apvts.getParameter(mainParamId), ""),
      randomSlider(*apvts.getParameter(randomParamId), ""),
      mainSliderAttachment(apvts, mainParamId, mainSlider),
      randomSliderAttachment(apvts, randomParamId, randomSlider)
{
    addAndMakeVisible(mainSlider);
    addAndMakeVisible(randomSlider);
}

RandomizableSlider::~RandomizableSlider()
{
}

void RandomizableSlider::resized()
{
    auto bounds = getLocalBounds();

    mainSlider.setBounds(bounds);
    bounds.removeFromBottom(bounds.getWidth() * 0.6);
    bounds.removeFromLeft(bounds.getWidth() * 0.6);
    randomSlider.setBounds(bounds);
}

void RandomizableSlider::paint(juce::Graphics &g) {
    g.setColour(juce::Colours::darkred);
    g.fillRect(getLocalBounds());
}
