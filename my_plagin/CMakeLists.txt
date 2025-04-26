cmake_minimum_required(VERSION 3.10)
project(eq_plagin VERSION 0.1.0)


# Пути к библиотекам GTK и сами библиотеки
link_directories(/usr/lib/x86_64-linux-gnu)
link_libraries(libgtk-3.so)
#Дополнительные библиотки, которые необходимы GTK:
link_libraries(gobject-2.0 gio-2.0)


juce_add_plugin(${PROJECT_NAME}
    COMPANY_NAME artm1904
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    PLUGIN_MANUFACTURER_CODE WLSD
    PLUGIN_CODE AUPL
    FORMATS VST3
    PRODUCT_NAME "eq_plagin"
)

target_sources(${PROJECT_NAME}
    PRIVATE
    source/PluginEditor.cpp
    source/PluginProcessor.cpp
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
    include/
)


target_compile_definitions(${PROJECT_NAME}
    PUBLIC

    JUCE_WEB_BROWSER=0 # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
    JUCE_USE_CURL=0 # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
    JUCE_VST3_CAN_REPLACE_VST2=0)


target_link_libraries(${PROJECT_NAME}
    PRIVATE
    # AudioPluginData           # If we'd created a binary data target, we'd link to it here
    juce::juce_audio_utils
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)
