#include "rollback.h"
#include <QDebug>

Rollback::Rollback(QObject *parent) : QObject(parent) {} // Constructor for Rollback, initializing QObject with an optional parent

void Rollback::registerKeyForRollback(RegistryKey* key) { // Registers a critical key for potential rollback
    if (key->isCritical()) { // Checks if the key is marked as critical
        qDebug() << "[ROLLBACK] Key registered for rollback:" << key->name();
    }
}

void Rollback::cancelRollback(RegistryKey* key) { // Cancels the rollback for a specific key
    if (key) { // Checks if the key pointer is valid
        key->setRollbackCancelled(true); // Marks rollback as cancelled for this key
        if (!key->newValue().isEmpty()) { // Checks if a new value has been set
            key->setValue(key->newValue());
            key->settings()->sync();

            // Add debug validation step
            QString confirmedValue = key->getCurrentValue(); // Reads the confirmed value from the registry
            if (confirmedValue == key->newValue()) { // Verifies that the registry was updated correctly
                qDebug() << "[CANCEL ROLLBACK] Value set to new value for:" << key->name();
                qDebug() << "[CONFIRMATION] Registry value successfully updated to:" << confirmedValue;
            } else {
                qWarning() << "[CONFIRMATION FAILURE] Registry value did not update as expected for key:" << key->name();
                qDebug() << "Expected new value:" << key->newValue() << ", but current value found:" << confirmedValue;
            }
        } else {
            qDebug() << "[CANCEL ROLLBACK] No new value stored, no change made for key:" << key->name();
        }
    }
}

void Rollback::rollbackIfNeeded(RegistryKey* key) { // Checks if a rollback is needed for a specific key
    if (key) { // Checks if the key pointer is valid
        if (key->isRollbackCancelled()) { // Checks if rollback has been cancelled
            qDebug() << "[INFO] Rollback cancelled for key:" << key->name();
            key->setValue(key->newValue());
            key->setRollbackCancelled(false);
        } else if (key->isCritical() && key->getCurrentValue() != key->previousValue()) { // Checks if rollback is needed for a critical key
            qDebug() << "[ROLLBACK NEEDED] Restoring previous value for key:" << key->valueName();
            key->setNewValue(key->getCurrentValue());
            restorePreviousValue(key);
            emit rollbackPerformed(key->valueName());
            key->setRollbackCancelled(false);
        }
    }
}

void Rollback::restorePreviousValue(RegistryKey* key) { // Restores a key to its previous value
    QString previousValue = key->previousValue();

    if (key->getCurrentValue() != previousValue) { // Checks if the current value differs from the previous value
        key->settings()->setValue(key->valueName(), previousValue);
        key->settings()->sync();

        QString confirmedValue = key->getCurrentValue(); // Reads back the confirmed value
        if (confirmedValue == previousValue) { // Verifies that the registry was updated correctly
            qDebug() << "[ROLLBACK] Successfully restored previous value for key:" << key->name() << "to" << previousValue;
        } else {
            qWarning() << "[ROLLBACK FAILURE] Registry did not update correctly for key:" << key->name();
        }
    } else {
        qDebug() << "[ROLLBACK] No restoration needed; current value matches previous value for key:" << key->name();
    }
}
