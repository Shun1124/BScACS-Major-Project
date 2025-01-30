#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QVariantList>

class Database : public QObject {
    Q_OBJECT

public:
    explicit Database(QObject *parent = nullptr);
    ~Database();

    Q_INVOKABLE bool createSchema();
    Q_INVOKABLE void ensureConnection();

    // UserSettings methods
    Q_INVOKABLE bool insertUserSettings(const QString &email, const QString &phone, int threshold);
    Q_INVOKABLE QVariantList getAllUserSettings();

    // ConfigurationSettings methods
    Q_INVOKABLE bool insertOrUpdateConfiguration(const QString &keyName, const QString &keyPath, const QString &keyValue, bool isCritical);
    Q_INVOKABLE QVariantList getAllConfigurations();

    // Changes methods
    Q_INVOKABLE bool insertChange(const QString &keyName, const QString &oldValue, const QString &newValue, bool acknowledged);
    Q_INVOKABLE QVariantList getAllChanges();

    Q_INVOKABLE bool updateAcknowledgmentStatus(const QString &keyName);

    QString resolveEncryptionKeysPath();

    Q_INVOKABLE QVariantList searchChangeHistory(const QString &date, const QString &keyName);

private:
    QSqlDatabase db;
};

#endif // DATABASE_H
