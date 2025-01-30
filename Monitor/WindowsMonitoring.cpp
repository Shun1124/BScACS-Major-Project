#include "WindowsMonitoring.h" // Includes the Monitoring class definition
#include "WindowsJsonUtils.h" // Includes JSON utilities for reading data from JSON files
#include <QDebug> // Enables logging and debugging output
#include <QDir> // Provides directory handling capabilities
#include <QCoreApplication> // Provides application-level information like directory paths
#include "Database.h"
#include <QSqlQuery>
#include <QSqlError>

// Constructor for the Monitoring class
WindowsMonitoring::WindowsMonitoring(Settings *settings, QObject *parent)
    : MonitoringBase(parent), m_monitoringActive(false), m_settings(settings), m_alert(settings, this) { // Initializes Monitoring with settings and alert
    connect(&m_timer, &QTimer::timeout, this, &WindowsMonitoring::checkForChanges); // Connects timer timeout signal to checkForChanges slot

    // Connect rollbackPerformed signal to send alerts when a rollback is performed
    connect(&m_rollback, &WindowsRollback::rollbackPerformed, this, [this](const QString &valueName) { // Lambda function for rollback alerts
        QString alertMessage = "[CRITICAL] Rollback performed for key: " + valueName; // Constructs alert message
        emit criticalChangeDetected(alertMessage); // Emits signal for critical change
        // qDebug() << "[MONITORING] Emitting criticalChangeDetected signal with message:" << alertMessage; // Logs emitted message
        m_alert.sendAlert(alertMessage); // Sends alert message
    });


    QString filePath = QDir::cleanPath(QCoreApplication::applicationDirPath() + "/../../monitoredKeys.json");

    // Check if the monitoredConfigs.json file exists at the constructed path
    if (QFile::exists(filePath)) { // Checks if the file exists
        QList<RegistryKey*> registryKeys = WindowsJsonUtils::readKeysFromJson(filePath); // Reads registry keys from JSON
        m_registryKeys = registryKeys; // Stores registry keys in m_registryKeys
        m_registryKeysModel.setRegistryKeys(m_registryKeys); // Updates the model with registry keys
        emit registryKeysChanged(); // Emits signal for registry keys update
        qDebug() << "[MONITORING INIT] Successfully loaded monitoredConfigs.json from:" << filePath; // Logs success

        // Store monitored configurations in the database
        Database db;
        for (RegistryKey *key : m_registryKeys) {
            db.insertOrUpdateConfiguration(
                key->name(),
                key->keyPath(),
                key->value(),
                key->isCritical()
                );
        }
        qDebug() << "[MONITORING INIT] All monitored configurations stored in the database.";
    } else {
        qWarning() << "[MONITORING INIT] monitoredConfigs.json file not found at:" << filePath; // Warns if file not found
    }

    if (m_settings) {
        QString email = m_settings->getEmail();
        QString phone = m_settings->getPhoneNumber();

        if (email.isEmpty() || phone.isEmpty()) {
            // Attempt to fetch the values from the database
            Database db;
            QVariantList userSettings = db.getAllUserSettings();
            if (!userSettings.isEmpty()) {
                QVariantMap user = userSettings.first().toMap();
                if (email.isEmpty()) {
                    email = user["email"].toString();
                    m_settings->setEmail(email);
                }
                if (phone.isEmpty()) {
                    phone = user["phone"].toString();
                    m_settings->setPhoneNumber(phone);
                }
            }
        }

        qDebug() << "[MONITORING INIT] Final email:" << email << "Final phone:" << phone;

        if (!email.isEmpty() && !phone.isEmpty()) {
            Database db;
            if (!db.insertUserSettings(email, phone, 0)) {
                qWarning() << "[MONITORING INIT] Failed to store user settings in the database.";
            } else {
                qDebug() << "[MONITORING INIT] User settings stored in the database.";
            }
        } else {
            qWarning() << "[MONITORING INIT] Email or phone number is still empty. Alerts will not be sent.";
        }
    } else {
        qWarning() << "[MONITORING INIT] Settings object is null.";
    }
}

void WindowsMonitoring::startMonitoring() { // Starts the monitoring process
    if (!m_monitoringActive) { // Checks if monitoring is inactive
        m_monitoringActive = true; // Sets monitoring to active
        m_timer.start(1000); // Starts the timer with a 1-second interval
        qDebug() << "[START MONITORING] Monitoring started."; // Logs monitoring start
        emit statusChanged("Monitoring started"); // Emits signal to indicate monitoring status change
    }
}

void WindowsMonitoring::stopMonitoring() { // Stops the monitoring process
    if (m_monitoringActive) { // Checks if monitoring is active
        m_monitoringActive = false; // Sets monitoring to inactive
        m_timer.stop(); // Stops the timer
        qDebug() << "[STOP MONITORING] Monitoring stopped."; // Logs monitoring stop
        emit statusChanged("Monitoring stopped"); // Emits signal to indicate monitoring status change
    }
}

