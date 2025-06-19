cmake_minimum_required(VERSION 3.16)
project(Decode VERSION 3.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Required libraries
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
find_package(OpenSSL REQUIRED)

# for MacOS
# macOS Bundle Setup
set(MACOSX_BUNDLE_ICON_FILE decode.icns)
set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.hellmark.decode")
set(MACOSX_BUNDLE_BUNDLE_NAME "Decode")
set(MACOSX_BUNDLE_COPYRIGHT "Copyright 2025 Hellmark Programming Group")
set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}")
set(MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}")

# Set icon and Info.plist if present
set(APP_ICON_MAC "${CMAKE_SOURCE_DIR}/decode.icns")
set(MY_INFO_PLIST "${CMAKE_SOURCE_DIR}/Info.plist")
if(APPLE AND EXISTS "${APP_ICON_MAC}")
    set_source_files_properties(${APP_ICON_MAC} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
endif()

# Add executable with bundle support for macOS
if(APPLE)
    set(MACOSX_BUNDLE_FLAG MACOSX_BUNDLE)
else()
    set(MACOSX_BUNDLE_FLAG "")
endif()

# Adding the required components for the build
qt_add_resources(APP_RESOURCES "resources.qrc")
add_executable(Decode ${MACOSX_BUNDLE_FLAG}
    main.cpp
    MainWindow.cpp
    MainWindow.h
    SettingsDialog.cpp
    SettingsDialog.h
    encoders/AESCodec.cpp
    encoders/AESCodec.h
    encoders/Atbash.cpp
    encoders/Atbash.h
    encoders/Base64Codec.cpp
    encoders/Base64Codec.h
    encoders/BinaryCodec.cpp
    encoders/BinaryCodec.h
    encoders/HexCodec.cpp
    encoders/HexCodec.h
    encoders/CaesarCipher.cpp
    encoders/CaesarCipher.h
    encoders/MorseCodec.cpp
    encoders/MorseCodec.h
    encoders/PigLatin.cpp
    encoders/PigLatin.h
    encoders/Rot13.cpp
    encoders/Rot13.h
    ${APP_RESOURCES}
    ${APP_ICON_MAC}
)

target_link_libraries(Decode PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets OpenSSL::Crypto)

# Finalize Qt executable (for plugin deployment, etc.)
# Apply Info.plist to bundle
if(APPLE)
    find_package(Qt6MacDeployQt QUIET)
    set_target_properties(Decode PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${MY_INFO_PLIST}"
    )
    # Try to finalize executable if tools are present
    find_package(Qt6MacDeployQt QUIET)
    if (Qt6MacDeployQt_FOUND)
        include(Qt6MacDeployQt)
        qt6_finalize_executable(Decode)
    else()
        message(WARNING "Qt6MacDeployQt not found. Manually run macdeployqt after building.")
    endif()
endif()
