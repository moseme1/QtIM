#include "sqlrepository.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDateTime>

// 数据库名称常量（需在头文件定义）
#define DB_NAME "qt_im.db"

SqlRepository::SqlRepository(QObject *parent) : QObject(parent)
{
    initDb();
}

SqlRepository::~SqlRepository()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
}

bool SqlRepository::initDb()
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(DB_NAME);

    if (!m_db.open()) {
        qDebug() << "数据库打开失败：" << m_db.lastError().text();
        return false;
    }

    // ========== 原有表创建（完全保留） ==========
    // 创建用户表
    QSqlQuery userQuery;
    QString userSql = "CREATE TABLE IF NOT EXISTS user ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                      "account VARCHAR(20) UNIQUE NOT NULL,"
                      "password VARCHAR(32) NOT NULL,"
                      "nickname VARCHAR(20) NOT NULL)";
    if (!userQuery.exec(userSql)) {
        qDebug() << "创建用户表失败：" << userQuery.lastError().text();
        return false;
    }

    // 创建联系人表
    QSqlQuery contactQuery;
    QString contactSql = "CREATE TABLE IF NOT EXISTS contact ("
                         "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                         "user_id INTEGER NOT NULL,"
                         "friend_id INTEGER NOT NULL,"
                         "group_name VARCHAR(20) NOT NULL,"
                         "remark VARCHAR(20),"
                         "FOREIGN KEY(user_id) REFERENCES user(id),"
                         "FOREIGN KEY(friend_id) REFERENCES user(id))";
    if (!contactQuery.exec(contactSql)) {
        qDebug() << "创建联系人表失败：" << contactQuery.lastError().text();
        return false;
    }

    // 创建消息表
    QSqlQuery msgQuery;
    QString msgSql = "CREATE TABLE IF NOT EXISTS message ("
                     "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                     "sender_id INTEGER NOT NULL,"
                     "receiver_id INTEGER NOT NULL,"
                     "content TEXT NOT NULL,"
                     "send_time DATETIME DEFAULT CURRENT_TIMESTAMP,"
                     "msg_type INTEGER NOT NULL,"
                     "is_read INTEGER DEFAULT 0,"
                     "FOREIGN KEY(sender_id) REFERENCES user(id))";
    if (!msgQuery.exec(msgSql)) {
        qDebug() << "创建消息表失败：" << msgQuery.lastError().text();
        return false;
    }

    // ========== 新增：创建在线用户表 + 私聊消息表 ==========
    // 1. 在线用户表（记录当前登录用户）
    QSqlQuery onlineQuery;
    QString onlineSql = "CREATE TABLE IF NOT EXISTS online_user ("
                        "user_id INTEGER PRIMARY KEY,"
                        "user_account VARCHAR(20) UNIQUE NOT NULL,"
                        "login_time DATETIME DEFAULT CURRENT_TIMESTAMP,"
                        "FOREIGN KEY(user_id) REFERENCES user(id))";
    if (!onlineQuery.exec(onlineSql)) {
        qDebug() << "创建在线用户表失败：" << onlineQuery.lastError().text();
    }

    // 2. 私聊消息表（存储定向私聊消息）
    QSqlQuery privateChatQuery;
    QString privateChatSql = "CREATE TABLE IF NOT EXISTS private_chat ("
                             "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                             "sender_id INTEGER NOT NULL,"
                             "receiver_id INTEGER NOT NULL,"
                             "content TEXT NOT NULL,"
                             "send_time DATETIME DEFAULT CURRENT_TIMESTAMP,"
                             "is_read INTEGER DEFAULT 0,"
                             "FOREIGN KEY(sender_id) REFERENCES user(id),"
                             "FOREIGN KEY(receiver_id) REFERENCES user(id))";
    if (!privateChatQuery.exec(privateChatSql)) {
        qDebug() << "创建私聊消息表失败：" << privateChatQuery.lastError().text();
    }

    return true;
}

