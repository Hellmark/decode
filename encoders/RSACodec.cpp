#include "RSACodec.h"

#include <QByteArray>
#include <QFile>
#include <QDebug>

#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/err.h>

QByteArray RSACodec::encode(const QByteArray &input, const QString &keyPath) {
    QFile file(keyPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open public key file:" << keyPath;
        return {};
    }
    QByteArray pubKeyData = file.readAll();
    file.close();

    BIO *bio = BIO_new_mem_buf(pubKeyData.constData(), pubKeyData.size());
    if (!bio) {
        qWarning() << "BIO_new_mem_buf failed";
        return {};
    }
    EVP_PKEY *pubKey = PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr);
    if (!pubKey) {
        qWarning() << "PEM_read_bio_PUBKEY failed";
        BIO_free(bio);
        return {};
    }

    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(pubKey, nullptr);
    if (!ctx || EVP_PKEY_encrypt_init(ctx) <= 0 ||
        EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0) {
        EVP_PKEY_free(pubKey);
        BIO_free(bio);
        EVP_PKEY_CTX_free(ctx);
        return {};
    }

    size_t outLen = 0;
    if (EVP_PKEY_encrypt(ctx, nullptr, &outLen,
                         reinterpret_cast<const unsigned char *>(input.constData()), input.size()) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(pubKey);
        BIO_free(bio);
        return {};
    }

    QByteArray output(outLen, 0);
    if (EVP_PKEY_encrypt(ctx, reinterpret_cast<unsigned char *>(output.data()), &outLen,
                         reinterpret_cast<const unsigned char *>(input.constData()), input.size()) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(pubKey);
        BIO_free(bio);
        return {};
    }

    output.resize(outLen);

    EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_free(pubKey);
    BIO_free(bio);

    return output;
}

QByteArray RSACodec::decode(const QByteArray &input, const QString &keyPath) {
    QFile file(keyPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open private key file:" << keyPath;
        return {};
    }
    QByteArray privKeyData = file.readAll();
    file.close();

    BIO *bio = BIO_new_mem_buf(privKeyData.constData(), privKeyData.size());
    if (!bio) {
        qWarning() << "BIO_new_mem_buf failed";
        return {};
    }

    EVP_PKEY *privKey = PEM_read_bio_PrivateKey(bio, nullptr, nullptr, nullptr);
    if (!privKey) {
        qWarning() << "PEM_read_bio_PUBKEY failed";
        BIO_free(bio);
        return {};
    }

    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(privKey, nullptr);
    if (!ctx || EVP_PKEY_decrypt_init(ctx) <= 0 ||
        EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0) {
        EVP_PKEY_free(privKey);
        BIO_free(bio);
        EVP_PKEY_CTX_free(ctx);
        return {};
    }

    size_t outLen = 0;
    if (EVP_PKEY_decrypt(ctx, nullptr, &outLen,
                         reinterpret_cast<const unsigned char *>(input.constData()), input.size()) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(privKey);
        BIO_free(bio);
        return {};
    }

    QByteArray output(outLen, 0);
    if (EVP_PKEY_decrypt(ctx, reinterpret_cast<unsigned char *>(output.data()), &outLen,
                         reinterpret_cast<const unsigned char *>(input.constData()), input.size()) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(privKey);
        BIO_free(bio);
        return {};
    }

    output.resize(outLen);

    EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_free(privKey);
    BIO_free(bio);

    return output;
}
