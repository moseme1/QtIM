#include <QApplication>
#include "loginwindow.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    // Qt6高DPI适配
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QApplication a(argc, argv);

    // 创建登录窗口
    LoginWindow *loginWin = new LoginWindow();
    // 登录成功后打开主窗口
    QObject::connect(loginWin, &LoginWindow::loginFinish, [=](int userId) {
        MainWindow *mainWin = new MainWindow(userId);
        mainWin->show();
    });
    // 显示登录窗口
    loginWin->show();

    return a.exec();
}
