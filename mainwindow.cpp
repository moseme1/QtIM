#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QDebug>
#include <QStandardItem>
#include <QStandardItem>

MainWindow::MainWindow(QString loginUser, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_loginUser(loginUser)
    , m_currentChatUser("")
{
    ui->setupUi(this);
    this->setWindowTitle(QString("QtIM - %1").arg(m_loginUser));

    // ========== 1. 初始化联系人列表（QListView） ==========
    m_contactModel = new QStandardItemModel(this);
    // 添加测试联系人
    QStringList contacts = {"用户A", "用户B", "用户C"};
    for(const QString &name : contacts){
        QStandardItem *item = new QStandardItem(name);
        m_contactModel->appendRow(item);
    }
    ui->lv_contacts->setModel(m_contactModel);

    // ========== 2. 初始化网络连接 ==========
    m_socket = new QTcpSocket(this);
    // 连接本地服务器（IP/端口根据实际修改）
    m_socket->connectToHost("127.0.0.1", 8888);
    // 绑定网络信号
    connect(m_socket, &QTcpSocket::connected, this, &MainWindow::onSocketConnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &MainWindow::onSocketReadyRead);

    // ========== 3. 初始化聊天区域 ==========
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

// 联系人列表点击（QListView）
void MainWindow::on_lv_contacts_clicked(const QModelIndex &index)
{
    // 获取选中的联系人名称
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
        ui->browser_chat->append("系统：请先选择联系人！");
        return;
    }
    if(!m_socket->isWritable()){
        ui->browser_chat->append("系统：未连接到服务器！");
        return;
    }

    // 2. 显示自己的消息
    ui->browser_chat->append(QString("我：%1").arg(msgContent));
    ui->edit_input->clear();

    // 3. 封装消息并发送到服务器
    QString sendMsg = QString("%1|%2|%3")
                          .arg(m_loginUser)
                          .arg(m_currentChatUser)
                          .arg(msgContent);
    m_socket->write(sendMsg.toUtf8());
}

// 连接服务器成功
void MainWindow::onSocketConnected()
{
    ui->browser_chat->append("系统：已连接到聊天服务器！");
}

// 接收服务器消息
void MainWindow::onSocketReadyRead()
{
    QByteArray data = m_socket->readAll();
    QString recvMsg = QString::fromUtf8(data);

    QStringList parts = recvMsg.split("|");
    if(parts.size() == 2){
        // 正常聊天消息：对方用户名|内容
        ui->browser_chat->append(QString("%1：%2").arg(parts[0]).arg(parts[1]));
    } else {
        // 系统提示
        ui->browser_chat->append(QString("系统：%1").arg(recvMsg));
    }
}

// 退出登录
void MainWindow::on_action_exit_login_triggered()
{
    // 关闭主窗口，返回登录界面（可根据你的逻辑修改）
    this->close();
}
