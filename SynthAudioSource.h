#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_formats/juce_audio_formats.h>

// This class was inspired by JUCE SamplerSound class
class MultigrainSound : public juce::SynthesiserSound
{
public:
    MultigrainSound(const juce::String& soundName,
                    juce::AudioFormatReader& source, 
                    const juce::BigInteger& notes,
                    int midiNoteForNormalPitch, 
                    double attackTimeSecs, 
                    double releaseTimeSecs, 
                    double maxSampleLengthSecs);
    ~MultigrainSound() override;

    const juce::String& getName() const noexcept { return name; }

    juce::AudioSampleBuffer* getAudioData() const noexcept { return data.get(); }

    void setEnvelopeParameters (juce::ADSR::Parameters parametersToUse)    { params = parametersToUse; }

    //==============================================================================
    bool appliesToNote (int midiNoteNumber) override;
    bool appliesToChannel (int midiChannel) override;

private:
    friend class MultigrainVoice;

    juce::String name;
    std::unique_ptr<juce::AudioBuffer<float>> data;
    double sourceSampleRate;
    juce::BigInteger midiNotes;
    int length = 0, midiRootNote = 0;

    juce::ADSR::Parameters params;

    JUCE_LEAK_DETECTOR(MultigrainSound);

};

class Grain : public juce::AudioSource
{
public:
    Grain(MultigrainSound& sound, int durationInSamples, int initPos);
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void releaseResources() override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo &bufferToFill) override;
    void activate();
    void resetGrain();

private:
    int samplesRemaining;
    int samplePosition;
    MultigrainSound& sound;

    juce::ADSR adsr;
    bool isActive;
};

class MultigrainVoice : public juce::SynthesiserVoice
{
public:
    MultigrainVoice(juce::AudioProcessorValueTreeState& apvts);
    ~MultigrainVoice() override;

    bool canPlaySound(juce::SynthesiserSound* sound) override;
    
    void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*, int /*currentPitchWheelPosition*/) override;
    void stopNote(float /*velocity*/, bool allowTailOff) override;

    void pitchWheelMoved(int newValue) override;
    void controllerMoved(int controllerNumber, int newValue) override;

    void renderNextBlock(juce::AudioSampleBuffer& outputBuffer, int startSample, int numSamples) override;
private:
    double pitchRatio = 0;
    double sourceSamplePosition = 0;
    float lgain = 0, rgain = 0;

    unsigned int samplesTillNextOnset; 
    unsigned int nextGrainToActivateIndex;

    juce::ADSR adsr;

    juce::OwnedArray<Grain> grains;

    juce::AudioProcessorValueTreeState& apvts;

    JUCE_LEAK_DETECTOR(MultigrainVoice);
};

class SynthAudioSource : public juce::AudioSource
{
public:
    SynthAudioSource (juce::MidiKeyboardState& keyboardState, juce::AudioProcessorValueTreeState& apvts);
    ~SynthAudioSource();

    void setUsingSineWaveSound();
    
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void releaseResources() override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    juce::Synthesiser& getSynth();

private:
    juce::AudioProcessorValueTreeState& apvts;
    juce::MidiKeyboardState& keyboardState;
    juce::Synthesiser synth;

    JUCE_LEAK_DETECTOR(SynthAudioSource);
};