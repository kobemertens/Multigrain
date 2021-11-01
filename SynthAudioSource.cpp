#include "SynthAudioSource.h"

// MultigrainSound
MultigrainSound::MultigrainSound(const juce::String& soundName,
                                 juce::AudioFormatReader& source,
                                 const juce::BigInteger& notes,
                                 int midiNoteForNormalPitch,
                                 double maxSampleLengthSeconds)
    : name(soundName),
      sourceSampleRate(source.sampleRate),
      midiNotes(notes),
      midiRootNote(midiNoteForNormalPitch)
{
    if (sourceSampleRate > 0 && source.lengthInSamples > 0)
    {
        length = juce::jmin ((int) source.lengthInSamples,
                       (int) (maxSampleLengthSeconds * sourceSampleRate));

        data.reset (new juce::AudioBuffer<float> (juce::jmin (2, (int) source.numChannels), length + 4));

        source.read (data.get(), 0, length + 4, 0, true, true);
    }
}

MultigrainSound::~MultigrainSound() {}

bool MultigrainSound::appliesToNote(int /*midiNoteNumber*/)
{
    // return midiNotes[midiNoteNumber];
    return true;
}

bool MultigrainSound::appliesToChannel(int /*midiChannel*/)
{
    return true;
}

// GrainSource
GrainSource::GrainSource(MultigrainSound& sourceData, GrainEnvelope& env)
    : sourceData(sourceData), env(env) {}

void GrainSource::init(double startPosition, double pitchRatio)
{
    this->pitchRatio = pitchRatio;
    if (startPosition >= sourceData.length)
        startPosition -= sourceData.length;

    if (startPosition < 0.)
        startPosition = sourceData.length + startPosition;

    sourceSamplePosition = startPosition;
}

void GrainSource::processNextBlock(juce::AudioSampleBuffer& bufferToProcess, int startSample, int numSamples)
{
    juce::AudioSampleBuffer* data = sourceData.getAudioData();
    const float* const inL = data->getReadPointer (0);
    const float* const inR = data->getNumChannels() > 1 ? data->getReadPointer (1) : nullptr;

    float* outL = bufferToProcess.getWritePointer (0, startSample);
    float* outR = bufferToProcess.getNumChannels() > 1 ? bufferToProcess.getWritePointer (1, startSample) : nullptr;
    float nextEnvelopeSample;
    while(--numSamples >= 0)
    {
        auto pos = (int) sourceSamplePosition;
        auto alpha = (float) (sourceSamplePosition - pos);
        auto invAlpha = 1.0f - alpha;

        // just using a very simple linear interpolation here..
        float l = (inL[pos] * invAlpha + inL[pos + 1] * alpha);
        float r = (inR != nullptr) ? (inR[pos] * invAlpha + inR[pos + 1] * alpha)
                                   : l;

        nextEnvelopeSample = env.getNextSample();
        l *= nextEnvelopeSample;
        r *= nextEnvelopeSample;

        if (outR != nullptr)
        {
            *outL++ += l;
            *outR++ += r;
        }
        else
        {
            *outL++ += (l + r) * 0.5f;
        }

        sourceSamplePosition += pitchRatio;

        if (sourceSamplePosition >= sourceData.length)
            sourceSamplePosition -= sourceData.length;
    }
}

// Grain
Grain::Grain(MultigrainSound& sound)
    : source(sound, envelope),
      isActive(false)
{}

void Grain::activate(int durationSamples, double sourcePosition, double pitchRatio, float grainAmplitude)
{
    samplesRemaining = durationSamples;
    source.init(sourcePosition, pitchRatio);
    envelope.init(durationSamples, grainAmplitude);
    isActive = true;
}

void Grain::renderNextBlock(juce::AudioSampleBuffer& outputBuffer, int startSample, int numSamples)
{
    if (!isActive)
        return;

    auto samplesToProcess = juce::jmin(numSamples, samplesRemaining);
    source.processNextBlock(outputBuffer, startSample, samplesToProcess);
    // envelope.processNextBlock(outputBuffer, startSample, samplesToProcess);

    samplesRemaining -= samplesToProcess;

    if(samplesRemaining == 0)
        isActive = false;

    if(samplesRemaining < 0)
        jassertfalse; // this should not happen
}

