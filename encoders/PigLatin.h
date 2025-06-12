#ifndef PIGLATIN_H
#define PIGLATIN_H

#include <QString>

class PigLatin {
public:
    static QString encode(const QString &input);
    static QString decode(const QString &input);
};

#endif // PIGLATIN_H