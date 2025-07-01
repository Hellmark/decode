#ifndef RSACODEC_H
#define RSACODEC_H

#include <QString>
#include <QByteArray>

class RSACodec {
public:
    static QByteArray encode(const QByteArray &input, const QString &keyPath);
    static QByteArray decode(const QByteArray &input, const QString &keyPath);
};

#endif // RSACODEC_H
