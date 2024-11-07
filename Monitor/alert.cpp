#include "alert.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/core/Aws.h>
#include <aws/sns/model/PublishRequest.h>
#include <aws/sesv2/model/SendEmailRequest.h>
#include <aws/sesv2/model/Content.h>
#include <aws/sesv2/model/Body.h>
#include <aws/sesv2/model/Message.h>
#include <iostream>
#include <QDir>
#include <QCoreApplication>

Alert::Alert(Settings *settings, QObject *parent)
    : QObject(parent), m_settings(settings) { // Constructor for the Alert class initializing settings
    Aws::Auth::AWSCredentials credentials;
    Aws::Client::ClientConfiguration config;

    QString appDirPath = QCoreApplication::applicationDirPath();
    QString credentialsPath = QDir::cleanPath(appDirPath + "/../../awsconfig.json"); // Constructs path to AWS config file

    if (loadAwsCredentials(credentialsPath, credentials, config)) { // Loads AWS credentials if config file is valid
        m_snsClient = std::make_unique<Aws::SNS::SNSClient>(credentials, config);
        m_sesv2Client = std::make_unique<Aws::SESV2::SESV2Client>(credentials, config);
    } else {
        qWarning("Failed to initialize AWS clients with credentials from JSON");
    }
    // Log initial settings for verification
    qDebug() << "[ALERT INIT] Email:" << m_settings->getEmail();
    qDebug() << "[ALERT INIT] Phone number:" << m_settings->getPhoneNumber();
}

void Alert::sendAlert(const QString &message) {
    qDebug() << "[ALERT] sendAlert called with message:" << message;
    if (!m_settings) { // Checks if settings are initialized
        qWarning("Settings not initialized");
        return;
    }
    if (!m_settings->getEmail().isEmpty()) { // Checks if email is set
        qDebug() << "[ALERT] Email is set, sending email alert...";
        sendEmailAlert(message); // Sends email alert
    } else {
        qDebug() << "[ALERT] Email not set. Skipping email alert.";
    }
    if (!m_settings->getPhoneNumber().isEmpty()) { // Checks if phone number is set
        qDebug() << "[ALERT] Phone number is set, sending SMS alert...";
        sendSmsAlert(message); // Sends SMS alert
    } else {
        qDebug() << "[ALERT] Phone number not set. Skipping SMS alert.";
    }
}

void Alert::sendSmsAlert(const QString &message) {
    if (!m_snsClient) { // Checks if SNS client is initialized
        qWarning("SNS client not initialized");
        return;
    }
    QString phoneNumber = m_settings->getPhoneNumber(); // Gets phone number from settings
    if (phoneNumber.isEmpty()) { // Checks if phone number is empty
        qWarning("Phone number is not set. SMS alert cannot be sent.");
        return;
    }

    Aws::SNS::Model::PublishRequest request; // Creates SNS publish request for SMS
    request.SetMessage(message.toStdString());
    request.SetPhoneNumber(phoneNumber.toStdString());

    auto outcome = m_snsClient->Publish(request); // Publishes the SMS request
    if (!outcome.IsSuccess()) { // Checks if the SMS send was unsuccessful
        std::cerr << "Failed to send SMS: " << outcome.GetError().GetMessage() << std::endl;
    } else {
        std::cout << "[INFO] SMS sent successfully!" << std::endl;
    }
}

void Alert::sendEmailAlert(const QString &message) {
    if (!m_sesv2Client) { // Checks if SES client is initialized
        qWarning("SES client not initialized");
        return;
    }
    QString email = m_settings->getEmail(); // Gets email from settings
    if (email.isEmpty()) { // Checks if email is empty
        qWarning("Email is not set. Email alert cannot be sent.");
        return;
    }

    Aws::SESV2::Model::SendEmailRequest request; // Creates SES email request
    Aws::SESV2::Model::Content subjectContent;
    subjectContent.SetData("Critical Alert");

    Aws::SESV2::Model::Body emailBody; // Sets email body
    Aws::SESV2::Model::Content bodyContent;
    bodyContent.SetData(message.toStdString());
    emailBody.SetText(bodyContent);

    Aws::SESV2::Model::Message emailMessage; // Creates the email message object
    emailMessage.SetSubject(subjectContent);
    emailMessage.SetBody(emailBody);

    Aws::SESV2::Model::EmailContent emailContent; // Creates email content object
    emailContent.SetSimple(emailMessage);

    request.SetContent(emailContent); // Assigns email content to the request
    request.SetFromEmailAddress("ingridh2630@gmail.com");

    Aws::SESV2::Model::Destination destination; // Defines email destination
    destination.AddToAddresses(email.toStdString());
    request.SetDestination(destination);

    auto outcome = m_sesv2Client->SendEmail(request); // Sends the email request
    if (!outcome.IsSuccess()) { // Checks if email send was unsuccessful
        std::cerr << "Failed to send email: " << outcome.GetError().GetMessage() << std::endl;
    } else {
        std::cout << "[INFO] Email sent successfully!" << std::endl;
    }
}

bool loadAwsCredentials(const QString &filePath, Aws::Auth::AWSCredentials &credentials, Aws::Client::ClientConfiguration &config) {
    QFile file(filePath); // Opens file at specified path
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) { // Checks if file opens successfully
        qWarning("Failed to open AWS config file");
        return false;
    }

    QByteArray fileData = file.readAll(); // Reads data from file
    QJsonDocument jsonDoc = QJsonDocument::fromJson(fileData);
    QJsonObject jsonObj = jsonDoc.object();

    QString accessKeyId = jsonObj["accessKeyId"].toString();
    QString secretAccessKey = jsonObj["secretAccessKey"].toString();
    QString region = jsonObj["region"].toString();

    if (accessKeyId.isEmpty() || secretAccessKey.isEmpty() || region.isEmpty()) { // Checks for missing fields
        qWarning("AWS config file is missing required fields");// Logs missing field warning
        return false;
    }

    credentials = Aws::Auth::AWSCredentials(accessKeyId.toStdString(), secretAccessKey.toStdString()); // Sets credentials
    config.region = region.toStdString();

    return true;
}
