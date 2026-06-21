#include "SynthVoice.h"
#include "SynthSound.h"
#include "ParameterIDs.h"

SynthVoice::SynthVoice (juce::AudioProcessorValueTreeState& stateToUse)
    : apvts (stateToUse)
{
    pWtPos     = apvts.getRawParameterValue (ParamID::wtPos);
    pCutoff    = apvts.getRawParameterValue (ParamID::cutoff);
    pResonance = apvts.getRawParameterValue (ParamID::resonance);
    pFilterEnv = apvts.getRawParameterValue (ParamID::filterEnv);
    pAttack    = apvts.getRawParameterValue (ParamID::attack);
    pDecay     = apvts.getRawParameterValue (ParamID::decay);
    pSustain   = apvts.getRawParameterValue (ParamID::sustain);
    pRelease   = apvts.getRawParameterValue (ParamID::release);
    pLfoRate   = apvts.getRawParameterValue (ParamID::lfoRate);
    pLfoDepth  = apvts.getRawParameterValue (ParamID::lfoDepth);
}

void SynthVoice::prepare (const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    osc.prepare (spec.sampleRate);
    adsr.setSampleRate (spec.sampleRate);

    juce::dsp::ProcessSpec monoSpec = spec;
    monoSpec.numChannels = 1;
    filter.prepare (monoSpec);
    filter.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
}

bool SynthVoice::canPlaySound (juce::SynthesiserSound* sound)
{
    return dynamic_cast<SynthSound*> (sound) != nullptr;
}

void SynthVoice::startNote (int midiNoteNumber, float velocity,
                            juce::SynthesiserSound*, int /*pitchWheel*/)
{
    level = velocity;

    osc.resetPhase();
    osc.setFrequency ((float) juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber));

    lfoPhase   = 0.0f;
    lastEnv    = 0.0f;
    modCounter = 0;

    filter.reset();

    adsrParams.attack  = pAttack->load();
    adsrParams.decay   = pDecay->load();
    adsrParams.sustain = pSustain->load();
    adsrParams.release = pRelease->load();
    adsr.setParameters (adsrParams);
    adsr.noteOn();
}

void SynthVoice::stopNote (float /*velocity*/, bool allowTailOff)
{
    if (allowTailOff)
    {
        adsr.noteOff();
    }
    else
    {
        adsr.reset();
        clearCurrentNote();
    }
}

void SynthVoice::pitchWheelMoved (int) {}
void SynthVoice::controllerMoved (int, int) {}

void SynthVoice::updateModulation()
{
    // Keep the envelope shape current (cheap, lets users tweak ADSR live).
    adsrParams.attack  = pAttack->load();
    adsrParams.decay   = pDecay->load();
    adsrParams.sustain = pSustain->load();
    adsrParams.release = pRelease->load();
    adsr.setParameters (adsrParams);

    const float baseCutoff = pCutoff->load();
    const float fenvAmt    = pFilterEnv->load();
    const float lfoDepth   = pLfoDepth->load();

    const float lfoValue = std::sin (lfoPhase * juce::MathConstants<float>::twoPi);

    const float octaves = fenvAmt * lastEnv * filterEnvOct
                        + lfoDepth * lfoValue * lfoOct;

    float modCutoff = baseCutoff * std::pow (2.0f, octaves);
    modCutoff = juce::jlimit (20.0f, 20000.0f, modCutoff);

    // Map 0..1 resonance to a musical Q range.
    const float q = juce::jmap (pResonance->load(), 0.0f, 1.0f, 0.5f, 10.0f);

    filter.setCutoffFrequency (modCutoff);
    filter.setResonance (q);

    // Advance the LFO by the control-rate block.
    const float lfoRate = pLfoRate->load();
    lfoPhase += lfoRate * (float) controlRate / (float) sampleRate;
    while (lfoPhase >= 1.0f) lfoPhase -= 1.0f;
}

void SynthVoice::renderNextBlock (juce::AudioBuffer<float>& outputBuffer,
                                  int startSample, int numSamples)
{
    if (! adsr.isActive())
        return;

    const float wtPos       = pWtPos->load();
    const int   numChannels = outputBuffer.getNumChannels();

    for (int i = 0; i < numSamples; ++i)
    {
        if (modCounter == 0)
            updateModulation();

        if (++modCounter >= controlRate)
            modCounter = 0;

        const float oscOut = osc.getNextSample (wtPos);
        const float env    = adsr.getNextSample();
        lastEnv = env;

        float sample = oscOut * env * level;
        sample = filter.processSample (0, sample);

        const int dest = startSample + i;
        for (int ch = 0; ch < numChannels; ++ch)
            outputBuffer.addSample (ch, dest, sample);
    }

    if (! adsr.isActive())
        clearCurrentNote();
}
