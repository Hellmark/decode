#ifndef BASE64CODEC_H
#define BASE64CODEC_H

#include <QString>

class Base64Codec {
public:
    static QString encode(const QString &input);
    static QString decode(const QString &input);
};

#endif // BASE64CODEC_H