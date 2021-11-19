#include "LookAndFeel.h"
#include "RotarySliderWithLabels.h"
#include <BinaryData.h>

void LookAndFeel::drawRotarySlider (juce::Graphics& g,
                                    int x, int y, int width, int height,
                                    float sliderPosProportional,
                                    float rotaryStartAngle,
                                    float rotaryEndAngle,
                                    juce::Slider& slider)
{
    using namespace juce;

    auto bounds = Rectangle<float>(x, y, width, height);

    auto shadowBounds = bounds.transformed(AffineTransform().translated(3.f, 3.f));
    g.setColour(Colours::black);
    g.fillEllipse(shadowBounds);
    g.setColour(Colour::fromRGB(245, 221, 144));
    g.fillEllipse(bounds);

    auto sliderBorderThickness = 10.f;
    g.setColour(Colour::fromRGB(88, 107, 164));
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

const juce::Font& LookAndFeel::getMonoFont()
{
    static juce::Font mono(juce::Font(juce::Typeface::createSystemTypefaceFor(
        BinaryData::myfont_ttf,
        BinaryData::myfont_ttfSize
    )));

    return mono;
}