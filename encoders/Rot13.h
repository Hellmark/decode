#ifndef ROT13_H
#define ROT13_H
#include <QString>

class Rot13 {
public:
    static QString transform(const QString &text);
};
#endif
