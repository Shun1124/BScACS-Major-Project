#include "jsonUtils.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>

namespace JsonUtils {

QList<RegistryKey*> readKeysFromJson(const QString &filePath) { // Function to read registry keys from a JSON file and return them as a list
    QFile file(filePath);
    QList<RegistryKey*> registryKeys;

    if (!file.open(QIODevice::ReadOnly)) { // Attempts to open the file in read-only mode
        qWarning() << "Could not open file:" << filePath; // Logs a warning if the file can't be opened
        return registryKeys; // Returns an empty list if the file cannot be opened
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonArray keysArray = doc.array(); // Retrieves the JSON array from the document

    for (const QJsonValue &value : keysArray) {
        QJsonObject obj = value.toObject();
        QString hive = obj["hive"].toString();
        QString keyPath = obj["keyPath"].toString();
        QString valueName = obj["valueName"].toString();
        bool isCritical = obj["isCritical"].toBool();

        registryKeys.append(new RegistryKey(hive, keyPath, valueName, isCritical)); // Creates a new RegistryKey object and appends it to the list
    }

    qDebug() << "Registry Keys model size:" << registryKeys.size();
    return registryKeys;
}

} // namespace JsonUtils // Closes the JsonUtils namespace
