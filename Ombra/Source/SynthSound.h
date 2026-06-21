#pragma once

#include <JuceHeader.h>

// A sound that applies to every note and channel. The actual synthesis lives in
// SynthVoice; this just tells the Synthesiser which voices can play which notes.
struct SynthSound : public juce::SynthesiserSound
{
    bool appliesToNote    (int /*midiNoteNumber*/) override { return true; }
    bool appliesToChannel (int /*midiChannel*/)    override { return true; }
};