// ========== 原有接口实现（完全保留，未做任何修改） ==========
bool SqlRepository::registerUser(const QString &account, const QString &pwd, const QString &nickname)
{
    QSqlQuery query;
    query.prepare("INSERT INTO user(account, password, nickname) VALUES(?, ?, ?)");
    query.addBindValue(account);
    query.addBindValue(pwd); // 实际项目需MD5加密
    query.addBindValue(nickname);

    if (!query.exec()) {
        qDebug() << "注册失败：" << query.lastError().text();
        return false;
    }
    return true;
}

bool SqlRepository::loginUser(const QString &account, const QString &pwd, QString &nickname, int &userId)
{
    QSqlQuery query;
    query.prepare("SELECT id, nickname FROM user WHERE account=? AND password=?");
    query.addBindValue(account);
    query.addBindValue(pwd);

    if (query.exec() && query.next()) {
        userId = query.value(0).toInt();
        nickname = query.value(1).toString();
        return true;
    }
    return false;
}

bool SqlRepository::addContact(int userId, int friendId, const QString &groupName, const QString &remark)
{
    QSqlQuery query;
    query.prepare("INSERT INTO contact(user_id, friend_id, group_name, remark) VALUES(?, ?, ?, ?)");
    query.addBindValue(userId);
    query.addBindValue(friendId);
    query.addBindValue(groupName);
    query.addBindValue(remark);

    if (!query.exec()) {
        qDebug() << "添加联系人失败：" << query.lastError().text();
        return false;
    }
    return true;
}

QList<QMap<QString, QString>> SqlRepository::getContacts(int userId)
{
    QList<QMap<QString, QString>> contacts;
    QSqlQuery query;
    query.prepare("SELECT u.id, u.nickname, c.group_name, c.remark "
                  "FROM contact c LEFT JOIN user u ON c.friend_id = u.id "
                  "WHERE c.user_id=?");
    query.addBindValue(userId);

    if (query.exec()) {
        while (query.next()) {
            QMap<QString, QString> contact;
            contact["id"] = query.value(0).toString();
            contact["nickname"] = query.value(1).toString();
            contact["group"] = query.value(2).toString();
            contact["remark"] = query.value(3).toString();
            contacts.append(contact);
        }
    }
    return contacts;
}

bool SqlRepository::saveMessage(int senderId, int receiverId, const QString &content, MsgType type)
{
    QSqlQuery query;
    query.prepare("INSERT INTO message(sender_id, receiver_id, content, msg_type) VALUES(?, ?, ?, ?)");
    query.addBindValue(senderId);
    query.addBindValue(receiverId);
    query.addBindValue(content);
    query.addBindValue(type);

    if (!query.exec()) {
        qDebug() << "保存消息失败：" << query.lastError().text();
        return false;
    }
    return true;
}

QList<QMap<QString, QString>> SqlRepository::getMessageHistory(int userId1, int userId2)
{
    QList<QMap<QString, QString>> messages;
    QSqlQuery query;
    query.prepare("SELECT sender_id, content, send_time FROM message "
                  "WHERE (sender_id=? AND receiver_id=?) OR (sender_id=? AND receiver_id=?) "
                  "ORDER BY send_time ASC");
    query.addBindValue(userId1);
    query.addBindValue(userId2);
    query.addBindValue(userId2);
    query.addBindValue(userId1);

    if (query.exec()) {
        while (query.next()) {
            QMap<QString, QString> msg;
            msg["sender"] = query.value(0).toString();
            msg["content"] = query.value(1).toString();
            msg["time"] = query.value(2).toString();
            messages.append(msg);
        }
    }
    return messages;
}

// ========== 新增：私聊+在线用户接口实现（优化+补充） ==========
// 核心新增：根据账号查询用户ID（客户端解析在线列表必需）
int SqlRepository::getUserIdByAccount(const QString &account)
{
    if (!m_db.isOpen()) return -1;

    QSqlQuery query;
    query.prepare("SELECT id FROM user WHERE account = ?");
    query.addBindValue(account);

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    qDebug() << "未找到账号对应的用户ID：" << account;
    return -1;
}

