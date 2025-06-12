#include "BinaryCodec.h"
#include <QStringList>

QString BinaryCodec::encode(const QString &input) {
    QString result;
    for (QChar ch : input) {
        result += QString("%1 ").arg(ch.unicode(), 8, 2, QChar('0'));
    }
    return result.trimmed();
}

QString BinaryCodec::decode(const QString &input) {
    QStringList bits = input.split(" ", Qt::SkipEmptyParts);
    QString result;
    for (const QString &bit : bits) {
        bool ok;
        ushort code = bit.toUShort(&ok, 2);
        if (ok) result += QChar(code);
    }
    return result;
}
