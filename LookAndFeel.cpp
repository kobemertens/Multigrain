#include "LookAndFeel.h"
#include "RotarySliderWithLabels.h"

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

    auto sliderBorderThickness = 10.f;
    g.setColour(Colours::royalblue);
    // g.drawEllipse(
    //     bounds.withSizeKeepingCentre(
    //         bounds.getWidth() - sliderBorderThickness,
    //         bounds.getHeight() - sliderBorderThickness
    //     ),
    //     sliderBorderThickness
    // );

    if(auto* rswl = dynamic_cast<RotarySliderWithLabels*>(&slider))
    {
        auto center = bounds.getCentre();
        Path p;

        Rectangle<float> r;
        r.setLeft(center.getX() - 3);
        r.setRight(center.getX() + 3);
        r.setTop(bounds.getY());
        r.setBottom(center.getY() - rswl->getTextHeight() * 1.5f);
        p.addRoundedRectangle(r, 2.f);

        jassert(rotaryStartAngle < rotaryEndAngle);

        auto sliderAngRad = jmap(sliderPosProportional, 0.f, 1.f, rotaryStartAngle, rotaryEndAngle);
        p.applyTransform(AffineTransform().rotated(sliderAngRad, center.getX(), center.getY()));

        g.fillPath(p);
    }
}