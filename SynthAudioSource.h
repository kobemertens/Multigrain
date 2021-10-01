#include <juce_audio_processors/juce_audio_processors.h>

class MultigrainSound : public juce::SynthesiserSound
{
public:
    MultigrainSound();
    ~MultigrainSound();

    bool appliesToNote    (int) override;
    bool appliesToChannel (int) override;
};

class MultigrainVoice : public juce::SynthesiserVoice
{
public:
    MultigrainVoice();
    ~MultigrainVoice();

    bool canPlaySound(juce::SynthesiserSound* sound) override;
    void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*, int /*currentPitchWheelPosition*/) override;
    void stopNote(float /*velocity*/, bool allowTailOff) override;
    void pitchWheelMoved(int) override;
    void controllerMoved(int, int) override;
    void renderNextBlock(juce::AudioSampleBuffer& outputBuffer, int startSample, int numSamples) override;
private:
};

class SynthAudioSource : public juce::AudioSource
{
public:
    SynthAudioSource (juce::MidiKeyboardState& keyboardState);
    ~SynthAudioSource();

    void setUsingSineWaveSound();
    
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void releaseResources() override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;

private:
    juce::MidiKeyboardState& keyboardState;
    juce::Synthesiser synth;
};