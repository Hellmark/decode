#ifndef CAESARCIPHER_H
#define CAESARCIPHER_H

#include <QString>

class CaesarCipher {
public:
    static QString encode(const QString &text, int shift);
    static QString decode(const QString &text, int shift);
};

#endif // CAESARCIPHER_H