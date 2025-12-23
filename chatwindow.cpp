#include "chatwindow.h"
#include <QDateTime>
#include <QListWidgetItem>  // 列表项操作
#include <QTimer>           // 定时刷新
#include <QPushButton>      // 手动绑定发送按钮
#include <QListWidget>      // 手动绑定列表点击
#include <QDebug>           // 新增：调试日志
#include "globaldefine.h"   // 确保SingleChat/Private枚举存在

// 构造函数初始化列表补充：m_refreshTimer初始化
ChatWindow::ChatWindow(int userId, int friendId, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChatWindow()),
    m_userId(userId),
    m_friendId(friendId),
    m_targetUserId(friendId),  // 默认使用传入的friendId作为私聊目标
    m_refreshTimer(new QTimer(this)),
    m_dataMgr(nullptr)  // 先初始化为nullptr，避免野指针
{
    ui->setupUi(this);
    m_dataMgr = DataManager::getInstance(); // 初始化数据管理类

    // ========== 核心修复：手动绑定所有信号（删除UI自动绑定） ==========
    // 1. 输入框文本变化 → 控制发送按钮启用状态
    connect(ui->le_msg_input, &QLineEdit::textChanged, this, [=](const QString &text) {
        ui->btn_send->setEnabled(!text.trimmed().isEmpty() && m_targetUserId != -1);
    });

    // 2. 发送按钮点击 → 绑定到on_btn_send_clicked
    connect(ui->btn_send, &QPushButton::clicked, this, &ChatWindow::on_btn_send_clicked);

    // 3. 消息接收信号 → 绑定到onMsgReceived
    connect(m_dataMgr, &DataManager::msgReceived, this, &ChatWindow::onMsgReceived);

    // 4. 在线用户列表点击 → 绑定到on_lw_online_users_itemClicked
    connect(ui->lw_online_users, &QListWidget::itemClicked, this, &ChatWindow::on_lw_online_users_itemClicked);

    // ========== 核心新增：绑定在线用户更新信号（解决多客户端同步） ==========
    connect(m_dataMgr, &DataManager::onlineUserUpdated, this, &ChatWindow::refreshOnlineUserList);

    // ========== 在线用户列表初始化 ==========
    // 1. 初始化在线用户列表
    initOnlineUserList();

    // 2. 定时刷新在线列表（1秒/次）
    m_refreshTimer->setInterval(1000);
    connect(m_refreshTimer, &QTimer::timeout, this, &ChatWindow::refreshOnlineUserList);
    m_refreshTimer->start();

    // ========== 窗口标题初始化 ==========
    setWindowTitle(QString("与%1聊天").arg(m_friendId));

    // ========== 加载历史私聊消息 ==========
    loadPrivateChatHistory();

    qDebug() << "ChatWindow初始化完成，当前用户ID：" << m_userId;
}

// 析构函数
ChatWindow::~ChatWindow()
{
    if (m_refreshTimer) { // 判空避免空指针
        m_refreshTimer->stop();
        delete m_refreshTimer;
        m_refreshTimer = nullptr;
    }
    delete ui;
}

// ========== 新增：初始化在线用户列表（完整实现） ==========
void ChatWindow::initOnlineUserList()
{
    if (m_userId == -1 || !m_dataMgr) return; // 判空保护

    // 清空原有列表
    ui->lw_online_users->clear();

    // 获取在线用户列表（排除当前用户）
    QMap<int, QString> onlineList = m_dataMgr->getOnlineUserList(m_userId);
    qDebug() << "获取到在线用户列表：" << onlineList;

    // 填充列表项
    for (auto it = onlineList.begin(); it != onlineList.end(); ++it) {
        int userId = it.key();
        QString account = it.value();

        QListWidgetItem *item = new QListWidgetItem(account);
        item->setData(Qt::UserRole, userId); // 存储用户ID
        ui->lw_online_users->addItem(item);

        // 若当前好友在在线列表中，默认选中
        if (userId == m_friendId) {
            ui->lw_online_users->setCurrentItem(item);
        }
    }
}

// ========== 新增：定时刷新在线用户列表 ==========
void ChatWindow::refreshOnlineUserList()
{
    if (!ui->lw_online_users || !m_dataMgr) return; // 判空保护

    // 保存当前选中项ID
    QListWidgetItem *currentItem = ui->lw_online_users->currentItem();
    int lastTargetId = -1;
    if (currentItem) {
        lastTargetId = currentItem->data(Qt::UserRole).toInt();
    }

    // 重新加载列表
    initOnlineUserList();

    // 恢复选中项
    if (lastTargetId != -1) {
        for (int i = 0; i < ui->lw_online_users->count(); ++i) {
            QListWidgetItem *item = ui->lw_online_users->item(i);
            if (item && item->data(Qt::UserRole).toInt() == lastTargetId) {
                ui->lw_online_users->setCurrentItem(item);
                break;
            }
        }
    }
}

