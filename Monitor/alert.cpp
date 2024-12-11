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
    }
    // Log initial settings for verification
    qDebug() << "[ALERT INIT] Email:" << m_settings->getEmail(); // Outputs email setting for verification
    qDebug() << "[ALERT INIT] Phone number:" << m_settings->getPhoneNumber(); // Outputs phone number setting for verification
}

// Helper method to resolve AWS config path
QString Alert::resolveAwsConfigPath() {
    #ifdef Q_OS_MAC
        return QDir::cleanPath(QCoreApplication::applicationDirPath() + "/../../../../../awsconfig.json");
    #else
        return QDir::cleanPath(QCoreApplication::applicationDirPath() + "/../awsconfig.json");
    #endif
}

void Alert::sendAlert(const QString &message) {
    qDebug() << "[ALERT] sendAlert called with message:" << message; // Logs alert message for debugging
    if (!m_settings) { // Checks if settings are initialized
        qWarning("Settings not initialized"); // Logs a warning if settings are missing
        return;
    }
    if (!m_settings->getEmail().isEmpty()) { // Checks if email is set
        qDebug() << "[ALERT] Email is set, sending email alert..."; // Logs email sending attempt
        sendEmailAlert(message); // Sends email alert
    } else {
        qDebug() << "[ALERT] Email not set. Skipping email alert."; // Logs skipping of email alert
    }
    if (!m_settings->getPhoneNumber().isEmpty()) { // Checks if phone number is set
        qDebug() << "[ALERT] Phone number is set, sending SMS alert..."; // Logs SMS sending attempt
        sendSmsAlert(message); // Sends SMS alert
    } else {
        qDebug() << "[ALERT] Phone number not set. Skipping SMS alert."; // Logs skipping of SMS alert
    }
}

void Alert::sendSmsAlert(const QString &message) {
    if (!m_snsClient) { // Checks if SNS client is initialized
        qWarning("SNS client not initialized"); // Logs warning if SNS client is missing
        return;
    }
    QString phoneNumber = m_settings->getPhoneNumber(); // Gets phone number from settings
    if (phoneNumber.isEmpty()) { // Checks if phone number is empty
        qWarning("Phone number is not set. SMS alert cannot be sent."); // Logs inability to send SMS
        return;
    }

    Aws::SNS::Model::PublishRequest request; // Creates SNS publish request for SMS
    request.SetMessage(message.toStdString()); // Sets the message content for SMS
    request.SetPhoneNumber(phoneNumber.toStdString()); // Sets the recipient phone number for SMS

    auto outcome = m_snsClient->Publish(request); // Publishes the SMS request
    if (!outcome.IsSuccess()) { // Checks if the SMS send was unsuccessful
        std::cerr << "Failed to send SMS: " << outcome.GetError().GetMessage() << std::endl; // Logs error message
    } else {
        std::cout << "[INFO] SMS sent successfully!" << std::endl; // Logs successful SMS send
    }
}

void Alert::sendEmailAlert(const QString &message) {
    if (!m_sesv2Client) { // Checks if SES client is initialized
        qWarning("SES client not initialized"); // Logs warning if SES client is missing
        return;
    }
    QString email = m_settings->getEmail(); // Gets email from settings
    if (email.isEmpty()) { // Checks if email is empty
        qWarning("Email is not set. Email alert cannot be sent."); // Logs inability to send email
        return;
    }

    Aws::SESV2::Model::SendEmailRequest request; // Creates SES email request
    Aws::SESV2::Model::Content subjectContent; // Sets subject content of the email
    subjectContent.SetData("Critical Alert"); // Specifies subject line text

    Aws::SESV2::Model::Body emailBody; // Sets email body
    Aws::SESV2::Model::Content bodyContent; // Creates body content
    bodyContent.SetData(message.toStdString()); // Sets message content in the body
    emailBody.SetText(bodyContent); // Adds content to email body

    Aws::SESV2::Model::Message emailMessage; // Creates the email message object
    emailMessage.SetSubject(subjectContent); // Sets email subject
    emailMessage.SetBody(emailBody); // Sets email body

    Aws::SESV2::Model::EmailContent emailContent; // Creates email content object
    emailContent.SetSimple(emailMessage); // Sets message as simple content

    request.SetContent(emailContent); // Assigns email content to the request
    request.SetFromEmailAddress("ingridh2630@gmail.com"); // Sets sender email (replace with verified address)

    Aws::SESV2::Model::Destination destination; // Defines email destination
    destination.AddToAddresses(email.toStdString()); // Adds recipient email
    request.SetDestination(destination); // Sets the email destination

    auto outcome = m_sesv2Client->SendEmail(request); // Sends the email request
    if (!outcome.IsSuccess()) { // Checks if email send was unsuccessful
        std::cerr << "Failed to send email: " << outcome.GetError().GetMessage() << std::endl; // Logs error message
    } else {
        std::cout << "[INFO] Email sent successfully!" << std::endl; // Logs successful email send
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
