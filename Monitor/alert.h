// alert.h
#ifndef ALERT_H
#define ALERT_H

#include <QObject>
#include <aws/sns/SNSClient.h>
#include <aws/sesv2/SESV2Client.h>
#include <QString>
#include <memory>
#include "settings.h"

class Alert : public QObject {
    Q_OBJECT

public:
    explicit Alert(Settings *settings, QObject *parent = nullptr);

    // Send alert based on critical changes
    void sendAlert(const QString &message);

private:
    Settings *m_settings;
    std::unique_ptr<Aws::SNS::SNSClient> m_snsClient;
    std::unique_ptr<Aws::SESV2::SESV2Client> m_sesv2Client;

    void sendSmsAlert(const QString &message);
    void sendEmailAlert(const QString &message);
};

bool loadAwsCredentials(const QString &filePath, Aws::Auth::AWSCredentials &credentials, Aws::Client::ClientConfiguration &config);

#endif // ALERT_H
