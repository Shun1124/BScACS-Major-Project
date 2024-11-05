#include "rollback.h"
#include <QDebug>


Rollback::Rollback(QObject *parent) : QObject(parent) {}

void Rollback::registerKeyForRollback(RegistryKey* key) {
    if (key->isCritical()) {
        qDebug() << "[ROLLBACK] Key registered for rollback:" << key->name();
    }
}

void Rollback::cancelRollback(RegistryKey* key) {
    if (key) {
        key->setRollbackCancelled(true);
        if (!key->newValue().isEmpty()) {
            key->setValue(key->newValue());
            key->settings()->sync();

            // Add debug validation step
            QString confirmedValue = key->getCurrentValue();
            if (confirmedValue == key->newValue()) {
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

void Rollback::rollbackIfNeeded(RegistryKey* key) {
    if (key) {
        if (key->isRollbackCancelled()) {
            qDebug() << "[INFO] Rollback cancelled for key:" << key->name();
            // Revert to new value after rollback cancellation
            key->setValue(key->newValue()); // Set the previously stored new value
            key->setRollbackCancelled(false); // Reset the cancellation status for future use
        } else if (key->isCritical() && key->getCurrentValue() != key->previousValue()) {
            qDebug() << "[ROLLBACK NEEDED] Restoring previous value for key:" << key->name();
            // Store the new value before rolling back
            key->setNewValue(key->getCurrentValue()); // Add a method in RegistryKey for this
            restorePreviousValue(key);
            emit rollbackPerformed(key->name());
        }
    }
}

void Rollback::restorePreviousValue(RegistryKey* key) {
    QString previousValue = key->previousValue();

    if (key->getCurrentValue() != previousValue) {
        key->settings()->setValue(key->valueName(), previousValue);
        key->settings()->sync();

        QString confirmedValue = key->getCurrentValue();
        if (confirmedValue == previousValue) {
            qDebug() << "[ROLLBACK] Successfully restored previous value for key:" << key->name() << "to" << previousValue;
        } else {
            qWarning() << "[ROLLBACK FAILURE] Registry did not update correctly for key:" << key->name();
        }
    } else {
        qDebug() << "[ROLLBACK] No restoration needed; current value matches previous value for key:" << key->name();
    }
}
