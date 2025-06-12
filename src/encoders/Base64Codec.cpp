#include "Base64Codec.h"
#include <QByteArray>

QString Base64Codec::encode(const QString &input) {
    return QString(QByteArray::fromStdString(input.toStdString()).toBase64());
}

QString Base64Codec::decode(const QString &input) {
    return QString(QByteArray::fromBase64(input.toUtf8()));
}