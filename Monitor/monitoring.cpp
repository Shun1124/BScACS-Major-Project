#include "monitoring.h"
#include "jsonUtils.h"
#include <QDebug>
#include <QDir>
#include <QCoreApplication>

// Constructor for the Monitoring class
Monitoring::Monitoring(Settings *settings, QObject *parent)
    : QObject(parent), m_monitoringActive(false), m_settings(settings), m_alert(settings, this) {
    connect(&m_timer, &QTimer::timeout, this, &Monitoring::checkForChanges);

    // Connect rollbackPerformed signal to send alerts when a rollback is performed
    connect(&m_rollback, &Rollback::rollbackPerformed, this, [this](const QString &valueName) {
        QString alertMessage = "[CRITICAL] Rollback performed for key: " + valueName; // Constructs alert message
        emit criticalChangeDetected(alertMessage);
        // qDebug() << "[MONITORING] Emitting criticalChangeDetected signal with message:" << alertMessage;
        m_alert.sendAlert(alertMessage);
    });

    // Construct the absolute path to the monitoredKeys.json file in the project directory
    QString projectDirPath = QCoreApplication::applicationDirPath();
    QString filePath = QDir(projectDirPath).filePath("../../monitoredKeys.json");

    // Normalize the path to ensure it points to the intended directory
    filePath = QDir::cleanPath(filePath);

    // Check if the monitoredKeys.json file exists at the constructed path
    if (QFile::exists(filePath)) { // Checks if the file exists
        QList<RegistryKey*> registryKeys = JsonUtils::readKeysFromJson(filePath);
        m_registryKeys = registryKeys;
        m_registryKeysModel.setRegistryKeys(m_registryKeys);
        emit registryKeysChanged();
        qDebug() << "[MONITORING INIT] Successfully loaded monitoredKeys.json from:" << filePath;
    } else {
        qWarning() << "[MONITORING INIT] monitoredKeys.json file not found at:" << filePath;
    }

    if (m_settings) { // Checks if settings object is initialized
        qDebug() << "[MONITORING INIT] Initial email set to:" << m_settings->getEmail();
        qDebug() << "[MONITORING INIT] Initial phone number set to:" << m_settings->getPhoneNumber();
    } else {
        qWarning() << "[MONITORING INIT] Settings object is null";
    }
}

void Monitoring::startMonitoring() { // Starts the monitoring process
    if (!m_monitoringActive) { // Checks if monitoring is inactive
        m_monitoringActive = true;
        m_timer.start(1000);
        qDebug() << "[START MONITORING] Monitoring started.";
        emit statusChanged("Monitoring started");
    }
}

void Monitoring::stopMonitoring() { // Stops the monitoring process
    if (m_monitoringActive) { // Checks if monitoring is active
        m_monitoringActive = false; // Sets monitoring to inactive
        m_timer.stop(); // Stops the timer
        qDebug() << "[STOP MONITORING] Monitoring stopped."; // Logs monitoring stop
        emit statusChanged("Monitoring stopped");
    }
}

void Monitoring::checkForChanges() { // Checks for changes in registry keys
    // qDebug() << "[CHECK FOR CHANGES] Checking for changes in registry keys."; // Logs change check
    for (RegistryKey *key : m_registryKeys) { // Iterates through each registry key
        // qDebug() << "[DEBUG] Checking key:" << key->name(); // Logs the key being checked

        QString currentValue = key->getCurrentValue(); // Gets the current value of the registry key
        if (currentValue != key->value()) { // Checks if the value has changed
            qDebug() << "[DEBUG] Change detected for key:" << key->name() << ", Previous value:" << key->value() << ", Current value:" << currentValue;

            if (key->isCritical()) { // Checks if the key is marked as critical
                key->setRollbackCancelled(false);
                key->setNewValue(currentValue);
                m_rollback.rollbackIfNeeded(key);
            } else {
                key->incrementChangeCount(); // Increments change count for non-critical key
                QString thresholdStr = m_settings->getNonCriticalAlertThreshold();
                int threshold = thresholdStr.isEmpty() ? 0 : thresholdStr.toInt();

                // Additional debug outputs
                qDebug() << "[DEBUG] Current change count for key:" << key->name() << "is" << key->changeCount();
                qDebug() << "[DEBUG] Non-critical alert threshold is:" << threshold;

                if (key->changeCount() >= threshold && threshold > 0) { // Checks if change count meets threshold
                    qDebug() << "[ALERT] Non-critical change threshold reached for key:" << key->name();

                    emit keyChanged(key->name(), currentValue);
                    emit logMessage("[ALERT] Non-critical change threshold reached for " + key->name() + ": new value is " + currentValue);

                    // Send alert if settings are configured
                    QString alertMessage = "[ALERT] Non-critical change threshold reached for " + key->name() + ": new value is " + currentValue;
                    m_alert.sendAlert(alertMessage);

                    key->resetChangeCount();
                    qDebug() << "[DEBUG] Change count reset for key:" << key->name();
                }
                key->setValue(currentValue);
            }
        }
    }
}

void Monitoring::allowChange(const QString &keyName) { // Allows change for a specified key by cancelling rollback
    qDebug() << "[ALLOW CHANGE] Attempting to cancel rollback for key:" << keyName;
    for (RegistryKey *key : m_registryKeys) { // Searches for the specified key
        if (key->name() == keyName) { // Checks if the key matches
            qDebug() << "[ALLOW CHANGE] Key found, calling cancelRollback for:" << key->name();
            key->setRollbackCancelled(true);
            m_rollback.cancelRollback(key);
            key->setPreviousValue(key->newValue());
            return;
        }
    }
}

void Monitoring::setKeyCriticalStatus(const QString &keyName, bool isCritical) { // Sets critical status for a specified key
    // qDebug() << "setKeyCriticalStatus called for" << keyName << "with critical status" << isCritical; // Logs critical status change
    for (int i = 0; i < m_registryKeys.size(); ++i) { // Loops through registry keys to find the specified key
        if (m_registryKeys[i]->name() == keyName) { // Checks if key matches
            m_registryKeys[i]->setCritical(isCritical);
            QModelIndex modelIndex = m_registryKeysModel.index(i);
            emit m_registryKeysModel.dataChanged(modelIndex, modelIndex, {RegistryKeyModel::DisplayTextRole});
            if (isCritical) { // Registers the key for rollback if it is critical
                m_rollback.registerKeyForRollback(m_registryKeys[i]);
            }
            break;
        }
    }
}

RegistryKeyModel* Monitoring::registryKeys() { // Returns the model containing registry keys
    return &m_registryKeysModel;
}
