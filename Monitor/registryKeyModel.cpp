#include "registrykeymodel.h" // Includes the RegistryKeyModel class definition
#include <QDebug> // Enables logging and debugging output

RegistryKeyModel::RegistryKeyModel(QObject *parent)
    : QAbstractListModel(parent) {} // Constructor for RegistryKeyModel, initializing the base QAbstractListModel with an optional parent

void RegistryKeyModel::setRegistryKeys(const QList<RegistryKey*> &keys) { // Sets the list of registry keys in the model
    beginResetModel(); // Begins a model reset operation
    m_registryKeys = keys; // Assigns the provided list of registry keys to m_registryKeys
    endResetModel(); // Ends the model reset operation and notifies views of the change
}

int RegistryKeyModel::rowCount(const QModelIndex &parent) const { // Returns the number of rows in the model
    if (parent.isValid()) // Checks if the parent index is valid
        return 0; // Returns 0 for valid parent indices, as this is a flat list model
    return m_registryKeys.size(); // Returns the number of registry keys in the model
}

QVariant RegistryKeyModel::data(const QModelIndex &index, int role) const { // Provides data for each role in the model
    if (!index.isValid()) // Checks if the index is valid
        return QVariant(); // Returns an invalid QVariant if the index is not valid

    RegistryKey *key = m_registryKeys[index.row()]; // Retrieves the RegistryKey at the specified row

    if (role == NameRole) { // Checks if the requested role is NameRole
        return key->name(); // Returns the key's name
    } else if (role == IsCriticalRole) { // Checks if the requested role is IsCriticalRole
        return key->isCritical(); // Returns whether the key is critical
    } else if (role == DisplayTextRole) { // Checks if the requested role is DisplayTextRole
        QString displayText = key->displayText(); // Retrieves the display text for the key
        // qDebug() << "DisplayTextRole for index" << index.row() << ":" << displayText; // Logs the display text for debugging
        return displayText; // Returns the display text for the key
    }
    return QVariant(); // Returns an invalid QVariant if the role is unrecognized
}

QHash<int, QByteArray> RegistryKeyModel::roleNames() const { // Defines the role names for the model
    QHash<int, QByteArray> roles; // Creates a hash to map role IDs to role names
    roles[NameRole] = "name"; // Maps NameRole to the "name" string
    roles[IsCriticalRole] = "isCritical"; // Maps IsCriticalRole to the "isCritical" string
    roles[DisplayTextRole] = "displayText"; // Maps DisplayTextRole to the "displayText" string
    return roles; // Returns the roles hash
}
