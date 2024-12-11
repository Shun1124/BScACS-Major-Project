#ifndef WINDOWSMONITORING_H
#define WINDOWSMONITORING_H 

#include <QObject> // Base class for all Qt objects, providing signals and slots
#include <QList> // Provides QList class for handling lists of objects
#include <QTimer> // Provides timer functionality
#include "registryKey.h" // Includes the RegistryKey class for handling registry keys
#include "registryKeyModel.h" // Includes RegistryKeyModel for managing registry key data in a model format
#include "WindowsRollback.h" // Includes Rollback class for rollback functionality
#include "alert.h" // Includes Alert class to send alerts
#include "settings.h" // Includes Settings class for application settings

class WindowsMonitoring : public QObject { // Defines the Monitoring class, inheriting from QObject
    Q_OBJECT // Macro enabling the Qt meta-object features like signals and slots
    Q_PROPERTY(RegistryKeyModel* registryKeys READ registryKeys NOTIFY registryKeysChanged) // Exposes registryKeys property to QML

public:
    explicit WindowsMonitoring(Settings *settings, QObject *parent = nullptr); // Constructor for Monitoring, accepting settings and optional parent

    Q_INVOKABLE void startMonitoring(); // Exposes startMonitoring method to QML for starting monitoring
    Q_INVOKABLE void stopMonitoring(); // Exposes stopMonitoring method to QML for stopping monitoring
    Q_INVOKABLE void setKeyCriticalStatus(const QString &keyName, bool isCritical); // Exposes method to change key critical status in QML
    Q_INVOKABLE void allowChange(const QString &keyName); // Exposes method to allow change for a key, cancelling rollback if necessary

    RegistryKeyModel* registryKeys(); // Getter function for the registryKeys property

signals:
    void statusChanged(const QString &status); // Signal emitted when the monitoring status changes
    void keyChanged(const QString &key, const QString &value); // Signal emitted when a registry key changes
    void registryKeysChanged(); // Signal emitted when the list of registry keys changes
    void logMessage(QString message); // Signal emitted to log a message
    void criticalChangeDetected(QString message); // Signal emitted when a critical change is detected

private:
    void checkForChanges(); // Private method that checks for changes in monitored registry keys

    QList<RegistryKey*> m_registryKeys; // List of pointers to monitored registry keys
    RegistryKeyModel m_registryKeysModel; // Model for handling registry keys in a model format
    WindowsRollback m_rollback; // Rollback object to manage rollback operations
    Alert m_alert; // Alert instance for sending alerts
    Settings *m_settings; // Pointer to Settings for accessing configuration data
    QTimer m_timer; // Timer for periodic monitoring checks
    bool m_monitoringActive; // Flag to track if monitoring is currently active

};

#endif // WINDOWSMONITORING_H
