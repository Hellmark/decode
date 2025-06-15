#ifndef HEXCODEC_H
#define HEXCODEC_H

#include <QString>

class HexCodec {
public:
    static QString transform(const QString &text, bool decode);
};

#endif // HEXCODEC_H
