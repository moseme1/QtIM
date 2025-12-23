#include "datamanager.h"
#include <QDateTime>
#include <QDebug>
#include <QMutex>
#include <QMutexLocker>
#include "globaldefine.h"  // 引入全局枚举/常量

DataManager *DataManager::m_instance = nullptr;

DataManager::DataManager(QObject *parent) : QObject(parent)
{
    m_sqlRepo = new SqlRepository();
    m_contactModel = new ContactModel();
    m_messageModel = new MessageModel();
    m_netMgr = NetworkManager::getInstance();
    m_currentUserId = -1; // 初始化当前用户ID，避免野值

    // 绑定网络消息接收信号（仅绑定一次）
    connect(m_netMgr, &NetworkManager::recvMsg, this, &DataManager::onRecvNetworkMsg);
    // 预绑定连接成功信号（避免重复绑定）
    connect(m_netMgr, &NetworkManager::connSuccess, this, &DataManager::onConnSuccess, Qt::UniqueConnection);
}

DataManager::~DataManager()
{
    // 退出时发送下线通知+移除在线状态
    if (m_currentUserId != -1) {
        QString offlineMsg = QString("offline|%1").arg(m_currentUserId);
        m_netMgr->sendMsg(offlineMsg);
        removeOnlineUser(m_currentUserId);
    }

    delete m_sqlRepo;
    delete m_contactModel;
    delete m_messageModel;
}

DataManager *DataManager::getInstance()
{
    // 线程安全的单例模式（避免多线程创建多个实例）
    static QMutex mutex;
    QMutexLocker locker(&mutex);
    if (!m_instance) {
        m_instance = new DataManager();
    }
    return m_instance;
}

// 新增：对外提供当前用户ID（UI层需要）
int DataManager::getCurrentUserId() const
{
    return m_currentUserId;
}

bool DataManager::userRegister(const QString &account, const QString &pwd, const QString &nickname)
{
    return m_sqlRepo->registerUser(account, pwd, nickname);
}

// 临时存储登录信息（避免lambda捕获局部变量）
QString g_loginAccount;
QString g_loginNickname;
int g_loginUserId = -1;

bool DataManager::userLogin(const QString &account, const QString &pwd)
{
    QString nickname;
    int userId;
    // 先校验登录信息，确保数据库操作不崩溃
    if (!m_sqlRepo || !m_sqlRepo->loginUser(account, pwd, nickname, userId)) {
        emit loginFailed("账号或密码错误");
        return false;
    }

    // 保存登录信息到全局临时变量（避免lambda捕获局部变量）
    g_loginAccount = account;
    g_loginNickname = nickname;
    g_loginUserId = userId;
    m_currentUserId = userId;

    // 先触发登录成功（更新UI），再连接服务器
    emit loginSuccess(nickname, userId);
    m_netMgr->connectServer(); // 异步连接，不阻塞UI

    loadContacts(userId); // 加载本地联系人，避免UI白屏
    return true;
}

// 连接成功后的统一处理（避免lambda崩溃）
void DataManager::onConnSuccess()
{
    // 仅当登录信息有效时发送上线通知
    if (g_loginUserId == -1 || g_loginAccount.isEmpty()) {
        qWarning() << "登录信息无效，跳过上线通知";
        return;
    }

    // 发送上线通知
    QString onlineMsg = QString("online|%1|%2|%3")
                            .arg(g_loginUserId)
                            .arg(g_loginAccount)
                            .arg(g_loginNickname);
    m_netMgr->sendMsg(onlineMsg);
    qDebug() << "网络连接成功，发送上线通知：" << onlineMsg;

    // 清空临时变量前，先把自己的账号存入数据库在线表
    addOnlineUser(g_loginUserId, g_loginAccount);

    // 清空临时变量，避免重复发送
    g_loginUserId = -1;
    // 保留g_loginAccount，避免解析在线列表时丢失自己的账号（关键修复）
    // g_loginAccount.clear();
    g_loginNickname.clear();
}

ContactModel *DataManager::getContactModel()
{
    return m_contactModel;
}

MessageModel *DataManager::getMessageModel()
{
    return m_messageModel;
}

void DataManager::loadContacts(int userId)
{
    // 判空保护，避免访问空指针导致白屏
    if (!m_sqlRepo || !m_contactModel) return;
    auto contacts = m_sqlRepo->getContacts(userId);
    m_contactModel->setContacts(contacts);
}

void DataManager::loadMessageHistory(int userId1, int userId2)
{
    if (!m_sqlRepo || !m_messageModel) return;
    auto messages = m_sqlRepo->getMessageHistory(userId1, userId2);
    m_messageModel->setMessages(messages);
}

void DataManager::sendMessage(int senderId, int receiverId, const QString &content, MsgType type)
{
    if (!m_sqlRepo) return;
    // 本地保存消息
    m_sqlRepo->saveMessage(senderId, receiverId, content, type);
    // 网络发送消息（强转枚举，避免警告）
    QString msg = QString("%1|%2|%3|%4").arg(senderId).arg(receiverId).arg(content).arg(static_cast<int>(type));
    m_netMgr->sendMsg(msg);

    // 更新本地Model（避免UI无数据）
    QMap<QString, QString> msgMap;
    msgMap["sender"] = QString::number(senderId);
    msgMap["content"] = content;
    msgMap["time"] = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    m_messageModel->addMessage(msgMap);

    emit msgSentSuccess();
}

