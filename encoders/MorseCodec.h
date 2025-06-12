#ifndef MORSECODEC_H
#define MORSECODEC_H

#include <QString>

class MorseCodec {
public:
    static QString encode(const QString &text);
    static QString decode(const QString &morse);
};

#endif // MORSECODEC_H