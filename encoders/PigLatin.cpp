#include "PigLatin.h"
#include <QStringList>
#include <QRegularExpression>

QString PigLatin::encode(const QString &input) {
    QStringList words = input.split(QRegularExpression("\s+"));
    for (QString &word : words) {
        if (word.length() > 0) {
            QString first = word.left(1);
            word = word.mid(1) + first + "ay";
        }
    }
    return words.join(" ");
}

QString PigLatin::decode(const QString &input) {
    QStringList words = input.split(QRegularExpression("\s+"));
    for (QString &word : words) {
        if (word.endsWith("ay") && word.length() > 2) {
            word = word.at(word.length() - 3) + word.left(word.length() - 3);
        }
    }
    return words.join(" ");
}