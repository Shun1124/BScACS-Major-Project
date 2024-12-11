#ifndef PLISTFILEMODEL_H
#define PLISTFILEMODEL_H

#include <QAbstractListModel>
#include "plistFile.h"

class PlistFileModel : public QAbstractListModel {
    Q_OBJECT

public:
    explicit PlistFileModel(QObject *parent = nullptr);

    enum PlistFileRoles {
        NameRole = Qt::UserRole + 1,
        IsCriticalRole,
        DisplayTextRole
    };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setPlistFiles(const QList<PlistFile*> &files);
    void resetModel();

private:
    QList<PlistFile*> m_plistFiles;
};

#endif // PLISTFILEMODEL_H
