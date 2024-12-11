#include "registryKey.h" // Includes the RegistryKey class definition
#include <QDebug> // Enables logging and debugging output

RegistryKey::RegistryKey(const QString &hive, const QString &keyPath, const QString &valueName, bool isCritical, QObject *parent)
    : QObject(parent), m_hive(hive), m_keyPath(keyPath), m_valueName(valueName), m_isCritical(isCritical) { // Constructor initializes registry key properties
    QString fullKey = (m_hive == "HKEY_CURRENT_USER" ? "HKEY_CURRENT_USER\\" : "HKEY_LOCAL_MACHINE\\") + m_keyPath; // Sets full registry key path
    m_settings = new QSettings(fullKey, QSettings::NativeFormat); // Creates QSettings object to interact with registry
    m_value = readCurrentValue(); // Reads the current value from the registry
    m_previousValue = m_value; // Initializes previous value as the current value
    updateDisplayText(); // Updates the display text based on critical status
    qDebug() << "[INIT] Key:" << m_valueName << ", Initial Value:" << m_value; // Logs initialization details
}

bool RegistryKey::isRollbackCancelled() const { // Returns whether rollback is cancelled for this key
    return m_rollbackCancelled;
}

void RegistryKey::setRollbackCancelled(bool cancelled) { // Sets the rollback cancelled status
    m_rollbackCancelled = cancelled;
}

void RegistryKey::updateDisplayText() { // Updates the display text shown for this registry key
    m_displayText = m_isCritical ? m_valueName + " - Critical" : m_valueName; // Adds " - Critical" if key is critical
    emit displayTextChanged(); // Emits signal to notify display text change
}

bool RegistryKey::isCritical() const { // Returns whether the key is marked as critical
    return m_isCritical;
}

void RegistryKey::setCritical(bool critical) { // Sets the critical status of the key
    if (m_isCritical != critical) { // Only updates if the status is actually changed
        m_isCritical = critical; // Updates critical status
        qDebug() << "[INFO] RegistryKey" << m_valueName << "critical status set to" << m_isCritical; // Logs status change
        updateDisplayText(); // Updates the display text to reflect new status
        emit isCriticalChanged(); // Emits signal to notify critical status change
    }
}

QString RegistryKey::displayText() const { // Returns the display text for this key
    return m_displayText;
}

QString RegistryKey::name() const { // Returns the key path (name) of the registry key
    return m_valueName;
}

QString RegistryKey::value() const { // Returns the current stored value of the registry key
    return m_value;
}

QString RegistryKey::getCurrentValue() const { // Reads and returns the current value directly from the registry
    return readCurrentValue();
}

QString RegistryKey::previousValue() const { // Returns the previous value of the registry key
    return m_previousValue;
}

void RegistryKey::setPreviousValue(const QString &value) { // Sets the previous value for the registry key
    m_previousValue = value;
}

void RegistryKey::setNewValue(const QString &value) { // Sets the new value to be written to the registry
    m_newValue = value;
}

QString RegistryKey::newValue() const { // Returns the new value for the registry key
    return m_newValue;
}

void RegistryKey::setValue(const QString &value) { // Sets a new value in the registry and updates internal state
    if (m_value != value) { // Only proceeds if the new value is different from the current value
        m_previousValue = m_value; // Updates previous value
        m_value = value; // Sets new value

        // Set the new value in the registry
        m_settings->setValue(m_valueName, m_value); // Writes the value to the registry
        m_settings->sync(); // Commits the change to ensure it is applied

        // Reinitialize settings to ensure value is correctly read
        delete m_settings; // Deletes old QSettings instance
        m_settings = new QSettings((m_hive == "HKEY_CURRENT_USER" ? "HKEY_CURRENT_USER\\" : "HKEY_LOCAL_MACHINE\\")
                                       + m_keyPath, QSettings::NativeFormat); // Creates a new QSettings instance

        QString confirmedValue = m_settings->value(m_valueName).toString(); // Reads back the value to confirm
        qDebug() << "[SET] Key:" << m_valueName << ", Confirmed Registry Value after set:" << confirmedValue; // Logs confirmed value

        if (confirmedValue != m_value) { // Checks if confirmed value matches expected value
            qWarning() << "[WARNING] Registry value mismatch after set for key:" << m_keyPath; // Warns if values do not match
        }
    }
}

QString RegistryKey::readCurrentValue() const { // Reads the current value directly from the registry
    return m_settings->value(m_valueName).toString(); // Returns the value as a string
}
