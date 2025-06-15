#ifndef AESCODEC_H
#define AESCODEC_H

#include <QString>

class AesCodec {
public:
    static QString encode(const QString &text, const QString &key);
    static QString decode(const QString &text, const QString &key);
};

#endif // AESCODEC_H
