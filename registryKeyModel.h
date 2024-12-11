#ifndef REGISTRYKEYMODEL_H // Preprocessor directive to prevent multiple inclusions of this header file
#define REGISTRYKEYMODEL_H // Defines REGISTRYKEYMODEL_H if not already defined

#include <QAbstractListModel> // Includes QAbstractListModel, which provides a base class for list models in Qt
#include "registryKey.h" // Includes the RegistryKey class definition

class RegistryKeyModel : public QAbstractListModel { // Defines the RegistryKeyModel class, inheriting from QAbstractListModel
    Q_OBJECT // Macro enabling the Qt meta-object features like signals and slots

public:
    explicit RegistryKeyModel(QObject *parent = nullptr); // Constructor for RegistryKeyModel with an optional parent parameter

    enum RegistryKeyRoles { // Enum defining custom roles for the model
        NameRole = Qt::UserRole + 1, // Custom role for the key's name, starting at Qt::UserRole + 1 to avoid conflicts
        IsCriticalRole, // Custom role for the critical status of a key
        DisplayTextRole // Custom role for the display text of a key
    };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override; // Returns the number of rows in the model
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override; // Returns data for a specific index and role
    QHash<int, QByteArray> roleNames() const override; // Returns a hash map of role IDs to role names

    void setRegistryKeys(const QList<RegistryKey*> &keys); // Sets the list of registry keys in the model
    void resetModel(); // Resets the model to refresh its data

private:
    QList<RegistryKey*> m_registryKeys; // Stores the list of RegistryKey pointers being managed by the model
};

#endif // REGISTRYKEYMODEL_H // End of multiple inclusion guard for REGISTRYKEYMODEL_H