// GrainEnvelope
void GrainEnvelope::init(int durationSamples, float grainAmplitude)
{
    amplitude = 0;
    currentSample = 0;
    this->durationSamples = durationSamples;
    this->grainAmplitude = grainAmplitude;
    attackSamples = durationSamples / 2;
    releaseSamples = durationSamples - attackSamples - 1;
    amplitudeIncrement = grainAmplitude / (float) attackSamples;
}

void GrainEnvelope::processNextBlock(juce::AudioSampleBuffer& bufferToProcess, int startSample, int numSamples)
{
    float* outL = bufferToProcess.getWritePointer(0, startSample);
    float* outR = bufferToProcess.getNumChannels() > 1 ? bufferToProcess.getWritePointer(1, startSample) : nullptr;

    while(--numSamples >= 0)
    {
        *outL++ *= amplitude;
        if(outR != nullptr)
        {
            *outR++ *= getNextSample();
        }
    }
}

float GrainEnvelope::getNextSample()
{
    auto returnValue = amplitude;

    if (currentSample == attackSamples)
        amplitudeIncrement = -(grainAmplitude / (float) releaseSamples);
    amplitude += amplitudeIncrement;
    currentSample++;

    return returnValue;
}

// MultigrainVoice
MultigrainVoice::MultigrainVoice(juce::AudioProcessorValueTreeState& apvts, MultigrainSound& sound)
    : apvts(apvts),
      samplesTillNextOnset(0),
      nextGrainToActivateIndex(0),
      sound(sound)
{
    // init grain array
    for(int i = 0; i < 8; i++)
        grains.add(new Grain(sound));
}

MultigrainVoice::~MultigrainVoice(){}

bool MultigrainVoice::canPlaySound(juce::SynthesiserSound* sound)
{
    return dynamic_cast<MultigrainSound*>(sound) != nullptr;
}

void MultigrainVoice::startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound* s, int /*currentPitchWheelPosition*/)
{
    if (auto* sound = dynamic_cast<const MultigrainSound*>(s))
    {
        deactivateGrains();
        pitchRatio = std::pow(2.0, (midiNoteNumber - sound->midiRootNote) / 12.0)
            *sound->sourceSampleRate / getSampleRate();

        currentNoteInHertz = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
        samplesTillNextOnset = 0;
        grainSpawnPosition = apvts.getParameter("Position")->getValue()*sound->length;

        lgain = velocity;
        rgain = velocity;

        adsr.setSampleRate(getSampleRate());
        juce::ADSR::Parameters params(
            (float) apvts.getRawParameterValue("Synth Attack")->load()  / 1000.f,
            (float) apvts.getRawParameterValue("Synth Decay")->load()   / 1000.f,
            (float) apvts.getRawParameterValue("Synth Sustain")->load() / 100.f,
            (float) apvts.getRawParameterValue("Synth Release")->load() / 1000.f
        );
        adsr.setParameters(params);
        adsr.noteOn();
    }
}

void MultigrainVoice::stopNote(float /*velocity*/, bool allowTailOff)
{
    if (allowTailOff)
    {
        adsr.noteOff();
    }
    else
    {
        clearCurrentNote();
        adsr.reset();
    }
}

void MultigrainVoice::pitchWheelMoved(int) {}

void MultigrainVoice::controllerMoved(int, int) {}

