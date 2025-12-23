#ifndef MESSAGEMODEL_H
#define MESSAGEMODEL_H

#include <QAbstractListModel>
#include <QList>
#include <QMap>

class MessageModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum MessageRoles {
        SenderRole = Qt::UserRole + 1,
        ContentRole,
        TimeRole
    };

    explicit MessageModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setMessages(const QList<QMap<QString, QString>> &messages);
    void addMessage(const QMap<QString, QString> &message);
    void clearMessages();

private:
    QList<QMap<QString, QString>> m_messages;
};

#endif // MESSAGEMODEL_H