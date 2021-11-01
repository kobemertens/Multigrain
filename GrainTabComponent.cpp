#include "GrainTabComponent.h"

GrainParamsComponent::GrainParamsComponent(APVTS& apvts)
    : apvts(apvts),
      numGrainsSlider(*apvts.getParameter("Num Grains"), ""),
      positionSlider(*apvts.getParameter("Position"), "%"),
      grainDurationSlider(*apvts.getParameter("Grain Duration"), ""),
      randomPositionSlider(*apvts.getParameter("Position Random"), "%"),
      grainSpeedSlider(apvts, "Grain Speed", "Grain Speed Random"),

      numGrainsSliderAttachment(apvts, "Num Grains", numGrainsSlider),
      positionSliderAttachment(apvts, "Position", positionSlider),
      grainDurationSliderAttachment(apvts, "Grain Duration", grainDurationSlider),
      randomPositionSliderAttachment(apvts, "Position Random", randomPositionSlider)
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

    auto lowerFirst = lowerHalf.removeFromLeft(lowerHalf.getWidth()*0.33);
    auto lowerSecond = lowerHalf.removeFromLeft(lowerHalf.getWidth()*0.5);
    auto lowerThird = lowerHalf;

    numGrainsSlider.setBounds(leftUpper);
    positionSlider.setBounds(rightUpper);
    grainDurationSlider.setBounds(lowerFirst);
    randomPositionSlider.setBounds(lowerSecond);
    grainSpeedSlider.setBounds(lowerThird);
}

std::vector<juce::Component*> GrainParamsComponent::getComps()
{
    return
    {
        &grainDurationSlider,
        &numGrainsSlider,
        &positionSlider,
        &randomPositionSlider,
        &grainSpeedSlider
    };
}