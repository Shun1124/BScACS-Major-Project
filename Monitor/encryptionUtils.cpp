#include "EncryptionUtils.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>
#include <QCoreApplication>
#include <QDebug>
#include <openssl/evp.h>
#include <openssl/rand.h>

QByteArray EncryptionUtils::encryptionKey;
QByteArray EncryptionUtils::encryptionIv;

QString EncryptionUtils::resolveEncryptionKeysPath() {
#ifdef Q_OS_MAC
    return QDir::cleanPath(QCoreApplication::applicationDirPath() + "/../../../../../encryptionKeys.json");
#else
    return QDir::cleanPath(QCoreApplication::applicationDirPath() + "/../../encryptionKeys.json");
#endif
}

void EncryptionUtils::loadEncryptionKeys(const QString &filePath) {
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly)) {
        qFatal("Failed to open encryption key file at: %s", qPrintable(filePath));
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject obj = doc.object();

    if (!obj.contains("key") || !obj.contains("iv")) {
        qFatal("Encryption key file is missing required fields: 'key' or 'iv'.");
    }

    encryptionKey = QByteArray::fromBase64(obj["key"].toString().toUtf8());
    encryptionIv = QByteArray::fromBase64(obj["iv"].toString().toUtf8());

    if (encryptionKey.size() != 32) {
        qFatal("Encryption key must be 32 bytes for AES-256.");
    }

    if (encryptionIv.size() != 16) {
        qFatal("Encryption IV must be 16 bytes for AES-CBC.");
    }

    qDebug() << "Encryption keys loaded successfully from:" << filePath;
    qDebug() << "Key (Base64):" << obj["key"].toString();
    qDebug() << "IV (Base64):" << obj["iv"].toString();
}

void EncryptionUtils::initialize() {
    QString encryptionKeysPath = resolveEncryptionKeysPath();
    loadEncryptionKeys(encryptionKeysPath);
}

QByteArray EncryptionUtils::encrypt(const QString &data) {
    if (data.isEmpty()) {
        qWarning() << "Input data is empty. Nothing to encrypt.";
        return QByteArray();
    }

    QByteArray input = data.toUtf8();
    QByteArray output(input.size() + EVP_CIPHER_block_size(EVP_aes_256_cbc()), 0);

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        qWarning() << "Failed to create EVP_CIPHER_CTX for encryption.";
        return QByteArray();
    }

    qDebug() << "Encrypting with Key:" << encryptionKey.toBase64();
    qDebug() << "Encrypting with IV:" << encryptionIv.toBase64();

    if (!EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr,
                            reinterpret_cast<const unsigned char *>(encryptionKey.data()),
                            reinterpret_cast<const unsigned char *>(encryptionIv.data()))) {
        qWarning() << "Failed to initialize encryption.";
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }

    int outlen = 0, totalOutLen = 0;

    if (!EVP_EncryptUpdate(ctx, reinterpret_cast<unsigned char *>(output.data()), &outlen,
                           reinterpret_cast<const unsigned char *>(input.data()), input.size())) {
        qWarning() << "Encryption update failed.";
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }
    totalOutLen += outlen;

    if (!EVP_EncryptFinal_ex(ctx, reinterpret_cast<unsigned char *>(output.data()) + totalOutLen, &outlen)) {
        qWarning() << "Encryption finalization failed.";
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }
    totalOutLen += outlen;
    output.resize(totalOutLen);

    EVP_CIPHER_CTX_free(ctx);
    qDebug() << "Encrypted data (Base64):" << output.toBase64();
    return output.toBase64();
}

QString EncryptionUtils::decrypt(const QByteArray &encryptedData) {
    if (encryptedData.isEmpty()) {
        qWarning() << "Encrypted data is empty. Cannot decrypt.";
        return "";  // Return empty string as fallback
    }

    qDebug() << "Raw encrypted data (Base64):" << encryptedData;
    QByteArray decodedData = QByteArray::fromBase64(encryptedData);

    if (decodedData.isEmpty()) {
        qWarning() << "Decoded data is empty or invalid Base64. Cannot decrypt.";
        return "";  // Return empty string as fallback
    }

    qDebug() << "Decoded encrypted data:" << decodedData;

    QByteArray output(decodedData.size() + EVP_CIPHER_block_size(EVP_aes_256_cbc()), 0);

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        qWarning() << "Failed to create EVP_CIPHER_CTX for decryption.";
        return "";
    }

    qDebug() << "Decrypting with Key:" << encryptionKey.toBase64()
             << "IV:" << encryptionIv.toBase64();

    if (!EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr,
                            reinterpret_cast<const unsigned char *>(encryptionKey.data()),
                            reinterpret_cast<const unsigned char *>(encryptionIv.data()))) {
        qWarning() << "Failed to initialize decryption.";
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }

    int outlen = 0, totalOutLen = 0;

    if (!EVP_DecryptUpdate(ctx, reinterpret_cast<unsigned char *>(output.data()), &outlen,
                           reinterpret_cast<const unsigned char *>(decodedData.data()), decodedData.size())) {
        qWarning() << "Decryption update failed.";
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }
    totalOutLen += outlen;

    if (!EVP_DecryptFinal_ex(ctx, reinterpret_cast<unsigned char *>(output.data()) + totalOutLen, &outlen)) {
        qWarning() << "Decryption finalization failed. Check key/IV or data integrity.";
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }
    totalOutLen += outlen;
    output.resize(totalOutLen);

    EVP_CIPHER_CTX_free(ctx);
    qDebug() << "Decrypted data:" << QString::fromUtf8(output);
    return QString::fromUtf8(output);
}
