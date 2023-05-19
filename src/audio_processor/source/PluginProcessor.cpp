#include "../PluginProcessor.h"
#include "../../ui/PluginEditor.h"

//==============================================================================
MultigrainAudioProcessor::MultigrainAudioProcessor()
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
      synthAudioSource(keyboardState, apvts),
      masterGain(apvts.getRawParameterValue("Master Gain")),
      applyReverb(apvts.getRawParameterValue("Reverb Toggle"))
{
}

MultigrainAudioProcessor::~MultigrainAudioProcessor()
{
}

//==============================================================================
const juce::String MultigrainAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MultigrainAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MultigrainAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MultigrainAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MultigrainAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MultigrainAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MultigrainAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MultigrainAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String MultigrainAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void MultigrainAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void MultigrainAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::ignoreUnused (sampleRate, samplesPerBlock);

    synthAudioSource.prepareToPlay(samplesPerBlock, sampleRate);
    reverb.setSampleRate(sampleRate);
}

void MultigrainAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.

    synthAudioSource.releaseResources();
}

bool MultigrainAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}

void MultigrainAudioProcessor::processBlock(
    juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer& midiMessages
)
{
    for (auto theMessage : midiMessages)
        keyboardState.processNextMidiEvent(theMessage.getMessage());

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    juce::AudioSourceChannelInfo ci (buffer);
    synthAudioSource.getNextAudioBlock(ci);
    float* outL = buffer.getWritePointer (0, 0);
    float* outR = buffer.getWritePointer (1, 0);

    if (*applyReverb >= 0.5f)
        reverb.processStereo(outL, outR, buffer.getNumSamples());

    buffer.applyGain(*masterGain);
}

//==============================================================================

bool
MultigrainAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor*
MultigrainAudioProcessor::createEditor()
{
    return new AudioPluginAudioProcessorEditor (*this);
    // return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================

void
MultigrainAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::ignoreUnused (destData);
}

void
MultigrainAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    juce::ignoreUnused (data, sizeInBytes);
}

juce::AudioProcessorValueTreeState::ParameterLayout
MultigrainAudioProcessor::createParameterLayout()
{
    auto theLayout = juce::AudioProcessorValueTreeState::ParameterLayout();

    theLayout.add(
        std::make_unique<juce::AudioParameterFloat>(
            "Master Gain",
            "Master Gain",
            juce::NormalisableRange<float>(0.f, 1.f, .0001f, 1.f),
            1.f
        )
    );

    // Determines the number of mGrains. 2 mGrains are offset by 180 deg etc..
    theLayout.add(
        std::make_unique<juce::AudioParameterInt>(
            "Num Grains",
            "Num Grains",
            1,
            8,
            1
        )
    );

    // Increases the grain period by a factor ranging from 1 to 1000
    theLayout.add(
        std::make_unique<juce::AudioParameterFloat>(
            "Grain Duration",
            "Duration",
            juce::NormalisableRange<float>(1.f, 1000.f, .0001f, .2f),
            1.f
        )
    );

    // Grain duration randomness. A setting of 100 % varies between half and twice the grain period.
    theLayout.add(std::make_unique<juce::AudioParameterFloat>("Grain Duration Random",
                                                           "Duration Random",
                                                           juce::NormalisableRange<float>(0.f, 1.f, .0001f, .1f),
                                                           0.f));

    // Set between -12 and +12 semitones. Grains are played randomly at their original pitch or the set value.
    theLayout.add(std::make_unique<juce::AudioParameterInt>("Grain Pitch Interval",
                                                         "Pitch Interval",
                                                         -12,
                                                         12,
                                                         0));

    // Allows to set a scale in which the mGrains are played randomly
    // layout.add(std::make_unique<juce::AudioParameterChoice>("Grain Pitch Random",
    //                                                         "Pitch Scale",
    //                                                         juce::StringArray{},
    //                                                         0));

    // Playback position of the mGrains.
    theLayout.add(std::make_unique<juce::AudioParameterFloat>("Position",
                                                           "Position",
                                                           juce::NormalisableRange<float>(0.f, 1.f, .0001f, 1.f),
                                                           0.f));

    theLayout.add(std::make_unique<juce::AudioParameterFloat>("Synth Attack",
                                                         "Attack",
                                                         juce::NormalisableRange<float>(0.f, 30000.f, 1.f, .2f),
                                                         0.f));

    theLayout.add(std::make_unique<juce::AudioParameterFloat>("Synth Decay",
                                                         "Decay",
                                                         juce::NormalisableRange<float>(0.f, 30000.f, 1.f, .2f),
                                                         1000.f));

    theLayout.add(std::make_unique<juce::AudioParameterInt>("Synth Sustain",
                                                         "Sustain",
                                                         0,
                                                         100,
                                                         100));

    theLayout.add(std::make_unique<juce::AudioParameterFloat>("Synth Release",
                                                         "Release",
                                                         juce::NormalisableRange<float>(0.f, 30000.f, 1.f, .2f),
                                                         1000.f));

    theLayout.add(std::make_unique<juce::AudioParameterBool>("Reverb Toggle",
                                                          "Reverb Toggle",
                                                          false));

    // Randomizes the playback position of the mGrains. Calculated separately for each channel of the sample
    theLayout.add(std::make_unique<juce::AudioParameterFloat>("Position Random",
                                                           "Position Random",
                                                           juce::NormalisableRange<float>(0.f, 1.f, .0001f, .2f),
                                                           0.f));

    theLayout.add(std::make_unique<juce::AudioParameterFloat>("Grain Speed",
                                                           "Speed",
                                                           juce::NormalisableRange<float>(-2.f, 2.f, .0001f, 1.f),
                                                           0.f));

    juce::StringArray grainShapeChoices;
    grainShapeChoices.add("Triangle");
    grainShapeChoices.add("Hanning");
    // Sets the shape of the grain envelope
    theLayout.add(
        std::make_unique<juce::AudioParameterChoice>(
            "Grain Envelope Shape",
            "Shape",
            grainShapeChoices,
            0
        )
    );

    theLayout.add(
        std::make_unique <juce::AudioParameterInt>(
            "Root Note",
            "Root Note",
            0,     // Min: C1
            127,   // Max: G9
            60     // Default: C4
        )
    );

    return theLayout;
}

SynthAudioSource&
MultigrainAudioProcessor::getSynthAudioSource()
{
    return synthAudioSource;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MultigrainAudioProcessor();
}
