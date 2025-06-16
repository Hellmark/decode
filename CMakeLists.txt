cmake_minimum_required(VERSION 3.16)
project(Decode VERSION 3.0 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
find_package(OpenSSL REQUIRED)
qt_add_resources(APP_RESOURCES "resources.qrc")


add_executable(Decode
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
)

target_link_libraries(Decode PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets OpenSSL::Crypto)
