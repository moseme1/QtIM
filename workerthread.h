#ifndef WORKERTHREAD_H
#define WORKERTHREAD_H

#include <QThread>
#include <QTcpSocket>
#include <QTimer>
#include "globaldefine.h"

class WorkerThread : public QThread
{
    Q_OBJECT
public:
    explicit WorkerThread(QObject *parent = nullptr);
    ~WorkerThread();

    void connectToServer();
    void disconnectFromServer();
    void sendMessage(const QString &msg);
    // ========== 核心新增：判断是否已连接服务器 ==========
    bool isConnected() const;

signals:
    void connected();
    void disconnected();
    void messageReceived(const QString &msg);
    void errorOccurred(const QString &err);

protected:
    void run() override;

private slots:
    void onSocketConnected();
    void onSocketDisconnected();
    void onReadyRead();
    void onSocketError(QAbstractSocket::SocketError err);
    void sendHeartbeat();

private:
    QTcpSocket *m_socket;
    QTimer *m_heartbeatTimer;
    QString m_serverIp;
    quint16 m_serverPort;
    bool m_isRunning;
};

#endif // WORKERTHREAD_H
