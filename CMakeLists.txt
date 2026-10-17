cmake_minimum_required(VERSION 3.22)
project(BlissBox VERSION 1.0.0 LANGUAGES C CXX)

include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 29396c22c93392d6738e021b83196283d6e4d850) # JUCE 8.0.12
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/JUCE/CMakeLists.txt")
    set(FETCHCONTENT_SOURCE_DIR_JUCE "${CMAKE_CURRENT_SOURCE_DIR}/external/JUCE")
endif()
FetchContent_MakeAvailable(JUCE)

if(WIN32)
    if(NOT JUCE_WEBVIEW2_PACKAGE_LOCATION)
        set(JUCE_WEBVIEW2_PACKAGE_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/external" CACHE PATH "WebView2 SDK packages" FORCE)
    endif()
    if(NOT EXISTS "${JUCE_WEBVIEW2_PACKAGE_LOCATION}/Microsoft.Web.WebView2.1.0.3485.44/build/native/include/WebView2.h")
        file(DOWNLOAD "https://www.nuget.org/api/v2/package/Microsoft.Web.WebView2/1.0.3485.44"
            "${CMAKE_CURRENT_BINARY_DIR}/webview2.zip"
            EXPECTED_HASH SHA256=bc09150b179246ac90189649b13be8e6b11b3ac200e817e18df106e1f3cf489e
            TLS_VERIFY ON)
        file(ARCHIVE_EXTRACT INPUT "${CMAKE_CURRENT_BINARY_DIR}/webview2.zip"
            DESTINATION "${JUCE_WEBVIEW2_PACKAGE_LOCATION}/Microsoft.Web.WebView2.1.0.3485.44")
    endif()
endif()

juce_add_plugin(BlissBox
    COMPANY_NAME "BlissBox"
    PLUGIN_MANUFACTURER_CODE Manu
    PLUGIN_CODE Evy0
    FORMATS VST3 Standalone
    PRODUCT_NAME "BlissBox"
    NEEDS_WEB_BROWSER TRUE
    NEEDS_WEBVIEW2 TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE)
juce_generate_juce_header(BlissBox)

file(GLOB BLISSBOX_IMAGES CONFIGURE_DEPENDS "Design/Images/*.png")
juce_add_binary_data(BlissBoxImages SOURCES ${BLISSBOX_IMAGES}
    WebUI/index.html WebUI/app.js WebUI/style.css WebUI/faceplate.svg)
target_sources(BlissBox PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/HybridEditor.cpp
    Source/BlissBoxSlider.cpp)
target_compile_features(BlissBox PUBLIC cxx_std_17)
target_compile_definitions(BlissBox PUBLIC
    JUCE_WEB_BROWSER=1
    JUCE_USE_WIN_WEBVIEW2_WITH_STATIC_LINKING=1
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(BlissBox PRIVATE
    BlissBoxImages
    juce::juce_audio_utils
    juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags)

option(BLISSBOX_BUILD_TESTS "Build the real WebView2 integration test" OFF)
if(BLISSBOX_BUILD_TESTS)
    juce_add_gui_app(BlissBoxBrowserTest PRODUCT_NAME "BlissBox Browser Test")
    target_sources(BlissBoxBrowserTest PRIVATE Tests/BrowserIntegration.cpp)
    target_include_directories(BlissBoxBrowserTest PRIVATE Source $<TARGET_PROPERTY:BlissBox,INCLUDE_DIRECTORIES>)
    target_link_libraries(BlissBoxBrowserTest PRIVATE BlissBox)
endif()
