//
// Created by kobem on 28/05/2022.
//

#pragma once

#include <array>
#include <functional>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

#include "LookAndFeel.h"

class NoteSelector : public juce::Component
{
    class Button : public juce::Component,
                          juce::Timer
    {
    public:
        Button(
            bool isActive, 
            int noteId
        );
        ~Button() override = default;

        void paint(juce::Graphics& g) override;
        void mouseUp(const juce::MouseEvent& event) override;

        void timerCallback() override;

    private:
        bool isActive = false;
        float animationDuration = 100.f; // in milliseconds
        bool isAnimating = false;
        float animationCompletion = 0.f;
        float animationDelta = -1/(animationDuration*0.06);
        int noteId;
    };
public:
    NoteSelector();

    ~NoteSelector() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    LookAndFeel mLnf;
    std::array<Button, 12> buttons;
};