void WindowsMonitoring::checkForChanges() { // Checks for changes in registry keys
    // qDebug() << "[CHECK FOR CHANGES] Checking for changes in registry keys."; // Logs change check
    for (RegistryKey *key : m_registryKeys) { // Iterates through each registry key
        // qDebug() << "[DEBUG] Checking key:" << key->name(); // Logs the key being checked

        QString keyPath = key->keyPath();
        QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

        QString currentValue = key->getCurrentValue(); // Gets the current value of the registry key
        if (currentValue != key->value()) { // Checks if the value has changed
            qDebug() << "[DEBUG] Change detected for key:" << key->name() << ", Previous value:" << key->value() << ", Current value:" << currentValue;


            Database db;
            db.insertChange(key->name(), key->value(), currentValue, false);

            if (key->isCritical()) { // Checks if the key is marked as critical
                key->setRollbackCancelled(false); // Reset rollback cancellation for fresh change
                key->setNewValue(currentValue); // Sets new value for critical key
                m_rollback.rollbackIfNeeded(key); // Initiates rollback if needed

                QString alertMessage = "[CRITICAL ALERT] Key: " + key->name() + " changed. New value: " + currentValue;
                m_alert.sendAlert(alertMessage); // Sends email and SMS alert for critical changes

            } else {
                key->incrementChangeCount(); // Increments change count for non-critical key
                QString thresholdStr = m_settings->getNonCriticalAlertThreshold(); // Gets non-critical alert threshold
                int threshold = thresholdStr.isEmpty() ? 0 : thresholdStr.toInt(); // Converts threshold to integer or sets to 0

                // Additional debug outputs
                qDebug() << "[DEBUG] Current change count for key:" << key->name() << "is" << key->changeCount(); // Logs current change count
                qDebug() << "[DEBUG] Non-critical alert threshold is:" << threshold; // Logs non-critical alert threshold

                if (key->changeCount() >= threshold && threshold > 0) { // Checks if change count meets threshold
                    qDebug() << "[ALERT] Non-critical change threshold reached for key:" << key->name(); // Logs threshold reached

                    emit keyChanged(key->name(), currentValue); // Emits signal for key change
                    emit logMessage("[ALERT] Non-critical change threshold reached for " + key->name() + ": new value is " + currentValue); // Logs alert

                    // Send alert if settings are configured
                    QString alertMessage = "[ALERT] Non-critical change threshold reached for " + key->name() + ": new value is " + currentValue;
                    m_alert.sendAlert(alertMessage); // Sends alert message

                    key->resetChangeCount(); // Resets change count after alert
                    qDebug() << "[DEBUG] Change count reset for key:" << key->name(); // Logs reset
                }
                db.insertOrUpdateConfiguration(key->name(), keyPath, currentValue, false);
                key->setValue(currentValue); // Updates the key value
            }
        }
    }
}

void WindowsMonitoring::allowChange(const QString &keyName) {
    qDebug() << "[ALLOW CHANGE] Attempting to acknowledge change for key:" << keyName;

    Database db;
    // Query to verify if the change has already been acknowledged
    QVariantList changes = db.getAllChanges();
    bool alreadyAcknowledged = false;
    for (const QVariant &change : changes) {
        QVariantMap changeMap = change.toMap();
        if (changeMap["key_name"] == keyName && changeMap["acknowledged"].toBool()) {
            alreadyAcknowledged = true;
            break;
        }
    }

    if (!alreadyAcknowledged) {
        if (db.updateAcknowledgmentStatus(keyName)) {
            qDebug() << "[ALLOW CHANGE] Change acknowledged for key:" << keyName;
            emit changeAcknowledged(keyName);
        } else {
            qDebug() << "[ALLOW CHANGE] Failed to acknowledge change for key:" << keyName;
        }
    } else {
        qDebug() << "[ALLOW CHANGE] Change already acknowledged for key:" << keyName;
    }

    // Proceed with rollback cancellation regardless of acknowledgment status
    for (RegistryKey *key : m_registryKeys) {
        if (key->name() == keyName) {
            key->setRollbackCancelled(true);
            m_rollback.cancelRollback(key);
            key->setPreviousValue(key->newValue());
            return;
        }
    }
}


void WindowsMonitoring::setKeyCriticalStatus(const QString &keyName, bool isCritical) { // Sets critical status for a specified key
    // qDebug() << "setKeyCriticalStatus called for" << keyName << "with critical status" << isCritical; // Logs critical status change
    for (int i = 0; i < m_registryKeys.size(); ++i) { // Loops through registry keys to find the specified key
        if (m_registryKeys[i]->name() == keyName) { // Checks if key matches
            m_registryKeys[i]->setCritical(isCritical); // Sets the critical status
            QModelIndex modelIndex = m_registryKeysModel.index(i); // Gets model index for the key
            emit m_registryKeysModel.dataChanged(modelIndex, modelIndex, {RegistryKeyModel::DisplayTextRole}); // Emits data change for the model

            Database db;
            QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

            if (isCritical) { // Registers the key for rollback if it is critical
                m_rollback.registerKeyForRollback(m_registryKeys[i]);
                db.insertOrUpdateConfiguration(
                    m_registryKeys[i]->name(),
                    m_registryKeys[i]->keyPath(),
                    m_registryKeys[i]->value(),
                    true // isCritical
                    );
            } else {
                // Update ConfigurationSettings to mark the key as non-critical
                db.insertOrUpdateConfiguration(
                    m_registryKeys[i]->name(),
                    m_registryKeys[i]->keyPath(),
                    m_registryKeys[i]->value(),
                    false // isCritical
                    );
            }
            break; // Exits the loop once the key is found and updated
        }
    }
}

RegistryKeyModel* WindowsMonitoring::registryKeys() { // Returns the model containing registry keys
    return &m_registryKeysModel; // Returns a pointer to the registry keys model
}


