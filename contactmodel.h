#ifndef CONTACTMODEL_H
#define CONTACTMODEL_H

#include <QAbstractListModel>
#include <QList>
#include <QMap>

class ContactModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum ContactRoles {
        IdRole = Qt::UserRole + 1,
        NicknameRole,
        GroupRole,
        RemarkRole
    };

    explicit ContactModel(QObject *parent = nullptr);

    // 重写虚函数
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    // 数据操作
    void setContacts(const QList<QMap<QString, QString>> &contacts);
    void addContact(const QMap<QString, QString> &contact);

private:
    QList<QMap<QString, QString>> m_contacts;
};

#endif // CONTACTMODEL_H