#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// 原有头文件（不动）
#include <QTcpSocket>
#include <QStandardItemModel>
#include <QMap>
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
    // 原有槽函数（完全不动）
    void on_btn_send_clicked();
    void on_lv_contacts_clicked(const QModelIndex &index);
    void onSocketReadyRead();
    void onSocketConnected();
    void on_action_exit_login_triggered();
    void updateOnlineUserList();

    // ========== 仅新增这1个槽函数（广播发送） ==========
    void on_btn_broadcast_send_clicked();

private:
    // 原有成员变量（完全不动）
    Ui::MainWindow *ui;
    QTcpSocket *m_socket;
    QString m_loginUser;
    QString m_currentChatUser;
    int m_currentChatUserId;
    QStandardItemModel *m_contactModel;
    QMap<QString, int> m_accountToUserId;
};
#endif // MAINWINDOW_H
