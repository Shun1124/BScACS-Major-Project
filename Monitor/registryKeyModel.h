#ifndef REGISTRYKEYMODEL_H
#define REGISTRYKEYMODEL_H

#include <QAbstractListModel>
#include "registryKey.h"

class RegistryKeyModel : public QAbstractListModel {
    Q_OBJECT

public:
    explicit RegistryKeyModel(QObject *parent = nullptr);

    enum RegistryKeyRoles {
        NameRole = Qt::UserRole + 1,
        IsCriticalRole,
        DisplayTextRole
    };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setRegistryKeys(const QList<RegistryKey*> &keys);
    void resetModel();

private:
    QList<RegistryKey*> m_registryKeys;
};

#endif // REGISTRYKEYMODEL_H
