#include "SynthAudioSource.h"

// MultigrainSound
MultigrainSound::MultigrainSound(const juce::String& soundName,
                                 juce::AudioFormatReader& source,
                                 const juce::BigInteger& notes,
                                 int midiNoteForNormalPitch,
                                 double attackTimeSecs,
                                 double releaseTimeSecs,
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

        params.attack  = static_cast<float> (attackTimeSecs);
        params.release = static_cast<float> (releaseTimeSecs);
    }
}

MultigrainSound::~MultigrainSound()
{
}

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
GrainSource::GrainSource(MultigrainSound& sourceData)
    : sourceData(sourceData) {}

void GrainSource::init(int startPositionSample, double pitchRatio)
{
    this->pitchRatio = pitchRatio;
    sourceSamplePosition = (double) startPositionSample;
    isDepleted = startPositionSample >= sourceData.length;
}

void GrainSource::processNextBlock(juce::AudioSampleBuffer& bufferToProcess, int startSample, int numSamples)
{
    if (isDepleted)
        return;

    juce::AudioSampleBuffer* data = sourceData.getAudioData();
    const float* const inL = data->getReadPointer (0);
    const float* const inR = data->getNumChannels() > 1 ? data->getReadPointer (1) : nullptr;

    float* outL = bufferToProcess.getWritePointer (0, startSample);
    float* outR = bufferToProcess.getNumChannels() > 1 ? bufferToProcess.getWritePointer (1, startSample) : nullptr;

    while(--numSamples >= 0)
    {
        auto pos = (int) sourceSamplePosition;
        auto alpha = (float) (sourceSamplePosition - pos);
        auto invAlpha = 1.0f - alpha;

        // just using a very simple linear interpolation here..
        float l = (inL[pos] * invAlpha + inL[pos + 1] * alpha);
        float r = (inR != nullptr) ? (inR[pos] * invAlpha + inR[pos + 1] * alpha)
                                   : l;

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

        if (sourceSamplePosition > sourceData.length)
        {
            isDepleted = true;
            break;
        }
    }
}

// Grain
Grain::Grain(MultigrainSound& sound)
    : source(sound),
      isActive(false)
{}

void Grain::activate(int durationSamples, int sourcePosition, double pitchRatio, float grainAmplitude)
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

    if(samplesRemaining - numSamples <= 0) // disable grains whose source is depleted here
        isActive = false;
}

// GrainEnvelope
void GrainEnvelope::init(int durationSamples, float grainAmplitude)
{
    amplitude = 0;
    rdur = 1.f/durationSamples;
    rdur2 = rdur*rdur;
    slope = 4.f * grainAmplitude * (rdur - rdur2);
    curve = -8.f * grainAmplitude * rdur2;
}

void GrainEnvelope::processNextBlock(juce::AudioSampleBuffer& bufferToProcess, int startSample, int numSamples)
{
    float* outL = bufferToProcess.getWritePointer(0, startSample);
    float* outR = bufferToProcess.getNumChannels() > 1 ? bufferToProcess.getWritePointer(1, startSample) : nullptr;

    while(--numSamples >= 0)
    {
        amplitude += slope;
        *outL++ *= amplitude;
        if(outR != nullptr)
        {
            *outR++ *= amplitude;
        }
        slope += curve;
    }
}

// MultigrainVoice
MultigrainVoice::MultigrainVoice(juce::AudioProcessorValueTreeState& apvts, MultigrainSound& sound)
    : apvts(apvts),
      samplesTillNextOnset(0),
      nextGrainToActivateIndex(0),
      sound(sound)
{
    // init grain array
    auto maxDuration = apvts.getParameter("Grain Duration")->getNormalisableRange().getRange().getEnd();
    auto maxRate = apvts.getParameter("Grain Rate")->getNormalisableRange().getRange().getEnd();
    for(int i = 0; i < maxDuration*maxRate; i++)
        grains.add(new Grain(sound));
}

MultigrainVoice::~MultigrainVoice(){}

bool MultigrainVoice::canPlaySound(juce::SynthesiserSound* sound)
{
    return dynamic_cast<MultigrainSound*>(sound) != nullptr;
}

void MultigrainVoice::startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound* s, int /*currentPitchWheelPosition*/)
{
    // do things to start the note (envelope etc)
    if (auto* sound = dynamic_cast<const MultigrainSound*>(s))
    {
        pitchRatio = std::pow(2.0, (midiNoteNumber - sound->midiRootNote) / 12.0)
            *sound->sourceSampleRate / getSampleRate();

        sourceSamplePosition = apvts.getParameter("Position")->getValue() * sound->length;

        lgain = velocity;
        rgain = velocity;
        
        adsr.setSampleRate(sound->sourceSampleRate);
        adsr.setParameters(sound->params);
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
    if (auto* playingSound = static_cast<MultigrainSound*> (getCurrentlyPlayingSound().get()))
    {
        // Render all active grains
        for(Grain* grain : grains)
            grain->renderNextBlock(outputBuffer, startSample, numSamples);

        // Check if new grains need to be activated
        while (samplesTillNextOnset < numSamples)
        {
            Grain& grain = activateNextGrain();
            grain.renderNextBlock(outputBuffer, startSample + samplesTillNextOnset, numSamples - samplesTillNextOnset);
            samplesTillNextOnset += (unsigned int) (1/apvts.getRawParameterValue("Grain Rate")->load()*getSampleRate()); // TODO allow randomness here
        }

        samplesTillNextOnset -= numSamples;

        adsr.applyEnvelopeToBuffer(outputBuffer, startSample, numSamples);
    }
}

Grain& MultigrainVoice::activateNextGrain()
{
    Grain* grain = grains[nextGrainToActivateIndex];
    grains[nextGrainToActivateIndex]->activate(
        apvts.getRawParameterValue("Grain Duration")->load()*getSampleRate(),
        apvts.getParameter("Position")->getValue()*sound.getAudioData()->getNumSamples(),
        1., // TODO use actual pitchRatio of the pressed key
        1.f // TODO allow randomization of this value
    );
    nextGrainToActivateIndex++;
    if (nextGrainToActivateIndex == grains.size())
        nextGrainToActivateIndex = 0;

    return *grain;
}

// SynthAudioSource
SynthAudioSource::SynthAudioSource(juce::MidiKeyboardState& keyboardState, juce::AudioProcessorValueTreeState& apvts)
    : keyboardState(keyboardState),
      apvts(apvts) {}

SynthAudioSource::~SynthAudioSource(){}

void SynthAudioSource::setUsingSineWaveSound()
{
    synth.clearSounds();
}

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

void SynthAudioSource::initSynthAudioSource(MultigrainSound* sound)
{
    synth.addSound(sound);
    synth.addVoice(new MultigrainVoice(apvts, *sound));
}