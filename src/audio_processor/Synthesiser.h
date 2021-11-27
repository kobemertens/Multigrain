// Improved version of the JUCE Synthesiser

#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

/**
 * Represents a voice that a Synthesiser can use to play a SynthesiserSound
 *
 * A voice plays a single sound at a time, and a synthesiser holds an array of
 * voices so that it can play polyphonically.
*/
class SynthesiserVoice : public juce::AudioSource
{
public:
    SynthesiserVoice();

    virtual ~SynthesiserVoice();

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;

    void releaseResources() override;

    int getCurrentlyPlayingNote() const noexcept { return currentlyPlayingNote; }
    double getSampleRate() const noexcept { return currentSampleRate; }
    juce::SynthesiserSound::Ptr getCurrentlyPlayingSound() const noexcept { return currentlyPlayingSound; }
    virtual bool canPlaySound(juce::SynthesiserSound*) = 0;

    virtual void startNote(int midiNoteNumber,
                           float velocity,
                           juce::SynthesiserSound* sound,
                           int currentPitchWheelPosition) = 0;

    virtual void stopNote (float velocity, bool allowTailOff) = 0;
    virtual bool isVoiceActive() const;
    /** Called to let the voice know that the pitch wheel has been moved.
        This will be called during the rendering callback, so must be fast and thread-safe.
    */
    virtual void pitchWheelMoved (int newPitchWheelValue) = 0;

    /** Called to let the voice know that a midi controller has been moved.
        This will be called during the rendering callback, so must be fast and thread-safe.
    */
    virtual void controllerMoved (int controllerNumber, int newControllerValue) = 0;

    /** Called to let the voice know that the aftertouch has changed.
        This will be called during the rendering callback, so must be fast and thread-safe.
    */
    virtual void aftertouchChanged (int newAftertouchValue);

    /** Called to let the voice know that the channel pressure has changed.
        This will be called during the rendering callback, so must be fast and thread-safe.
    */
    virtual void channelPressureChanged (int newChannelPressureValue);
    virtual bool isPlayingChannel (int midiChannel) const;

    /** Returns true if the key that triggered this voice is still held down.
        Note that the voice may still be playing after the key was released (e.g because the
        sostenuto pedal is down).
    */
    bool isKeyDown() const noexcept                             { return keyIsDown; }

    /** Allows you to modify the flag indicating that the key that triggered this voice is still held down.
        @see isKeyDown
    */
    void setKeyDown (bool isNowDown) noexcept                   { keyIsDown = isNowDown; }

    /** Returns true if the sustain pedal is currently active for this voice. */
    bool isSustainPedalDown() const noexcept                    { return sustainPedalDown; }

    /** Modifies the sustain pedal flag. */
    void setSustainPedalDown (bool isNowDown) noexcept          { sustainPedalDown = isNowDown; }

    /** Returns true if the sostenuto pedal is currently active for this voice. */
    bool isSostenutoPedalDown() const noexcept                  { return sostenutoPedalDown; }

    /** Modifies the sostenuto pedal flag. */
    void setSostenutoPedalDown (bool isNowDown) noexcept        { sostenutoPedalDown = isNowDown; }

    /** Returns true if a voice is sounding in its release phase **/
    bool isPlayingButReleased() const noexcept
    {
        return isVoiceActive() && ! (isKeyDown() || isSostenutoPedalDown() || isSustainPedalDown());
    }

    /** Returns true if this voice started playing its current note before the other voice did. */
    bool wasStartedBefore (const SynthesiserVoice& other) const noexcept;

protected:
    void clearCurrentNote();

private:
    friend class Synthesiser;
    double currentSampleRate = 44100.0;
    int currentlyPlayingNote = -1, currentlyPlayingMidiChannel = 0;
    juce::uint32 noteOnTime = 0;
    juce::SynthesiserSound::Ptr currentlyPlayingSound;
    bool keyIsDown = false, sustainPedalDown = false, sostenutoPedalDown = false;
    juce::AudioSampleBuffer localBuffer;

    JUCE_LEAK_DETECTOR(SynthesiserVoice)
};

class Synthesiser : public juce::AudioProcessor
{
public:
    Synthesiser();

    virtual ~Synthesiser();

    void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override;

    void releaseResources() override;

    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

    const juce::String getName() const override;

    void clearVoices();

    int getNumVoices() const noexcept { return voices.size(); }

    double getSampleRate() const noexcept { return sampleRate; }

