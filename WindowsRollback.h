#ifndef WINDOWSROLLBACK_H 
#define WINDOWSROLLBACK_H 

#include <QObject> // Includes QObject base class for signal and slot functionality
#include "registryKey.h" // Includes the RegistryKey class for managing registry keys

class WindowsRollback : public QObject { // Defines the Rollback class, inheriting from QObject
    Q_OBJECT // Macro enabling the Qt meta-object features like signals and slots

public:
    explicit WindowsRollback(QObject *parent = nullptr); // Constructor for Rollback, with an optional parent parameter
    void registerKeyForRollback(RegistryKey* key); // Registers a critical key for potential rollback
    void rollbackIfNeeded(RegistryKey* key); // Checks if rollback is needed and performs it if required
    void cancelRollback(RegistryKey* key); // Cancels a rollback for a specified key

signals:
    void rollbackPerformed(QString keyName); // Signal emitted when a rollback is performed for a specific key

private:
    void restorePreviousValue(RegistryKey* key); // Restores a key to its previous value if a rollback is required
};

#endif // WINDOWSROLLBACK_H 
