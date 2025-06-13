#ifndef PIGLATIN_H
#define PIGLATIN_H
#include <QString>

class PigLatin {
public:
    static QString transform(const QString &text, bool decode);
};
#endif
