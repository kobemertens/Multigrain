#include "GrainTabComponent.h"

GrainParamsComponent::GrainParamsComponent(APVTS& apvts)
    : apvts(apvts),
      numGrainsSlider(*apvts.getParameter("Num Grains"), ""),
      positionSlider(*apvts.getParameter("Position"), "%"),
      grainDurationSlider(*apvts.getParameter("Grain Duration"), ""),
      randomPositionSlider(*apvts.getParameter("Random Position"), "%"),

      numGrainsSliderAttachment(apvts, "Num Grains", numGrainsSlider),
      positionSliderAttachment(apvts, "Position", positionSlider),
      grainDurationSliderAttachment(apvts, "Grain Duration", grainDurationSlider),
      randomPositionSliderAttachment(apvts, "Random Position", randomPositionSlider)
{
    for (auto* comp : getComps())
    {
        addAndMakeVisible(comp);
    }
}

void GrainParamsComponent::resized()
{
    auto bounds = getLocalBounds();
    auto upperHalf = bounds.removeFromTop(bounds.getHeight()*0.5);
    auto lowerHalf = bounds;

    auto leftUpper = upperHalf.removeFromLeft(upperHalf.getWidth()*0.5);
    auto rightUpper = upperHalf;

    auto leftLower = lowerHalf.removeFromLeft(lowerHalf.getWidth()*0.5);
    auto rightLower = lowerHalf;

    numGrainsSlider.setBounds(leftUpper);
    positionSlider.setBounds(rightUpper);
    grainDurationSlider.setBounds(leftLower);
    randomPositionSlider.setBounds(rightLower);
}

std::vector<juce::Component*> GrainParamsComponent::getComps()
{
    return
    {
        &grainDurationSlider,
        &numGrainsSlider,
        &positionSlider,
        &randomPositionSlider
    };
}