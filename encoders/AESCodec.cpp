#include "AESCodec.h"
#include <QByteArray>
#include <QCryptographicHash>
#include <QDataStream>
#include <QBuffer>
#include <QDebug>

#include <openssl/evp.h>
#include <openssl/aes.h>

static QByteArray deriveKey(const QString &password) {
    return QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).left(32);
}

QString AESCodec::encode(const QString &text, const QString &key) {
    QByteArray input = text.toUtf8();
    QByteArray output;
    QByteArray aesKey = deriveKey(key);
    QByteArray iv(16, 0); // Zero IV for simplicity — replace with random IV in production

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return QString();

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, reinterpret_cast<const unsigned char *>(aesKey.data()),
                           reinterpret_cast<const unsigned char *>(iv.data())) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return QString();
    }

    int outLen = input.size() + AES_BLOCK_SIZE;
    output.resize(outLen);
    int actualOutLen = 0;

    if (EVP_EncryptUpdate(ctx, reinterpret_cast<unsigned char *>(output.data()), &actualOutLen,
                          reinterpret_cast<const unsigned char *>(input.data()), input.size()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return QString();
    }

    int finalLen = 0;
    if (EVP_EncryptFinal_ex(ctx, reinterpret_cast<unsigned char *>(output.data()) + actualOutLen, &finalLen) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return QString();
    }

    output.resize(actualOutLen + finalLen);
    EVP_CIPHER_CTX_free(ctx);
    return output.toBase64();
}

QString AESCodec::decode(const QString &text, const QString &key) {
    QByteArray input = QByteArray::fromBase64(text.toUtf8());
    QByteArray output;
    QByteArray aesKey = deriveKey(key);
    QByteArray iv(16, 0); // Same IV used in encode

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return QString();

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, reinterpret_cast<const unsigned char *>(aesKey.data()),
                           reinterpret_cast<const unsigned char *>(iv.data())) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return QString();
    }

    int outLen = input.size();
    output.resize(outLen);
    int actualOutLen = 0;

    if (EVP_DecryptUpdate(ctx, reinterpret_cast<unsigned char *>(output.data()), &actualOutLen,
                          reinterpret_cast<const unsigned char *>(input.data()), input.size()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return QString();
    }

    int finalLen = 0;
    if (EVP_DecryptFinal_ex(ctx, reinterpret_cast<unsigned char *>(output.data()) + actualOutLen, &finalLen) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return QString();
    }

    output.resize(actualOutLen + finalLen);
    EVP_CIPHER_CTX_free(ctx);
    return QString::fromUtf8(output);
}
