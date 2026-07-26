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
        // Words are separated by '/', letters within a word by spaces.
        QStringList words = text.split('/', Qt::SkipEmptyParts);
        QString result;
        for (int w = 0; w < words.size(); ++w) {
            if (w > 0) result += ' ';
            const QStringList parts = words[w].split(' ', Qt::SkipEmptyParts);
            for (const QString &part : parts)
                result += fromMorse.value(part, '?');
        }
        return result;
    } else {
        // Encode word by word so word boundaries survive as " / ".
        QStringList encodedWords;
        for (const QString &word : text.toUpper().split(' ', Qt::SkipEmptyParts)) {
            QStringList letters;
            for (QChar c : word) {
                if (toMorse.contains(c))
                    letters += toMorse[c];
            }
            encodedWords += letters.join(" ");
        }
        return encodedWords.join(" / ");
    }
}
