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
    Q_PROPERTY(RegistryKeyModel* registryKeys READ registryKeys NOTIFY registryKeysChanged) // Exposes registryKeys property to QML

public:
    explicit Monitoring(Settings *settings, QObject *parent = nullptr); // Constructor for Monitoring, accepting settings and optional parent

    Q_INVOKABLE void startMonitoring();
    Q_INVOKABLE void stopMonitoring();
    Q_INVOKABLE void setKeyCriticalStatus(const QString &keyName, bool isCritical);
    Q_INVOKABLE void allowChange(const QString &keyName);

    RegistryKeyModel* registryKeys(); // Getter function for the registryKeys property

signals:
    void statusChanged(const QString &status);
    void keyChanged(const QString &key, const QString &value);
    void registryKeysChanged();
    void logMessage(QString message);
    void criticalChangeDetected(QString message);

private:
    void checkForChanges(); // Private method that checks for changes in monitored registry keys

    QList<RegistryKey*> m_registryKeys;
    RegistryKeyModel m_registryKeysModel;
    Rollback m_rollback;
    Alert m_alert;
    Settings *m_settings;
    QTimer m_timer;
    bool m_monitoringActive;
};

#endif
