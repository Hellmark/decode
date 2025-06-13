#ifndef BINARYCODEC_H
#define BINARYCODEC_H
#include <QString>

class BinaryCodec {
public:
    static QString transform(const QString &text, bool decode);
};
#endif
