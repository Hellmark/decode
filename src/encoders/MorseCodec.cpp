#include "MorseCodec.h"
#include <QMap>
#include <QStringList>

static QMap<QChar, QString> morseTable {
    {'A', ".-"}, {'B', "-..."}, {'C', "-.-."}, {'D', "-.."},
    {'E', "."}, {'F', "..-."}, {'G', "--."}, {'H', "...."},
    {'I', ".."}, {'J', ".---"}, {'K', "-.-"}, {'L', ".-.."},
    {'M', "--"}, {'N', "-."}, {'O', "---"}, {'P', ".--."},
    {'Q', "--.-"}, {'R', ".-."}, {'S', "..."}, {'T', "-"},
    {'U', "..-"}, {'V', "...-"}, {'W', ".--"}, {'X', "-..-"},
    {'Y', "-.--"}, {'Z', "--.."},
    {'1', ".----"}, {'2', "..---"}, {'3', "...--"}, {'4', "....-"},
    {'5', "....."}, {'6', "-...."}, {'7', "--..."}, {'8', "---.."},
    {'9', "----."}, {'0', "-----"},
    {' ', "/"}
};

QString MorseCodec::encode(const QString &text) {
    QStringList out;
    for (QChar ch : text.toUpper()) {
        out << morseTable.value(ch, "");
    }
    return out.join(" ");
}

QString MorseCodec::decode(const QString &morse) {
    QMap<QString, QChar> reverseMap;
    for (auto it = morseTable.begin(); it != morseTable.end(); ++it)
        reverseMap[it.value()] = it.key();

    QStringList parts = morse.split(" ");
    QString result;
    for (const QString &code : parts) {
        result += reverseMap.value(code, '?');
    }
    return result;
}