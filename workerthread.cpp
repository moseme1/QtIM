#include "workerthread.h"

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
}

WorkerThread::~WorkerThread()
{
    m_isRunning = false;
    quit();
    wait();
    m_socket->deleteLater();
    m_heartbeatTimer->deleteLater();
}

void WorkerThread::connectToServer()
{
    m_socket->connectToHost(m_serverIp, m_serverPort);
}

void WorkerThread::disconnectFromServer()
{
    m_heartbeatTimer->stop();
    m_socket->disconnectFromHost();
}

void WorkerThread::sendMessage(const QString &msg)
{
    if (m_socket->state() == QTcpSocket::ConnectedState) {
        m_socket->write(msg.toUtf8());
        m_socket->flush();
    }
}

void WorkerThread::run()
{
    m_isRunning = true;
    exec();
}

void WorkerThread::onSocketConnected()
{
    emit connected();
    m_heartbeatTimer->start();
}

void WorkerThread::onSocketDisconnected()
{
    emit disconnected();
    m_heartbeatTimer->stop();

    // 自动重连
    if (m_isRunning) {
        QTimer::singleShot(RECONNECT_INTERVAL, this, &WorkerThread::connectToServer);
    }
}

void WorkerThread::onReadyRead()
{
    QByteArray data = m_socket->readAll();
    emit messageReceived(QString::fromUtf8(data));
}

void WorkerThread::onSocketError(QAbstractSocket::SocketError err)
{
    emit errorOccurred(m_socket->errorString());
}

void WorkerThread::sendHeartbeat()
{
    sendMessage("HEARTBEAT"); // 心跳包标识
}