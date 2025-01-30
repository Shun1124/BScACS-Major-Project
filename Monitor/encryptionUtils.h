#ifndef ENCRYPTIONUTILS_H
#define ENCRYPTIONUTILS_H

#include <QString>
#include <QByteArray>

class EncryptionUtils {
public:
    static void initialize(); // Initializes the encryption utils (loads keys dynamically)
    static void loadEncryptionKeys(const QString &filePath);
    static QByteArray encrypt(const QString &data);
    static QString decrypt(const QByteArray &encryptedData);

private:
    static QByteArray encryptionKey;
    static QByteArray encryptionIv;

    static QString resolveEncryptionKeysPath(); // Helper method to resolve the key file path
};

#endif // ENCRYPTIONUTILS_H
