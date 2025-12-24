#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include <QWidget>
#include <QMap>       // 新增：存储在线用户/消息
#include <QTimer>     // 新增：定时刷新在线列表
#include <QListWidgetItem> // 新增：列表项点击参数

#include "ui_chatwindow.h"
#include "datamanager.h"

namespace Ui {
class ChatWindow;
}

class ChatWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ChatWindow(int userId, int friendId, QWidget *parent = nullptr);
    ~ChatWindow();

    // 新增：设置当前登录用户ID（用于在线列表）
    void setCurrentUserId(int userId);
    // 新增：初始化在线用户列表（UI新增的lw_online_users）
    void initOnlineUserList();

private slots:
    // ========== 保留你原有槽函数 ==========
    void on_btn_send_clicked(); // 自动关联发送按钮
    void onMsgReceived(const QMap<QString, QString> &msg);

    // ========== 新增：私聊相关槽函数 ==========
    void on_lw_online_users_itemClicked(QListWidgetItem *item); // 选择在线用户
    void refreshOnlineUserList(); // 定时刷新在线列表
    // 在private中新增：

private:
    // ========== 保留你原有成员变量 ==========
    void updateMsgDisplay(const QString &sender, const QString &content, const QString &time);

    Ui::ChatWindow *ui;
    int m_userId;          // 原有：当前用户ID
    int m_friendId;        // 原有：好友ID
    DataManager *m_dataMgr;// 原有：数据管理类

    // ========== 新增：私聊/在线列表相关变量 ==========
    int m_targetUserId;        // 选中的私聊对象ID（复用m_friendId，优先级更高）
    QString m_targetUserAccount; // 选中的私聊对象账号
    QTimer *m_refreshTimer;    // 定时刷新在线列表的定时器

    // 新增：加载私聊消息历史（适配新的private_chat表）
    void loadPrivateChatHistory();
};

#endif // CHATWINDOW_H
