#ifndef REGISTRYKEY_H // Preprocessor directive to prevent multiple inclusions of this header file
#define REGISTRYKEY_H // Defines REGISTRYKEY_H if not already defined

#include <QString> // Includes QString class for handling strings
#include <QSettings> // Includes QSettings class for reading and writing registry settings
#include <QObject> // Base class for all Qt objects, providing signals and slots

class RegistryKey : public QObject { // Defines the RegistryKey class, inheriting from QObject
    Q_OBJECT // Macro enabling the Qt meta-object features like signals and slots
    Q_PROPERTY(QString displayText READ displayText NOTIFY displayTextChanged) // Exposes displayText property to QML, with a notification signal
    Q_PROPERTY(bool isCritical READ isCritical WRITE setCritical NOTIFY isCriticalChanged) // Exposes isCritical property to QML, with a read, write, and notification

public:
    explicit RegistryKey(const QString &hive, const QString &keyPath, const QString &valueName, bool isCritical, QObject *parent = nullptr); // Constructor for RegistryKey

    bool isCritical() const; // Getter for the critical status
    void setCritical(bool critical); // Setter for the critical status
    QString name() const; // Returns the name (key path) of the registry key
    QString value() const; // Returns the current value of the registry key
    QString previousValue() const; // Returns the previous value of the registry key
    void setPreviousValue(const QString &value); // Sets the previous value of the registry key
    void setValue(const QString &value); // Sets a new value for the registry key
    QString getCurrentValue() const; // Reads and returns the current value from the registry
    QString displayText() const; // Returns the display text for this key
    void setNewValue(const QString &value); // Sets the new value to be written to the registry
    QString newValue() const; // Returns the new value of the registry key

    QSettings* settings() const { return m_settings; } // Getter for the QSettings instance
    void setSettings(QSettings* settings); // Setter for the QSettings instance
    QString hive() const { return m_hive; } // Returns the hive of the registry key
    QString keyPath() const { return m_keyPath; } // Returns the key path
    QString valueName() const { return m_valueName; } // Returns the value name

    // New methods
    bool isRollbackCancelled() const; // Checks if rollback is cancelled
    void setRollbackCancelled(bool cancelled); // Sets the rollback cancelled status

    int changeCount() const { return m_changeCount; } // Returns the change count for the key
    void incrementChangeCount() { m_changeCount++; } // Increments the change count
    void resetChangeCount() { m_changeCount = 0; } // Resets the change count

signals:
    void displayTextChanged(); // Signal emitted when display text changes
    void isCriticalChanged(); // Signal emitted when critical status changes

private:
    QString m_hive; // Stores the registry hive (e.g., HKEY_CURRENT_USER)
    QString m_keyPath; // Stores the path to the registry key
    QString m_valueName; // Stores the name of the registry value
    bool m_isCritical; // Indicates if the key is critical
    QString m_value; // Holds the current value of the key
    QString m_previousValue; // Holds the previous value of the key
    QString m_displayText; // Holds the display text for the key
    QSettings *m_settings; // Pointer to QSettings for accessing the registry
    QString m_newValue; // Stores the new value for the key
    int m_changeCount = 0; // Tracks the number of changes for non-critical keys

    // New property to track if rollback is cancelled
    bool m_rollbackCancelled = false; // Tracks if rollback for this key is cancelled

    void updateDisplayText(); // Updates the display text based on the critical status
    QString readCurrentValue() const; // Reads the current value directly from the registry
};

#endif // REGISTRYKEY_H // End of multiple inclusion guard for REGISTRYKEY_H