    SynthesiserVoice* getVoice(int index) const;

    SynthesiserVoice* addVoice(SynthesiserVoice* newVoice);

    void removeVoice(int index);

    void clearSounds();

    int getNumSounds() const noexcept { return sounds.size(); }

    juce::SynthesiserSound* getSound(int index) const noexcept { return sounds[index]; }
    juce::SynthesiserSound* addSound(const juce::SynthesiserSound::Ptr& newSound);

    void removeSound(int index);

    void setNoteStealingEnabled (bool shouldStealNotes);
    /** Returns true if note-stealing is enabled.
        @see setNoteStealingEnabled
    */
    bool isNoteStealingEnabled() const noexcept                     { return shouldStealNotes; }

    //==============================================================================
    /** Triggers a note-on event.

        The default method here will find all the sounds that want to be triggered by
        this note/channel. For each sound, it'll try to find a free voice, and use the
        voice to start playing the sound.

        Subclasses might want to override this if they need a more complex algorithm.

        This method will be called automatically according to the midi data passed into
        renderNextBlock(), but may be called explicitly too.

        The midiChannel parameter is the channel, between 1 and 16 inclusive.
    */
    virtual void noteOn (int midiChannel,
                         int midiNoteNumber,
                         float velocity);

    /** Triggers a note-off event.

        This will turn off any voices that are playing a sound for the given note/channel.

        If allowTailOff is true, the voices will be allowed to fade out the notes gracefully
        (if they can do). If this is false, the notes will all be cut off immediately.

        This method will be called automatically according to the midi data passed into
        renderNextBlock(), but may be called explicitly too.

        The midiChannel parameter is the channel, between 1 and 16 inclusive.
    */
    virtual void noteOff (int midiChannel,
                          int midiNoteNumber,
                          float velocity,
                          bool allowTailOff);

    /** Turns off all notes.

        This will turn off any voices that are playing a sound on the given midi channel.

        If midiChannel is 0 or less, then all voices will be turned off, regardless of
        which channel they're playing. Otherwise it represents a valid midi channel, from
        1 to 16 inclusive.

        If allowTailOff is true, the voices will be allowed to fade out the notes gracefully
        (if they can do). If this is false, the notes will all be cut off immediately.

        This method will be called automatically according to the midi data passed into
        renderNextBlock(), but may be called explicitly too.
    */
    virtual void allNotesOff (int midiChannel,
                              bool allowTailOff);

    /** Sends a pitch-wheel message to any active voices.

        This will send a pitch-wheel message to any voices that are playing sounds on
        the given midi channel.

        This method will be called automatically according to the midi data passed into
        renderNextBlock(), but may be called explicitly too.

        @param midiChannel          the midi channel, from 1 to 16 inclusive
        @param wheelValue           the wheel position, from 0 to 0x3fff, as returned by MidiMessage::getPitchWheelValue()
    */
    virtual void handlePitchWheel (int midiChannel,
                                   int wheelValue);

    /** Sends a midi controller message to any active voices.

        This will send a midi controller message to any voices that are playing sounds on
        the given midi channel.

        This method will be called automatically according to the midi data passed into
        renderNextBlock(), but may be called explicitly too.

        @param midiChannel          the midi channel, from 1 to 16 inclusive
        @param controllerNumber     the midi controller type, as returned by MidiMessage::getControllerNumber()
        @param controllerValue      the midi controller value, between 0 and 127, as returned by MidiMessage::getControllerValue()
    */
    virtual void handleController (int midiChannel,
                                   int controllerNumber,
                                   int controllerValue);

    /** Sends an aftertouch message.

        This will send an aftertouch message to any voices that are playing sounds on
        the given midi channel and note number.

        This method will be called automatically according to the midi data passed into
        renderNextBlock(), but may be called explicitly too.

        @param midiChannel          the midi channel, from 1 to 16 inclusive
        @param midiNoteNumber       the midi note number, 0 to 127
        @param aftertouchValue      the aftertouch value, between 0 and 127,
                                    as returned by MidiMessage::getAftertouchValue()
    */
    virtual void handleAftertouch (int midiChannel, int midiNoteNumber, int aftertouchValue);

    /** Sends a channel pressure message.

        This will send a channel pressure message to any voices that are playing sounds on
        the given midi channel.

        This method will be called automatically according to the midi data passed into
        renderNextBlock(), but may be called explicitly too.

        @param midiChannel              the midi channel, from 1 to 16 inclusive
        @param channelPressureValue     the pressure value, between 0 and 127, as returned
                                        by MidiMessage::getChannelPressureValue()
    */
    virtual void handleChannelPressure (int midiChannel, int channelPressureValue);

