#include "settings.h"
#include <QDebug>

Settings::Settings(QObject *parent) : QObject(parent) {} // Constructor for Settings, initializing QObject with an optional parent

QString Settings::getEmail() const { // Getter method for the email address
    return m_email;
}

void Settings::setEmail(const QString &email) { // Setter method for the email address
    m_email = email;
    qDebug() << "[SETTINGS] Email set to:" << m_email;
}

QString Settings::getPhoneNumber() const { // Getter method for the phone number
    return m_phoneNumber;
}

void Settings::setPhoneNumber(const QString &phoneNumber) { // Setter method for the phone number
    m_phoneNumber = phoneNumber;
    qDebug() << "[SETTINGS] Phone number set to:" << m_phoneNumber;
}

QString Settings::getNonCriticalAlertThreshold() const { // Getter method for the non-critical alert threshold
    return m_nonCriticalAlertThreshold;
}

void Settings::setNonCriticalAlertThreshold(const QString &threshold) { // Setter method for the non-critical alert threshold
    // qDebug() << "[DEBUG] Entering setNonCriticalAlertThreshold()"; // Logs the entry into this function for debugging
    if (m_nonCriticalAlertThreshold != threshold) { // Checks if the new threshold differs from the current one
        m_nonCriticalAlertThreshold = threshold;
        emit nonCriticalAlertThresholdChanged();
        qDebug() << "[SETTINGS] Non-critical alert threshold set to:" << m_nonCriticalAlertThreshold;
    }
    // qDebug() << "[DEBUG] Exiting setNonCriticalAlertThreshold()"; // Logs the exit from this function for debugging
}
