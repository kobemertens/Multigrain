//
// Created by kobem on 28/05/2022.
//

#include "./NoteSelector.h"

#include <utility>

NoteSelector::NoteSelector() : buttons{
    Button(false, 0),
    Button(false, 1),
    Button(false, 2),
    Button(false, 3),
    Button(false, 4),
    Button(false, 5),
    Button(false, 6),
    Button(false, 7),
    Button(false, 8),
    Button(false, 9),
    Button(false, 10),
    Button(false, 11),
}
{
    for (auto& button : buttons) {
        addAndMakeVisible(button);
    }
}

void NoteSelector::paint(juce::Graphics& g) 
{
    g.setColour(juce::Colours::white);
    auto bounds = getLocalBounds();
    g.fillRect(bounds);
}

void NoteSelector::resized() 
{
    auto bounds = getLocalBounds();
    const auto vPadding = 10;

    bounds.removeFromTop(vPadding);
    bounds.removeFromBottom(vPadding);

    const auto upperRowY = bounds.getHeight() / 4;
    const auto lowerRowY = (int) ((float) bounds.getHeight()*3.f / 4.f);
    const auto upperIndices = std::array{ 1, 3, 6, 8, 10};
    const auto lowerIndices = std::array{ 0, 2, 4, 5, 7, 9, 11};
    const auto wn = bounds.getWidth() / 7;
    const auto radius = bounds.getHeight() / 2;
    for (auto i = 0; i < 7; i++)
    {
        buttons[lowerIndices[i]].setBounds(
            bounds.getX() + wn/2 + i*wn - radius/2,
            bounds.getY() + lowerRowY - radius/2, 
            radius,
            radius
        );
    }
    auto counter = 0;
    for (auto i = 0; i < 6; i++)
    {
        if (i == 2) {
            continue;
        }
        buttons[upperIndices[counter]].setBounds(
            bounds.getX() + wn + i*wn - radius/2.f,
            bounds.getY() + upperRowY - radius/2.f, 
            radius,
            radius
        );
        counter++;
    }
}

NoteSelector::Button::Button(bool isActive, int noteId) 
: isActive(isActive),
  noteId(noteId) {}

void NoteSelector::Button::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();
    if (isAnimating) {
        auto ogBounds = bounds;
        bounds.expand(-4.f, -4.f);
        g.drawEllipse(bounds, 8.f);
        auto size = ogBounds.getWidth()/2;
        ogBounds.expand(-size*(1-animationCompletion), -size*(1-animationCompletion));
        g.fillEllipse(ogBounds);
    } else if (isActive && !isAnimating) {
        g.fillEllipse(bounds);
    } else {
        bounds.expand(-4.f, -4.f);
        g.drawEllipse(bounds, 8.f);
    }
}

void NoteSelector::Button::mouseUp(const juce::MouseEvent& event) 
{
    if (!isAnimating) {
        isAnimating = true;
        startTimer(16);
        animationCompletion = isActive ? 1.f : 0.f;
    }
    animationDelta = -animationDelta;
    isActive = !isActive;
}

void NoteSelector::Button::timerCallback()
{
    animationCompletion += animationDelta;
    if (animationCompletion > 1.f || animationCompletion < 0.f) {
        stopTimer();
        isAnimating = false;
    }
    repaint();
}