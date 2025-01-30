#include "alert.h" // Includes the header file for the Alert class definition
#include <QFile> // Provides file handling capabilities
#include <QJsonDocument> // Enables JSON document manipulation
#include <QJsonObject> // Allows creation and manipulation of JSON objects
#include <aws/core/auth/AWSCredentialsProvider.h> // Provides AWS credentials for authentication
#include <aws/core/Aws.h> // Initializes the core AWS SDK
#include <aws/sns/model/PublishRequest.h> // Provides model for publishing SNS requests
#include <aws/sesv2/model/SendEmailRequest.h> // Provides model for sending SES email requests
#include <aws/sesv2/model/Content.h> // Represents the content structure in an SES email
#include <aws/sesv2/model/Body.h> // Represents the body structure in an SES email
#include <aws/sesv2/model/Message.h> // Defines the message structure in SES
#include <iostream> // Allows for console output
#include <QDir> // Provides directory manipulation capabilities
#include <QCoreApplication> // Provides access to application directory and environment
#include "Database.h"

Alert::Alert(Settings *settings, QObject *parent)
    : QObject(parent), m_settings(settings) { // Constructor for the Alert class initializing settings
    Aws::Auth::AWSCredentials credentials; // Stores AWS credentials for authentication
    Aws::Client::ClientConfiguration config; // Configures AWS client settings

    QString credentialsPath = resolveAwsConfigPath();

    if (loadAwsCredentials(credentialsPath, credentials, config)) { // Loads AWS credentials if config file is valid
        m_snsClient = std::make_unique<Aws::SNS::SNSClient>(credentials, config); // Initializes SNS client with credentials
        m_sesv2Client = std::make_unique<Aws::SESV2::SESV2Client>(credentials, config); // Initializes SES client with credentials
    } else {
        qWarning("Failed to initialize AWS clients with credentials from JSON"); // Logs failure to initialize clients
        return;
    }
    // Log initial settings for verification
    qDebug() << "[ALERT INIT] Email:" << m_settings->getEmail(); // Outputs email setting for verification
    qDebug() << "[ALERT INIT] Phone number:" << m_settings->getPhoneNumber(); // Outputs phone number setting for verification
    m_database = new Database(this);
}

// Helper method to resolve AWS config path
QString Alert::resolveAwsConfigPath() {
    #ifdef Q_OS_MAC
        return QDir::cleanPath(QCoreApplication::applicationDirPath() + "/../../../../../awsconfig.json");
    #else
        return QDir::cleanPath(QCoreApplication::applicationDirPath() + "/../../awsconfig.json");
    #endif
}

void Alert::sendAlert(const QString &message) {
    qDebug() << "[ALERT] sendAlert called with message:" << message;

    if (!m_database) {
        qWarning("[ALERT] Database not initialized.");
        return;
    }

    QVariantList userSettings = m_database->getAllUserSettings();
    if (userSettings.isEmpty()) {
        qWarning("[ALERT] No user settings found in the database. Skipping alerts.");
        return; // Exit early if no user settings are found
    }

    bool alertSent = false; // Track whether any alert was successfully sent

    for (const auto &setting : userSettings) {
        QVariantMap user = setting.toMap();
        QString email = user["email"].toString();
        QString phone = user["phone"].toString();

        if (!email.isEmpty()) {
            qDebug() << "[ALERT] Sending email to:" << email;
            sendEmailAlert(email, message);
            alertSent = true;
        } else {
            qDebug() << "[ALERT] Email not set for user.";
        }

        if (!phone.isEmpty()) {
            qDebug() << "[ALERT] Sending SMS to:" << phone;
            sendSmsAlert(phone, message);
            alertSent = true;
        } else {
            qDebug() << "[ALERT] Phone number not set for user.";
        }
    }

    if (!alertSent) {
        qWarning("[ALERT] No valid email or phone number found. No alerts were sent.");
    }
}

void Alert::sendSmsAlert(const QString &phoneNumber, const QString &message) {
    if (!m_snsClient) {
        qWarning("[SMS ALERT] SNS client not initialized.");
        return;
    }

    Aws::SNS::Model::PublishRequest request;
    request.SetMessage(message.toStdString());
    request.SetPhoneNumber(phoneNumber.toStdString());

    auto outcome = m_snsClient->Publish(request);
    if (!outcome.IsSuccess()) {
        std::cerr << "[SMS ALERT] Failed to send SMS to " << phoneNumber.toStdString()
        << ". Error: " << outcome.GetError().GetMessage() << std::endl;
    } else {
        std::cout << "[SMS ALERT] SMS sent successfully to " << phoneNumber.toStdString() << std::endl;
    }
}

void Alert::sendEmailAlert(const QString &email, const QString &message) {
    if (!m_sesv2Client) {
        qWarning("[EMAIL ALERT] SES client not initialized.");
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
    request.SetFromEmailAddress("ingridh2630@gmail.com");

    Aws::SESV2::Model::Destination destination;
    destination.AddToAddresses(email.toStdString());
    request.SetDestination(destination);

    auto outcome = m_sesv2Client->SendEmail(request);
    if (!outcome.IsSuccess()) {
        std::cerr << "[EMAIL ALERT] Failed to send email to " << email.toStdString()
        << ". Error: " << outcome.GetError().GetMessage() << std::endl;
    } else {
        std::cout << "[EMAIL ALERT] Email sent successfully to " << email.toStdString() << std::endl;
    }
}

bool loadAwsCredentials(const QString &filePath, Aws::Auth::AWSCredentials &credentials, Aws::Client::ClientConfiguration &config) {
    QFile file(filePath); // Opens file at specified path
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) { // Checks if file opens successfully
        qWarning("Failed to open AWS config file"); // Logs failure to open file
        return false;
    }

    QByteArray fileData = file.readAll(); // Reads data from file
    QJsonDocument jsonDoc = QJsonDocument::fromJson(fileData); // Parses JSON data
    QJsonObject jsonObj = jsonDoc.object(); // Creates JSON object from document

    QString accessKeyId = jsonObj["accessKeyId"].toString(); // Retrieves access key ID from JSON
    QString secretAccessKey = jsonObj["secretAccessKey"].toString(); // Retrieves secret access key from JSON
    QString region = jsonObj["region"].toString(); // Retrieves region from JSON

    if (accessKeyId.isEmpty() || secretAccessKey.isEmpty() || region.isEmpty()) { // Checks for missing fields
        qWarning("AWS config file is missing required fields"); // Logs missing field warning
        return false;
    }

    credentials = Aws::Auth::AWSCredentials(accessKeyId.toStdString(), secretAccessKey.toStdString()); // Sets credentials
    config.region = region.toStdString(); // Sets region configuration

    return true; // Returns true if credentials loaded successfully
}
