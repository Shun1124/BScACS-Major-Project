#ifndef MACOSROLLBACK_H 
#define MACOSROLLBACK_H 

#include <QObject> // Includes QObject base class for signal and slot functionality
#include "plistFile.h" // Includes the RegistryKey class for managing registry keys

class MacOSRollback : public QObject { // Defines the Rollback class, inheriting from QObject
    Q_OBJECT // Macro enabling the Qt meta-object features like signals and slots

public:
    explicit MacOSRollback(QObject *parent = nullptr); // Constructor for Rollback, with an optional parent parameter
    void plistFileForRollback(PlistFile* file); // Registers a critical key for potential rollback
    void rollbackIfNeeded(PlistFile* file); // Checks if rollback is needed and performs it if required
    void cancelRollback(PlistFile* file); // Cancels a rollback for a specified key

signals:
    void rollbackPerformed(QString fileName); // Signal emitted when a rollback is performed for a specific key

private:
    void restorePreviousValue(PlistFile* file); // Restores a key to its previous value if a rollback is required
};

#endif // MACOSROLLBACK_H 
