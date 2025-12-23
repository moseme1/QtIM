#include "loginwindow.h"

LoginWindow::LoginWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LoginWindow()), // 初始化UI
    m_isRegisterMode(false)
{
    ui->setupUi(this); // 加载UI布局
    m_dataMgr = DataManager::getInstance();

    // 绑定业务信号槽
    connect(m_dataMgr, &DataManager::loginSuccess, this, &LoginWindow::onLoginSuccess);
    connect(m_dataMgr, &DataManager::loginFailed, this, &LoginWindow::onLoginFailed);
}

LoginWindow::~LoginWindow()
{
    delete ui; // 释放UI对象
}

// 自动关联btn_login的clicked信号（命名规则：on_控件名_信号名）
void LoginWindow::on_loginBtn_clicked()
{
    QString account = ui->le_account->text().trimmed();
    QString pwd = ui->le_pwd->text().trimmed();

    if (account.isEmpty() || pwd.isEmpty()) {
        ui->label_tip->setText("账号或密码不能为空！");
        return;
    }

    m_dataMgr->userLogin(account, pwd);
}

// 自动关联btn_register的clicked信号
void LoginWindow::on_registerBtn_clicked()
{
    m_isRegisterMode = !m_isRegisterMode;
    if (m_isRegisterMode) {
        ui->le_nickname->show();
        ui->btn_register->setText("取消注册");
        ui->btn_login->setText("提交注册");
        setWindowTitle("QtIM - 注册");
    } else {
        ui->le_nickname->hide();
        ui->btn_register->setText("注册");
        ui->btn_login->setText("登录");
        setWindowTitle("QtIM - 登录");
        return;
    }

    // 提交注册
    QString account = ui->le_account->text().trimmed();
    QString pwd = ui->le_pwd->text().trimmed();
    QString nickname = ui->le_nickname->text().trimmed();

    if (account.isEmpty() || pwd.isEmpty() || nickname.isEmpty()) {
        ui->label_tip->setText("信息不能为空！");
        return;
    }

    if (m_dataMgr->userRegister(account, pwd, nickname)) {
        ui->label_tip->setText("注册成功，请登录！");
        m_isRegisterMode = false;
        ui->le_nickname->hide();
        ui->btn_register->setText("注册");
        ui->btn_login->setText("登录");
        setWindowTitle("QtIM - 登录");
    } else {
        ui->label_tip->setText("注册失败，账号已存在！");
    }
}

void LoginWindow::onLoginSuccess(const QString &nickname, int userId)
{
    ui->label_tip->setText(QString("登录成功，欢迎%1！").arg(nickname));
    emit loginFinish(userId);
    close();
}

void LoginWindow::onLoginFailed(const QString &reason)
{
    ui->label_tip->setText(reason);
}