#ifndef WINDOWSJSONUTILS_H 
#define WINDOWSJSONUTILS_H 

#include <QString> // Includes the QString class for handling string data
#include <QList> // Includes the QList class for managing lists of items
#include "registryKey.h" // Includes the RegistryKey class definition for use in this header

namespace WindowsJsonUtils { // Defines the JsonUtils namespace to organize related functions
QList<RegistryKey*> readKeysFromJson(const QString &filePath); // Declares a function to read registry keys from a JSON file
}

#endif // WINDOWSJSONUTILS_H 
