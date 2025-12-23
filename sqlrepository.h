#ifndef SQLREPOSITORY_H
#define SQLREPOSITORY_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QMap>   // 新增：用于存储在线用户列表（id->account）
#include <QList>  // 原有已包含，无需重复加
#include "globaldefine.h"

class SqlRepository : public QObject
{
    Q_OBJECT
public:
    explicit SqlRepository(QObject *parent = nullptr);
    ~SqlRepository();

    // ========== 原有所有接口（完全保留，未做任何修改） ==========
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

    // ========== 新增：私聊+在线用户核心接口（补充getUserIdByAccount） ==========
    // 核心新增：根据账号查询用户ID（客户端解析在线列表必需）
    int getUserIdByAccount(const QString &account);

    // 在线用户管理（登录/登出用）
    bool addOnlineUser(int userId, const QString &account);       // 登录后添加在线用户
    bool removeOnlineUser(int userId);                            // 登出/关闭程序移除在线用户
    QMap<int, QString> getOnlineUserList(int currentUserId);      // 获取在线用户列表（排除自己）

    // 私聊消息管理（定向发消息/查记录用）
    bool sendPrivateMessage(int senderId, int receiverId, const QString &content); // 发送私聊消息
    QList<QMap<QString, QString>> getPrivateChatHistory(int currentUserId, int targetUserId); // 获取私聊记录

private:
    QSqlDatabase m_db; // 原有成员变量，保留
};

#endif // SQLREPOSITORY_H
