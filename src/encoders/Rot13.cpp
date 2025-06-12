#include "Rot13.h"

QString Rot13::encode(const QString &input) {
    QString output = input;
    for (int i = 0; i < output.length(); ++i) {
        QChar &ch = output[i];
        if (ch.isLetter()) {
            QChar base = ch.isUpper() ? 'A' : 'a';
            ch = QChar((ch.unicode() - base.unicode() + 13) % 26 + base.unicode());
        }
    }
    return output;
}

QString Rot13::decode(const QString &input) {
    return encode(input); // ROT13 is symmetric
}