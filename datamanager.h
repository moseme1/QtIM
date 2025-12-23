#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QObject>
#include <QMap>   // 新增：用于在线用户列表的映射
#include <QList>  // 原有已包含，无需重复加
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

    // ========== 新增：对外暴露当前用户ID（UI层必需） ==========
    int getCurrentUserId() const;

    // ========== 原有接口（完全保留，未做任何修改） ==========
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

    // ========== 新增：私聊+在线用户核心接口（仅加这部分） ==========
    // 在线用户管理（供LoginWindow/主窗口调用）
    bool addOnlineUser(int userId, const QString &account);       // 登录后添加在线用户
    bool removeOnlineUser(int userId);                            // 登出/关闭程序移除在线用户
    QMap<int, QString> getOnlineUserList(int currentUserId);      // 获取在线用户列表（排除自己）

    // 私聊消息管理（供聊天窗口调用）
    bool sendPrivateMessage(int senderId, int receiverId, const QString &content); // 发送私聊消息
    QList<QMap<QString, QString>> getPrivateChatHistory(int currentUserId, int targetUserId); // 获取私聊记录

signals:
    // ========== 原有信号（完全保留） ==========
    void loginSuccess(const QString &nickname, int userId);
    void loginFailed(const QString &reason);
    void msgSentSuccess();
    void msgReceived(const QMap<QString, QString> &msg);

    // ========== 核心新增：在线用户更新信号（解决多客户端同步） ==========
    void onlineUserUpdated();

private slots:
    // ========== 原有槽函数（完全保留） ==========
    void onRecvNetworkMsg(const QString &msg);

    // ========== 新增：连接成功处理槽函数（解决lambda崩溃） ==========
    void onConnSuccess();

private:
    // ========== 原有成员变量（完全保留） ==========
    explicit DataManager(QObject *parent = nullptr);
    static DataManager *m_instance;

    SqlRepository *m_sqlRepo;
    ContactModel *m_contactModel;
    MessageModel *m_messageModel;
    NetworkManager *m_netMgr;

    int m_currentUserId; // 当前登录用户ID
};

#endif // DATAMANAGER_H
