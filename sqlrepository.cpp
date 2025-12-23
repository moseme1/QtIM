#include "sqlrepository.h"

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

    return true;
}

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