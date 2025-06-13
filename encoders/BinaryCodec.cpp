#include <QStringList>
#include "BinaryCodec.h"

QString BinaryCodec::transform(const QString &text, bool decode) {
    if (decode) {
        QString result;
        QStringList parts = text.split(' ', Qt::SkipEmptyParts);
        for (const QString &part : parts)
            result += QChar(part.toInt(nullptr, 2));
        return result;
    } else {
        QString result;
        for (QChar c : text)
            result += QString::number(c.unicode(), 2).rightJustified(8, '0') + ' ';
        return result.trimmed();
    }
}
