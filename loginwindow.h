#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QWidget>

namespace Ui {
class LoginWindow;
}

class LoginWindow : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();

signals:
    void loginFinish(int userId); // 仅保留登录成功的信号

private:
    Ui::LoginWindow *ui;
};

#endif // LOGINWINDOW_H
