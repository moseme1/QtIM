#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include <QWidget>
#include "ui_chatwindow.h"
#include "datamanager.h"

namespace Ui {
class ChatWindow;
}

class ChatWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ChatWindow(int userId, int friendId, QWidget *parent = nullptr);
    ~ChatWindow();

private slots:
    void on_btn_send_clicked(); // 自动关联发送按钮
    void onMsgReceived(const QMap<QString, QString> &msg);

private:
    void updateMsgDisplay(const QString &sender, const QString &content, const QString &time);

    Ui::ChatWindow *ui;
    int m_userId;
    int m_friendId;
    DataManager *m_dataMgr;
};

#endif // CHATWINDOW_H