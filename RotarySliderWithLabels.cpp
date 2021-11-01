#include "RotarySliderWithLabels.h"

void RotarySliderWithLabels::paint(juce::Graphics& g)
{
    using namespace juce;

    auto startAng = degreesToRadians(180.f + 45.f);
    auto endAng = degreesToRadians(180.f - 45.f) + MathConstants<float>::twoPi;

    auto range = getRange();

    auto sliderBounds = getSliderBounds();

    g.setColour(Colours::red);
    g.drawRect(getLocalBounds());
    g.setColour(Colours::yellow);
    g.drawRect(sliderBounds);

    getLookAndFeel().drawRotarySlider(
        g,
        sliderBounds.getX(),
        sliderBounds.getY(),
        sliderBounds.getWidth(),
        sliderBounds.getHeight(),
        valueToProportionOfLength(getValue()),
        startAng,
        endAng,
        *this
    );

    auto center = sliderBounds.toFloat().getCentre();
    auto radius = sliderBounds.getWidth() * 0.5f;

    auto textBoxBounds = getTextBoxBounds();
    g.setColour(Colours::black);
    g.setFont(getTextHeight());
    g.drawText(getDisplayString(), textBoxBounds, Justification::centred);

    g.setColour(Colours::green);
    g.drawRect(textBoxBounds);
}

juce::Rectangle<int> RotarySliderWithLabels::getSliderBounds() const
{
    auto bounds = getLocalBounds();

    auto size = juce::jmin(bounds.getWidth(), bounds.getHeight());

    size -= getTextHeight() * 2;
    juce::Rectangle<int> r;
    r.setSize(size, size);
    r.setCentre(bounds.getCentreX(), 0);
    r.setY(2);

    return r;
}

juce::Rectangle<int> RotarySliderWithLabels::getTextBoxBounds() const
{
    auto sliderBounds = getSliderBounds();
    auto localBounds = getLocalBounds();
    auto textHeight = getTextHeight();

    return juce::Rectangle<int>(
        localBounds.getX(),
        sliderBounds.getBottomLeft().getY(),
        localBounds.getWidth(),
        localBounds.getHeight() - sliderBounds.getHeight()
    );
}

juce::String RotarySliderWithLabels::getDisplayString() const
{
    if(!isMouseOverOrDragging())
        return param->getName(9999);

    juce::String str;
    bool isPercent = false;
    if(auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(param))
    {
        float val = getValue();
        if(floatParam->paramID == "Position")
        {
            val *= 100;
            isPercent = true;

        }
        str = juce::String(val, 0);
    }
    else if (auto* intParam = dynamic_cast<juce::AudioParameterInt*>(param))
    {
        int val = getValue();
        str = juce::String(val);
    }
    else
    {
        jassertfalse; // this should not happen!
    }

    if (suffix.isNotEmpty())
    {
        str << " ";
        str << suffix;
    }

    return str;
}