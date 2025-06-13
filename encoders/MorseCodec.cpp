#include "MorseCodec.h"
#include <QMap>
#include <QStringList>

QString MorseCodec::transform(const QString &text, bool decode) {
    static const QMap<QChar, QString> toMorse = {
        {'A', ".-"}, {'B', "-..."}, {'C', "-.-."}, {'D', "-.."}, {'E', "."},
        {'F', "..-."}, {'G', "--."}, {'H', "...."}, {'I', ".."}, {'J', ".---"},
        {'K', "-.-"}, {'L', ".-.."}, {'M', "--"}, {'N', "-."}, {'O', "---"},
        {'P', ".--."}, {'Q', "--.-"}, {'R', ".-.",}, {'S', "..."}, {'T', "-"},
        {'U', "..-"}, {'V', "...-"}, {'W', ".--"}, {'X', "-..-"}, {'Y', "-.--"}, {'Z', "--.."},
        {'1', ".----"}, {'2', "..---"}, {'3', "...--"}, {'4', "....-"}, {'5', "....."},
        {'6', "-...."}, {'7', "--..."}, {'8', "---.."}, {'9', "----."}, {'0', "-----"},
    };
    static QMap<QString, QChar> fromMorse;
    if (fromMorse.isEmpty()) {
        for (auto it = toMorse.begin(); it != toMorse.end(); ++it)
            fromMorse[it.value()] = it.key();
    }
    if (decode) {
        QStringList parts = text.split(" ", Qt::SkipEmptyParts);
        QString result;
        for (const QString &part : parts)
            result += fromMorse.value(part, '?');
        return result;
    } else {
        QString result;
        for (QChar c : text.toUpper()) {
            if (toMorse.contains(c))
                result += toMorse[c] + " ";
        }
        return result.trimmed();
    }
}
