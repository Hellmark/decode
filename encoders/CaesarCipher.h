#ifndef CAESARCIPHER_H
#define CAESARCIPHER_H
#include <QString>

class CaesarCipher {
public:
    static QString transform(const QString &text, int shift, bool decode);
};
#endif
