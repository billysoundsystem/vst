#pragma once

#include <JuceHeader.h>

// A dark, cinematic look for the rotary controls.
class OmbraLookAndFeel : public juce::LookAndFeel_V4
{
public:
    OmbraLookAndFeel()
    {
        setColour (juce::Slider::textBoxTextColourId,    juce::Colour (0xffd8d2c8));
        setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour (juce::Label::textColourId,            juce::Colour (0xffb8b2a6));
    }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider&) override
    {
        const auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (4.0f);
        const auto radius  = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
        const auto centre  = bounds.getCentre();
        const auto angle   = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        const auto lineW   = juce::jmax (2.0f, radius * 0.12f);
        const auto arcR    = radius - lineW * 0.5f;

        // Track
        juce::Path track;
        track.addCentredArc (centre.x, centre.y, arcR, arcR, 0.0f,
                             rotaryStartAngle, rotaryEndAngle, true);
        g.setColour (juce::Colour (0xff2a2622));
        g.strokePath (track, juce::PathStrokeType (lineW, juce::PathStrokeType::curved,
                                                   juce::PathStrokeType::rounded));

        // Value arc
        juce::Path value;
        value.addCentredArc (centre.x, centre.y, arcR, arcR, 0.0f,
                             rotaryStartAngle, angle, true);
        g.setColour (juce::Colour (0xffc8743a)); // warm amber
        g.strokePath (value, juce::PathStrokeType (lineW, juce::PathStrokeType::curved,
                                                   juce::PathStrokeType::rounded));

        // Knob body
        const float bodyR = arcR - lineW;
        g.setColour (juce::Colour (0xff1c1a18));
        g.fillEllipse (centre.x - bodyR, centre.y - bodyR, bodyR * 2.0f, bodyR * 2.0f);
        g.setColour (juce::Colour (0xff3a3530));
        g.drawEllipse (centre.x - bodyR, centre.y - bodyR, bodyR * 2.0f, bodyR * 2.0f, 1.0f);

        // Pointer
        juce::Path pointer;
        const float pw = juce::jmax (2.0f, bodyR * 0.14f);
        pointer.addRoundedRectangle (-pw * 0.5f, -bodyR + 2.0f, pw, bodyR * 0.55f, pw * 0.5f);
        pointer.applyTransform (juce::AffineTransform::rotation (angle).translated (centre));
        g.setColour (juce::Colour (0xffe8e2d6));
        g.fillPath (pointer);
    }
};
