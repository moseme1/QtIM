#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// 新增：包含网络头文件
#include <QTcpSocket>
// 新增：包含ListView模型头文件
#include <QStandardItemModel>
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QString loginUser, QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_btn_send_clicked();
    void on_lv_contacts_clicked(const QModelIndex &index);
    void onSocketReadyRead();
    void onSocketConnected();
    void on_action_exit_login_triggered();

private:
    Ui::MainWindow *ui;
    QTcpSocket *m_socket;               // 网络Socket（声明）
    QString m_loginUser;                // 当前登录用户名（声明）
    QString m_currentChatUser;          // 当前选中的联系人（声明）
    QStandardItemModel *m_contactModel; // ListView数据源模型（声明）
};
#endif // MAINWINDOW_H