void DataManager::onRecvNetworkMsg(const QString &msg)
{
    qDebug() << "收到网络消息：" << msg;

    // ========== 1. 处理服务器广播的在线用户列表（核心修复） ==========
    if (msg.startsWith("SYSTEM|ONLINE_USERS")) {
        QStringList parts = msg.split("|");
        if (parts.size() >= 3) {
            // 清空原有在线列表，避免脏数据
            QMap<int, QString> oldOnlineUsers = getOnlineUserList(m_currentUserId);
            for (int userId : oldOnlineUsers.keys()) {
                removeOnlineUser(userId);
            }

            // 解析服务器广播的在线账号列表
            QStringList onlineAccounts = parts[2].split(",");
            for (const QString &account : onlineAccounts) {
                // 修复：保留g_loginAccount，正确跳过自己
                if (account.isEmpty() || account == g_loginAccount) {
                    qDebug() << "跳过自己的账号：" << account;
                    continue;
                }

                // 根据账号查询用户ID（需SqlRepository实现该接口）
                int userId = m_sqlRepo->getUserIdByAccount(account);
                if (userId != -1 && userId != m_currentUserId) {
                    qDebug() << "添加在线用户：" << userId << " -> " << account;
                    addOnlineUser(userId, account);
                }
            }

            emit onlineUserUpdated(); // 强制刷新UI在线列表
            qDebug() << "在线列表更新完成，触发UI刷新";
            return;
        }
    }

    // ========== 2. 处理上线通知 ==========
    if (msg.startsWith("online")) {
        QStringList parts = msg.split("|");
        if (parts.size() >= 4) {
            int userId = parts[1].toInt();
            QString account = parts[2];
            // 跳过自己的上线通知
            if (userId == m_currentUserId) {
                qDebug() << "跳过自己的上线通知";
                return;
            }
            // 添加到本地在线列表
            addOnlineUser(userId, account);
            emit onlineUserUpdated(); // 触发UI刷新在线列表
            qDebug() << "收到上线通知，添加用户：" << account;
            return;
        }
    }

    // ========== 3. 处理下线通知 ==========
    else if (msg.startsWith("offline")) {
        QStringList parts = msg.split("|");
        if (parts.size() >= 2) {
            // 兼容两种格式：offline|用户ID  /  offline|账号
            QString value = parts[1];
            int userId = value.toInt();
            if (userId == 0) {
                // 按账号查询用户ID
                userId = m_sqlRepo->getUserIdByAccount(value);
            }

            if (userId != -1 && userId != m_currentUserId) {
                removeOnlineUser(userId);
                emit onlineUserUpdated(); // 触发UI刷新在线列表
                qDebug() << "收到下线通知，移除用户：" << value;
            }
            return;
        }
    }

    // ========== 4. 处理普通消息（兼容原有逻辑） ==========
    QStringList parts = msg.split("|");
    if (parts.size() < 4) {
        qWarning() << "消息格式错误，跳过处理：" << msg;
        return;
    }

    QMap<QString, QString> msgMap;
    msgMap["sender"] = parts[0];
    msgMap["content"] = parts[2];
    msgMap["time"] = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

    // 本地保存（判空保护+强转枚举）
    if (m_sqlRepo) {
        m_sqlRepo->saveMessage(parts[0].toInt(), parts[1].toInt(), parts[2], static_cast<MsgType>(parts[3].toInt()));
    }
    // 更新Model（避免UI崩溃）
    if (m_messageModel) {
        m_messageModel->addMessage(msgMap);
    }
    emit msgReceived(msgMap);
}

// ========== 在线用户+私聊接口（增加判空保护） ==========
bool DataManager::addOnlineUser(int userId, const QString &account)
{
    if (!m_sqlRepo) return false;
    bool ret = m_sqlRepo->addOnlineUser(userId, account);
    qDebug() << "添加在线用户到数据库：" << account << " -> " << ret;
    return ret;
}

bool DataManager::removeOnlineUser(int userId)
{
    if (!m_sqlRepo) return false;
    bool ret = m_sqlRepo->removeOnlineUser(userId);
    qDebug() << "从数据库移除在线用户：" << userId << " -> " << ret;
    return ret;
}

QMap<int, QString> DataManager::getOnlineUserList(int currentUserId)
{
    if (!m_sqlRepo) return QMap<int, QString>();
    QMap<int, QString> list = m_sqlRepo->getOnlineUserList(currentUserId);
    qDebug() << "获取在线用户列表：" << list;
    return list;
}

bool DataManager::sendPrivateMessage(int senderId, int receiverId, const QString &content)
{
    if (!m_sqlRepo) return false;
    bool saveOk = m_sqlRepo->sendPrivateMessage(senderId, receiverId, content);
    if (!saveOk) return false;

    // 强转枚举，适配globaldefine
    QString msg = QString("%1|%2|%3|%4")
                      .arg(senderId)
                      .arg(receiverId)
                      .arg(content)
                      .arg(static_cast<int>(MsgType::Private));
    m_netMgr->sendMsg(msg);
    return true;
}

QList<QMap<QString, QString>> DataManager::getPrivateChatHistory(int currentUserId, int targetUserId)
{
    if (!m_sqlRepo) return QList<QMap<QString, QString>>();
    return m_sqlRepo->getPrivateChatHistory(currentUserId, targetUserId);
}
