#include "MacOSJsonUtils.h" // Includes the header file for JsonUtils functions
#include <QFile> // Provides file handling capabilities
#include <QJsonDocument> // Enables JSON document manipulation
#include <QJsonArray> // Provides handling for JSON arrays
#include <QJsonObject> // Provides handling for JSON objects
#include <QDebug> // Enables logging and debugging outputs

namespace MacOSJsonUtils { // Defines the JsonUtils namespace to organize related functions

QList<PlistFile*> readFilesFromJson(const QString &filePath) { // Function to read registry keys from a JSON file and return them as a list
    QFile file(filePath); // Creates a QFile object for the specified file path
    QList<PlistFile*> plistFiles; // Initializes an empty list to store RegistryKey pointers

    if (!file.open(QIODevice::ReadOnly)) { // Attempts to open the file in read-only mode
        qWarning() << "Could not open file:" << filePath; // Logs a warning if the file can't be opened
        return plistFiles; // Returns an empty list if the file cannot be opened
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll()); // Reads all content from the file and parses it as a JSON document
    QJsonArray filesArray = doc.array(); // Retrieves the JSON array from the document

    for (const QJsonValue &value : filesArray) { // Iterates through each item in the JSON array
        QJsonObject obj = value.toObject(); // Converts each item to a JSON object
        QString plistPath = obj["plistPath"].toString(); // Extracts the "hive" string from the JSON object
        QString filePath = obj["filePath"].toString(); // Extracts the "keyPath" string from the JSON object
        QString valueName = obj["valueName"].toString(); // Extracts the "valueName" string from the JSON object
        bool isCritical = obj["isCritical"].toBool(); // Extracts the "isCritical" boolean from the JSON object

        plistFiles.append(new PlistFile(plistPath, filePath, valueName, isCritical)); // Creates a new RegistryKey object and appends it to the list
    }

    qDebug() << "Registry Keys model size:" << plistFiles.size(); // Logs the size of the populated registry keys list
    return plistFiles; // Returns the list of registry keys
}

} // namespace JsonUtils // Closes the JsonUtils namespace
