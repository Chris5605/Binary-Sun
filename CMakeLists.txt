cmake_minimum_required(VERSION 3.22)

project(BinarySunDistortion VERSION 0.1.0)

set(JUCE_DIR "" CACHE PATH "Path to the JUCE source directory")

if(NOT TARGET juce::juce_audio_utils)
    if(JUCE_DIR AND EXISTS "${JUCE_DIR}/CMakeLists.txt")
        add_subdirectory("${JUCE_DIR}" "${CMAKE_BINARY_DIR}/JUCE")
    else()
        find_package(JUCE CONFIG QUIET)
    endif()
endif()

if(NOT TARGET juce::juce_audio_utils)
    message(FATAL_ERROR
        "JUCE was not found. Configure with -DJUCE_DIR=<path to the JUCE source directory>.")
endif()

juce_add_plugin(BinarySunDistortion
    COMPANY_NAME "Binary Sun"
    BUNDLE_ID "com.binarysun.distortion"
    PLUGIN_MANUFACTURER_CODE BSun
    PLUGIN_CODE Bsun
    FORMATS VST3
    PRODUCT_NAME "Binary Sun Distortion"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE)

target_sources(BinarySunDistortion
    PRIVATE
        Source/PluginEditor.cpp
        Source/PluginProcessor.cpp)

target_compile_definitions(BinarySunDistortion
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(BinarySunDistortion
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
