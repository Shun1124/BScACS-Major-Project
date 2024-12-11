#include "plistFileModel.h"
#include <QDebug>

PlistFileModel::PlistFileModel(QObject *parent) : QAbstractListModel(parent) {}

void PlistFileModel::setPlistFiles(const QList<PlistFile*> &files) {
    beginResetModel();
    m_plistFiles = files;
    endResetModel();
}

int PlistFileModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid())
        return 0;
    return m_plistFiles.size();
}

QVariant PlistFileModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();

    PlistFile *file = m_plistFiles[index.row()];

    if (role == NameRole) {
        return file->name();
    } else if (role == IsCriticalRole) {
        return file->isCritical();
    } else if (role == DisplayTextRole) {
        QString displayText = file->displayText();
        return displayText;
    }
    return QVariant();
}

QHash<int,QByteArray> PlistFileModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[IsCriticalRole] = "isCritical";
    roles[DisplayTextRole] = "displayText";
    return roles;
}
