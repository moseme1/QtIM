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
    // 登录成功后打开主窗口（传递用户名，而非int的userId）
    QObject::connect(loginWin, &LoginWindow::loginFinish, [=](const QString &userName) {
        // 修正：传入QString类型的用户名，匹配MainWindow构造函数
        MainWindow *mainWin = new MainWindow(userName);
        mainWin->show();
    });
    // 显示登录窗口
    loginWin->show();

    return a.exec();
}
