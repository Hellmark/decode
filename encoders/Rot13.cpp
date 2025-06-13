#include "Rot13.h"

QString Rot13::transform(const QString &text) {
    QString result;
    for (QChar c : text) {
        if (c.isLetter()) {
            QChar base = c.isUpper() ? 'A' : 'a';
            result += QChar(base.unicode() + (c.unicode() - base.unicode() + 13) % 26);
        } else {
            result += c;
        }
    }
    return result;
}
