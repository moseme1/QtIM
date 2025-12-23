#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QWidget>

namespace Ui { class LoginWindow; }

class LoginWindow : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();

    // 核心：添加loginFinish信号声明（QString类型参数）
signals:
    void loginFinish(const QString &userName);

private:
    Ui::LoginWindow *ui;
};
#endif // LOGINWINDOW_H
