cmake_minimum_required(VERSION 3.24)

project(YWDPlugNative VERSION 0.1.0 LANGUAGES CXX)

if(NOT WIN32)
    message(FATAL_ERROR "YWD-Plug Native currently targets Windows only.")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.8 REQUIRED COMPONENTS
    Core
    Gui
    Quick
    QuickControls2
    Concurrent
)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(ywd_plug WIN32
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
