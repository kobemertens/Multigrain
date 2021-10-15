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
    friend class GrainSource;

    juce::String name;
    std::unique_ptr<juce::AudioBuffer<float>> data;
    double sourceSampleRate;
    juce::BigInteger midiNotes;
    int length = 0, midiRootNote = 0;

    juce::ADSR::Parameters params;

    JUCE_LEAK_DETECTOR(MultigrainSound);

};

/**
 * Write samples from sourceData to buffer according to pitch ratio.
 */
class GrainSource
{
public:
    GrainSource(MultigrainSound& sourceData);
    void processNextBlock(juce::AudioSampleBuffer& bufferToProcess, int startSample, int numSamples); // write information about pitch here
    void init(int startPositionSample, double pitchRatio);

    bool isDepleted; // returns true if there are no samples left in the source

private:
    double sourceSamplePosition;
    double pitchRatio;
    MultigrainSound& sourceData;
};

/**
 * Process an incoming buffer by applying the envelope.
 */
class GrainEnvelope
{
public:
    void processNextBlock(juce::AudioSampleBuffer& bufferToProcess, int startSample, int numSamples);
    void init(int durationSamples, float grainAmplitude);
private:
    float amplitude;
    float rdur;
    float rdur2;
    float slope;
    float curve;
};

/**
 * Applies envelope to source. Deactivates itself when completed.
 */
class Grain
{
public:
    Grain(MultigrainSound& sound);
    void renderNextBlock(juce::AudioSampleBuffer& outputBuffer, int startSample, int numSamples);
    void activate(int durationSamples, int sourcePosition, double pitchRatio, float grainAmplitude);
private:
    GrainSource source;
    GrainEnvelope envelope;

    bool isActive;
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
    Grain& activateNextGrain();
    double pitchRatio = 0;
    double sourceSamplePosition = 0;
    float lgain = 0, rgain = 0;

    int samplesTillNextOnset;
    unsigned int nextGrainToActivateIndex;

    juce::ADSR adsr;

    juce::OwnedArray<Grain> grains;

    juce::AudioProcessorValueTreeState& apvts;
    MultigrainSound& sound;

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

    void initSynthAudioSource(MultigrainSound* sound);
private:
    juce::MidiKeyboardState& keyboardState;
    juce::AudioProcessorValueTreeState& apvts;
    juce::Synthesiser synth;

    JUCE_LEAK_DETECTOR(SynthAudioSource);
};