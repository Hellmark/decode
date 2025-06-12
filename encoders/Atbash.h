#ifndef ATBASH_H
#define ATBASH_H

#include <QString>

class Atbash {
public:
    static QString encode(const QString &input);
    static QString decode(const QString &input);
};

#endif // ATBASH_H