#include "WindowsMonitoring.h" // Includes the Monitoring class definition
#include "WindowsJsonUtils.h" // Includes JSON utilities for reading data from JSON files
#include <QDebug> // Enables logging and debugging output
#include <QDir> // Provides directory handling capabilities
#include <QCoreApplication> // Provides application-level information like directory paths

// Constructor for the Monitoring class
WindowsMonitoring::WindowsMonitoring(Settings *settings, QObject *parent)
    : QObject(parent), m_monitoringActive(false), m_settings(settings), m_alert(settings, this) { // Initializes Monitoring with settings and alert
    connect(&m_timer, &QTimer::timeout, this, &WindowsMonitoring::checkForChanges); // Connects timer timeout signal to checkForChanges slot

    // Connect rollbackPerformed signal to send alerts when a rollback is performed
    connect(&m_rollback, &WindowsRollback::rollbackPerformed, this, [this](const QString &valueName) { // Lambda function for rollback alerts
        QString alertMessage = "[CRITICAL] Rollback performed for key: " + valueName; // Constructs alert message
        emit criticalChangeDetected(alertMessage); // Emits signal for critical change
        // qDebug() << "[MONITORING] Emitting criticalChangeDetected signal with message:" << alertMessage; // Logs emitted message
        m_alert.sendAlert(alertMessage); // Sends alert message
    });


    QString filePath = QDir::cleanPath(QCoreApplication::applicationDirPath() + "/../monitoredKeys.json");

    // Check if the monitoredConfigs.json file exists at the constructed path
    if (QFile::exists(filePath)) { // Checks if the file exists
        QList<RegistryKey*> registryKeys = WindowsJsonUtils::readKeysFromJson(filePath); // Reads registry keys from JSON
        m_registryKeys = registryKeys; // Stores registry keys in m_registryKeys
        m_registryKeysModel.setRegistryKeys(m_registryKeys); // Updates the model with registry keys
        emit registryKeysChanged(); // Emits signal for registry keys update
        qDebug() << "[MONITORING INIT] Successfully loaded monitoredConfigs.json from:" << filePath; // Logs success
    } else {
        qWarning() << "[MONITORING INIT] monitoredConfigs.json file not found at:" << filePath; // Warns if file not found
    }

    if (m_settings) { // Checks if settings object is initialized
        qDebug() << "[MONITORING INIT] Initial email set to:" << m_settings->getEmail(); // Logs initial email
        qDebug() << "[MONITORING INIT] Initial phone number set to:" << m_settings->getPhoneNumber(); // Logs initial phone number
    } else {
        qWarning() << "[MONITORING INIT] Settings object is null"; // Warns if settings object is null
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

        QString currentValue = key->getCurrentValue(); // Gets the current value of the registry key
        if (currentValue != key->value()) { // Checks if the value has changed
            qDebug() << "[DEBUG] Change detected for key:" << key->name() << ", Previous value:" << key->value() << ", Current value:" << currentValue;

            if (key->isCritical()) { // Checks if the key is marked as critical
                key->setRollbackCancelled(false); // Reset rollback cancellation for fresh change
                key->setNewValue(currentValue); // Sets new value for critical key
                m_rollback.rollbackIfNeeded(key); // Initiates rollback if needed
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
                key->setValue(currentValue); // Updates the key value
            }
        }
    }
}

void WindowsMonitoring::allowChange(const QString &keyName) { // Allows change for a specified key by cancelling rollback
    qDebug() << "[ALLOW CHANGE] Attempting to cancel rollback for key:" << keyName; // Logs attempt to cancel rollback
    for (RegistryKey *key : m_registryKeys) { // Searches for the specified key
        if (key->name() == keyName) { // Checks if the key matches
            qDebug() << "[ALLOW CHANGE] Key found, calling cancelRollback for:" << key->name(); // Logs key found
            key->setRollbackCancelled(true); // Marks rollback as cancelled for this key
            m_rollback.cancelRollback(key); // Cancels rollback for this key
            key->setPreviousValue(key->newValue()); // Sets the new value for the key
            return; // Exits the function after processing
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
            if (isCritical) { // Registers the key for rollback if it is critical
                m_rollback.registerKeyForRollback(m_registryKeys[i]);
            }
            break; // Exits the loop once the key is found and updated
        }
    }
}

RegistryKeyModel* WindowsMonitoring::registryKeys() { // Returns the model containing registry keys
    return &m_registryKeysModel; // Returns a pointer to the registry keys model
}