#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QObject>
#include "sqlrepository.h"
#include "contactmodel.h"
#include "messagemodel.h"
#include "networkmanager.h"

class DataManager : public QObject
{
    Q_OBJECT
public:
    static DataManager *getInstance();
    ~DataManager();

    // 用户操作
    bool userRegister(const QString &account, const QString &pwd, const QString &nickname);
    bool userLogin(const QString &account, const QString &pwd);

    // 数据获取
    ContactModel *getContactModel();
    MessageModel *getMessageModel();
    void loadContacts(int userId);
    void loadMessageHistory(int userId1, int userId2);

    // 消息操作
    void sendMessage(int senderId, int receiverId, const QString &content, MsgType type);

signals:
    void loginSuccess(const QString &nickname, int userId);
    void loginFailed(const QString &reason);
    void msgSentSuccess();
    void msgReceived(const QMap<QString, QString> &msg);

private slots:
    void onRecvNetworkMsg(const QString &msg);

private:
    explicit DataManager(QObject *parent = nullptr);
    static DataManager *m_instance;

    SqlRepository *m_sqlRepo;
    ContactModel *m_contactModel;
    MessageModel *m_messageModel;
    NetworkManager *m_netMgr;

    int m_currentUserId; // 当前登录用户ID
};

#endif // DATAMANAGER_H