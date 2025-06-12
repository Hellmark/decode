#ifndef ROT13_H
#define ROT13_H

#include <QString>

class Rot13 {
public:
    static QString encode(const QString &input);
    static QString decode(const QString &input);
};

#endif // ROT13_H