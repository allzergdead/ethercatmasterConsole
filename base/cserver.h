#ifndef CSERVER_H
#define CSERVER_H

#include <QObject>

#include <QTimer>
#include <QUdpSocket>
#include <QTcpServer>
#include <QTcpSocket>
#include <QByteArray>
#include <QVector>
#include <QFile>
#include <QMutex>
#include <QDataStream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

#define NORMAL_PORT 9100
#define MOTION_PORT 9101
#define LOG_PORT 9101


class cServer;

struct InfoHeader
{
    uint headID = 0;
    uint size = 0;
    uint maxPage = 0;
    uint nowPage = 0;
    uint mark = 0;
};

struct TcpSockInfo
{
    cServer *self = nullptr;
    //QTcpSocket *sock = nullptr;
    int clientFd = 0;
    pthread_t threadt = 0;
};



class cServer : public QObject
{
     Q_OBJECT
public:

    cServer();
    //服务器
    QTcpServer normalServer;
    QTcpServer motionServer;
    QTcpServer logServer;
    //常规指令接口
    pthread_t normalListent = 0;
    static void *waitNormalTcpConnect(void *arg);
    static void *normalSockRecv(void *arg);
    //运动控制指令接口
    static void *waitMotionTcpConnect(void *arg);
    static void *MotionSockRecv(void *arg);
    //日志接口
    static void *waitLogTcpConnect(void *arg);
    static void *logSockSend(void *arg);
    //初始化函数
    int runServer();

    //命令解析函数
    QJsonObject runCmd(QJsonObject obj);
};




// 通过已连接的 QTcpSocket 发送 JSON 数据的函数
bool sendData(int socketFd, const QByteArray &data);
// 通过已连接的 QTcpSocket 接收 JSON 数据的函数
QJsonDocument receiveData(int socketFd, bool &success);



#endif // CSERVER_H
