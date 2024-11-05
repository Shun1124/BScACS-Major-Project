#ifndef MONITORING_H
#define MONITORING_H

#include <QObject>
#include <QList>
#include <QTimer>
#include "registryKey.h"
#include "registrykeymodel.h"
#include "rollback.h"
#include "alert.h"
#include "settings.h"

class Monitoring : public QObject {
    Q_OBJECT
    Q_PROPERTY(RegistryKeyModel* registryKeys READ registryKeys NOTIFY registryKeysChanged)

public:
    explicit Monitoring(Settings *settings, QObject *parent = nullptr);

    Q_INVOKABLE void startMonitoring();
    Q_INVOKABLE void stopMonitoring();
    Q_INVOKABLE void setKeyCriticalStatus(const QString &keyName, bool isCritical);
    Q_INVOKABLE void allowChange(const QString &keyName);

    RegistryKeyModel* registryKeys();

signals:
    void statusChanged(const QString &status);
    void keyChanged(const QString &key, const QString &value);
    void registryKeysChanged();
    void logMessage(QString message);
    void criticalChangeDetected(QString message);

private:
    void checkForChanges();

    QList<RegistryKey*> m_registryKeys;
    RegistryKeyModel m_registryKeysModel;
    Rollback m_rollback;
    Alert m_alert;  // Added Alert instance
    Settings *m_settings;  // Added pointer to Settings
    QTimer m_timer;
    bool m_monitoringActive;
};

#endif // MONITORING_H
