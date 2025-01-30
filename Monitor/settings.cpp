#include "settings.h" // Includes the Settings class definition
#include <QDebug> // Enables logging and debugging output
#include "Database.h"

Settings::Settings(QObject *parent) : QObject(parent) {} // Constructor for Settings, initializing QObject with an optional parent

QString Settings::getEmail() const { // Getter method for the email address
    return m_email; // Returns the stored email address
}

void Settings::setEmail(const QString &email) {
    if (email.isEmpty()) {
        qWarning() << "[SETTINGS] Attempted to set empty email.";
    } else {
        qDebug() << "[SETTINGS] Email set to:" << email;
    }
    m_email = email;

    // Update the database with the new email
    Database db;
    if (!db.insertUserSettings(m_email, m_phoneNumber, m_nonCriticalAlertThreshold.toInt())) {
        qWarning() << "[SETTINGS] Failed to update email in the database.";
    }
}

QString Settings::getPhoneNumber() const { // Getter method for the phone number
    return m_phoneNumber; // Returns the stored phone number
}

void Settings::setPhoneNumber(const QString &phoneNumber) {
    if (phoneNumber.isEmpty()) {
        qWarning() << "[SETTINGS] Attempted to set empty phone number.";
    } else {
        qDebug() << "[SETTINGS] Phone number set to:" << phoneNumber;
    }
    m_phoneNumber = phoneNumber;

    // Update the database with the new phone number
    Database db;
    if (!db.insertUserSettings(m_email, m_phoneNumber, m_nonCriticalAlertThreshold.toInt())) {
        qWarning() << "[SETTINGS] Failed to update phone number in the database.";
    }
}

QString Settings::getNonCriticalAlertThreshold() const { // Getter method for the non-critical alert threshold
    return m_nonCriticalAlertThreshold; // Returns the stored threshold value
}

void Settings::setNonCriticalAlertThreshold(const QString &threshold) {
    if (m_nonCriticalAlertThreshold != threshold) {
        m_nonCriticalAlertThreshold = threshold;
        emit nonCriticalAlertThresholdChanged();
        qDebug() << "[SETTINGS] Non-critical alert threshold set to:" << m_nonCriticalAlertThreshold;

        // Update the threshold in the database
        Database db;
        if (!db.insertUserSettings(m_email, m_phoneNumber, m_nonCriticalAlertThreshold.toInt())) {
            qWarning() << "[SETTINGS] Failed to update non-critical threshold in the database.";
        } else {
            qDebug() << "[SETTINGS] Non-critical threshold updated in the database.";
        }
    }
}

