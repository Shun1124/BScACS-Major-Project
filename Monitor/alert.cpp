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
    : QObject(parent), m_settings(settings) {
    Aws::Auth::AWSCredentials credentials;
    Aws::Client::ClientConfiguration config;

    QString appDirPath = QCoreApplication::applicationDirPath();
    QString credentialsPath = QDir::cleanPath(appDirPath + "/../../awsconfig.json");

    if (loadAwsCredentials(credentialsPath, credentials, config)) {
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
    if (!m_settings) {
        qWarning("Settings not initialized");
        return;
    }
    if (!m_settings->getEmail().isEmpty()) {
        qDebug() << "[ALERT] Email is set, sending email alert...";
        sendEmailAlert(message);
    } else {
        qDebug() << "[ALERT] Email not set. Skipping email alert.";
    }
    if (!m_settings->getPhoneNumber().isEmpty()) {
        qDebug() << "[ALERT] Phone number is set, sending SMS alert...";
        sendSmsAlert(message);
    } else {
        qDebug() << "[ALERT] Phone number not set. Skipping SMS alert.";
    }
}

void Alert::sendSmsAlert(const QString &message) {
    if (!m_snsClient) {
        qWarning("SNS client not initialized");
        return;
    }
    QString phoneNumber = m_settings->getPhoneNumber();
    if (phoneNumber.isEmpty()) {
        qWarning("Phone number is not set. SMS alert cannot be sent.");
        return;
    }

    Aws::SNS::Model::PublishRequest request;
    request.SetMessage(message.toStdString());
    request.SetPhoneNumber(phoneNumber.toStdString());

    auto outcome = m_snsClient->Publish(request);
    if (!outcome.IsSuccess()) {
        std::cerr << "Failed to send SMS: " << outcome.GetError().GetMessage() << std::endl;
    } else {
        std::cout << "SMS sent successfully!" << std::endl;
    }
}

void Alert::sendEmailAlert(const QString &message) {
    if (!m_sesv2Client) {
        qWarning("SES client not initialized");
        return;
    }
    QString email = m_settings->getEmail();
    if (email.isEmpty()) {
        qWarning("Email is not set. Email alert cannot be sent.");
        return;
    }

    Aws::SESV2::Model::SendEmailRequest request;
    Aws::SESV2::Model::Content subjectContent;
    subjectContent.SetData("Critical Alert");

    Aws::SESV2::Model::Body emailBody;
    Aws::SESV2::Model::Content bodyContent;
    bodyContent.SetData(message.toStdString());
    emailBody.SetText(bodyContent);

    Aws::SESV2::Model::Message emailMessage;
    emailMessage.SetSubject(subjectContent);
    emailMessage.SetBody(emailBody);

    Aws::SESV2::Model::EmailContent emailContent;
    emailContent.SetSimple(emailMessage);

    request.SetContent(emailContent);
    request.SetFromEmailAddress("ingridh2630@gmail.com");  // Replace with your SES-verified email

    Aws::SESV2::Model::Destination destination;
    destination.AddToAddresses(email.toStdString());
    request.SetDestination(destination);

    auto outcome = m_sesv2Client->SendEmail(request);
    if (!outcome.IsSuccess()) {
        std::cerr << "Failed to send email: " << outcome.GetError().GetMessage() << std::endl;
    } else {
        std::cout << "Email sent successfully!" << std::endl;
    }
}

bool loadAwsCredentials(const QString &filePath, Aws::Auth::AWSCredentials &credentials, Aws::Client::ClientConfiguration &config) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning("Failed to open AWS config file");
        return false;
    }

    QByteArray fileData = file.readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(fileData);
    QJsonObject jsonObj = jsonDoc.object();

    QString accessKeyId = jsonObj["accessKeyId"].toString();
    QString secretAccessKey = jsonObj["secretAccessKey"].toString();
    QString region = jsonObj["region"].toString();

    if (accessKeyId.isEmpty() || secretAccessKey.isEmpty() || region.isEmpty()) {
        qWarning("AWS config file is missing required fields");
        return false;
    }

    credentials = Aws::Auth::AWSCredentials(accessKeyId.toStdString(), secretAccessKey.toStdString());
    config.region = region.toStdString();

    return true;
}
