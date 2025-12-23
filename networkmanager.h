#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include "workerthread.h"

class NetworkManager : public QObject
{
    Q_OBJECT
public:
    static NetworkManager *getInstance();
    ~NetworkManager();

    void connectServer();
    void disconnectServer();
    void sendMsg(const QString &msg);

signals:
    void connSuccess();
    void connFailed(const QString &err);
    void recvMsg(const QString &msg);

private slots:
    void onThreadConnected();
    void onThreadDisconnected();
    void onThreadRecvMsg(const QString &msg);
    void onThreadError(const QString &err);

private:
    explicit NetworkManager(QObject *parent = nullptr);
    static NetworkManager *m_instance;
    WorkerThread *m_workerThread;
};

#endif // NETWORKMANAGER_H
