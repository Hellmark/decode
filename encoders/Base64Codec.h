#ifndef BASE64CODEC_H
#define BASE64CODEC_H
#include <QString>

class Base64Codec {
public:
    static QString transform(const QString &text, bool decode);
};
#endif
