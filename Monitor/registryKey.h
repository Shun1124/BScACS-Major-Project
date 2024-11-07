#ifndef REGISTRYKEY_H
#define REGISTRYKEY_H

#include <QString>
#include <QSettings>
#include <QObject>

class RegistryKey : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString displayText READ displayText NOTIFY displayTextChanged) // Exposes displayText property to QML, with a notification signal
    Q_PROPERTY(bool isCritical READ isCritical WRITE setCritical NOTIFY isCriticalChanged) // Exposes isCritical property to QML, with a read, write, and notification

public:
    explicit RegistryKey(const QString &hive, const QString &keyPath, const QString &valueName, bool isCritical, QObject *parent = nullptr); // Constructor for RegistryKey

    bool isCritical() const;
    void setCritical(bool critical);
    QString name() const;
    QString value() const;
    QString previousValue() const;
    void setPreviousValue(const QString &value);
    void setValue(const QString &value);
    QString getCurrentValue() const;
    QString displayText() const;
    void setNewValue(const QString &value);
    QString newValue() const;

    QSettings* settings() const { return m_settings; }
    void setSettings(QSettings* settings);
    QString hive() const { return m_hive; }
    QString keyPath() const { return m_keyPath; }
    QString valueName() const { return m_valueName; }

    bool isRollbackCancelled() const;
    void setRollbackCancelled(bool cancelled);

    int changeCount() const { return m_changeCount; }
    void incrementChangeCount() { m_changeCount++; }
    void resetChangeCount() { m_changeCount = 0; }

signals:
    void displayTextChanged();
    void isCriticalChanged();

private:
    QString m_hive;
    QString m_keyPath;
    QString m_valueName;
    bool m_isCritical;
    QString m_value;
    QString m_previousValue;
    QString m_displayText;
    QSettings *m_settings;
    QString m_newValue;
    int m_changeCount = 0;
    bool m_rollbackCancelled = false;
    void updateDisplayText();
    QString readCurrentValue() const;
};

#endif
