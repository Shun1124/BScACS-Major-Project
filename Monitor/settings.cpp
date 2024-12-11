#include "settings.h" // Includes the Settings class definition
#include <QDebug> // Enables logging and debugging output

Settings::Settings(QObject *parent) : QObject(parent) {} // Constructor for Settings, initializing QObject with an optional parent

QString Settings::getEmail() const { // Getter method for the email address
    return m_email; // Returns the stored email address
}

void Settings::setEmail(const QString &email) { // Setter method for the email address
    m_email = email; // Sets the email address to the provided value
    qDebug() << "[SETTINGS] Email set to:" << m_email; // Logs the newly set email address
}

QString Settings::getPhoneNumber() const { // Getter method for the phone number
    return m_phoneNumber; // Returns the stored phone number
}

void Settings::setPhoneNumber(const QString &phoneNumber) { // Setter method for the phone number
    m_phoneNumber = phoneNumber; // Sets the phone number to the provided value
    qDebug() << "[SETTINGS] Phone number set to:" << m_phoneNumber; // Logs the newly set phone number
}

QString Settings::getNonCriticalAlertThreshold() const { // Getter method for the non-critical alert threshold
    return m_nonCriticalAlertThreshold; // Returns the stored threshold value
}

void Settings::setNonCriticalAlertThreshold(const QString &threshold) { // Setter method for the non-critical alert threshold
    // qDebug() << "[DEBUG] Entering setNonCriticalAlertThreshold()"; // Logs the entry into this function for debugging
    if (m_nonCriticalAlertThreshold != threshold) { // Checks if the new threshold differs from the current one
        m_nonCriticalAlertThreshold = threshold; // Sets the threshold to the new value
        emit nonCriticalAlertThresholdChanged(); // Emits a signal indicating that the threshold has changed
        qDebug() << "[SETTINGS] Non-critical alert threshold set to:" << m_nonCriticalAlertThreshold; // Logs the newly set threshold
    }
    // qDebug() << "[DEBUG] Exiting setNonCriticalAlertThreshold()"; // Logs the exit from this function for debugging
}
