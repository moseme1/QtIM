#include "chatwindow.h"
#include <QDateTime>

ChatWindow::ChatWindow(int userId, int friendId, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChatWindow()),
    m_userId(userId),
    m_friendId(friendId)
{
    ui->setupUi(this);
    m_dataMgr = DataManager::getInstance();

    // 绑定输入框文本变化信号（启用/禁用发送按钮）
    connect(ui->le_msg_input, &QLineEdit::textChanged, this, [=](const QString &text) {
        ui->btn_send->setEnabled(!text.trimmed().isEmpty());
    });
    // 绑定消息接收信号
    connect(m_dataMgr, &DataManager::msgReceived, this, &ChatWindow::onMsgReceived);

    setWindowTitle(QString("与%1聊天").arg(friendId));
}

ChatWindow::~ChatWindow()
{
    delete ui;
}

// 自动关联发送按钮点击
void ChatWindow::on_btn_send_clicked()
{
    QString content = ui->le_msg_input->text().trimmed();
    if (content.isEmpty()) return;

    // 发送消息
    m_dataMgr->sendMessage(m_userId, m_friendId, content, SingleChat);

    // 清空输入框
    ui->le_msg_input->clear();

    // 展示发送的消息
    updateMsgDisplay(QString::number(m_userId), content, QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
}

void ChatWindow::onMsgReceived(const QMap<QString, QString> &msg)
{
    // 仅展示当前好友的消息
    if (msg["sender"].toInt() == m_friendId) {
        updateMsgDisplay(msg["sender"], msg["content"], msg["time"]);
    }
}

void ChatWindow::updateMsgDisplay(const QString &sender, const QString &content, const QString &time)
{
    QString msgHtml;
    if (sender.toInt() == m_userId) {
        // 自己发送的消息：居右，蓝色背景
        msgHtml = QString("<div style='text-align: right; margin: 5px;'>"
                          "<span style='background-color: #66ccff; padding: 5px; border-radius: 5px;'>%1</span>"
                          "<br><small style='color: #999;'>%2</small>"
                          "</div>").arg(content).arg(time);
    } else {
        // 对方消息：居左，灰色背景
        msgHtml = QString("<div style='text-align: left; margin: 5px;'>"
                          "<span style='background-color: #eeeeee; padding: 5px; border-radius: 5px;'>%1</span>"
                          "<br><small style='color: #999;'>%2</small>"
                          "</div>").arg(content).arg(time);
    }

    ui->te_msg_display->insertHtml(msgHtml);
    // 滚动到底部
    QTextCursor cursor = ui->te_msg_display->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->te_msg_display->setTextCursor(cursor);
}
