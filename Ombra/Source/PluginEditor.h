#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "OmbraLookAndFeel.h"

class OmbraAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit OmbraAudioProcessorEditor (OmbraAudioProcessor&);
    ~OmbraAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    struct Knob
    {
        juce::Slider slider;
        juce::Label  label;
        std::unique_ptr<SliderAttachment> attachment;
    };

    // One labelled group of knobs (e.g. "FILTER" with Cutoff / Reso / Env).
    struct Group
    {
        juce::String title;
        juce::Label  header;
        std::vector<std::unique_ptr<Knob>> knobs;
    };

    Knob& addKnob (Group& group, const juce::String& paramID, const juce::String& name);

    OmbraAudioProcessor& processor;
    OmbraLookAndFeel     lnf;

    std::vector<std::unique_ptr<Group>> groups;
    juce::MidiKeyboardComponent keyboard;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OmbraAudioProcessorEditor)
};
