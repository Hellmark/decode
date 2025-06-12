
cmake_minimum_required(VERSION 3.16)
project(Decode VERSION 1.0 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2)

qt_add_resources(Decode "resources.qrc")

qt_add_executable(Decode
    main.cpp
    main.qml
    resources.qrc
    AppController.cpp
    AppController.h
    encoders/Base64Codec.cpp
    encoders/Base64Codec.h
    encoders/BinaryCodec.cpp
    encoders/BinaryCodec.h
    encoders/Rot13.cpp
    encoders/Rot13.h
    encoders/CaesarCipher.cpp
    encoders/CaesarCipher.h
    encoders/PigLatin.cpp
    encoders/PigLatin.h
    encoders/MorseCodec.cpp
    encoders/MorseCodec.h
    encoders/Atbash.cpp
    encoders/Atbash.h
)

target_link_libraries(Decode PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
)
