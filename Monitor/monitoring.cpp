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
    connect(&m_rollback, &Rollback::rollbackPerformed, this, [this](const QString &keyName) {
        QString alertMessage = "[CRITICAL] Rollback performed for key: " + keyName;
        emit criticalChangeDetected(alertMessage);
        qDebug() << "[MONITORING] Emitting criticalChangeDetected signal with message:" << alertMessage;
        m_alert.sendAlert(alertMessage);
    });

    // Construct the absolute path to the monitoredKeys.json file in the project directory
    QString projectDirPath = QCoreApplication::applicationDirPath();
    QString filePath = QDir(projectDirPath).filePath("../../monitoredKeys.json");

    // Normalize the path to ensure it points to the intended directory
    filePath = QDir::cleanPath(filePath);

    // Check if the monitoredKeys.json file exists at the constructed path
    if (QFile::exists(filePath)) {
        QList<RegistryKey*> registryKeys = JsonUtils::readKeysFromJson(filePath);
        m_registryKeys = registryKeys;
        m_registryKeysModel.setRegistryKeys(m_registryKeys);
        emit registryKeysChanged();
        qDebug() << "[MONITORING INIT] Successfully loaded monitoredKeys.json from:" << filePath;
    } else {
        qWarning() << "[MONITORING INIT] monitoredKeys.json file not found at:" << filePath;
    }

    if (m_settings) {
        qDebug() << "[MONITORING INIT] Initial email set to:" << m_settings->getEmail();
        qDebug() << "[MONITORING INIT] Initial phone number set to:" << m_settings->getPhoneNumber();
    } else {
        qWarning() << "[MONITORING INIT] Settings object is null";
    }
}


void Monitoring::startMonitoring() {
    if (!m_monitoringActive) {
        m_monitoringActive = true;
        m_timer.start(1000);
        qDebug() << "[START MONITORING] Monitoring started.";
        emit statusChanged("Monitoring started");
    }
}

void Monitoring::stopMonitoring() {
    if (m_monitoringActive) {
        m_monitoringActive = false;
        m_timer.stop();
        qDebug() << "[STOP MONITORING] Monitoring stopped.";
        emit statusChanged("Monitoring stopped");
    }
}

void Monitoring::checkForChanges() {
    qDebug() << "[CHECK FOR CHANGES] Checking for changes in registry keys.";
    for (RegistryKey *key : m_registryKeys) {
        qDebug() << "[DEBUG] Checking key:" << key->name();  // Log the key being checked

        QString currentValue = key->getCurrentValue();
        if (currentValue != key->value()) {
            qDebug() << "[DEBUG] Change detected for key:" << key->name() << ", Previous value:" << key->value() << ", Current value:" << currentValue;

            if (key->isCritical()) {
                key->setNewValue(currentValue);
                qDebug() << "[CRITICAL CHANGE DETECTED] Storing new value and emitting signal for key:" << key->name();
                m_rollback.rollbackIfNeeded(key);
            } else {
                key->incrementChangeCount();
                QString thresholdStr = m_settings->getNonCriticalAlertThreshold();
                int threshold = thresholdStr.isEmpty() ? 0 : thresholdStr.toInt();

                // Additional debug outputs
                qDebug() << "[DEBUG] Current change count for key:" << key->name() << "is" << key->changeCount();
                qDebug() << "[DEBUG] Non-critical alert threshold is:" << threshold;

                if (key->changeCount() >= threshold && threshold > 0) {
                    qDebug() << "[ALERT] Non-critical change threshold reached for key:" << key->name();

                    emit keyChanged(key->name(), currentValue);
                    emit logMessage("[ALERT] Non-critical change threshold reached for " + key->name() + ": new value is " + currentValue);

                    // Send alert if settings are configured
                    QString alertMessage = "[ALERT] Non-critical change threshold reached for " + key->name() + ": new value is " + currentValue;
                    m_alert.sendAlert(alertMessage);

                    key->resetChangeCount();  // Reset after alerting
                    qDebug() << "[DEBUG] Change count reset for key:" << key->name();
                }
                key->setValue(currentValue);
            }
        }
    }
}

void Monitoring::allowChange(const QString &keyName) {
    qDebug() << "[ALLOW CHANGE] Attempting to cancel rollback for key:" << keyName;
    for (RegistryKey *key : m_registryKeys) {
        if (key->name() == keyName) {
            qDebug() << "[ALLOW CHANGE] Key found, calling cancelRollback for:" << key->name();
            key->setRollbackCancelled(true);
            m_rollback.cancelRollback(key);
            key->setValue(key->newValue());
            return;
        }
    }
    qDebug() << "[ALLOW CHANGE] Key not found:" << keyName;
}

void Monitoring::setKeyCriticalStatus(const QString &keyName, bool isCritical) {
    qDebug() << "setKeyCriticalStatus called for" << keyName << "with critical status" << isCritical;
    for (int i = 0; i < m_registryKeys.size(); ++i) {
        if (m_registryKeys[i]->name() == keyName) {
            m_registryKeys[i]->setCritical(isCritical);
            QModelIndex modelIndex = m_registryKeysModel.index(i);
            emit m_registryKeysModel.dataChanged(modelIndex, modelIndex, {RegistryKeyModel::DisplayTextRole});
            if (isCritical) {
                m_rollback.registerKeyForRollback(m_registryKeys[i]);
            }
            break;
        }
    }
}

RegistryKeyModel* Monitoring::registryKeys() {
    return &m_registryKeysModel;
}
