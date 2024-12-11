#include "MacOSRollback.h" // Includes the Rollback class definition
#include <QDebug> // Enables logging and debugging output

MacOSRollback::MacOSRollback(QObject *parent) : QObject(parent) {} // Constructor for Rollback, initializing QObject with an optional parent

void MacOSRollback::plistFileForRollback(PlistFile* file) { // Registers a critical key for potential rollback
    if (file->isCritical()) { // Checks if the key is marked as critical
        qDebug() << "[ROLLBACK] Key registered for rollback:" << file->name(); // Logs key registration for rollback
    }
}

void MacOSRollback::cancelRollback(PlistFile* file) { // Cancels the rollback for a specific key
    if (file) { // Checks if the key pointer is valid
        file->setRollbackCancelled(true); // Marks rollback as cancelled for this key
        if (!file->newValue().isEmpty()) { // Checks if a new value has been set
            file->setValue(file->newValue()); // Sets the registry key to the new value
            file->settings()->sync(); // Ensures the change is written to the registry

            // Add debug validation step
            QString confirmedValue = file->getCurrentValue(); // Reads the confirmed value from the registry
            if (confirmedValue == file->newValue()) { // Verifies that the registry was updated correctly
                qDebug() << "[CANCEL ROLLBACK] Value set to new value for:" << file->name(); // Logs successful value update
                qDebug() << "[CONFIRMATION] Registry value successfully updated to:" << confirmedValue; // Logs confirmation of the update
            } else {
                qWarning() << "[CONFIRMATION FAILURE] Registry value did not update as expected for key:" << file->name(); // Warns if update failed
                qDebug() << "Expected new value:" << file->newValue() << ", but current value found:" << confirmedValue; // Logs expected vs. actual values
            }
        } else {
            qDebug() << "[CANCEL ROLLBACK] No new value stored, no change made for key:" << file->name(); // Logs if no new value was available
        }
    }
}

void MacOSRollback::rollbackIfNeeded(PlistFile* file) { // Checks if a rollback is needed for a specific key
    if (file) { // Checks if the file pointer is valid
        if (file->isRollbackCancelled()) { // Checks if rollback has been cancelled
            qDebug() << "[INFO] Rollback cancelled for key:" << file->name(); // Logs rollback cancellation
            file->setValue(file->newValue()); // Sets the key to the new value instead
            file->setRollbackCancelled(false); // Resets the rollback cancellation status for future operations
        } else if (file->isCritical() && file->getCurrentValue() != file->previousValue()) { // Checks if rollback is needed for a critical key
            qDebug() << "[ROLLBACK NEEDED] Restoring previous value for key:" << file->valueName(); // Logs the need for a rollback
            file->setNewValue(file->getCurrentValue()); // Stores the current value before rollback
            restorePreviousValue(file); // Calls function to restore the previous value
            emit rollbackPerformed(file->valueName()); // Emits signal indicating a rollback was performed
            file->setRollbackCancelled(false); // Ensure rollback reset
        }
    }
}

void MacOSRollback::restorePreviousValue(PlistFile* file) { // Restores a key to its previous value
    QString previousValue = file->previousValue(); // Retrieves the previous value for the key

    if (file->getCurrentValue() != previousValue) { // Checks if the current value differs from the previous value
        file->settings()->setValue(file->valueName(), previousValue); // Sets the registry key to the previous value
        file->settings()->sync(); // Ensures the change is committed to the registry

        QString confirmedValue = file->getCurrentValue(); // Reads back the confirmed value
        if (confirmedValue == previousValue) { // Verifies that the registry was updated correctly
            qDebug() << "[ROLLBACK] Successfully restored previous value for key:" << file->name() << "to" << previousValue; // Logs successful restoration
        } else {
            qWarning() << "[ROLLBACK FAILURE] Registry did not update correctly for key:" << file->name(); // Warns if the rollback failed
        }
    } else {
        qDebug() << "[ROLLBACK] No restoration needed; current value matches previous value for key:" << file->name(); // Logs if no restoration was needed
    }
}
