#include <QStringList>
#include "PigLatin.h"

QString PigLatin::transform(const QString &text, bool decode) {
    if (decode) return text; // Simplified
    QStringList words = text.split(' ');
    for (QString &word : words) {
        if (word.length() > 0)
            word = word.mid(1) + word[0] + "ay";
    }
    return words.join(" ");
}
