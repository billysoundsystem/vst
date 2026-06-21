#pragma once

#include <array>
#include <cmath>

//==============================================================================
//  WavetableSet
//  ------------
//  Generates a small bank of single-cycle waveforms (frames) using additive
//  synthesis. The oscillator morphs continuously between adjacent frames,
//  giving the "wavetable" character. Harmonic counts are limited per frame to
//  keep aliasing reasonable for a v1.
//==============================================================================
struct WavetableSet
{
    static constexpr int tableSize = 2048;
    static constexpr int numFrames = 4;

    // +1 guard sample at the end of each frame for safe linear interpolation.
    std::array<std::array<float, tableSize + 1>, numFrames> frames {};

    void generate()
    {
        constexpr double twoPi = 6.283185307179586476925286766559;

        // Frame 0: pure sine (1 harmonic)
        buildFrame (0, [] (int n) -> float
        {
            return (n == 1) ? 1.0f : 0.0f;
        }, 1);

        // Frame 1: soft / triangle-like (odd harmonics, 1/n^2, alternating sign)
        buildFrame (1, [] (int n) -> float
        {
            if (n % 2 == 0) return 0.0f;
            const float sign = (((n - 1) / 2) % 2 == 0) ? 1.0f : -1.0f;
            return sign / float (n * n);
        }, 31);

        // Frame 2: sawtooth (all harmonics, 1/n)
        buildFrame (2, [] (int n) -> float
        {
            return 1.0f / float (n);
        }, 48);

        // Frame 3: square (odd harmonics, 1/n)
        buildFrame (3, [] (int n) -> float
        {
            return (n % 2 == 1) ? (1.0f / float (n)) : 0.0f;
        }, 48);
    }

private:
    template <typename HarmonicFn>
    void buildFrame (int frameIndex, HarmonicFn amplitudeForHarmonic, int maxHarmonic)
    {
        constexpr double twoPi = 6.283185307179586476925286766559;
        auto& frame = frames[(size_t) frameIndex];

        float peak = 0.0f;

        for (int i = 0; i < tableSize; ++i)
        {
            const double phase = (double) i / (double) tableSize;
            double value = 0.0;

            for (int n = 1; n <= maxHarmonic; ++n)
            {
                const float amp = amplitudeForHarmonic (n);
                if (amp != 0.0f)
                    value += (double) amp * std::sin (twoPi * (double) n * phase);
            }

            frame[(size_t) i] = (float) value;
            peak = std::max (peak, std::abs (frame[(size_t) i]));
        }

        // Normalise to peak 1.0 so all frames have matching loudness.
        if (peak > 0.0f)
            for (int i = 0; i < tableSize; ++i)
                frame[(size_t) i] /= peak;

        // Guard sample = first sample (table wraps).
        frame[(size_t) tableSize] = frame[0];
    }
};

//==============================================================================
//  WavetableOscillator
//  --------------------
//  A single oscillator that reads from a shared WavetableSet and morphs between
//  frames according to a 0..1 "position".
//==============================================================================
class WavetableOscillator
{
public:
    void setTables (const WavetableSet* tablesToUse) noexcept { tables = tablesToUse; }
    void prepare (double sampleRate)          noexcept { sr = sampleRate; }
    void resetPhase ()                         noexcept { phase = 0.0f; }

    void setFrequency (float freqHz) noexcept
    {
        increment = freqHz * (float) WavetableSet::tableSize / (float) sr;
    }

    // position: 0..1, morphs across the frames.
    float getNextSample (float position) noexcept
    {
        if (tables == nullptr)
            return 0.0f;

        position = juceClamp (position, 0.0f, 1.0f);

        const float framePos = position * (float) (WavetableSet::numFrames - 1);
        int   f0 = (int) framePos;
        if (f0 > WavetableSet::numFrames - 1) f0 = WavetableSet::numFrames - 1;
        if (f0 < 0)                            f0 = 0;
        int   f1 = (f0 + 1 < WavetableSet::numFrames) ? f0 + 1 : f0;
        const float frameFrac = framePos - (float) f0;

        const int   idx  = (int) phase;
        const float frac = phase - (float) idx;

        const auto& frame0 = tables->frames[(size_t) f0];
        const auto& frame1 = tables->frames[(size_t) f1];

        const float s0 = frame0[(size_t) idx] * (1.0f - frac) + frame0[(size_t) (idx + 1)] * frac;
        const float s1 = frame1[(size_t) idx] * (1.0f - frac) + frame1[(size_t) (idx + 1)] * frac;

        const float out = s0 * (1.0f - frameFrac) + s1 * frameFrac;

        phase += increment;
        if (phase >= (float) WavetableSet::tableSize)
            phase -= (float) WavetableSet::tableSize;

        return out;
    }

private:
    static float juceClamp (float v, float lo, float hi) noexcept
    {
        return v < lo ? lo : (v > hi ? hi : v);
    }

    const WavetableSet* tables = nullptr;
    double sr        = 44100.0;
    float  phase     = 0.0f;   // 0 .. tableSize
    float  increment = 0.0f;
};
