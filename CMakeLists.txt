
cmake_minimum_required(VERSION 3.16)
project(Decode VERSION 1.0 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

add_executable(Decode
    main.cpp
    MainWindow.cpp
    MainWindow.h
    SettingsDialog.cpp
    SettingsDialog.h
    encoders/Atbash.cpp
    encoders/Atbash.h
    encoders/Base64Codec.cpp
    encoders/Base64Codec.h
    encoders/BinaryCodec.cpp
    encoders/BinaryCodec.h
    encoders/CaesarCipher.cpp
    encoders/CaesarCipher.h
    encoders/MorseCodec.cpp
    encoders/MorseCodec.h
    encoders/PigLatin.cpp
    encoders/PigLatin.h
    encoders/Rot13.cpp
    encoders/Rot13.h
)

target_link_libraries(Decode PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)
