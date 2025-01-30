#include "Database.h"
#include "EncryptionUtils.h"
#include <QDir>
#include <QCoreApplication>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QCryptographicHash>

Database::Database(QObject *parent) : QObject(parent) {
    if (QSqlDatabase::contains("qt_sql_default_connection")) {
        db = QSqlDatabase::database("qt_sql_default_connection");
    } else {
        db = QSqlDatabase::addDatabase("QMYSQL", "qt_sql_default_connection");
        db.setHostName("localhost");
        db.setPort(3306);
        db.setDatabaseName("MonitorDB");
        db.setUserName("monitor_user");
        db.setPassword("Monitor1230.");
    }

    if (!db.isOpen() && !db.open()) {
        qDebug() << "Failed to connect to database:" << db.lastError().text();
    } else {
        qDebug() << "Database connection established.";
        createSchema();
    }

    // Load encryption keys dynamically
    QString encryptionKeysPath = resolveEncryptionKeysPath();
    EncryptionUtils::loadEncryptionKeys(encryptionKeysPath);
}

QString Database::resolveEncryptionKeysPath() {
#ifdef Q_OS_MAC
    return QDir::cleanPath(QCoreApplication::applicationDirPath() + "/../../../../../encryptionKeys.json");
#else
    return QDir::cleanPath(QCoreApplication::applicationDirPath() + "/../../encryptionKeys.json");
#endif
}

Database::~Database() {
    if (db.isOpen()) {
        db.close();
    }
}

void Database::ensureConnection() {
    if (!db.isOpen()) {
        qWarning() << "[DATABASE] Database connection is closed. Attempting to reopen...";
        if (!db.open()) {
            qCritical() << "[DATABASE] Failed to reopen database connection:" << db.lastError().text();
        } else {
            qDebug() << "[DATABASE] Database connection reopened successfully.";
        }
    }
}

bool Database::createSchema() {
    QSqlQuery query;

    QString createUserSettingsTable = R"(
        CREATE TABLE IF NOT EXISTS UserSettings (
            id INT AUTO_INCREMENT PRIMARY KEY,
            user_email VARCHAR(512) UNIQUE NULL,
            phone_number VARCHAR(512) UNIQUE NULL,
            non_critical_threshold INT DEFAULT 0,
            timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
        );
    )";

    if (!query.exec(createUserSettingsTable)) {
        qDebug() << "Failed to create UserSettings table:" << query.lastError().text();
        return false;
    }

    QString createConfigurationTable = R"(
        CREATE TABLE IF NOT EXISTS ConfigurationSettings (
            id INT AUTO_INCREMENT PRIMARY KEY,
            key_name VARCHAR(255) UNIQUE NOT NULL,
            key_path TEXT NOT NULL,
            key_value TEXT NOT NULL,
            is_critical BOOLEAN DEFAULT FALSE,
            timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
        );
    )";

    if (!query.exec(createConfigurationTable)) {
        qDebug() << "Failed to create ConfigurationSettings table:" << query.lastError().text();
        return false;
    }

    QString createChangesTable = R"(
        CREATE TABLE IF NOT EXISTS Changes (
            id INT AUTO_INCREMENT PRIMARY KEY,
            key_name VARCHAR(255) NOT NULL,
            old_value TEXT NOT NULL,
            new_value TEXT NOT NULL,
            acknowledged BOOLEAN DEFAULT FALSE,
            timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
        );
    )";

    if (!query.exec(createChangesTable)) {
        qDebug() << "Failed to create Changes table:" << query.lastError().text();
        return false;
    }

    qDebug() << "Tables created or already exist.";
    return true;
}

