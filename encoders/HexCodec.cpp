#include "HexCodec.h"
#include <QByteArray>

QString HexCodec::transform(const QString &text, bool decode) {
    if (decode) {
        QByteArray hex = QByteArray::fromHex(text.simplified().remove(' ').toUtf8());
        return QString::fromUtf8(hex);
    } else {
        QByteArray bytes = text.toUtf8();
        return bytes.toHex(' ').toUpper();
    }
}
