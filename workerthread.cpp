#include "workerthread.h"
#include <QDebug> // 新增：调试日志（可选）

WorkerThread::WorkerThread(QObject *parent) : QThread(parent)
    , m_serverIp(SERVER_IP)
    , m_serverPort(SERVER_PORT)
    , m_isRunning(false)
{
    m_socket = new QTcpSocket();
    m_heartbeatTimer = new QTimer();
    m_heartbeatTimer->setInterval(HEARTBEAT_INTERVAL);

    // 绑定信号槽
    connect(m_socket, &QTcpSocket::connected, this, &WorkerThread::onSocketConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &WorkerThread::onSocketDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &WorkerThread::onReadyRead);
    connect(m_socket, &QTcpSocket::errorOccurred, this, &WorkerThread::onSocketError);
    connect(m_heartbeatTimer, &QTimer::timeout, this, &WorkerThread::sendHeartbeat);

    qDebug() << "WorkerThread初始化完成，服务器地址：" << m_serverIp << ":" << m_serverPort;
}

WorkerThread::~WorkerThread()
{
    m_isRunning = false;
    quit();
    wait();
    m_socket->deleteLater();
    m_heartbeatTimer->deleteLater();

    qDebug() << "WorkerThread已销毁";
}

void WorkerThread::connectToServer()
{
    if (m_socket->state() != QTcpSocket::UnconnectedState) {
        qWarning() << "Socket未处于未连接状态，跳过连接请求";
        return;
    }

    qDebug() << "尝试连接服务器：" << m_serverIp << ":" << m_serverPort;
    m_socket->connectToHost(m_serverIp, m_serverPort);
}

void WorkerThread::disconnectFromServer()
{
    qDebug() << "断开服务器连接";
    m_heartbeatTimer->stop();
    m_socket->disconnectFromHost();
}

void WorkerThread::sendMessage(const QString &msg)
{
    if (m_socket->state() != QTcpSocket::ConnectedState) {
        qWarning() << "Socket未连接，消息发送失败：" << msg;
        return;
    }

    qDebug() << "发送网络消息：" << msg;
    m_socket->write(msg.toUtf8());
    m_socket->flush();
}

// ========== 核心新增：实现isConnected()接口 ==========
bool WorkerThread::isConnected() const
{
    // 判空 + 检查Socket连接状态
    bool connected = m_socket && (m_socket->state() == QTcpSocket::ConnectedState);
    qDebug() << "Socket连接状态：" << connected;
    return connected;
}

void WorkerThread::run()
{
    m_isRunning = true;
    qDebug() << "WorkerThread开始运行";
    exec();
}

void WorkerThread::onSocketConnected()
{
    qDebug() << "服务器连接成功";
    emit connected();
    m_heartbeatTimer->start();
}

void WorkerThread::onSocketDisconnected()
{
    qDebug() << "服务器连接断开";
    emit disconnected();
    m_heartbeatTimer->stop();

    // 自动重连
    if (m_isRunning) {
        qDebug() << RECONNECT_INTERVAL/1000 << "秒后尝试重连";
        QTimer::singleShot(RECONNECT_INTERVAL, this, &WorkerThread::connectToServer);
    }
}

void WorkerThread::onReadyRead()
{
    QByteArray data = m_socket->readAll();
    QString msg = QString::fromUtf8(data);
    qDebug() << "收到服务器数据：" << msg;
    emit messageReceived(msg);
}

void WorkerThread::onSocketError(QAbstractSocket::SocketError err)
{
    QString errStr = m_socket->errorString();
    qWarning() << "Socket错误：" << err << " - " << errStr;
    emit errorOccurred(errStr);
}

void WorkerThread::sendHeartbeat()
{
    sendMessage("HEARTBEAT"); // 心跳包标识
    qDebug() << "发送心跳包";
}
