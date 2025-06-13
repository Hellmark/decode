#include "Base64Codec.h"
#include <QByteArray>

QString Base64Codec::transform(const QString &text, bool decode) {
    if (decode)
        return QByteArray::fromBase64(text.toUtf8());
    return text.toUtf8().toBase64();
}
