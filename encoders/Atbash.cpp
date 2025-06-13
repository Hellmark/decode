#include "Atbash.h"

QString Atbash::transform(const QString &text) {
    QString result;
    for (QChar c : text) {
        if (c.isUpper())
            result += QChar('Z' - (c.unicode() - 'A'));
        else if (c.isLower())
            result += QChar('z' - (c.unicode() - 'a'));
        else
            result += c;
    }
    return result;
}
