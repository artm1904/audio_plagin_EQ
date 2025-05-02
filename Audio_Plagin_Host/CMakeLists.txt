cmake_minimum_required(VERSION 3.22)

project(AudioPluginHost VERSION 0.0.1)

#add_subdirectory(JUCE)  
set(JUCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../contrib/juce)

juce_add_gui_app(${PROJECT_NAME}
    BUNDLE_ID com.juce.pluginhost
    ICON_BIG "${CMAKE_CURRENT_SOURCE_DIR}/Source/JUCEAppIcon.png"
    MICROPHONE_PERMISSION_ENABLED TRUE
    PLUGINHOST_AU TRUE)

juce_generate_juce_header(AudioPluginHost)

target_sources(${PROJECT_NAME} PRIVATE
    Source/HostStartup.cpp
    Source/Plugins/ARAPlugin.cpp
    Source/Plugins/IOConfigurationWindow.cpp
    Source/Plugins/InternalPlugins.cpp
    Source/Plugins/PluginGraph.cpp
    Source/UI/GraphEditorPanel.cpp
    Source/UI/MainHostWindow.cpp)

juce_add_binary_data(AudioPluginHostData SOURCES
    ${JUCE_DIR}/examples/Assets/cassette_recorder.wav
    ${JUCE_DIR}/examples/Assets/cello.wav
    ${JUCE_DIR}/examples/Assets/guitar_amp.wav
    ${JUCE_DIR}/examples/Assets/proaudio.path
    ${JUCE_DIR}/examples/Assets/reverb_ir.wav
    ${JUCE_DIR}/examples/Assets/singing.ogg)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    JUCE_ALSA=1
    JUCE_CONTENT_SHARING=1
    JUCE_DIRECTSOUND=1
    JUCE_DISABLE_CAUTIOUS_PARAMETER_ID_CHECKING=1
    JUCE_PLUGINHOST_LADSPA=1
    JUCE_PLUGINHOST_LV2=1
    JUCE_PLUGINHOST_VST3=1
    JUCE_PLUGINHOST_VST=0
    JUCE_PLUGINHOST_ARA=0
    JUCE_USE_CAMERA=0
    JUCE_USE_CDBURNER=0
    JUCE_USE_CDREADER=0
    JUCE_USE_CURL=0
    JUCE_USE_FLAC=0
    JUCE_USE_OGGVORBIS=1
    JUCE_VST3_HOST_CROSS_PLATFORM_UID=1
    JUCE_WASAPI=1
    JUCE_WEB_BROWSER=0
    PIP_JUCE_EXAMPLES_DIRECTORY_STRING="contrib/juce/examples"
    # This is a temporary workaround to allow builds to complete on Xcode 15.
    # Add -Wl,-ld_classic to the OTHER_LDFLAGS build setting if you need to
    # deploy to older versions of macOS/iOS.
    JUCE_SILENCE_XCODE_15_LINKER_WARNING=1)

target_link_libraries(${PROJECT_NAME} PRIVATE
    AudioPluginHostData
    juce::juce_audio_utils
    juce::juce_cryptography
    juce::juce_dsp
    juce::juce_opengl
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)

juce_add_bundle_resources_directory(${PROJECT_NAME} ${JUCE_DIR}/examples/Assets)
