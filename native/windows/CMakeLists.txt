cmake_minimum_required(VERSION 3.24)

project(YWDPlugNative VERSION 0.1.0 LANGUAGES CXX)

if(NOT WIN32)
    message(FATAL_ERROR "YWD-Plug Native currently targets Windows only.")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.8 REQUIRED COMPONENTS
    Core
    Gui
    Quick
    QuickControls2
    Concurrent
)

qt_standard_project_setup(REQUIRES 6.8)

# Preserve the user-supplied ICO exactly as-is for Qt/runtime use, but normalize
# a build-only copy to classic DIB frames for older Windows SDK rc.exe versions
# that reject PNG-compressed ICO frames with RC2176.
set(YWD_PLUG_RC_ICON "${CMAKE_CURRENT_BINARY_DIR}/ywd-plug-win-rc.ico")
execute_process(
    COMMAND powershell.exe
        -NoProfile
        -ExecutionPolicy Bypass
        -File "${CMAKE_CURRENT_SOURCE_DIR}/tools/Normalize-WindowsIcon.ps1"
        -InputPath "${CMAKE_CURRENT_SOURCE_DIR}/resources/ywd-plug-win.ico"
        -OutputPath "${YWD_PLUG_RC_ICON}"
    RESULT_VARIABLE YWD_PLUG_ICON_NORMALIZE_RESULT
    OUTPUT_VARIABLE YWD_PLUG_ICON_NORMALIZE_STDOUT
    ERROR_VARIABLE YWD_PLUG_ICON_NORMALIZE_STDERR
)

if(NOT YWD_PLUG_ICON_NORMALIZE_RESULT EQUAL 0 OR NOT EXISTS "${YWD_PLUG_RC_ICON}")
    message(FATAL_ERROR
        "Failed to generate RC-compatible Windows icon.\n"
        "stdout: ${YWD_PLUG_ICON_NORMALIZE_STDOUT}\n"
        "stderr: ${YWD_PLUG_ICON_NORMALIZE_STDERR}")
endif()

message(STATUS "Generated RC-compatible Windows icon: ${YWD_PLUG_RC_ICON}")

file(TO_NATIVE_PATH "${YWD_PLUG_RC_ICON}" YWD_PLUG_ICON_PATH)
string(REPLACE "\\" "\\\\" YWD_PLUG_ICON_PATH "${YWD_PLUG_ICON_PATH}")
configure_file(
    resources/ywd-plug-win.rc
    ${CMAKE_CURRENT_BINARY_DIR}/ywd-plug-win.rc
    @ONLY
)

qt_add_executable(ywd_plug WIN32
    ${CMAKE_CURRENT_BINARY_DIR}/ywd-plug-win.rc
    src/main.cpp
    src/app/AppController.cpp
    src/app/AppController.h
    src/app/WindowsChrome.cpp
    src/app/WindowsChrome.h
    src/app/WindowStateManager.cpp
    src/app/WindowStateManager.h
    src/serial/WinSerialPort.cpp
    src/serial/WinSerialPort.h
    src/radios/dm32uv/DM32Connection.cpp
    src/radios/dm32uv/DM32Connection.h
    src/radios/dm32uv/DM32SelectiveRead.cpp
    src/radios/dm32uv/DM32MemoryBlock.h
    src/radios/dm32uv/DM32ContactBlock.h
    src/radios/dm32uv/DM32ChannelDecoder.cpp
    src/radios/dm32uv/DM32ChannelDecoder.h
    src/radios/dm32uv/DM32CodeplugDecoder.cpp
    src/radios/dm32uv/DM32CodeplugDecoder.h
    src/radios/dm32uv/DM32ContactDecoder.cpp
    src/radios/dm32uv/DM32ContactDecoder.h
    src/radios/dm32uv/DM32Constants.h
)

qt_add_qml_module(ywd_plug
    URI YWDPlug
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/SplashWindow.qml
        qml/ConnectionPage.qml
        qml/ChannelsPage.qml
        qml/ZonesPage.qml
        qml/ScanListsPage.qml
        qml/ContactsPage.qml
        qml/RXGroupsPage.qml
        qml/components/NavButton.qml
        qml/components/StatusPill.qml
    RESOURCES
        resources/ywd-plug.svg
)

qt_add_resources(ywd_plug "branding_resources"
    PREFIX "/branding"
    FILES
        resources/ywd-plug-win.ico
        resources/ywd-plug-win-logo1.png
)

target_include_directories(ywd_plug PRIVATE src)

target_link_libraries(ywd_plug PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Concurrent
    dwmapi
)

set_target_properties(ywd_plug PROPERTIES
    OUTPUT_NAME "YWD-Plug"
    WIN32_EXECUTABLE TRUE
)

if(MSVC)
    target_compile_options(ywd_plug PRIVATE /W4 /permissive- /Zc:__cplusplus)
    target_compile_definitions(ywd_plug PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX WIN32_LEAN_AND_MEAN)
endif()

install(TARGETS ywd_plug
    BUNDLE DESTINATION .
    RUNTIME DESTINATION .
)
