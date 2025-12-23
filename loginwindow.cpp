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

        // ========== 新增：登录成功后添加到在线用户表 ==========
        // 获取当前登录的账号（用于在线表存储）
        QString loginAccount = ui->le_account->text().trimmed();
        // 调用DataManager的接口添加在线用户（后续需在DataManager中实现该接口）
        bool addOnlineOk = DataManager::getInstance()->addOnlineUser(id, loginAccount);
        if (addOnlineOk) {
            qDebug() << "添加在线用户成功：" << loginAccount << "(ID:" << id << ")";
        } else {
            qDebug() << "添加在线用户失败：" << loginAccount;
        }

        // 修正1：变量名错误（nickName → n，n是DataManager传递的用户名）
        emit loginFinish(n); // 发送QString类型的用户名，匹配信号声明
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
