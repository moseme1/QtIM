#ifndef GLOBALDEFINE_H
#define GLOBALDEFINE_H

#include <QString>

// 数据库配置
#define DB_NAME "QtIM.db"
#define DB_HOST "localhost"
#define DB_USER "root"
#define DB_PWD ""

// 网络配置
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8888
#define HEARTBEAT_INTERVAL 30000  // 心跳间隔30秒
#define RECONNECT_INTERVAL 5000   // 重连间隔5秒

// 消息类型枚举（修正值重复问题）
enum MsgType {
    SingleChat = 0,  // 单聊
    Private = 1,     // 私聊
    GroupChat = 2    // 群聊（修正值为2）
};

// 用户状态枚举
enum UserState {
    Online = 0,
    Offline = 1
};

#endif // GLOBALDEFINE_H
