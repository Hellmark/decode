#include "CaesarCipher.h"

QString CaesarCipher::transform(const QString &text, int shift, bool decode) {
    if (decode) shift = -shift;
    QString result;
    for (QChar c : text) {
        if (c.isLetter()) {
            QChar base = c.isUpper() ? 'A' : 'a';
            result += QChar(base.unicode() + (c.unicode() - base.unicode() + shift + 26) % 26);
        } else {
            result += c;
        }
    }
    return result;
}
