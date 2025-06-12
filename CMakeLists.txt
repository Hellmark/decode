
cmake_minimum_required(VERSION 3.16)
project(TextTransformerApp VERSION 1.0 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2)

qt_add_resources(TextTransformerApp "resources.qrc")

set(SOURCES
    main.cpp
    AppController.cpp
    encoders/Base64Codec.cpp
    encoders/BinaryCodec.cpp
    encoders/Rot13.cpp
    encoders/CaesarCipher.cpp
    encoders/PigLatin.cpp
    encoders/MorseCodec.cpp
    encoders/Atbash.cpp
)

set(HEADERS
    AppController.h
    encoders/Base64Codec.h
    encoders/BinaryCodec.h
    encoders/Rot13.h
    encoders/CaesarCipher.h
    encoders/PigLatin.h
    encoders/MorseCodec.h
    encoders/Atbash.h
)

qt_add_executable(TextTransformerApp
    ${SOURCES}
    ${HEADERS}
)

target_link_libraries(TextTransformerApp PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
)
