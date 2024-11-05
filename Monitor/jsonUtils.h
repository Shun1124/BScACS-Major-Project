#ifndef JSONUTILS_H
#define JSONUTILS_H

#include <QString>
#include <QList>
#include "registryKey.h"

namespace JsonUtils {
QList<RegistryKey*> readKeysFromJson(const QString &filePath);
}

#endif // JSONUTILS_H
