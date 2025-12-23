#include "loginwindow.h"
#include "ui_loginwindow.h"
#include "datamanager.h"
#include <QDebug>

LoginWindow::LoginWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LoginWindow())
{
    ui->setupUi(this);

    // 1. 强制显示昵称输入框（避免UI中隐藏）
    ui->le_nickname->setVisible(true);

    // 2. 手动绑定注册按钮
    connect(ui->btn_register, &QPushButton::clicked, this, [=](){
        qDebug() << "注册按钮点击";
        QString acc = ui->le_account->text().trimmed();
        QString pwd = ui->le_pwd->text().trimmed();
        QString nick = ui->le_nickname->text().trimmed();
        if(acc.isEmpty() || pwd.isEmpty() || nick.isEmpty()){
            ui->label_tip->setText("账号/密码/昵称不能为空");
            return;
        }
        bool ok = DataManager::getInstance()->userRegister(acc, pwd, nick);
        ui->label_tip->setText(ok ? "注册成功，请登录" : "账号已存在");
    });

    // 3. 手动绑定登录按钮
    connect(ui->btn_login, &QPushButton::clicked, this, [=](){
        qDebug() << "登录按钮点击";
        QString acc = ui->le_account->text().trimmed();
        QString pwd = ui->le_pwd->text().trimmed();
        if(acc.isEmpty() || pwd.isEmpty()){
            ui->label_tip->setText("账号/密码不能为空");
            return;
        }
        DataManager::getInstance()->userLogin(acc, pwd);
    });

    // 4. 绑定DataManager的信号
    connect(DataManager::getInstance(), &DataManager::loginSuccess, this, [=](const QString &n, int id){
        ui->label_tip->setText("登录成功，欢迎" + n);
        emit loginFinish(id);
        this->close();
    });
    connect(DataManager::getInstance(), &DataManager::loginFailed, this, [=](const QString &r){
        ui->label_tip->setText("登录失败：" + r);
    });
}

LoginWindow::~LoginWindow()
{
    delete ui;
}
