#include "PluginEditor.h"
#include "ParameterIDs.h"

OmbraAudioProcessorEditor::OmbraAudioProcessorEditor (OmbraAudioProcessor& p)
    : juce::AudioProcessorEditor (&p),
      processor (p),
      keyboard (p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setLookAndFeel (&lnf);

    auto makeGroup = [this] (const juce::String& title) -> Group&
    {
        auto group = std::make_unique<Group>();
        group->title = title;
        group->header.setText (title, juce::dontSendNotification);
        group->header.setFont (juce::Font (juce::FontOptions (13.0f, juce::Font::bold)));
        group->header.setColour (juce::Label::textColourId, juce::Colour (0xffc8743a));
        addAndMakeVisible (group->header);

        auto& ref = *group;
        groups.push_back (std::move (group));
        return ref;
    };

    auto& osc = makeGroup ("OSC");
    addKnob (osc, ParamID::wtPos, "Wave");

    auto& flt = makeGroup ("FILTER");
    addKnob (flt, ParamID::cutoff,    "Cutoff");
    addKnob (flt, ParamID::resonance, "Reso");
    addKnob (flt, ParamID::filterEnv, "Env");

    auto& env = makeGroup ("ENVELOPE");
    addKnob (env, ParamID::attack,  "A");
    addKnob (env, ParamID::decay,   "D");
    addKnob (env, ParamID::sustain, "S");
    addKnob (env, ParamID::release, "R");

    auto& lfo = makeGroup ("LFO");
    addKnob (lfo, ParamID::lfoRate,  "Rate");
    addKnob (lfo, ParamID::lfoDepth, "Depth");

    auto& fx = makeGroup ("FX");
    addKnob (fx, ParamID::drive,      "Drive");
    addKnob (fx, ParamID::chorus,     "Chorus");
    addKnob (fx, ParamID::delayTime,  "Dly Time");
    addKnob (fx, ParamID::delayFb,    "Dly Fbk");
    addKnob (fx, ParamID::delayMix,   "Dly Mix");
    addKnob (fx, ParamID::reverbSize, "Rev Size");
    addKnob (fx, ParamID::reverbMix,  "Rev Mix");
    addKnob (fx, ParamID::pan,        "Pan");

    auto& master = makeGroup ("MASTER");
    addKnob (master, ParamID::master, "Volume");

    addAndMakeVisible (keyboard);
    keyboard.setLowestVisibleKey (36); // C2

    setSize (860, 760);
}

OmbraAudioProcessorEditor::~OmbraAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

OmbraAudioProcessorEditor::Knob&
OmbraAudioProcessorEditor::addKnob (Group& group, const juce::String& paramID, const juce::String& name)
{
    auto knob = std::make_unique<Knob>();

    knob->slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    knob->slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 74, 14);
    addAndMakeVisible (knob->slider);

    knob->label.setText (name, juce::dontSendNotification);
    knob->label.setJustificationType (juce::Justification::centred);
    knob->label.setFont (juce::Font (juce::FontOptions (12.0f)));
    addAndMakeVisible (knob->label);

    knob->attachment = std::make_unique<SliderAttachment> (processor.apvts, paramID, knob->slider);

    auto& ref = *knob;
    group.knobs.push_back (std::move (knob));
    return ref;
}

void OmbraAudioProcessorEditor::paint (juce::Graphics& g)
{
    juce::ColourGradient bg (juce::Colour (0xff1a1714), 0.0f, 0.0f,
                             juce::Colour (0xff0e0c0a), 0.0f, (float) getHeight(), false);
    g.setGradientFill (bg);
    g.fillAll();

    g.setColour (juce::Colour (0xffe8e2d6));
    g.setFont (juce::Font (juce::FontOptions (26.0f, juce::Font::bold)));
    g.drawText ("OMBRA", 16, 10, 400, 30, juce::Justification::left);

    g.setColour (juce::Colour (0xff8a847a));
    g.setFont (juce::Font (juce::FontOptions (12.0f)));
    g.drawText ("wavetable synth", 18, 38, 400, 16, juce::Justification::left);
}

void OmbraAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (12);

    auto keyboardArea = area.removeFromBottom (80);
    keyboard.setBounds (keyboardArea);
    area.removeFromBottom (8);

    area.removeFromTop (44); // space for the title drawn in paint()

    const int knobW   = 84;
    const int knobH   = 64;
    const int labelH  = 14;
    const int headerH = 18;
    const int gap     = 8;

    const int left  = area.getX();
    const int right = area.getRight();
    int y = area.getY();

    for (auto& group : groups)
    {
        group->header.setBounds (left, y, area.getWidth(), headerH);
        y += headerH + 2;

        int x = left;
        int rowBottom = y;

        for (auto& knob : group->knobs)
        {
            if (x + knobW > right)
            {
                x = left;
                y = rowBottom + gap;
            }

            knob->label.setBounds (x, y, knobW, labelH);
            knob->slider.setBounds (x, y + labelH, knobW, knobH);

            rowBottom = juce::jmax (rowBottom, y + labelH + knobH);
            x += knobW;
        }

        y = rowBottom + gap * 2;
    }
}
