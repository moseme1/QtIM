#include "networkmanager.h"
#include <QDebug> // 新增：调试日志

NetworkManager *NetworkManager::m_instance = nullptr;

NetworkManager::NetworkManager(QObject *parent) : QObject(parent)
{
    m_workerThread = new WorkerThread();
    m_workerThread->start();

    connect(m_workerThread, &WorkerThread::connected, this, &NetworkManager::onThreadConnected);
    connect(m_workerThread, &WorkerThread::disconnected, this, &NetworkManager::onThreadDisconnected);
    connect(m_workerThread, &WorkerThread::messageReceived, this, &NetworkManager::onThreadRecvMsg);
    connect(m_workerThread, &WorkerThread::errorOccurred, this, &NetworkManager::onThreadError);

    qDebug() << "NetworkManager初始化完成，工作线程已启动";
}

NetworkManager::~NetworkManager()
{
    // 新增：发送下线通知后再断开连接
    if (m_workerThread->isConnected()) { // 需在WorkerThread中新增isConnected()接口
        // 发送下线通知（格式：offline|placeholder，实际userId由DataManager补充）
        sendMsg("offline|placeholder");
    }

    m_workerThread->disconnectFromServer();
    m_workerThread->quit();
    m_workerThread->wait();
    m_workerThread->deleteLater();

    qDebug() << "NetworkManager已销毁，工作线程已停止";
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
    qDebug() << "发起服务器连接请求";
    m_workerThread->connectToServer();
}

void NetworkManager::disconnectServer()
{
    qDebug() << "发起服务器断开连接请求";
    m_workerThread->disconnectFromServer();
}

void NetworkManager::sendMsg(const QString &msg)
{
    if (!m_workerThread->isConnected()) { // 判空保护：仅连接成功后发送
        qWarning() << "未连接到服务器，消息发送失败：" << msg;
        return;
    }

    qDebug() << "发送网络消息：" << msg;
    m_workerThread->sendMessage(msg);
}

void NetworkManager::onThreadConnected()
{
    qDebug() << "服务器连接成功";
    emit connSuccess();
}

void NetworkManager::onThreadDisconnected()
{
    qDebug() << "服务器连接断开";
    emit connFailed("网络断开连接");
}

void NetworkManager::onThreadRecvMsg(const QString &msg)
{
    // 过滤心跳包，仅处理业务消息
    if (msg == "HEARTBEAT") {
        qDebug() << "收到心跳包，忽略";
        return;
    }

    qDebug() << "收到服务器消息：" << msg;
    // 转发消息给DataManager处理
    emit recvMsg(msg);
}

void NetworkManager::onThreadError(const QString &err)
{
    qWarning() << "网络错误：" << err;
    emit connFailed(err);
}
