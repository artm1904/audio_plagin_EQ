cmake_minimum_required(VERSION 3.10)
project(play_audio VERSION 0.1.0)



juce_add_plugin(${PROJECT_NAME}
    # VERSION ...                                     # Set this if the plugin version is different to the project version
    # ICON_BIG ""   # ICON_* arguments specify a path to an image file to use as an icon for the Standalone
    #   ICON_SMALL "${CMAKE_SOURCE_DIR}/Assets/icon16.png " 
    COMPANY_NAME "play_audio" # Specify the name of the plugin's author
    COMPANY_COPYRIGHT "play_audio"
    COMPANY_WEBSITE "http://artm1904.www"
    COMPANY_EMAIL "aplay_audioartm@1904.com"
    IS_SYNTH FALSE # Is this a synth or an effect?
    NEEDS_MIDI_INPUT FALSE # Does the plugin need midi input?
    NEEDS_MIDI_OUTPUT FALSE # Does the plugin need midi output?
    # IS_MIDI_EFFECT TRUE/FALSE                 # Is this plugin a MIDI effect?
    # EDITOR_WANTS_KEYBOARD_FOCUS TRUE/FALSE    # Does the editor need keyboard focus?
    # COPY_PLUGIN_AFTER_BUILD TRUE/FALSE        # Should the plugin be installed to a default location after building?
    PLUGIN_MANUFACTURER_CODE CCCC # A four-character manufacturer id with at least one upper-case character
    PLUGIN_CODE XXXX # A unique four-character plugin id with at least one upper-case character
    FORMATS
    # The formats to build. Other valid formats are: AAX Unity VST AU AUv3
    Standalone
    AU
    # AUv3 
    VST3
    # Unity 
    ${AAX_BUILD_FLAG}
    VST3_CATEGORIES "Fx"
    AU_MAIN_TYPE "kAudioUnitType_Effect"
    # AU_SANDBOX_SAFE TRUE
    # AAX_CATEGORY ""
    # HARDENED_RUNTIME_ENABLED # macOS app settings
    # HARDENED_RUNTIME_OPTIONS
    # APP_SANDBOX_ENABLED
    # APP_SANDBOX_INHERIT
    # DESCRIPTION "" #when i set this option, the name of AAX plugin became this description(bug?)
    MICROPHONE_PERMISSION_ENABLED TRUE
    MICROPHONE_PERMISSION_TEXT "This applicaion requires a permission to use an audio input device of your computer. By Default, Built-In microphone will be used."
    PRODUCT_NAME "play_audio"
) 


target_sources(${PROJECT_NAME}
    PRIVATE
    src/PluginEditor.cpp
    src/PluginProcessor.cpp
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
    include/
)


target_compile_definitions(${PROJECT_NAME}
    PUBLIC

    JUCE_WEB_BROWSER=0 # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
    JUCE_USE_CURL=0 # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
    JUCE_VST3_CAN_REPLACE_VST2=0
)



target_link_libraries(${PROJECT_NAME}
    PRIVATE
    # AudioPluginData           # If we'd created a binary data target, we'd link to it here
    juce::juce_audio_utils
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_plugin_client
    juce::juce_audio_processors
    juce::juce_core
    juce::juce_cryptography
    juce::juce_data_structures
    juce::juce_dsp
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_opengl
    juce::juce_product_unlocking
)
