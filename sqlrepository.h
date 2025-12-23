#ifndef SQLREPOSITORY_H
#define SQLREPOSITORY_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include "globaldefine.h"

class SqlRepository : public QObject
{
    Q_OBJECT
public:
    explicit SqlRepository(QObject *parent = nullptr);
    ~SqlRepository();

    // 初始化数据库
    bool initDb();

    // 用户相关操作
    bool registerUser(const QString &account, const QString &pwd, const QString &nickname);
    bool loginUser(const QString &account, const QString &pwd, QString &nickname, int &userId);

    // 联系人相关操作
    bool addContact(int userId, int friendId, const QString &groupName, const QString &remark = "");
    QList<QMap<QString, QString>> getContacts(int userId);

    // 消息相关操作
    bool saveMessage(int senderId, int receiverId, const QString &content, MsgType type);
    QList<QMap<QString, QString>> getMessageHistory(int userId1, int userId2);

private:
    QSqlDatabase m_db;
};

#endif // SQLREPOSITORY_H