    /** Handles a sustain pedal event. */
    virtual void handleSustainPedal (int midiChannel, bool isDown);

    /** Handles a sostenuto pedal event. */
    virtual void handleSostenutoPedal (int midiChannel, bool isDown);

    /** Can be overridden to handle soft pedal events. */
    virtual void handleSoftPedal (int midiChannel, bool isDown);

    /** Can be overridden to handle an incoming program change message.
        The base class implementation of this has no effect, but you may want to make your
        own synth react to program changes.
    */
    virtual void handleProgramChange (int midiChannel,
                                      int programNumber);

    /** Sets a minimum limit on the size to which audio sub-blocks will be divided when rendering.

        When rendering, the audio blocks that are passed into renderNextBlock() will be split up
        into smaller blocks that lie between all the incoming midi messages, and it is these smaller
        sub-blocks that are rendered with multiple calls to renderVoices().

        Obviously in a pathological case where there are midi messages on every sample, then
        renderVoices() could be called once per sample and lead to poor performance, so this
        setting allows you to set a lower limit on the block size.

        The default setting is 32, which means that midi messages are accurate to about < 1ms
        accuracy, which is probably fine for most purposes, but you may want to increase or
        decrease this value for your synth.

        If shouldBeStrict is true, the audio sub-blocks will strictly never be smaller than numSamples.

        If shouldBeStrict is false (default), the first audio sub-block in the buffer is allowed
        to be smaller, to make sure that the first MIDI event in a buffer will always be sample-accurate
        (this can sometimes help to avoid quantisation or phasing issues).
    */
    void setMinimumRenderingSubdivisionSize (int numSamples, bool shouldBeStrict = false) noexcept;


protected:
    /** This is used to control access to the rendering callback and the note trigger methods. */
    juce::CriticalSection lock;
    juce::OwnedArray<SynthesiserVoice> voices;
    juce::ReferenceCountedArray<juce::SynthesiserSound> sounds;

    int lastPitchWheelValues[16];

    virtual void renderVoices(juce::AudioBuffer<float>& outputAudio,
                              int startSample, int numSamples);

    /** Searches through the voices to find one that's not currently playing, and
        which can play the given sound.

        Returns nullptr if all voices are busy and stealing isn't enabled.

        To implement a custom note-stealing algorithm, you can either override this
        method, or (preferably) override findVoiceToSteal().
    */
    virtual SynthesiserVoice* findFreeVoice (juce::SynthesiserSound* soundToPlay,
                                             int midiChannel,
                                             int midiNoteNumber,
                                             bool stealIfNoneAvailable) const;

    /** Chooses a voice that is most suitable for being re-used.
        The default method will attempt to find the oldest voice that isn't the
        bottom or top note being played. If that's not suitable for your synth,
        you can override this method and do something more cunning instead.
    */
    virtual SynthesiserVoice* findVoiceToSteal (juce::SynthesiserSound* soundToPlay,
                                                int midiChannel,
                                                int midiNoteNumber) const;

    /** Starts a specified voice playing a particular sound.
        You'll probably never need to call this, it's used internally by noteOn(), but
        may be needed by subclasses for custom behaviours.
    */
    void startVoice (SynthesiserVoice* voice,
                     juce::SynthesiserSound* sound,
                     int midiChannel,
                     int midiNoteNumber,
                     float velocity);

    /** Stops a given voice.
        You should never need to call this, it's used internally by noteOff, but is protected
        in case it's useful for some custom subclasses. It basically just calls through to
        SynthesiserVoice::stopNote(), and has some assertions to sanity-check a few things.
    */
    void stopVoice (SynthesiserVoice*, float velocity, bool allowTailOff);

    /** Can be overridden to do custom handling of incoming midi events. */
    virtual void handleMidiEvent (const juce::MidiMessage&);

private:
    juce::uint32 lastNoteOnCounter = 0;
    int minimumSubBlockSize = 32;
    double sampleRate = 44100.0;
    bool subBlockSubdivisionIsStrict = false;
    bool shouldStealNotes = true;
    juce::BigInteger sustainPedalsDown;
    juce::AudioSampleBuffer localBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Synthesiser)
}; // Synthesiser