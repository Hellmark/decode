#ifndef BINARYCODEC_H
#define BINARYCODEC_H

#include <QString>

class BinaryCodec {
public:
    static QString encode(const QString &input);
    static QString decode(const QString &input);
};

#endif // BINARYCODEC_H