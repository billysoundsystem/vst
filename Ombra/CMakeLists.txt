cmake_minimum_required(VERSION 3.22)

project(Ombra VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------------------------------------------------------------------------
# JUCE is fetched automatically the first time you configure the project.
# (The first configure downloads JUCE, so it needs an internet connection and
#  takes a few minutes. After that it is cached.)
# ---------------------------------------------------------------------------
include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.4
    GIT_SHALLOW    TRUE)
FetchContent_MakeAvailable(JUCE)

# ---------------------------------------------------------------------------
# The plugin target.
# ---------------------------------------------------------------------------
juce_add_plugin(Ombra
    PRODUCT_NAME              "Ombra"
    COMPANY_NAME              "Billysoundsystem"
    PLUGIN_MANUFACTURER_CODE  Bsys
    PLUGIN_CODE               Omb1
    FORMATS                   VST3 Standalone
    IS_SYNTH                  TRUE
    NEEDS_MIDI_INPUT          TRUE
    NEEDS_MIDI_OUTPUT         FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    VST3_CATEGORIES           "Instrument" "Synth"
    COPY_PLUGIN_AFTER_BUILD   FALSE)

juce_generate_juce_header(Ombra)

target_sources(Ombra PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/SynthVoice.cpp)

target_compile_definitions(Ombra PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(Ombra
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags)