void MultigrainVoice::renderNextBlock(juce::AudioSampleBuffer& outputBuffer, int startSample, int numSamples)
{
    if (!isVoiceActive())
        return;

    if (auto* playingSound = static_cast<MultigrainSound*> (getCurrentlyPlayingSound().get()))
    {
        auto grainDurationFactor = apvts.getRawParameterValue("Grain Duration")->load();
        int numGrains = apvts.getRawParameterValue("Num Grains")->load();

        auto grainDurationSamples = getSampleRate() * grainDurationFactor / currentNoteInHertz;
        auto samplesBetweenOnsets = juce::roundDoubleToInt(grainDurationSamples/(float) numGrains);

        // auto grainDurationSamples = juce::roundDoubleToInt(
        //         getSampleRate() * grainDurationFactor / currentNoteInHertz
        // ); // TODO make samplesTillNextOnset floating point

        // auto samplesBetweenOnsets = juce::roundDoubleToInt(
        //         getSampleRate() * grainDurationFactor / (currentNoteInHertz*numGrains)
        // ); // TODO make samplesTillNextOnset floating point

        // Render all active grains
        for(Grain* grain : grains)
            grain->renderNextBlock(outputBuffer, startSample, numSamples);

        // Check if new grains need to be activated
        while (samplesTillNextOnset < numSamples)
        {
            Grain& grain = activateNextGrain(getNextGrainPosition(), juce::roundDoubleToInt(grainDurationSamples));
            grain.renderNextBlock(outputBuffer, startSample + samplesTillNextOnset, numSamples - samplesTillNextOnset);
            samplesTillNextOnset += samplesBetweenOnsets; // TODO allow randomness here
            updateGrainSpawnPosition(samplesBetweenOnsets);
        }

        samplesTillNextOnset -= numSamples;

        adsr.applyEnvelopeToBuffer(outputBuffer, startSample, numSamples);
        // outputBuffer.applyGain(1/(float) numGrains);

        if (!adsr.isActive())
            clearCurrentNote();
    }
}

void MultigrainVoice::updateGrainSpawnPosition(int samplesBetweenOnsets)
{
    grainSpawnPosition += (float) samplesBetweenOnsets*apvts.getRawParameterValue("Grain Speed")->load();
    grainSpawnPosition = std::fmod(grainSpawnPosition, sound.length);
}

double MultigrainVoice::getNextGrainPosition()
{
    auto randomRange = apvts.getParameter("Position Random")->getValue()*sound.length;
    auto randomDouble = randomGenerator.nextDouble();
    auto nextPosition = grainSpawnPosition + randomRange*randomDouble - randomRange/2;
    nextPosition = std::fmod(nextPosition, sound.length);
    return nextPosition;
}

Grain& MultigrainVoice::activateNextGrain(double sourcePosition, int grainDurationInSamples)
{
    Grain* grain = grains[nextGrainToActivateIndex];
    // if (grain->isActive)
    //     jassertfalse; // grain voicestealing is happening
    grain->activate(
        grainDurationInSamples,
        sourcePosition,
        pitchRatio,
        1.f // TODO allow randomization of this value
    );
    nextGrainToActivateIndex++;
    if (nextGrainToActivateIndex == grains.size())
        nextGrainToActivateIndex = 0;

    return *grain;
}

void MultigrainVoice::deactivateGrains()
{
    for(auto* grain : grains)
        grain->isActive = false;
}

// SynthAudioSource
SynthAudioSource::SynthAudioSource(juce::MidiKeyboardState& keyboardState, juce::AudioProcessorValueTreeState& apvts)
    : keyboardState(keyboardState),
      apvts(apvts) {}

SynthAudioSource::~SynthAudioSource(){}

void SynthAudioSource::prepareToPlay(int /*samplesPerBlockExpected*/, double sampleRate)
{
    synth.setCurrentPlaybackSampleRate(sampleRate);
}

void SynthAudioSource::releaseResources() {}

void SynthAudioSource::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    juce::MidiBuffer incomingMidi;
    keyboardState.processNextMidiBuffer (incomingMidi, bufferToFill.startSample, bufferToFill.numSamples, true);
    bufferToFill.clearActiveBufferRegion();

    synth.renderNextBlock(*bufferToFill.buffer, incomingMidi, bufferToFill.startSample, bufferToFill.numSamples);
}

juce::Synthesiser& SynthAudioSource::getSynth()
{
    return synth;
}

void SynthAudioSource::init(MultigrainSound* sound)
{
    // clear all previous sounds and voices
    synth.clearSounds();
    synth.clearVoices();

    synth.addSound(sound);
    for (int i = 0; i < numVoices; i++)
        synth.addVoice(new MultigrainVoice(apvts, *sound));
}