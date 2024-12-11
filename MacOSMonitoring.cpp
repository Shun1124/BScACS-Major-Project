#include "MacOSMonitoring.h" // Includes the Monitoring class definition
#include "MacOSJsonUtils.h" // Includes JSON utilities for reading data from JSON files
#include <QDebug> // Enables logging and debugging output
#include <QDir> // Provides directory handling capabilities
#include <QCoreApplication> // Provides application-level information like directory paths

// Constructor for the Monitoring class
MacOSMonitoring::MacOSMonitoring(Settings *settings, QObject *parent)
    : QObject(parent), m_monitoringActive(false), m_settings(settings), m_alert(settings, this) { // Initializes Monitoring with settings and alert
    connect(&m_timer, &QTimer::timeout, this, &MacOSMonitoring::checkForChanges); // Connects timer timeout signal to checkForChanges slot

    // Connect rollbackPerformed signal to send alerts when a rollback is performed
    connect(&m_rollback, &MacOSRollback::rollbackPerformed, this, [this](const QString &valueName) { // Lambda function for rollback alerts
        QString alertMessage = "[CRITICAL] Rollback performed for file: " + valueName; // Constructs alert message
        emit criticalChangeDetected(alertMessage); // Emits signal for critical change
        // qDebug() << "[MONITORING] Emitting criticalChangeDetected signal with message:" << alertMessage; // Logs emitted message
        m_alert.sendAlert(alertMessage); // Sends alert message
    });


    QString filePath = QDir::cleanPath(QCoreApplication::applicationDirPath() + "/../../../../../monitoredPlists.json");

    // Check if the monitoredConfigs.json file exists at the constructed path
    if (QFile::exists(filePath)) { // Checks if the file exists
        QList<PlistFile*> plistFiles = MacOSJsonUtils::readFilesFromJson(filePath); // Reads registry keys from JSON
        m_plistFiles = plistFiles; // Stores registry keys in m_plistFiles
        m_plistFilesModel.setPlistFiles(m_plistFiles); // Updates the model with registry keys
        qDebug() << "[DEBUG] Plist fiels cout in model:" << m_plistFiles.size();
        emit plistFilesChanged(); // Emits signal for registry keys update
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

void MacOSMonitoring::startMonitoring() { // Starts the monitoring process
    if (!m_monitoringActive) { // Checks if monitoring is inactive
        m_monitoringActive = true; // Sets monitoring to active
        m_timer.start(1000); // Starts the timer with a 1-second interval
        qDebug() << "[START MONITORING] Monitoring started."; // Logs monitoring start
        emit statusChanged("Monitoring started"); // Emits signal to indicate monitoring status change
    }
}

void MacOSMonitoring::stopMonitoring() { // Stops the monitoring process
    if (m_monitoringActive) { // Checks if monitoring is active
        m_monitoringActive = false; // Sets monitoring to inactive
        m_timer.stop(); // Stops the timer
        qDebug() << "[STOP MONITORING] Monitoring stopped."; // Logs monitoring stop
        emit statusChanged("Monitoring stopped"); // Emits signal to indicate monitoring status change
    }
}

void MacOSMonitoring::checkForChanges() { // Checks for changes in registry files
    // qDebug() << "[CHECK FOR CHANGES] Checking for changes in registry files."; // Logs change check
    for (PlistFile *file : m_plistFiles) { // Iterates through each registry file
        // qDebug() << "[DEBUG] Checking file:" << file->name(); // Logs the file being checked

        QString currentValue = file->getCurrentValue(); // Gets the current value of the registry file
        if (currentValue != file->value()) { // Checks if the value has changed
            qDebug() << "[DEBUG] Change detected for file:" << file->name() << ", Previous value:" << file->value() << ", Current value:" << currentValue;

            if (file->isCritical()) { // Checks if the file is marked as critical
                file->setRollbackCancelled(false); // Reset rollback cancellation for fresh change
                file->setNewValue(currentValue); // Sets new value for critical file
                m_rollback.rollbackIfNeeded(file); // Initiates rollback if needed
            } else {
                file->incrementChangeCount(); // Increments change count for non-critical file
                QString thresholdStr = m_settings->getNonCriticalAlertThreshold(); // Gets non-critical alert threshold
                int threshold = thresholdStr.isEmpty() ? 0 : thresholdStr.toInt(); // Converts threshold to integer or sets to 0

                // Additional debug outputs
                qDebug() << "[DEBUG] Current change count for file:" << file->name() << "is" << file->changeCount(); // Logs current change count
                qDebug() << "[DEBUG] Non-critical alert threshold is:" << threshold; // Logs non-critical alert threshold

                if (file->changeCount() >= threshold && threshold > 0) { // Checks if change count meets threshold
                    qDebug() << "[ALERT] Non-critical change threshold reached for file:" << file->name(); // Logs threshold reached

                    emit fileChanged(file->name(), currentValue); // Emits signal for file change
                    emit logMessage("[ALERT] Non-critical change threshold reached for " + file->name() + ": new value is " + currentValue); // Logs alert

                    // Send alert if settings are configured
                    QString alertMessage = "[ALERT] Non-critical change threshold reached for " + file->name() + ": new value is " + currentValue;
                    m_alert.sendAlert(alertMessage); // Sends alert message

                    file->resetChangeCount(); // Resets change count after alert
                    qDebug() << "[DEBUG] Change count reset for file:" << file->name(); // Logs reset
                }
                file->setValue(currentValue); // Updates the file value
            }
        }
    }
}

void MacOSMonitoring::allowChange(const QString &fileName) { // Allows change for a specified file by cancelling rollback
    qDebug() << "[ALLOW CHANGE] Attempting to cancel rollback for file:" << fileName; // Logs attempt to cancel rollback
    for (PlistFile *file : m_plistFiles) { // Searches for the specified file
        if (file->name() == fileName) { // Checks if the file matches
            qDebug() << "[ALLOW CHANGE] file found, calling cancelRollback for:" << file->name(); // Logs file found
            file->setRollbackCancelled(true); // Marks rollback as cancelled for this file
            m_rollback.cancelRollback(file); // Cancels rollback for this file
            file->setPreviousValue(file->newValue()); // Sets the new value for the file
            return; // Exits the function after processing
        }
    }
}

void MacOSMonitoring::setFileCriticalStatus(const QString &fileName, bool isCritical) { // Sets critical status for a specified file
    // qDebug() << "setfileCriticalStatus called for" << fileName << "with critical status" << isCritical; // Logs critical status change
    for (int i = 0; i < m_plistFiles.size(); ++i) { // Loops through registry files to find the specified file
        if (m_plistFiles[i]->name() == fileName) { // Checks if file matches
            m_plistFiles[i]->setCritical(isCritical); // Sets the critical status
            QModelIndex modelIndex = m_plistFilesModel.index(i); // Gets model index for the file
            emit m_plistFilesModel.dataChanged(modelIndex, modelIndex, {PlistFileModel::DisplayTextRole}); // Emits data change for the model
            if (isCritical) { // Registers the file for rollback if it is critical
                m_rollback.plistFileForRollback(m_plistFiles[i]);
            }
            break; // Exits the loop once the file is found and updated
        }
    }
}

PlistFileModel* MacOSMonitoring::plistFiles() { // Returns the model containing registry files
    return &m_plistFilesModel; // Returns a pointer to the registry files model
}