// ========== 新增：在线用户列表点击事件 ==========
void ChatWindow::on_lw_online_users_itemClicked(QListWidgetItem *item)
{
    if (!item || !m_dataMgr) return; // 判空保护

    // 更新私聊目标ID和账号
    m_targetUserId = item->data(Qt::UserRole).toInt();
    QString targetAccount = item->text();
    qDebug() << "选中私聊对象：" << targetAccount << "(ID:" << m_targetUserId << ")";

    // 更新窗口标题
    setWindowTitle(QString("与%1聊天").arg(targetAccount));

    // 清空聊天框并加载历史消息
    ui->te_msg_display->clear();
    loadPrivateChatHistory();

    // 激活输入框
    ui->le_msg_input->setFocus();
}

// ========== 新增：加载私聊历史消息 ==========
void ChatWindow::loadPrivateChatHistory()
{
    if (m_userId == -1 || m_targetUserId == -1 || !m_dataMgr) return; // 判空保护

    // 获取私聊历史记录
    QList<QMap<QString, QString>> chatHistory = m_dataMgr->getPrivateChatHistory(m_userId, m_targetUserId);
    qDebug() << "加载私聊历史消息数量：" << chatHistory.size();

    // 清空聊天框
    ui->te_msg_display->clear();

    // 遍历展示历史消息
    for (const auto &msg : chatHistory) {
        int senderId = msg["sender_id"].toInt();
        QString content = msg["content"];
        QString time = msg["send_time"]; // 修复：QString无需toString()

        // 展示消息
        updateMsgDisplay(QString::number(senderId), content, time);
    }
}

// ========== 发送按钮点击事件（核心逻辑） ==========
void ChatWindow::on_btn_send_clicked()
{
    if (m_userId == -1 || m_targetUserId == -1 || !m_dataMgr) return; // 判空保护

    // 获取输入内容
    QString content = ui->le_msg_input->text().trimmed();
    if (content.isEmpty()) return;

    // 发送私聊消息
    bool success = m_dataMgr->sendPrivateMessage(m_userId, m_targetUserId, content);
    if (success) {
        qDebug() << "私聊消息发送成功：" << content;
        // 清空输入框
        ui->le_msg_input->clear();
        // 展示发送的消息
        updateMsgDisplay(
            QString::number(m_userId),
            content,
            QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
            );
    } else {
        qDebug() << "私聊消息发送失败！";
    }
}

// ========== 消息接收事件 ==========
void ChatWindow::onMsgReceived(const QMap<QString, QString> &msg)
{
    if (m_targetUserId == -1 || !m_dataMgr) return; // 判空保护

    // 仅展示当前私聊对象的消息
    if (msg["sender"].toInt() == m_targetUserId) {
        qDebug() << "收到私聊消息：" << msg["content"];
        updateMsgDisplay(msg["sender"], msg["content"], msg["time"]);
    }
}

// ========== 消息展示格式化 ==========
void ChatWindow::updateMsgDisplay(const QString &sender, const QString &content, const QString &time)
{
    if (!ui->te_msg_display) return; // 判空保护

    QString msgHtml;
    if (sender.toInt() == m_userId) {
        // 自己的消息：居右，蓝色背景
        msgHtml = QString("<div style='text-align: right; margin: 5px;'>"
                          "<span style='background-color: #66ccff; padding: 5px; border-radius: 5px;'>%1</span>"
                          "<br><small style='color: #999;'>%2</small>"
                          "</div>").arg(content).arg(time);
    } else {
        // 对方的消息：居左，灰色背景
        msgHtml = QString("<div style='text-align: left; margin: 5px;'>"
                          "<span style='background-color: #eeeeee; padding: 5px; border-radius: 5px;'>%1</span>"
                          "<br><small style='color: #999;'>%2</small>"
                          "</div>").arg(content).arg(time);
    }

    // 插入HTML并滚动到底部
    ui->te_msg_display->insertHtml(msgHtml);
    QTextCursor cursor = ui->te_msg_display->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->te_msg_display->setTextCursor(cursor);
}
