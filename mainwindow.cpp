#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QDebug>
#include <QStandardItem>
#include <QMessageBox>
#include "datamanager.h"
#include "globaldefine.h"

MainWindow::MainWindow(QString loginUser, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_loginUser(loginUser)
    , m_currentChatUser("")
    , m_currentChatUserId(0)
{
    ui->setupUi(this);
    this->setWindowTitle(QString("QtIM - %1").arg(m_loginUser));

    // ========== 1. 初始化在线用户列表 ==========
    m_contactModel = new QStandardItemModel(this);
    ui->lv_contacts->setModel(m_contactModel);

    // 注释DataManager绑定（避免与服务器广播冲突，仅改这里）
    // connect(DataManager::getInstance(), &DataManager::onlineUserUpdated, this, [=]() {
    //     updateOnlineUserList();
    // });

    // 注释手动刷新（仅改这里）
    // updateOnlineUserList();

    // ========== 2. 初始化网络连接 ==========
    m_socket = new QTcpSocket(this);
    m_socket->connectToHost("127.0.0.1", 8888);
    connect(m_socket, &QTcpSocket::connected, this, &MainWindow::onSocketConnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &MainWindow::onSocketReadyRead);
    connect(m_socket, &QTcpSocket::disconnected, this, [=]() {
        ui->browser_chat->append("系统：与服务器断开连接！");
        // 新增：广播窗口也提示断开
        ui->browser_broadcast->append("系统：与服务器断开连接！");
    });

    // ========== 3. 初始化聊天区域 ==========
    ui->browser_chat->setVisible(false);
    ui->edit_input->setVisible(false);
    ui->btn_send->setVisible(false);
    // 新增：广播窗口默认显示（无需隐藏）
    ui->browser_broadcast->setVisible(true);
    ui->edit_broadcast_input->setVisible(true);
    ui->btn_broadcast_send->setVisible(true);
}

MainWindow::~MainWindow()
{
    delete ui;
    if(m_socket->isOpen()){
        m_socket->close();
    }
}

// 联系人点击事件（原有逻辑不动）
void MainWindow::on_lv_contacts_clicked(const QModelIndex &index)
{
    // 获取选中的在线用户账号
    m_currentChatUser = index.data().toString();
    m_currentChatUserId = 0; // 服务器按账号匹配，ID置空

    // 更新提示文字
    ui->label_tip->setText(QString("与%1聊天中").arg(m_currentChatUser));
    // 显示聊天控件
    ui->browser_chat->setVisible(true);
    ui->edit_input->setVisible(true);
    ui->btn_send->setVisible(true);
    // 清空输入框
    ui->edit_input->clear();
}

// 发送按钮点击事件（原有逻辑不动）
void MainWindow::on_btn_send_clicked()
{
    // 1. 校验
    QString msgContent = ui->edit_input->toPlainText().trimmed();
    if(msgContent.isEmpty()) {
        QMessageBox::warning(this, "提示", "消息内容不能为空！");
        return;
    }
    if(m_currentChatUser.isEmpty()){
        ui->browser_chat->append("系统：请先选择有效的在线联系人！");
        return;
    }
    if(!m_socket->isWritable()){
        ui->browser_chat->append("系统：未连接到服务器！");
        return;
    }

    // 2. 显示自己的消息
    ui->browser_chat->append(QString("我：%1").arg(msgContent));
    ui->edit_input->clear();

    // 3. 发送3段式格式：发送方账号|接收方账号|内容
    QString sendMsg = QString("%1|%2|%3")
                          .arg(m_loginUser)          // 自身账号
                          .arg(m_currentChatUser)    // 接收方账号
                          .arg(msgContent);
    m_socket->write(sendMsg.toUtf8());
    m_socket->flush();
    qDebug() << "[发送消息] 格式：" << sendMsg;
}

// 新增：广播发送按钮点击事件
void MainWindow::on_btn_broadcast_send_clicked()
{
    // 1. 校验
    QString broadcastContent = ui->edit_broadcast_input->toPlainText().trimmed();
    if(broadcastContent.isEmpty()) {
        QMessageBox::warning(this, "提示", "广播消息内容不能为空！");
        return;
    }
    if(!m_socket->isWritable()){
        ui->browser_broadcast->append("系统：未连接到服务器，无法发送广播！");
        return;
    }

    // 2. 显示自己发送的广播消息
    ui->browser_broadcast->append(QString("我（广播）：%1").arg(broadcastContent));
    ui->edit_broadcast_input->clear();

    // 3. 发送广播消息：格式为「broadcast|发送方账号|消息内容」
    QString broadcastMsg = QString("broadcast|%1|%2")
                               .arg(m_loginUser)
                               .arg(broadcastContent);
    m_socket->write(broadcastMsg.toUtf8());
    m_socket->flush();
    qDebug() << "[发送广播] 格式：" << broadcastMsg;
}

// 连接服务器成功（原有逻辑+广播窗口提示）
void MainWindow::onSocketConnected()
{
    ui->browser_chat->append("系统：已连接到聊天服务器！");
    // 新增：广播窗口也提示连接成功
    ui->browser_broadcast->append("系统：已连接到聊天服务器！");

    // 发送上线通知（账号格式）
    QString onlineMsg = QString("online|%1|%2|%3")
                            .arg(DataManager::getInstance()->getCurrentUserId())
                            .arg(m_loginUser)
                            .arg(m_loginUser); // 昵称暂用账号
    m_socket->write(onlineMsg.toUtf8());
    m_socket->flush();
}

// 接收服务器消息之列表刷新
void MainWindow::onSocketReadyRead()
{
    QByteArray data = m_socket->readAll();
    QString recvData = QString::fromUtf8(data);
    QStringList msgList = recvData.split("\n", Qt::SkipEmptyParts);

    for (QString recvMsg : msgList) {
        recvMsg = recvMsg.trimmed();
        qDebug() << "[接收单条消息]：" << recvMsg;

        // ========== 1. 解析在线列表广播（仅新增刷新） ==========
        if(recvMsg.startsWith("SYSTEM|ONLINE_USERS")){
            QStringList parts = recvMsg.split("|", Qt::SkipEmptyParts);
            if (parts.size() >= 3) {
                QStringList onlineAccounts = parts[2].split(",", Qt::SkipEmptyParts);
                // 直接更新在线列表UI
                m_contactModel->clear();
                m_accountToUserId.clear();
                for (QString account : onlineAccounts) {
                    QStandardItem *item = new QStandardItem(account);
                    m_contactModel->appendRow(item);
                    m_accountToUserId[account] = 0; // 临时置空ID
                }
                // 新增：强制刷新ListView（仅加这两行）
                ui->lv_contacts->update();
                ui->lv_contacts->repaint();

                qDebug() << "[手动更新在线列表]：" << onlineAccounts;
            }
            continue; // 解析完在线列表，跳过后续逻辑
        }


        // ========== 3. 原有私聊消息解析逻辑（不动） ==========
        QStringList parts = recvMsg.split("|", Qt::SkipEmptyParts);
        if(parts.size() == 2){
            QString senderAccount = parts[0];
            QString content = parts[1];
            ui->browser_chat->append(QString("%1：%2").arg(senderAccount).arg(content));
        } else if(parts.size() == 1){
            ui->browser_chat->append(QString("系统：%1").arg(recvMsg));
        } else {
            ui->browser_chat->append(QString("系统：未知消息格式 - %1").arg(recvMsg));
        }
    }
}

// 退出登录（原有逻辑不动）
void MainWindow::on_action_exit_login_triggered()
{
    // 发送下线通知（账号格式）
    if(m_socket->isWritable()){
        QString offlineMsg = QString("offline|%1").arg(m_loginUser);
        m_socket->write(offlineMsg.toUtf8());
        m_socket->flush();
    }
    this->close();
}

// 刷新在线用户列表UI（原有逻辑不动）
void MainWindow::updateOnlineUserList()
{
    // 1. 清空原有数据
    m_contactModel->clear();
    m_accountToUserId.clear();

    // 2. 获取最新在线用户列表
    int currentUserId = DataManager::getInstance()->getCurrentUserId();
    QMap<int, QString> onlineUsers = DataManager::getInstance()->getOnlineUserList(currentUserId);

    // 3. 填充列表并建立映射（保留显示自身的逻辑）
    for (auto it = onlineUsers.begin(); it != onlineUsers.end(); ++it) {
        int userId = it.key();
        QString account = it.value();

        // 保留显示自身（已删除排除自身的代码）
        QStandardItem *item = new QStandardItem(account);
        m_contactModel->appendRow(item);
        m_accountToUserId[account] = userId;
    }

    // 调试日志
    qDebug() << "[UI] 在线用户数：" << onlineUsers.size();
    qDebug() << "[UI] 账号→ID映射：" << m_accountToUserId;
}
