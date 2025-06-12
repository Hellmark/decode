#include "CaesarCipher.h"

QString CaesarCipher::encode(const QString &text, int shift) {
    QString result;
    for (QChar ch : text) {
        if (ch.isLetter()) {
            QChar base = ch.isUpper() ? 'A' : 'a';
            ch = QChar((ch.unicode() - base.unicode() + shift + 26) % 26 + base.unicode());
        }
        result += ch;
    }
    return result;
}

QString CaesarCipher::decode(const QString &text, int shift) {
    return encode(text, -shift);
}