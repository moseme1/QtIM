#include "messagemodel.h"

MessageModel::MessageModel(QObject *parent) : QAbstractListModel(parent)
{
}

int MessageModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_messages.size();
}

QVariant MessageModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_messages.size()) {
        return QVariant();
    }

    const auto &msg = m_messages[index.row()];
    switch (role) {
    case SenderRole:
        return msg["sender"];
    case ContentRole:
        return msg["content"];
    case TimeRole:
        return msg["time"];
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> MessageModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[SenderRole] = "sender";
    roles[ContentRole] = "content";
    roles[TimeRole] = "time";
    return roles;
}

void MessageModel::setMessages(const QList<QMap<QString, QString>> &messages)
{
    beginResetModel();
    m_messages = messages;
    endResetModel();
}

void MessageModel::addMessage(const QMap<QString, QString> &message)
{
    beginInsertRows(QModelIndex(), m_messages.size(), m_messages.size());
    m_messages.append(message);
    endInsertRows();
}

void MessageModel::clearMessages()
{
    beginResetModel();
    m_messages.clear();
    endResetModel();
}