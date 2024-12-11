#ifndef ALERT_H // Preprocessor directive to prevent multiple inclusions of this header file
#define ALERT_H // Defines ALERT_H if not already defined

#include <QObject> // Base class for all Qt objects, providing object-oriented features like signals and slots
#include <aws/sns/SNSClient.h> // Provides the AWS SNS client for sending SMS messages
#include <aws/sesv2/SESV2Client.h> // Provides the AWS SESV2 client for sending email messages
#include <QString> // Qt class for handling strings
#include <memory> // Provides smart pointers, such as std::unique_ptr, for resource management
#include "settings.h" // Includes the Settings class for managing configuration settings

class Alert : public QObject { // Defines the Alert class, inheriting from QObject to use Qt's meta-object system
    Q_OBJECT // Macro enabling the Qt meta-object features like signals and slots

public:
    explicit Alert(Settings *settings, QObject *parent = nullptr); // Constructor that initializes the Alert class with settings

    // Send alert based on critical changes
    void sendAlert(const QString &message); // Public method to trigger alert notifications based on message content

private:
    Settings *m_settings; // Pointer to Settings object to access user settings like email and phone number
    std::unique_ptr<Aws::SNS::SNSClient> m_snsClient; // Smart pointer to AWS SNS client for sending SMS messages
    std::unique_ptr<Aws::SESV2::SESV2Client> m_sesv2Client; // Smart pointer to AWS SESV2 client for sending email messages

    void sendSmsAlert(const QString &message); // Private method to send SMS alerts via SNS
    void sendEmailAlert(const QString &message); // Private method to send email alerts via SES

    QString resolveAwsConfigPath();
};

bool loadAwsCredentials(const QString &filePath, Aws::Auth::AWSCredentials &credentials, Aws::Client::ClientConfiguration &config); // Function to load AWS credentials from a file

#endif // ALERT_H // End of multiple inclusion guard for ALERT_H
