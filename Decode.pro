QT += core gui widgets

CONFIG += c++17
CONFIG += warn_on
CONFIG += resources
TEMPLATE = app
TARGET = Decode

# Source files
SOURCES += \
    main.cpp \
    MainWindow.cpp \
    SettingsDialog.cpp \
    encoders/AESCodec.cpp \
    encoders/Atbash.cpp \
    encoders/Base64Codec.cpp \
    encoders/BinaryCodec.cpp \
    encoders/HexCodec.cpp \
    encoders/CaesarCipher.cpp \
    encoders/MorseCodec.cpp \
    encoders/PigLatin.cpp \
    encoders/Rot13.cpp \
    encoders/RSACodec.cpp

# Header files
HEADERS += \
    MainWindow.h \
    SettingsDialog.h \
    encoders/AESCodec.h \
    encoders/Atbash.h \
    encoders/Base64Codec.h \
    encoders/BinaryCodec.h \
    encoders/HexCodec.h \
    encoders/CaesarCipher.h \
    encoders/MorseCodec.h \
    encoders/PigLatin.h \
    encoders/Rot13.h \
    encoders/RSACodec.h

# Resources
RESOURCES += resources.qrc

# macOS bundle setup
# if using homebrew, comment out the architecture you're not currently using
# You may also want to uncomment the libs and includepath lines if you're using homebrew
macx {
    ICON = decode.icns
    QMAKE_INFO_PLIST = Info.plist
    QMAKE_MACOSX_BUNDLE = yes
    QMAKE_BUNDLE = Decode
    QMAKE_TARGET_BUNDLE_PREFIX = com.hellmark
    QMAKE_BUNDLE_DISPLAY_NAME = Decode
    QMAKE_BUNDLE_COPYRIGHT = "Copyright 2025 Hellmark Programming Group"
    QMAKE_BUNDLE_VERSION = 3.0
    QMAKE_APPLE_DEVICE_ARCHS = x86_64 arm64
    # LIBS+= -L/usr/local/Cellar/openssl@3/3.5.0/lib
    # INCLUDEPATH+= /usr/local/Cellar/openssl@3/3.5.0/include
}

win32 {
    # Windows-specific settings, if required (e.g., icon via RC file)
    RC_FILE = decode.rc
}

# OpenSSL
LIBS += -lcrypto

# Include OpenSSL headers if needed manually (optional)
# INCLUDEPATH += /usr/local/include
# LIBS += -L/usr/local/lib
