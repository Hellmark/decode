#include "Atbash.h"

QString Atbash::encode(const QString &input) {
    QString result;
    for (QChar ch : input) {
        if (ch.isLower()) result += QChar('z' - (ch.unicode() - 'a'));
        else if (ch.isUpper()) result += QChar('Z' - (ch.unicode() - 'A'));
        else result += ch;
    }
    return result;
}

QString Atbash::decode(const QString &input) {
    return encode(input);
}