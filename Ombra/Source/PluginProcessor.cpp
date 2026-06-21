#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "SynthVoice.h"
#include "SynthSound.h"
#include "ParameterIDs.h"

#include <cmath>

//==============================================================================
OmbraAudioProcessor::OmbraAudioProcessor()
    : AudioProcessor (BusesProperties()
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
    wavetableSet.generate();

    synth.addSound (new SynthSound());

    for (int i = 0; i < 8; ++i)
    {
        auto* voice = new SynthVoice (apvts);
        voice->setTables (&wavetableSet);
        synth.addVoice (voice);
    }
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout
OmbraAudioProcessor::createParameterLayout()
{
    using APF   = juce::AudioParameterFloat;
    using Range = juce::NormalisableRange<float>;

    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    auto pid = [] (const char* id) { return juce::ParameterID { id, 1 }; };

    // --- Oscillator ---
    params.push_back (std::make_unique<APF> (pid (ParamID::wtPos), "Wavetable",
                                             Range (0.0f, 1.0f, 0.001f), 0.0f));

    // --- Filter ---
    Range cutoffRange (20.0f, 20000.0f, 1.0f);
    cutoffRange.setSkewForCentre (1000.0f);
    params.push_back (std::make_unique<APF> (pid (ParamID::cutoff), "Cutoff",
                                             cutoffRange, 1200.0f));
    params.push_back (std::make_unique<APF> (pid (ParamID::resonance), "Resonance",
                                             Range (0.0f, 1.0f, 0.001f), 0.2f));
    params.push_back (std::make_unique<APF> (pid (ParamID::filterEnv), "Filter Env",
                                             Range (0.0f, 1.0f, 0.001f), 0.4f));

    // --- Amplitude envelope ---
    Range timeRange (0.001f, 5.0f, 0.001f);
    timeRange.setSkewForCentre (0.4f);
    params.push_back (std::make_unique<APF> (pid (ParamID::attack),  "Attack",  timeRange, 0.01f));
    params.push_back (std::make_unique<APF> (pid (ParamID::decay),   "Decay",   timeRange, 0.30f));
    params.push_back (std::make_unique<APF> (pid (ParamID::sustain), "Sustain",
                                             Range (0.0f, 1.0f, 0.001f), 0.8f));
    params.push_back (std::make_unique<APF> (pid (ParamID::release), "Release", timeRange, 0.40f));

    // --- LFO ---
    Range lfoRateRange (0.01f, 20.0f, 0.001f);
    lfoRateRange.setSkewForCentre (2.0f);
    params.push_back (std::make_unique<APF> (pid (ParamID::lfoRate), "LFO Rate", lfoRateRange, 2.0f));
    params.push_back (std::make_unique<APF> (pid (ParamID::lfoDepth), "LFO Depth",
                                             Range (0.0f, 1.0f, 0.001f), 0.0f));

    // --- FX ---
    params.push_back (std::make_unique<APF> (pid (ParamID::drive), "Drive",
                                             Range (0.0f, 1.0f, 0.001f), 0.0f));
    params.push_back (std::make_unique<APF> (pid (ParamID::chorus), "Chorus",
                                             Range (0.0f, 1.0f, 0.001f), 0.0f));
    params.push_back (std::make_unique<APF> (pid (ParamID::delayTime), "Delay Time",
                                             Range (0.01f, 1.0f, 0.001f), 0.30f));
    params.push_back (std::make_unique<APF> (pid (ParamID::delayFb), "Delay Feedback",
                                             Range (0.0f, 0.95f, 0.001f), 0.35f));
    params.push_back (std::make_unique<APF> (pid (ParamID::delayMix), "Delay Mix",
                                             Range (0.0f, 1.0f, 0.001f), 0.0f));
    params.push_back (std::make_unique<APF> (pid (ParamID::reverbSize), "Reverb Size",
                                             Range (0.0f, 1.0f, 0.001f), 0.5f));
    params.push_back (std::make_unique<APF> (pid (ParamID::reverbMix), "Reverb Mix",
                                             Range (0.0f, 1.0f, 0.001f), 0.2f));
    params.push_back (std::make_unique<APF> (pid (ParamID::pan), "Pan",
                                             Range (-1.0f, 1.0f, 0.001f), 0.0f));

    // --- Master ---
    params.push_back (std::make_unique<APF> (pid (ParamID::master), "Master",
                                             Range (0.0f, 1.0f, 0.001f), 0.8f));

    return { params.begin(), params.end() };
}

//==============================================================================
void OmbraAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    const int numCh = juce::jmax (1, getTotalNumOutputChannels());

    synth.setCurrentPlaybackSampleRate (sampleRate);

    juce::dsp::ProcessSpec spec;
    spec.sampleRate       = sampleRate;
    spec.maximumBlockSize = (juce::uint32) samplesPerBlock;
    spec.numChannels      = (juce::uint32) numCh;

    for (int i = 0; i < synth.getNumVoices(); ++i)
        if (auto* v = dynamic_cast<SynthVoice*> (synth.getVoice (i)))
            v->prepare (spec);

    chorus.prepare (spec);
    chorus.reset();
    chorus.setRate (0.8f);
    chorus.setDepth (0.25f);
    chorus.setCentreDelay (7.0f);
    chorus.setFeedback (0.0f);

    delayLine.setMaximumDelayInSamples ((int) (sampleRate * 1.05) + 1);
    delayLine.prepare (spec);
    delayLine.reset();

    reverb.setSampleRate (sampleRate);
    reverb.reset();

    smoothedMaster.reset (sampleRate, 0.02);
    smoothedPan.reset (sampleRate, 0.02);
    smoothedDelaySamples.reset (sampleRate, 0.05);

    smoothedMaster.setCurrentAndTargetValue (apvts.getRawParameterValue (ParamID::master)->load());
    smoothedPan.setCurrentAndTargetValue (apvts.getRawParameterValue (ParamID::pan)->load());
    smoothedDelaySamples.setCurrentAndTargetValue (
        apvts.getRawParameterValue (ParamID::delayTime)->load() * (float) sampleRate);
}

