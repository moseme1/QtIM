#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QWidget>
// 包含自动生成的UI头文件
#include "ui_loginwindow.h"
#include "datamanager.h"

namespace Ui {
class LoginWindow; // 声明UI类
}

class LoginWindow : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow(); // 必须添加析构函数释放UI

signals:
    void loginFinish(int userId);

private slots:
    void on_loginBtn_clicked();    // 自动关联btn_login的clicked信号
    void on_registerBtn_clicked(); // 自动关联btn_register的clicked信号
    void onLoginSuccess(const QString &nickname, int userId);
    void onLoginFailed(const QString &reason);

private:
    Ui::LoginWindow *ui; // UI对象指针
    DataManager *m_dataMgr;
    bool m_isRegisterMode;
};

#endif // LOGINWINDOW_H