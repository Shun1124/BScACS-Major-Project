#ifndef SETTINGS_H // Preprocessor directive to prevent multiple inclusions of this header file
#define SETTINGS_H // Defines SETTINGS_H if not already defined

#include <QObject> // Includes QObject base class for signal and slot functionality
#include <QString> // Includes QString for handling strings

class Settings : public QObject { // Defines the Settings class, inheriting from QObject
    Q_OBJECT // Macro enabling the Qt meta-object features like signals and slots
    Q_PROPERTY(QString email READ getEmail WRITE setEmail NOTIFY emailChanged) // Exposes email property to QML with getter, setter, and notification signal
    Q_PROPERTY(QString phoneNumber READ getPhoneNumber WRITE setPhoneNumber NOTIFY phoneNumberChanged) // Exposes phoneNumber property to QML with getter, setter, and notification signal

public:
    explicit Settings(QObject *parent = nullptr); // Constructor for Settings, with an optional parent parameter
    Q_INVOKABLE QString getEmail() const; // Exposes getEmail method to QML as an invokable function
    Q_INVOKABLE void setEmail(const QString &email); // Exposes setEmail method to QML as an invokable function

    Q_INVOKABLE QString getPhoneNumber() const; // Exposes getPhoneNumber method to QML as an invokable function
    Q_INVOKABLE void setPhoneNumber(const QString &phoneNumber); // Exposes setPhoneNumber method to QML as an invokable function
    Q_INVOKABLE QString getNonCriticalAlertThreshold() const; // Exposes getNonCriticalAlertThreshold method to QML as an invokable function
    Q_INVOKABLE void setNonCriticalAlertThreshold(const QString &threshold); // Exposes setNonCriticalAlertThreshold method to QML as an invokable function

signals:
    void emailChanged(); // Signal emitted when the email property changes
    void phoneNumberChanged(); // Signal emitted when the phone number property changes
    void nonCriticalAlertThresholdChanged(); // Signal emitted when the non-critical alert threshold changes

private:
    QString m_email; // Stores the email address
    QString m_phoneNumber; // Stores the phone number
    QString m_nonCriticalAlertThreshold; // Stores the threshold for non-critical alerts
};

#endif // SETTINGS_H // End of multiple inclusion guard for SETTINGS_H
