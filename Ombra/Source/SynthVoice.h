#pragma once

#include <JuceHeader.h>
#include "WavetableOscillator.h"

//==============================================================================
//  SynthVoice
//  ----------
//  One voice of polyphony: wavetable oscillator -> per-voice resonant lowpass
//  filter -> amplitude envelope. The same ADSR also modulates the filter cutoff
//  (via "Filter Env Amount"), and a per-voice LFO adds movement to the cutoff.
//==============================================================================
class SynthVoice : public juce::SynthesiserVoice
{
public:
    explicit SynthVoice (juce::AudioProcessorValueTreeState& stateToUse);

    void setTables (const WavetableSet* tables) noexcept { osc.setTables (tables); }
    void prepare   (const juce::dsp::ProcessSpec& spec);

    bool canPlaySound (juce::SynthesiserSound* sound) override;

    void startNote (int midiNoteNumber, float velocity,
                    juce::SynthesiserSound* sound, int currentPitchWheelPosition) override;
    void stopNote  (float velocity, bool allowTailOff) override;

    void pitchWheelMoved (int newPitchWheelValue) override;
    void controllerMoved (int controllerNumber, int newControllerValue) override;

    void renderNextBlock (juce::AudioBuffer<float>& outputBuffer,
                          int startSample, int numSamples) override;

private:
    void updateModulation();

    juce::AudioProcessorValueTreeState& apvts;

    WavetableOscillator osc;
    juce::ADSR          adsr;
    juce::ADSR::Parameters adsrParams;
    juce::dsp::StateVariableTPTFilter<float> filter;

    // Cached raw-parameter pointers (atomic reads, cheap on the audio thread).
    std::atomic<float>* pWtPos      = nullptr;
    std::atomic<float>* pCutoff     = nullptr;
    std::atomic<float>* pResonance  = nullptr;
    std::atomic<float>* pFilterEnv  = nullptr;
    std::atomic<float>* pAttack     = nullptr;
    std::atomic<float>* pDecay      = nullptr;
    std::atomic<float>* pSustain    = nullptr;
    std::atomic<float>* pRelease    = nullptr;
    std::atomic<float>* pLfoRate    = nullptr;
    std::atomic<float>* pLfoDepth   = nullptr;

    double sampleRate = 44100.0;
    float  level      = 0.0f;   // velocity
    float  lfoPhase   = 0.0f;   // 0..1
    float  lastEnv    = 0.0f;   // last envelope value, for filter modulation
    int    modCounter = 0;

    static constexpr int   controlRate   = 16;   // recompute modulation every N samples
    static constexpr float filterEnvOct  = 5.0f; // max octaves of filter-env sweep
    static constexpr float lfoOct         = 2.0f; // max octaves of LFO sweep
};
