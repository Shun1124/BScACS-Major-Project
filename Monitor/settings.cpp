#include "settings.h"
#include <QDebug>

Settings::Settings(QObject *parent) : QObject(parent) {}

QString Settings::getEmail() const {
    return m_email;
}

void Settings::setEmail(const QString &email) {
    m_email = email;
    qDebug() << "[SETTINGS] Email set to:" << m_email;
}

QString Settings::getPhoneNumber() const {
    return m_phoneNumber;
}

void Settings::setPhoneNumber(const QString &phoneNumber) {
    m_phoneNumber = phoneNumber;
    qDebug() << "[SETTINGS] Phone number set to:" << m_phoneNumber;
}

QString Settings::getNonCriticalAlertThreshold() const {
    return m_nonCriticalAlertThreshold;
}

void Settings::setNonCriticalAlertThreshold(const QString &threshold) {
    qDebug() << "[DEBUG] Entering setNonCriticalAlertThreshold()";
    if (m_nonCriticalAlertThreshold != threshold) {
        m_nonCriticalAlertThreshold = threshold;
        emit nonCriticalAlertThresholdChanged();
        qDebug() << "[SETTINGS] Non-critical alert threshold set to:" << m_nonCriticalAlertThreshold;
    }
    qDebug() << "[DEBUG] Exiting setNonCriticalAlertThreshold()";
}
