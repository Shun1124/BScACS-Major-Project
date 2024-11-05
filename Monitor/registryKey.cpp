#include "registryKey.h"
#include <QDebug>

RegistryKey::RegistryKey(const QString &hive, const QString &keyPath, const QString &valueName, bool isCritical, QObject *parent)
    : QObject(parent), m_hive(hive), m_keyPath(keyPath), m_valueName(valueName), m_isCritical(isCritical) {
    QString fullKey = (m_hive == "HKEY_CURRENT_USER" ? "HKEY_CURRENT_USER\\" : "HKEY_LOCAL_MACHINE\\") + m_keyPath;
    m_settings = new QSettings(fullKey, QSettings::NativeFormat);
    m_value = readCurrentValue();
    m_previousValue = m_value;
    updateDisplayText();
    qDebug() << "[INIT] Key:" << m_keyPath << ", Initial Value:" << m_value;
}

bool RegistryKey::isRollbackCancelled() const {
    return m_rollbackCancelled;
}

void RegistryKey::setRollbackCancelled(bool cancelled) {
    m_rollbackCancelled = cancelled;
}

void RegistryKey::updateDisplayText() {
    QStringList pathSegments = m_keyPath.split("\\");
    QString baseName = pathSegments.last();
    m_displayText = m_isCritical ? baseName + " - Critical" : baseName;
    emit displayTextChanged();
}

bool RegistryKey::isCritical() const {
    return m_isCritical;
}

void RegistryKey::setCritical(bool critical) {
    if (m_isCritical != critical) {
        m_isCritical = critical;
        qDebug() << "RegistryKey" << m_keyPath << "critical status set to" << m_isCritical;
        updateDisplayText();
        emit isCriticalChanged();
    }
}

QString RegistryKey::displayText() const {
    return m_displayText;
}

QString RegistryKey::name() const {
    return m_keyPath;
}

QString RegistryKey::value() const {
    return m_value;
}

QString RegistryKey::getCurrentValue() const {
    return readCurrentValue();
}

QString RegistryKey::previousValue() const {
    return m_previousValue;
}

void RegistryKey::setPreviousValue(const QString &value) {
    m_previousValue = value;
}

void RegistryKey::setNewValue(const QString &value) {
    m_newValue = value;
}

QString RegistryKey::newValue() const {
    return m_newValue;
}

void RegistryKey::setValue(const QString &value) {
    if (m_value != value) {
        m_previousValue = m_value;
        m_value = value;

        // Set the new value in the registry
        m_settings->setValue(m_valueName, m_value);
        m_settings->sync(); // Ensure the change is committed

        // Reinitialize settings to ensure value is correctly read
        delete m_settings;
        m_settings = new QSettings((m_hive == "HKEY_CURRENT_USER" ? "HKEY_CURRENT_USER\\" : "HKEY_LOCAL_MACHINE\\")
                                       + m_keyPath, QSettings::NativeFormat);

        QString confirmedValue = m_settings->value(m_valueName).toString();
        qDebug() << "[SET] Key:" << m_keyPath << ", Confirmed Registry Value after set:" << confirmedValue;

        if (confirmedValue != m_value) {
            qWarning() << "[WARNING] Registry value mismatch after set for key:" << m_keyPath;
        }
    }
}

QString RegistryKey::readCurrentValue() const {
    return m_settings->value(m_valueName).toString();
}
