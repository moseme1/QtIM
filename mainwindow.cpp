#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "datamanager.h"
#include "chatwindow.h"
#include "loginwindow.h"
#include <QModelIndex>

// 构造函数
MainWindow::MainWindow(int userId, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow()),
    m_currentUserId(userId)
{
    // 加载UI布局
    ui->setupUi(this);
    // 获取业务层单例
    m_dataMgr = DataManager::getInstance();

    // 绑定联系人Model到ListView（显示昵称列）
    ui->lv_contacts->setModel(m_dataMgr->getContactModel());
    ui->lv_contacts->setModelColumn(1);

    // 设置窗口标题（显示当前用户ID）
    setWindowTitle(QString("QtIM - 用户%1").arg(userId));
}

// 析构函数
MainWindow::~MainWindow()
{
    // 释放UI对象
    delete ui;
}

// 联系人列表点击事件（自动关联UI控件 lv_contacts）
void MainWindow::on_lv_contacts_clicked(const QModelIndex &index)
{
    // 获取选中联系人的ID（从ContactModel的IdRole获取）
    int friendId = index.data(257).toInt(); // IdRole = Qt::UserRole +1 = 257

    // 若已打开该联系人的聊天窗口，直接切换
    if (m_chatWindows.contains(friendId)) {
        ui->stacked_chat->setCurrentWidget(m_chatWindows[friendId]);
        return;
    }

    // 未打开则创建新的聊天窗口
    ChatWindow *chatWin = new ChatWindow(m_currentUserId, friendId);
    // 缓存聊天窗口
    m_chatWindows[friendId] = chatWin;
    // 添加到堆叠窗口并切换
    ui->stacked_chat->addWidget(chatWin);
    ui->stacked_chat->setCurrentWidget(chatWin);

    // 加载该联系人的历史消息
    m_dataMgr->loadMessageHistory(m_currentUserId, friendId);
}

// 退出登录菜单点击事件（自动关联UI控件 action_exit_login）
void MainWindow::on_action_exit_login_triggered()
{
    // 断开网络连接
    NetworkManager::getInstance()->disconnectServer();
    // 关闭主窗口
    this->close();

    // 重新打开登录窗口
    LoginWindow *loginWin = new LoginWindow();
    loginWin->show();
}
