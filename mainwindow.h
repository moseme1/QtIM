#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
// 提前声明类，避免头文件循环包含
class ChatWindow;
class DataManager;

// 引入UI命名空间（Qt Designer生成）
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    // 构造函数：接收当前登录用户ID
    explicit MainWindow(int userId, QWidget *parent = nullptr);
    // 析构函数：释放UI对象
    ~MainWindow();

private slots:
    // 联系人列表点击槽函数（自动关联UI控件）
    void on_lv_contacts_clicked(const QModelIndex &index);
    // 退出登录菜单槽函数（自动关联UI控件）
    void on_action_exit_login_triggered();

private:
    Ui::MainWindow *ui;                  // UI对象指针
    int m_currentUserId;                 // 当前登录用户ID
    DataManager *m_dataMgr;              // 业务层实例
    QMap<int, ChatWindow*> m_chatWindows;// 缓存聊天窗口（key=好友ID，value=聊天窗口）
};

#endif // MAINWINDOW_H
