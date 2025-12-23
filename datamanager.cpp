#include "datamanager.h"
#include <QDateTime>

DataManager *DataManager::m_instance = nullptr;

DataManager::DataManager(QObject *parent) : QObject(parent)
{
    m_sqlRepo = new SqlRepository();
    m_contactModel = new ContactModel();
    m_messageModel = new MessageModel();
    m_netMgr = NetworkManager::getInstance();

    connect(m_netMgr, &NetworkManager::recvMsg, this, &DataManager::onRecvNetworkMsg);
}

DataManager::~DataManager()
{
    delete m_sqlRepo;
    delete m_contactModel;
    delete m_messageModel;
}

DataManager *DataManager::getInstance()
{
    if (!m_instance) {
        m_instance = new DataManager();
    }
    return m_instance;
}

bool DataManager::userRegister(const QString &account, const QString &pwd, const QString &nickname)
{
    return m_sqlRepo->registerUser(account, pwd, nickname);
}

bool DataManager::userLogin(const QString &account, const QString &pwd)
{
    QString nickname;
    int userId;
    if (m_sqlRepo->loginUser(account, pwd, nickname, userId)) {
        m_currentUserId = userId;
        emit loginSuccess(nickname, userId);
        m_netMgr->connectServer(); // 登录成功后连接服务器
        loadContacts(userId);
        return true;
    } else {
        emit loginFailed("账号或密码错误");
        return false;
    }
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
    auto contacts = m_sqlRepo->getContacts(userId);
    m_contactModel->setContacts(contacts);
}

void DataManager::loadMessageHistory(int userId1, int userId2)
{
    auto messages = m_sqlRepo->getMessageHistory(userId1, userId2);
    m_messageModel->setMessages(messages);
}

void DataManager::sendMessage(int senderId, int receiverId, const QString &content, MsgType type)
{
    // 本地保存消息
    m_sqlRepo->saveMessage(senderId, receiverId, content, type);
    // 网络发送消息（格式：senderId|receiverId|content|type）
    QString msg = QString("%1|%2|%3|%4").arg(senderId).arg(receiverId).arg(content).arg(type);
    m_netMgr->sendMsg(msg);

    // 更新本地Model
    QMap<QString, QString> msgMap;
    msgMap["sender"] = QString::number(senderId);
    msgMap["content"] = content;
    msgMap["time"] = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    m_messageModel->addMessage(msgMap);

    emit msgSentSuccess();
}

void DataManager::onRecvNetworkMsg(const QString &msg)
{
    // 解析消息：senderId|receiverId|content|type
    QStringList parts = msg.split("|");
    if (parts.size() < 4) return;

    QMap<QString, QString> msgMap;
    msgMap["sender"] = parts[0];
    msgMap["content"] = parts[2];
    msgMap["time"] = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

    // 本地保存
    m_sqlRepo->saveMessage(parts[0].toInt(), parts[1].toInt(), parts[2], (MsgType)parts[3].toInt());
    // 更新Model
    m_messageModel->addMessage(msgMap);
    emit msgReceived(msgMap);
}
