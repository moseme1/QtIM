#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QDebug>
#include <QStandardItem>
#include "datamanager.h"  // 引入DataManager核心类
#include "globaldefine.h"  // 引入MsgType枚举

MainWindow::MainWindow(QString loginUser, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_loginUser(loginUser)
    , m_currentChatUser("")
{
    ui->setupUi(this);
    this->setWindowTitle(QString("QtIM - %1").arg(m_loginUser));

    // ========== 1. 初始化在线用户列表（替换原测试数据） ==========
    m_contactModel = new QStandardItemModel(this);
    ui->lv_contacts->setModel(m_contactModel);  // 绑定到QListView

    // ========== 核心：绑定DataManager的在线更新信号 ==========
    // 在线列表数据更新时，自动刷新UI
    connect(DataManager::getInstance(), &DataManager::onlineUserUpdated, this, [=]() {
        updateOnlineUserList();
    });

    // 登录成功后手动刷新一次，确保初始列表正确
    updateOnlineUserList();

    // ========== 2. 初始化网络连接（保留原有逻辑） ==========
    m_socket = new QTcpSocket(this);
    m_socket->connectToHost("127.0.0.1", 8888);
    connect(m_socket, &QTcpSocket::connected, this, &MainWindow::onSocketConnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &MainWindow::onSocketReadyRead);

    // ========== 3. 初始化聊天区域（保留原有逻辑） ==========
    ui->browser_chat->setVisible(false); // 未选联系人时隐藏聊天区
    ui->edit_input->setVisible(false);
    ui->btn_send->setVisible(false);
}

MainWindow::~MainWindow()
{
    delete ui;
    if(m_socket->isOpen()){
        m_socket->close();
    }
}

// 联系人/在线列表点击（QListView）
void MainWindow::on_lv_contacts_clicked(const QModelIndex &index)
{
    // 获取选中的在线用户名称
    m_currentChatUser = index.data().toString();
    // 更新提示文字
    ui->label_tip->setText(QString("与%1聊天中").arg(m_currentChatUser));
    // 显示聊天控件
    ui->browser_chat->setVisible(true);
    ui->edit_input->setVisible(true);
    ui->btn_send->setVisible(true);
    // 清空输入框
    ui->edit_input->clear();
}

// 发送按钮点击
void MainWindow::on_btn_send_clicked()
{
    // 1. 校验
    QString msgContent = ui->edit_input->toPlainText().trimmed();
    if(msgContent.isEmpty()) return;
    if(m_currentChatUser.isEmpty()){
        ui->browser_chat->append("系统：请先选择在线联系人！");
        return;
    }
    if(!m_socket->isWritable()){
        ui->browser_chat->append("系统：未连接到服务器！");
        return;
    }

    // 2. 显示自己的消息
    ui->browser_chat->append(QString("我：%1").arg(msgContent));
    ui->edit_input->clear();

    // 3. 封装消息并发送到服务器（适配DataManager的消息格式）
    QString sendMsg = QString("%1|%2|%3|%4")
                          .arg(DataManager::getInstance()->getCurrentUserId())
                          .arg(m_currentChatUser) // 目标用户账号
                          .arg(msgContent)
                          .arg(static_cast<int>(MsgType::Private)); // 私聊类型
    m_socket->write(sendMsg.toUtf8());
}

// 连接服务器成功
void MainWindow::onSocketConnected()
{
    ui->browser_chat->append("系统：已连接到聊天服务器！");
}

// 接收服务器消息（修复getSqlRepo()报错）
void MainWindow::onSocketReadyRead()
{
    QByteArray data = m_socket->readAll();
    QString recvMsg = QString::fromUtf8(data);

    // 兼容在线列表广播和聊天消息
    if(recvMsg.startsWith("SYSTEM|ONLINE_USERS")){
        // 收到在线列表广播，交由DataManager处理后自动刷新UI
        return;
    }

    QStringList parts = recvMsg.split("|");
    if(parts.size() >= 4){
        // 标准聊天消息：发送者ID|接收者|内容|消息类型
        int senderId = parts[0].toInt();
        QString senderAccount;

        // 方法：从在线用户列表中根据ID匹配账号（无需访问SqlRepository）
        QMap<int, QString> onlineUsers = DataManager::getInstance()->getOnlineUserList(
            DataManager::getInstance()->getCurrentUserId()
            );
        if (onlineUsers.contains(senderId)) {
            senderAccount = onlineUsers.value(senderId);
        } else {
            senderAccount = QString("未知用户(%1)").arg(senderId); // 未找到时的默认显示
        }

        ui->browser_chat->append(QString("%1：%2").arg(senderAccount).arg(parts[2]));
    } else if(parts.size() == 2){
        // 兼容旧格式：对方用户名|内容
        ui->browser_chat->append(QString("%1：%2").arg(parts[0]).arg(parts[1]));
    } else {
        // 系统提示
        ui->browser_chat->append(QString("系统：%1").arg(recvMsg));
    }
}

// 退出登录
void MainWindow::on_action_exit_login_triggered()
{
    // 退出前发送下线通知
    if(m_socket->isWritable()){
        QString offlineMsg = QString("offline|%1").arg(DataManager::getInstance()->getCurrentUserId());
        m_socket->write(offlineMsg.toUtf8());
    }
    // 关闭主窗口，返回登录界面
    this->close();
}

// 新增：刷新在线用户列表UI（核心！）
void MainWindow::updateOnlineUserList()
{
    // 1. 清空原有列表
    m_contactModel->clear();

    // 2. 从DataManager获取最新在线用户列表
    int currentUserId = DataManager::getInstance()->getCurrentUserId();
    QMap<int, QString> onlineUsers = DataManager::getInstance()->getOnlineUserList(currentUserId);

    // 3. 将在线用户添加到QListView中
    for (const QString &account : onlineUsers.values()) {
        QStandardItem *item = new QStandardItem(account);  // 显示在线账号
        m_contactModel->appendRow(item);
    }

    // 调试日志：查看在线用户数量
    qDebug() << "[UI] 在线列表已刷新，当前在线用户数：" << onlineUsers.size();
}
