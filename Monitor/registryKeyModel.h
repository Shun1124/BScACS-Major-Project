#ifndef REGISTRYKEYMODEL_H
#define REGISTRYKEYMODEL_H

#include <QAbstractListModel>
#include "registryKey.h"

class RegistryKeyModel : public QAbstractListModel {
    Q_OBJECT

public:
    explicit RegistryKeyModel(QObject *parent = nullptr); // Constructor for RegistryKeyModel with an optional parent parameter

    enum RegistryKeyRoles {
        NameRole = Qt::UserRole + 1, // Custom role for the key's name, starting at Qt::UserRole + 1 to avoid conflicts
        IsCriticalRole, // Custom role for the critical status of a key
        DisplayTextRole // Custom role for the display text of a key
    };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setRegistryKeys(const QList<RegistryKey*> &keys);
    void resetModel();

private:
    QList<RegistryKey*> m_registryKeys;
};

#endif