bool Database::insertUserSettings(const QString &email, const QString &phone, int threshold) {
    ensureConnection();

    if (email.isEmpty() && phone.isEmpty()) {
        qWarning() << "[DATABASE] Both email and phone are empty. Skipping insertion.";
        return false;
    }

    // Use encryption for email and phone
    QByteArray encryptedEmail = email.isEmpty() ? QByteArray() : EncryptionUtils::encrypt(email);
    QByteArray encryptedPhone = phone.isEmpty() ? QByteArray() : EncryptionUtils::encrypt(phone);

    // Check if the email or phone already exists
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT id FROM UserSettings WHERE user_email = :email OR phone_number = :phone");
    checkQuery.bindValue(":email", encryptedEmail.toBase64());
    checkQuery.bindValue(":phone", encryptedPhone.toBase64());

    if (!checkQuery.exec()) {
        qWarning() << "[DATABASE] Failed to check for existing email/phone:" << checkQuery.lastError().text();
        return false;
    }

    if (checkQuery.next()) {
        // Update existing record
        int id = checkQuery.value("id").toInt();
        QSqlQuery updateQuery;
        updateQuery.prepare(R"(
            UPDATE UserSettings
            SET user_email = IF(:email != '', :email, user_email),
                phone_number = IF(:phone != '', :phone, phone_number),
                non_critical_threshold = :threshold
            WHERE id = :id
        )");
        updateQuery.bindValue(":email", encryptedEmail.isEmpty() ? QVariant() : encryptedEmail.toBase64());
        updateQuery.bindValue(":phone", encryptedPhone.isEmpty() ? QVariant() : encryptedPhone.toBase64());
        updateQuery.bindValue(":threshold", threshold);
        updateQuery.bindValue(":id", id);

        if (!updateQuery.exec()) {
            qWarning() << "[DATABASE] Failed to update user settings:" << updateQuery.lastError().text();
            return false;
        }

        qDebug() << "[DATABASE] Updated existing user settings for email/phone.";
        return true;
    } else {
        // Insert new record
        QSqlQuery insertQuery;
        insertQuery.prepare(R"(
            INSERT INTO UserSettings (user_email, phone_number, non_critical_threshold)
            VALUES (:email, :phone, :threshold)
        )");
        insertQuery.bindValue(":email", encryptedEmail.isEmpty() ? QVariant(QVariant::String) : encryptedEmail.toBase64());
        insertQuery.bindValue(":phone", encryptedPhone.isEmpty() ? QVariant(QVariant::String) : encryptedPhone.toBase64());
        insertQuery.bindValue(":threshold", threshold);

        if (!insertQuery.exec()) {
            qWarning() << "[DATABASE] Failed to insert new user settings:" << insertQuery.lastError().text();
            return false;
        }

        qDebug() << "[DATABASE] Inserted new user settings.";
        return true;
    }
}


QVariantList Database::getAllUserSettings() {
    ensureConnection();
    QVariantList dataList;
    QSqlQuery query("SELECT * FROM UserSettings");

    while (query.next()) {
        QVariantMap row;
        row["id"] = query.value("id");

        // Decode and decrypt email and phone
        QByteArray encryptedEmail = QByteArray::fromBase64(query.value("user_email").toByteArray());
        QByteArray encryptedPhone = QByteArray::fromBase64(query.value("phone_number").toByteArray());

        row["email"] = EncryptionUtils::decrypt(encryptedEmail);
        row["phone"] = EncryptionUtils::decrypt(encryptedPhone);

        row["threshold"] = query.value("non_critical_threshold");
        row["timestamp"] = query.value("timestamp");
        dataList.append(row);
    }

    return dataList;
}

bool Database::insertOrUpdateConfiguration(const QString &keyName, const QString &keyPath, const QString &keyValue, bool isCritical) {
    ensureConnection();
    QSqlQuery query;

    QByteArray encryptedKeyValue = EncryptionUtils::encrypt(keyValue);

    query.prepare(R"(
        INSERT INTO ConfigurationSettings (key_name, key_path, key_value, is_critical)
        VALUES (:keyName, :keyPath, :keyValue, :isCritical)
        ON DUPLICATE KEY UPDATE
        key_path = VALUES(key_path),
        key_value = VALUES(key_value),
        is_critical = VALUES(is_critical)
    )");
    query.bindValue(":keyName", keyName);
    query.bindValue(":keyPath", keyPath);
    query.bindValue(":keyValue", encryptedKeyValue);
    query.bindValue(":isCritical", isCritical);

    if (!query.exec()) {
        qDebug() << "Failed to insert/update configuration:" << query.lastError().text();
        return false;
    }

    return true;
}

