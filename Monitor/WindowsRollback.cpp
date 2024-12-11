#include "WindowsRollback.h" // Includes the Rollback class definition
#include <QDebug> // Enables logging and debugging output

WindowsRollback::WindowsRollback(QObject *parent) : QObject(parent) {} // Constructor for Rollback, initializing QObject with an optional parent

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

void WindowsRollback::rollbackIfNeeded(RegistryKey* key) { // Checks if a rollback is needed for a specific key
    if (key) { // Checks if the key pointer is valid
        if (key->isRollbackCancelled()) { // Checks if rollback has been cancelled
            qDebug() << "[INFO] Rollback cancelled for key:" << key->name(); // Logs rollback cancellation
            key->setValue(key->newValue()); // Sets the key to the new value instead
            key->setRollbackCancelled(false); // Resets the rollback cancellation status for future operations
        } else if (key->isCritical() && key->getCurrentValue() != key->previousValue()) { // Checks if rollback is needed for a critical key
            qDebug() << "[ROLLBACK NEEDED] Restoring previous value for key:" << key->valueName(); // Logs the need for a rollback
            key->setNewValue(key->getCurrentValue()); // Stores the current value before rollback
            restorePreviousValue(key); // Calls function to restore the previous value
            emit rollbackPerformed(key->valueName()); // Emits signal indicating a rollback was performed
            key->setRollbackCancelled(false); // Ensure rollback reset
        }
    }
}

void WindowsRollback::restorePreviousValue(RegistryKey* key) { // Restores a key to its previous value
    QString previousValue = key->previousValue(); // Retrieves the previous value for the key

    if (key->getCurrentValue() != previousValue) { // Checks if the current value differs from the previous value
        key->settings()->setValue(key->valueName(), previousValue); // Sets the registry key to the previous value
        key->settings()->sync(); // Ensures the change is committed to the registry

        QString confirmedValue = key->getCurrentValue(); // Reads back the confirmed value
        if (confirmedValue == previousValue) { // Verifies that the registry was updated correctly
            qDebug() << "[ROLLBACK] Successfully restored previous value for key:" << key->name() << "to" << previousValue; // Logs successful restoration
        } else {
            qWarning() << "[ROLLBACK FAILURE] Registry did not update correctly for key:" << key->name(); // Warns if the rollback failed
        }
    } else {
        qDebug() << "[ROLLBACK] No restoration needed; current value matches previous value for key:" << key->name(); // Logs if no restoration was needed
    }
}
