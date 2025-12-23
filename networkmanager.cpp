#include "networkmanager.h"

NetworkManager *NetworkManager::m_instance = nullptr;

NetworkManager::NetworkManager(QObject *parent) : QObject(parent)
{
    m_workerThread = new WorkerThread();
    m_workerThread->start();

    connect(m_workerThread, &WorkerThread::connected, this, &NetworkManager::onThreadConnected);
    connect(m_workerThread, &WorkerThread::disconnected, this, &NetworkManager::onThreadDisconnected);
    connect(m_workerThread, &WorkerThread::messageReceived, this, &NetworkManager::onThreadRecvMsg);
    connect(m_workerThread, &WorkerThread::errorOccurred, this, &NetworkManager::onThreadError);
}

NetworkManager::~NetworkManager()
{
    m_workerThread->disconnectFromServer();
    m_workerThread->quit();
    m_workerThread->wait();
    m_workerThread->deleteLater();
}

NetworkManager *NetworkManager::getInstance()
{
    if (!m_instance) {
        m_instance = new NetworkManager();
    }
    return m_instance;
}

void NetworkManager::connectServer()
{
    m_workerThread->connectToServer();
}

void NetworkManager::disconnectServer()
{
    m_workerThread->disconnectFromServer();
}

void NetworkManager::sendMsg(const QString &msg)
{
    m_workerThread->sendMessage(msg);
}

void NetworkManager::onThreadConnected()
{
    emit connSuccess();
}

void NetworkManager::onThreadDisconnected()
{
    emit connFailed("网络断开连接");
}

void NetworkManager::onThreadRecvMsg(const QString &msg)
{
    if (msg != "HEARTBEAT") { // 过滤心跳包
        emit recvMsg(msg);
    }
}

void NetworkManager::onThreadError(const QString &err)
{
    emit connFailed(err);
}