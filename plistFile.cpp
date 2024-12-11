#include "plistFile.h"
#include <QDebug>

PlistFile::PlistFile(const QString &plistPath, const QString &filePath, const QString &valueName,
    bool isCritical, QObject *parent) : QObject(parent), m_plistPath(plistPath),
    m_filePath(filePath), m_valueName(valueName), m_isCritical(isCritical) {
    QString fullFile = m_plistPath;
    m_settings = new QSettings(fullFile, QSettings::NativeFormat);
    m_value = readCurrentValue();
    m_previousValue = m_value;
    updateDisplayText();
    qDebug() << "[INIT] File:" << m_valueName << ", Initial Value:" << m_value;
}

bool PlistFile::isRollbackCancelled() const {
    return m_rollbackCancelled;
}

void PlistFile::setRollbackCancelled(bool cancelled) {
    m_rollbackCancelled = cancelled;
}

void PlistFile::updateDisplayText() {
    m_displayText = m_isCritical ? m_valueName + " - Critical" : m_valueName;
    emit displayTextChanged();
}

bool PlistFile::isCritical() const {
    return m_isCritical;
}

void PlistFile::setCritical(bool critical) {
    if (m_isCritical != critical) {
        m_isCritical = critical;
        qDebug() << "[INFO] PlistFile" << m_valueName << "criticalstatus set to" << m_isCritical;
    }
    updateDisplayText();
    emit isCriticalChanged();
}

QString PlistFile::displayText() const {
    return m_displayText;
}

QString PlistFile::name() const {
    return m_valueName;
}

QString PlistFile::value() const {
    return m_value;
}

QString PlistFile::getCurrentValue() const {
    return readCurrentValue();
}

QString PlistFile::previousValue() const {
    return m_previousValue;
}

void PlistFile::setPreviousValue(const QString &value) {
    m_previousValue = value;
}

void PlistFile::setNewValue(const QString &value) {
    m_newValue = value;
}

QString PlistFile::newValue() const {
    return m_newValue;
}

void PlistFile::setValue(const QString &value) {
    if (m_value != value) {
        m_previousValue = m_value;
        m_value = value;

        m_settings->setValue(m_valueName, m_value);
        m_settings->sync();

        QString confirmedValue = m_settings->value(m_valueName).toString();
        qDebug() << "[SET] File:" << m_valueName << ", Confirmed File Value after set:" << confirmedValue;

        if (confirmedValue != m_value) {
            qWarning() << "[WARNING] File value mismatch after set for file:" << m_filePath;
        }
    }
}

QString PlistFile::readCurrentValue() const {
    return m_settings->value(m_valueName).toString();
}
