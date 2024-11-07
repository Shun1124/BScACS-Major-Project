#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QString>

class Settings : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString email READ getEmail WRITE setEmail NOTIFY emailChanged)
    Q_PROPERTY(QString phoneNumber READ getPhoneNumber WRITE setPhoneNumber NOTIFY phoneNumberChanged)

public:
    explicit Settings(QObject *parent = nullptr); // Constructor for Settings, with an optional parent parameter
    Q_INVOKABLE QString getEmail() const; 
    Q_INVOKABLE void setEmail(const QString &email); 

    Q_INVOKABLE QString getPhoneNumber() const; 
    Q_INVOKABLE void setPhoneNumber(const QString &phoneNumber);
    Q_INVOKABLE QString getNonCriticalAlertThreshold() const;
    Q_INVOKABLE void setNonCriticalAlertThreshold(const QString &threshold);

signals:
    void emailChanged(); 
    void phoneNumberChanged(); 
    void nonCriticalAlertThresholdChanged(); 

private:
    QString m_email; 
    QString m_phoneNumber; 
    QString m_nonCriticalAlertThreshold; 
};

#endif 
