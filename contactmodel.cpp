#include "contactmodel.h"

ContactModel::ContactModel(QObject *parent) : QAbstractListModel(parent)
{
}

int ContactModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_contacts.size();
}

QVariant ContactModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_contacts.size()) {
        return QVariant();
    }

    const auto &contact = m_contacts[index.row()];
    switch (role) {
    case IdRole:
        return contact["id"];
    case NicknameRole:
        return contact["nickname"];
    case GroupRole:
        return contact["group"];
    case RemarkRole:
        return contact["remark"];
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> ContactModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[NicknameRole] = "nickname";
    roles[GroupRole] = "group";
    roles[RemarkRole] = "remark";
    return roles;
}

void ContactModel::setContacts(const QList<QMap<QString, QString>> &contacts)
{
    beginResetModel();
    m_contacts = contacts;
    endResetModel();
}

void ContactModel::addContact(const QMap<QString, QString> &contact)
{
    beginInsertRows(QModelIndex(), m_contacts.size(), m_contacts.size());
    m_contacts.append(contact);
    endInsertRows();
}