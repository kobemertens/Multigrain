//
// Created by kobem on 28/05/2022.
//

#include "./NoteSlider.h"

#include <utility>

NoteSlider::NoteSlider(
    juce::RangedAudioParameter &rootNote,
    juce::String labelText
):
    juce::Slider::Slider(
        juce::Slider::SliderStyle::RotaryVerticalDrag,
        juce::Slider::NoTextBox
    ),
    mLabelText(std::move(labelText)),
    mRootNote(rootNote)
{
    auto dragPixelsPerNote = 5.;
    setMouseDragSensitivity(
        dragPixelsPerNote
        *mRootNote.getNormalisableRange().getRange().getLength()
    );
    setSliderSnapsToMousePosition(false);
}

void NoteSlider::paint(juce::Graphics &g)
{
    using namespace juce;

    auto theLocalBounds = getLocalBounds();

    auto theLabelBounds = theLocalBounds.removeFromTop(theLocalBounds.getHeight() * .333);
    g.setColour(Colours::thistle);
    g.fillRect(theLabelBounds);

    auto theNoteNameBounds = theLocalBounds;
    g.setColour(Colours::cornsilk);
    g.fillRect(theNoteNameBounds);

    auto theNoteName = MidiMessage::getMidiNoteName(
        static_cast<std::size_t>(getValue()),
        /* useSharps= */ true,
        /* includeOctaveNumber= */ true,
        4
    );

    g.setColour(Colours::black);
    g.setFont(theNoteNameBounds.getHeight());
    g.drawText(theNoteName, theNoteNameBounds, Justification::centred);

    g.setFont(theLabelBounds.getHeight());
    g.drawText(mLabelText, theLabelBounds, Justification::centred);
}