bool OmbraAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::mono()
        || out == juce::AudioChannelSet::stereo();
}

//==============================================================================
void OmbraAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                        juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    const int numCh      = buffer.getNumChannels();

    buffer.clear();

    // Merge any notes from the on-screen keyboard into the MIDI stream.
    keyboardState.processNextMidiBuffer (midiMessages, 0, numSamples, true);

    // Render the synth voices.
    synth.renderNextBlock (buffer, midiMessages, 0, numSamples);

    // ---- Drive / saturation ----
    const float drive = apvts.getRawParameterValue (ParamID::drive)->load();
    if (drive > 0.0001f)
    {
        const float pre    = 1.0f + drive * 20.0f;
        const float makeup = 1.0f / std::tanh (pre);
        for (int ch = 0; ch < numCh; ++ch)
        {
            auto* d = buffer.getWritePointer (ch);
            for (int i = 0; i < numSamples; ++i)
            {
                const float in  = d[i];
                const float wet = std::tanh (in * pre) * makeup;
                d[i] = in * (1.0f - drive) + wet * drive;
            }
        }
    }

    // ---- Chorus ----
    chorus.setMix (apvts.getRawParameterValue (ParamID::chorus)->load());
    {
        juce::dsp::AudioBlock<float> block (buffer);
        juce::dsp::ProcessContextReplacing<float> ctx (block);
        chorus.process (ctx);
    }

    // ---- Delay (stereo, with feedback) ----
    const float delayMix = apvts.getRawParameterValue (ParamID::delayMix)->load();
    const float delayFb  = apvts.getRawParameterValue (ParamID::delayFb)->load();
    smoothedDelaySamples.setTargetValue (
        apvts.getRawParameterValue (ParamID::delayTime)->load() * (float) currentSampleRate);

    if (delayMix > 0.0001f || delayFb > 0.0001f)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            delayLine.setDelay (smoothedDelaySamples.getNextValue());
            for (int ch = 0; ch < numCh; ++ch)
            {
                auto* d = buffer.getWritePointer (ch);
                const float in      = d[i];
                const float delayed = delayLine.popSample (ch);
                delayLine.pushSample (ch, in + delayed * delayFb);
                d[i] = in * (1.0f - delayMix) + delayed * delayMix;
            }
        }
    }
    else
    {
        // Keep the line's history sane when bypassed.
        smoothedDelaySamples.skip (numSamples);
    }

    // ---- Reverb ----
    juce::Reverb::Parameters rp;
    const float reverbMix = apvts.getRawParameterValue (ParamID::reverbMix)->load();
    rp.roomSize   = apvts.getRawParameterValue (ParamID::reverbSize)->load();
    rp.wetLevel   = reverbMix;
    rp.dryLevel   = 1.0f - reverbMix;
    rp.width      = 1.0f;
    rp.damping    = 0.4f;
    rp.freezeMode = 0.0f;
    reverb.setParameters (rp);

    if (numCh >= 2)
        reverb.processStereo (buffer.getWritePointer (0), buffer.getWritePointer (1), numSamples);
    else if (numCh == 1)
        reverb.processMono (buffer.getWritePointer (0), numSamples);

    // ---- Pan + Master ----
    smoothedPan.setTargetValue (apvts.getRawParameterValue (ParamID::pan)->load());
    smoothedMaster.setTargetValue (apvts.getRawParameterValue (ParamID::master)->load());

    if (numCh >= 2)
    {
        auto* left  = buffer.getWritePointer (0);
        auto* right = buffer.getWritePointer (1);
        for (int i = 0; i < numSamples; ++i)
        {
            const float pan    = smoothedPan.getNextValue();
            const float master = smoothedMaster.getNextValue();
            const float angle  = (pan + 1.0f) * 0.25f * juce::MathConstants<float>::pi;
            left[i]  *= std::cos (angle) * master;
            right[i] *= std::sin (angle) * master;
        }
    }
    else if (numCh == 1)
    {
        auto* mono = buffer.getWritePointer (0);
        for (int i = 0; i < numSamples; ++i)
            mono[i] *= smoothedMaster.getNextValue();
    }
}

//==============================================================================
juce::AudioProcessorEditor* OmbraAudioProcessor::createEditor()
{
    return new OmbraAudioProcessorEditor (*this);
}

void OmbraAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void OmbraAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

//==============================================================================
// This creates the plugin instance the host loads.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new OmbraAudioProcessor();
}
