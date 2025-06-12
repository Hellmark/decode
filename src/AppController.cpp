#include "AppController.h"
#include "encoders/Base64Codec.h"
#include "encoders/Rot13.h"
#include "encoders/BinaryCodec.h"
#include "encoders/CaesarCipher.h"
#include "encoders/PigLatin.h"
#include "encoders/MorseCodec.h"
#include "encoders/Atbash.h"

AppController::AppController(QObject *parent) : QObject(parent) {}

QString AppController::encodeBase64(const QString &text) {
    return Base64Codec::encode(text);
}
QString AppController::decodeBase64(const QString &text) {
    return Base64Codec::decode(text);
}

QString AppController::encodeRot13(const QString &text) {
    return Rot13::encode(text);
}
QString AppController::decodeRot13(const QString &text) {
    return Rot13::decode(text);
}

QString AppController::encodeBinary(const QString &text) {
    return BinaryCodec::encode(text);
}
QString AppController::decodeBinary(const QString &text) {
    return BinaryCodec::decode(text);
}

QString AppController::encodeCaesar(const QString &text, int shift) {
    return CaesarCipher::encode(text, shift);
}
QString AppController::decodeCaesar(const QString &text, int shift) {
    return CaesarCipher::decode(text, shift);
}

QString AppController::encodePigLatin(const QString &text) {
    return PigLatin::encode(text);
}
QString AppController::decodePigLatin(const QString &text) {
    return PigLatin::decode(text);
}

QString AppController::encodeMorse(const QString &text) {
    return MorseCodec::encode(text);
}
QString AppController::decodeMorse(const QString &text) {
    return MorseCodec::decode(text);
}

QString AppController::encodeAtbash(const QString &text) {
    return Atbash::encode(text);
}
QString AppController::decodeAtbash(const QString &text) {
    return Atbash::decode(text);
}
