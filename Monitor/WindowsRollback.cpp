#include "WindowsRollback.h"
#include "Database.h"
#include <QDebug>
#include <QDateTime>

WindowsRollback::WindowsRollback(QObject *parent) : QObject(parent) {}

void WindowsRollback::rollbackIfNeeded(RegistryKey* key) {
    if (key) {
        QString previousValue = key->previousValue();
        QString keyPath = key->keyPath();
        QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

        if (key->isCritical() && key->getCurrentValue() != previousValue) {
            qDebug() << "[ROLLBACK] Restoring previous value for key:" << key->name();

            // Update key to previous value
            key->setNewValue(key->getCurrentValue());
            restorePreviousValue(key);

            Database db;
            db.insertOrUpdateConfiguration(key->name(), keyPath, previousValue, true);

            emit rollbackPerformed(key->name());
        }
    }
}
void WindowsRollback::registerKeyForRollback(RegistryKey* key) { // Registers a critical key for potential rollback
    if (key->isCritical()) { // Checks if the key is marked as critical
        qDebug() << "[ROLLBACK] Key registered for rollback:" << key->name(); // Logs key registration for rollback
    }
}

void WindowsRollback::cancelRollback(RegistryKey* key) { // Cancels the rollback for a specific key
    if (key) { // Checks if the key pointer is valid
        key->setRollbackCancelled(true); // Marks rollback as cancelled for this key
        if (!key->newValue().isEmpty()) { // Checks if a new value has been set
            key->setValue(key->newValue()); // Sets the registry key to the new value
            key->settings()->sync(); // Ensures the change is written to the registry

            // Add debug validation step
            QString confirmedValue = key->getCurrentValue(); // Reads the confirmed value from the registry
            if (confirmedValue == key->newValue()) { // Verifies that the registry was updated correctly
                qDebug() << "[CANCEL ROLLBACK] Value set to new value for:" << key->name(); // Logs successful value update
                qDebug() << "[CONFIRMATION] Registry value successfully updated to:" << confirmedValue; // Logs confirmation of the update
            } else {
                qWarning() << "[CONFIRMATION FAILURE] Registry value did not update as expected for key:" << key->name(); // Warns if update failed
                qDebug() << "Expected new value:" << key->newValue() << ", but current value found:" << confirmedValue; // Logs expected vs. actual values
            }
        } else {
            qDebug() << "[CANCEL ROLLBACK] No new value stored, no change made for key:" << key->name(); // Logs if no new value was available
        }
    }
}

void WindowsRollback::restorePreviousValue(RegistryKey* key) {
    QString previousValue = key->previousValue();

    if (key->getCurrentValue() != previousValue) {
        key->settings()->setValue(key->valueName(), previousValue);
        key->settings()->sync();

        QString confirmedValue = key->getCurrentValue();
        if (confirmedValue == previousValue) {
            qDebug() << "[ROLLBACK] Successfully restored key:" << key->name() << "to previous value.";
        } else {
            qWarning() << "[ROLLBACK] Failed to restore key:" << key->name() << "to previous value.";
        }
    }
}