QVariantList Database::getAllConfigurations() {
    ensureConnection();
    QVariantList dataList;
    QSqlQuery query("SELECT * FROM ConfigurationSettings");

    while (query.next()) {
        QVariantMap row;
        row["id"] = query.value("id");
        row["key_name"] = query.value("key_name");
        row["key_path"] = query.value("key_path");
        row["key_value"] = query.value("key_value");
        row["is_critical"] = query.value("is_critical");
        row["timestamp"] = query.value("timestamp");
        dataList.append(row);
    }

    return dataList;
}

bool Database::insertChange(const QString &keyName, const QString &oldValue, const QString &newValue, bool acknowledged) {
    ensureConnection();
    QSqlQuery query;
    query.prepare(R"(
        INSERT INTO Changes (key_name, old_value, new_value, acknowledged)
        VALUES (:keyName, :oldValue, :newValue, :acknowledged)
    )");
    query.bindValue(":keyName", keyName);
    query.bindValue(":oldValue", oldValue);
    query.bindValue(":newValue", newValue);
    query.bindValue(":acknowledged", acknowledged);

    if (!query.exec()) {
        qDebug() << "Failed to insert change:" << query.lastError().text();
        return false;
    }
    return true;
}


QVariantList Database::getAllChanges() {
    ensureConnection();
    QVariantList dataList;

    QSqlQuery query("SELECT * FROM Changes");

    while (query.next()) {
        QVariantMap row;
        row["id"] = query.value("id");
        row["key_name"] = query.value("key_name");
        row["old_value"] = query.value("old_value");
        row["new_value"] = query.value("new_value");
        row["acknowledged"] = query.value("acknowledged").toBool();
        row["timestamp"] = query.value("timestamp");
        dataList.append(row);
    }

    return dataList;
}

bool Database::updateAcknowledgmentStatus(const QString &keyName) {
    ensureConnection();
    QSqlQuery query;
    query.prepare("UPDATE Changes SET acknowledged = TRUE WHERE key_name = :keyName AND acknowledged = FALSE");
    query.bindValue(":keyName", keyName);

    if (!query.exec()) {
        qDebug() << "Failed to update acknowledgment status:" << query.lastError().text();
        return false;
    }
    return query.numRowsAffected() > 0;
}

QVariantList Database::searchChangeHistory(const QString &date, const QString &keyName) {
    ensureConnection();
    QVariantList results;

    if (!db.isOpen()) {
        qWarning() << "Database is not open.";
        return results;
    }

    QSqlQuery query;
    QString queryString = "SELECT id, key_name, old_value, new_value, timestamp FROM Changes WHERE 1=1";

    if (!date.isEmpty()) {
        queryString += " AND DATE(timestamp) = :date";
    }
    if (!keyName.isEmpty()) {
        queryString += " AND key_name = :keyName";
    }

    query.prepare(queryString);
    if (!date.isEmpty()) {
        query.bindValue(":date", date);
    }
    if (!keyName.isEmpty()) {
        query.bindValue(":keyName", keyName);
    }

    if (!query.exec()) {
        qWarning() << "Failed to execute search query:" << query.lastError().text();
        return results;
    }

    while (query.next()) {
        QVariantMap record;
        record["id"] = query.value("id");
        record["key_name"] = query.value("key_name");
        record["old_value"] = query.value("old_value");
        record["new_value"] = query.value("new_value");
        record["timestamp"] = query.value("timestamp");
        results.append(record);
    }

    return results;
}
