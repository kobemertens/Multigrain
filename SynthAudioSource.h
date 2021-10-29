#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_formats/juce_audio_formats.h>

/**
 * Manages sourceData buffer and channel-note-mask
 */
class MultigrainSound : public juce::SynthesiserSound
{
public:
    MultigrainSound(const juce::String& soundName,
                    juce::AudioFormatReader& source,
                    const juce::BigInteger& notes,
                    int midiNoteForNormalPitch,
                    double maxSampleLengthSecs);
    ~MultigrainSound() override;

    const juce::String& getName() const noexcept { return name; }

    juce::AudioSampleBuffer* getAudioData() const noexcept { return data.get(); }

    //==============================================================================
    bool appliesToNote (int midiNoteNumber) override;
    bool appliesToChannel (int midiChannel) override;

private:
    friend class MultigrainVoice;
    friend class GrainSource;

    juce::String name;
    std::unique_ptr<juce::AudioBuffer<float>> data;
    double sourceSampleRate;
    juce::BigInteger midiNotes;
    int length = 0, midiRootNote = 0;

    JUCE_LEAK_DETECTOR(MultigrainSound);

};

/**
 * Process an incoming buffer by applying the envelope.
 */
class GrainEnvelope
{
public:
    void processNextBlock(juce::AudioSampleBuffer& bufferToProcess, int startSample, int numSamples);
    void init(int durationSamples, float grainAmplitude);
    float getNextSample();
private:
    float amplitude;
    float grainAmplitude;
    int attackSamples;
    int releaseSamples;
    float amplitudeIncrement;
    int currentSample;
    int durationSamples;
};

/**
 * Write samples from sourceData to buffer according to pitch ratio.
 */
class GrainSource
{
public:
    GrainSource(MultigrainSound& sourceData, GrainEnvelope& env);
    void processNextBlock(juce::AudioSampleBuffer& bufferToProcess, int startSample, int numSamples); // write information about pitch here
    void init(double startPosition, double pitchRatio);

private:
    GrainEnvelope& env;
    double pitchRatio;
    double sourceSamplePosition;
    MultigrainSound& sourceData;
};


/**
 * Applies envelope to source. Deactivates itself when completed.
 */
class Grain
{
public:
    Grain(MultigrainSound& sound);
    void renderNextBlock(juce::AudioSampleBuffer& outputBuffer, int startSample, int numSamples);
    void activate(int durationSamples, double sourcePosition, double pitchRatio, float grainAmplitude);
    void deactivate();
    bool isActive;
private:
    GrainEnvelope envelope;
    GrainSource source;

    int samplesRemaining;
};

/**
 * Manages and schedules grains;
 */
class MultigrainVoice : public juce::SynthesiserVoice
{
public:
    MultigrainVoice(juce::AudioProcessorValueTreeState& apvts, MultigrainSound& sound);
    ~MultigrainVoice() override;

    bool canPlaySound(juce::SynthesiserSound* sound) override;

    void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*, int /*currentPitchWheelPosition*/) override;
    void stopNote(float /*velocity*/, bool allowTailOff) override;

    void pitchWheelMoved(int newValue) override;
    void controllerMoved(int controllerNumber, int newValue) override;

    void renderNextBlock(juce::AudioSampleBuffer& outputBuffer, int startSample, int numSamples) override;
private:
    Grain& activateNextGrain(double sourcePosition, int grainDurationInSamples);
    double getNextGrainPosition();
    void deactivateGrains();
    double pitchRatio = 0;
    double sourceSamplePosition = 0;
    float lgain = 0, rgain = 0;

    double currentNoteInHertz;

    int samplesTillNextOnset;
    unsigned int nextGrainToActivateIndex;
    double grainSpawnPosition;

    juce::ADSR adsr;

    juce::OwnedArray<Grain> grains;

    juce::AudioProcessorValueTreeState& apvts;
    MultigrainSound& sound;

    juce::Random randomGenerator;

    JUCE_LEAK_DETECTOR(MultigrainVoice);
};

class SynthAudioSource : public juce::AudioSource
{
public:
    SynthAudioSource (juce::MidiKeyboardState& keyboardState, juce::AudioProcessorValueTreeState& apvts);
    ~SynthAudioSource();

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void releaseResources() override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    juce::Synthesiser& getSynth();

    void init(MultigrainSound* sound);
private:
    juce::MidiKeyboardState& keyboardState;
    juce::AudioProcessorValueTreeState& apvts;
    juce::Synthesiser synth;

    JUCE_LEAK_DETECTOR(SynthAudioSource);
};