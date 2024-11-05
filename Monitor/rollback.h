#ifndef ROLLBACK_H
#define ROLLBACK_H

#include <QObject>
#include "registryKey.h"

class Rollback : public QObject {
    Q_OBJECT

public:
    explicit Rollback(QObject *parent = nullptr);
    void registerKeyForRollback(RegistryKey* key);
    void rollbackIfNeeded(RegistryKey* key);
    void cancelRollback(RegistryKey* key);  // New method to cancel rollback

signals:
    void rollbackPerformed(QString keyName);

private:
    void restorePreviousValue(RegistryKey* key);
};

#endif // ROLLBACK_H
