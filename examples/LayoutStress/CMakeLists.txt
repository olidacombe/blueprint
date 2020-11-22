cmake_minimum_required(VERSION 3.15)
set(PROJECT_NAME "LayoutStress")
project(${PROJECT_NAME} VERSION 0.1.0)

add_subdirectory(juce)
juce_add_module(blueprint)

juce_add_gui_app(${PROJECT_NAME})

juce_generate_juce_header(${PROJECT_NAME})

file(GLOB SRCS *.cpp)
target_sources(${PROJECT_NAME} PRIVATE ${SRCS})

target_compile_definitions(${PROJECT_NAME}
    PUBLIC
    JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
    JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
    JUCE_DEBUG=1
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
    juce::juce_core
    juce::juce_data_structures
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    blueprint

    # TODO not strictly require these
    # for non-plugins?
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_processors
    )
