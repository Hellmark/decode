#ifndef APPCONTROLLER_H
#define APPCONTROLLER_H

#include <QObject>
#include <QStringList>

class AppController : public QObject {
    Q_OBJECT
public:

    Q_INVOKABLE QString encodeRot13(const QString &text);
    Q_INVOKABLE QString decodeRot13(const QString &text);
    Q_INVOKABLE QString encodeBinary(const QString &text);
    Q_INVOKABLE QString decodeBinary(const QString &text);

    Q_INVOKABLE QString encodeCaesar(const QString &text, int shift);
    Q_INVOKABLE QString decodeCaesar(const QString &text, int shift);
    Q_INVOKABLE QString encodePigLatin(const QString &text);
    Q_INVOKABLE QString decodePigLatin(const QString &text);
    Q_INVOKABLE QString encodeMorse(const QString &text);
    Q_INVOKABLE QString decodeMorse(const QString &text);
    Q_INVOKABLE QString encodeAtbash(const QString &text);
    Q_INVOKABLE QString decodeAtbash(const QString &text);
    explicit AppController(QObject *parent = nullptr);

    Q_INVOKABLE QString encodeBase64(const QString &text);
    Q_INVOKABLE QString decodeBase64(const QString &text);
};

#endif // APPCONTROLLER_H