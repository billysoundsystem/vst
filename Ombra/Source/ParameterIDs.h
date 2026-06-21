#pragma once

// Parameter identifiers, kept in one place so the processor, voices and editor
// always agree on the strings.
namespace ParamID
{
    inline constexpr auto wtPos       = "WT_POS";
    inline constexpr auto cutoff      = "CUTOFF";
    inline constexpr auto resonance   = "RESO";
    inline constexpr auto filterEnv   = "FENV";

    inline constexpr auto attack      = "ATTACK";
    inline constexpr auto decay       = "DECAY";
    inline constexpr auto sustain     = "SUSTAIN";
    inline constexpr auto release     = "RELEASE";

    inline constexpr auto lfoRate     = "LFO_RATE";
    inline constexpr auto lfoDepth    = "LFO_DEPTH";

    inline constexpr auto drive       = "DRIVE";
    inline constexpr auto chorus      = "CHORUS";
    inline constexpr auto delayTime   = "DELAY_TIME";
    inline constexpr auto delayFb     = "DELAY_FB";
    inline constexpr auto delayMix    = "DELAY_MIX";
    inline constexpr auto reverbSize  = "REVERB_SIZE";
    inline constexpr auto reverbMix   = "REVERB_MIX";
    inline constexpr auto pan         = "PAN";

    inline constexpr auto master      = "MASTER";
}
