#ifndef MORSECODEC_H
#define MORSECODEC_H
#include <QString>

class MorseCodec {
public:
    static QString transform(const QString &text, bool decode);
};
#endif
