#include "registrykeymodel.h"
#include <QDebug>

RegistryKeyModel::RegistryKeyModel(QObject *parent)
    : QAbstractListModel(parent) {}

void RegistryKeyModel::setRegistryKeys(const QList<RegistryKey*> &keys) {
    beginResetModel();
    m_registryKeys = keys;
    endResetModel();
}

int RegistryKeyModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid())
        return 0;
    return m_registryKeys.size();
}

QVariant RegistryKeyModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();

    RegistryKey *key = m_registryKeys[index.row()];

    if (role == NameRole) {
        return key->name();
    } else if (role == IsCriticalRole) {
        return key->isCritical();
    } else if (role == DisplayTextRole) {
        QString displayText = key->displayText();
        qDebug() << "DisplayTextRole for index" << index.row() << ":" << displayText;
        return displayText;
    }
    return QVariant();
}

QHash<int, QByteArray> RegistryKeyModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[IsCriticalRole] = "isCritical";
    roles[DisplayTextRole] = "displayText";
    return roles;
}