// 1. 登录后添加在线用户
bool SqlRepository::addOnlineUser(int userId, const QString &account)
{
    if (!m_db.isOpen()) return false;

    QSqlQuery query;
    // 先删除旧记录（避免同一用户重复登录）
    query.prepare("DELETE FROM online_user WHERE user_id = ? OR user_account = ?");
    query.addBindValue(userId);
    query.addBindValue(account);
    query.exec();

    // 插入新的在线记录（带登录时间）
    query.prepare("INSERT INTO online_user (user_id, user_account, login_time) VALUES (?, ?, ?)");
    query.addBindValue(userId);
    query.addBindValue(account);
    query.addBindValue(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));

    if (!query.exec()) {
        qDebug() << "添加在线用户失败：" << query.lastError().text();
        return false;
    }
    return true;
}

// 2. 登出/关闭程序时移除在线用户
bool SqlRepository::removeOnlineUser(int userId)
{
    if (!m_db.isOpen()) return false;

    QSqlQuery query;
    query.prepare("DELETE FROM online_user WHERE user_id = ?");
    query.addBindValue(userId);

    if (!query.exec()) {
        qDebug() << "移除在线用户失败：" << query.lastError().text();
        return false;
    }
    return true;
}

// 3. 获取在线用户列表（排除当前登录用户）
QMap<int, QString> SqlRepository::getOnlineUserList(int currentUserId)
{
    QMap<int, QString> onlineList;
    if (!m_db.isOpen()) return onlineList;

    QSqlQuery query;
    query.prepare("SELECT user_id, user_account FROM online_user WHERE user_id != ?");
    query.addBindValue(currentUserId);

    if (query.exec()) {
        while (query.next()) {
            int userId = query.value(0).toInt();
            QString account = query.value(1).toString();
            onlineList.insert(userId, account);
        }
    } else {
        qDebug() << "获取在线用户列表失败：" << query.lastError().text();
    }
    return onlineList;
}

// 4. 发送私聊消息
bool SqlRepository::sendPrivateMessage(int senderId, int receiverId, const QString &content)
{
    if (!m_db.isOpen()) return false;

    QSqlQuery query;
    query.prepare("INSERT INTO private_chat (sender_id, receiver_id, content, send_time) VALUES (?, ?, ?, ?)");
    query.addBindValue(senderId);
    query.addBindValue(receiverId);
    query.addBindValue(content);
    query.addBindValue(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));

    if (!query.exec()) {
        qDebug() << "发送私聊消息失败：" << query.lastError().text();
        return false;
    }
    return true;
}

// 5. 获取私聊消息记录（双向消息，按时间排序）
QList<QMap<QString, QString>> SqlRepository::getPrivateChatHistory(int currentUserId, int targetUserId)
{
    QList<QMap<QString, QString>> chatHistory;
    if (!m_db.isOpen()) return chatHistory;

    QSqlQuery query;
    query.prepare("SELECT sender_id, receiver_id, content, send_time FROM private_chat "
                  "WHERE (sender_id = ? AND receiver_id = ?) OR (sender_id = ? AND receiver_id = ?) "
                  "ORDER BY send_time ASC");
    query.addBindValue(currentUserId);
    query.addBindValue(targetUserId);
    query.addBindValue(targetUserId);
    query.addBindValue(currentUserId);

    if (query.exec()) {
        while (query.next()) {
            QMap<QString, QString> msg;
            msg["sender_id"] = query.value(0).toString();
            msg["receiver_id"] = query.value(1).toString();
            msg["content"] = query.value(2).toString();
            msg["send_time"] = query.value(3).toString();
            chatHistory.append(msg);
        }
    } else {
        qDebug() << "获取私聊记录失败：" << query.lastError().text();
    }
    return chatHistory;
}
