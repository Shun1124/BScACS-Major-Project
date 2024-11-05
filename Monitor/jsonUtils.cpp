#include "jsonUtils.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>

namespace JsonUtils {

QList<RegistryKey*> readKeysFromJson(const QString &filePath) {
    QFile file(filePath);
    QList<RegistryKey*> registryKeys;

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open file:" << filePath;
        return registryKeys;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonArray keysArray = doc.array();

    for (const QJsonValue &value : keysArray) {
        QJsonObject obj = value.toObject();
        QString hive = obj["hive"].toString();
        QString keyPath = obj["keyPath"].toString();
        QString valueName = obj["valueName"].toString();
        bool isCritical = obj["isCritical"].toBool();

        registryKeys.append(new RegistryKey(hive, keyPath, valueName, isCritical));
    }

    qDebug() << "Registry Keys model size:" << registryKeys.size();
    return registryKeys;
}

} // namespace JsonUtils
