#include "registrykeymodel.h"
#include <QDebug>

RegistryKeyModel::RegistryKeyModel(QObject *parent)
    : QAbstractListModel(parent) {} // Constructor for RegistryKeyModel, initializing the base QAbstractListModel with an optional parent

void RegistryKeyModel::setRegistryKeys(const QList<RegistryKey*> &keys) { // Sets the list of registry keys in the model
    beginResetModel();
    m_registryKeys = keys;
    endResetModel();
}

int RegistryKeyModel::rowCount(const QModelIndex &parent) const { // Returns the number of rows in the model
    if (parent.isValid()) // Checks if the parent index is valid
        return 0;
    return m_registryKeys.size();
}

QVariant RegistryKeyModel::data(const QModelIndex &index, int role) const { // Provides data for each role in the model
    if (!index.isValid()) // Checks if the index is valid
        return QVariant();

    RegistryKey *key = m_registryKeys[index.row()]; // Retrieves the RegistryKey at the specified row

    if (role == NameRole) { // Checks if the requested role is NameRole
        return key->name();
    } else if (role == IsCriticalRole) { // Checks if the requested role is IsCriticalRole
        return key->isCritical();
    } else if (role == DisplayTextRole) { // Checks if the requested role is DisplayTextRole
        QString displayText = key->displayText();
        // qDebug() << "DisplayTextRole for index" << index.row() << ":" << displayText; // Logs the display text for debugging
        return displayText;
    }
    return QVariant();
}

QHash<int, QByteArray> RegistryKeyModel::roleNames() const { // Defines the role names for the model
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[IsCriticalRole] = "isCritical";
    roles[DisplayTextRole] = "displayText";
    return roles;
}
