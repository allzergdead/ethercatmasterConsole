#include "cserver.h"
#include "widStu.h"

cServer::cServer()
{
    pthread_create(&normalListent,NULL,waitNormalTcpConnect,(void *)this);
}

// 服务端代码
void startServer(void *func(void *),void *arg0) {
    int serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd == -1) {
        perror("创建服务器套接字失败");
        exit(EXIT_FAILURE);
    }

    sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(NORMAL_PORT);

    if (bind(serverFd, (sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("绑定失败");
        close(serverFd);
        exit(EXIT_FAILURE);
    }
    while(true){
        if (listen(serverFd, 3) == -1) {
            perror("监听失败");
            close(serverFd);
            exit(EXIT_FAILURE);
        }

        std::cout << "服务器正在等待连接..." << std::endl;

        sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        int clientFd = accept(serverFd, (sockaddr *)&clientAddr, &clientAddrLen);
        if (clientFd == -1) {
            perror("接受连接失败");
            close(serverFd);
            exit(EXIT_FAILURE);
        }

        std::cout << "客户端已连接" << std::endl;
        TcpSockInfo *info = new TcpSockInfo();
        info->clientFd = clientFd;
        info->self = (cServer *)arg0;

        pthread_create(&info->threadt,NULL,func,(void *)info);
    }
    // 关闭套接字
    //close(clientFd);
    close(serverFd);
}


//常规指令接口
void *cServer::waitNormalTcpConnect(void *arg)
{

    qDebug() << "waitNormalTcpConnect";
    startServer(normalSockRecv,arg);
    return nullptr;
}

void *cServer::normalSockRecv(void *arg)
{
    TcpSockInfo *sockInfo = (TcpSockInfo *)arg;
    //InfoHeader header;
    int clientFd = sockInfo->clientFd;
    cServer *self = sockInfo->self;
    QByteArray recv;
    QByteArray send;
    delete sockInfo;
    while(true){
        bool success = false;
        QJsonDocument doc = receiveData(clientFd,success);
        QJsonObject obj = doc.object();
        if(!success){
            ::close(clientFd);
            break;
        }
        qDebug() << "recv done";
        qDebug() << obj;
    }
    return nullptr;
}
//运动控制指令接口
void *cServer::waitMotionTcpConnect(void *arg)
{

}
void *cServer::MotionSockRecv(void *arg)
{

}
//日志接口
void *cServer::waitLogTcpConnect(void *arg)
{

}
void *cServer::logSockSend(void *arg)
{

}
//初始化函数
int cServer::runServer()
{

}

/****************
*   运动控制指令
*   1.cmc_power:
*   2.cmc_home:
*   3.cmc_jog:
*   4.cmc_moveabs:
*   5.cmc_movereal:
*   6.cmc_reset:
*   7.cmc_setpos:
*   8.cmc_stop:
*
******************/

int runMcCmd(QJsonObject obj)
{
    if(!obj.contains("cmd")){
        return -1;
    }
    QString cmd = obj["cmd"].toString();
    if(cmd == "cmc_power"){

    }else if(cmd == "cmc_home"){

    }else if(cmd == "cmc_jog"){

    }else if(cmd == "cmc_moveabs"){

    }else if(cmd == "cmc_movereal"){

    }else if(cmd == "cmc_reset"){

    }else if(cmd == "cmc_setpos"){

    }else if(cmd == "cmc_stop"){

    }else{

    }
}

/****************
*   规划命令
*   1.readlist:读取系统所有变量
*   2.read:读取变量
*   3.write:写入变量
*   4.motioncmd:运动控制函数
*   5.ethercatstart:启动总线
*   6.ethercatstop:停止总线
*   7.readconfig:读取总线配置
*   8.writeconfig:写入总线配置
*   9.readslaves:读取从站数据
*
******************/

QJsonDocument runCmd(QJsonDocument &doc , int &ret)
{
    QJsonObject obj = doc.object();
    QJsonDocument retdoc;
    if(!obj.contains("cmd")){
        ret = -1;
        return retdoc;
    }
    QString cmd = obj["cmd"].toString();
    if(cmd == "read_list"){
        QStringList list = findHasStr("");
        QJsonObject sendObj;
        sendObj.insert("cmd",cmd);
        QJsonObject dataObj;
        for(int i = 0; i < list.size();i++){
            QJsonObject varObj;
            cBaseVar *var = findCbase(list[i]);
            if(var == nullptr){
                continue;
            }
            //单个变量
            varObj.insert("value",var->toString());
            varObj.insert("dir",var->dir);
            varObj.insert("info",var->info);
            varObj.insert("type",var->type);
            //
            dataObj.insert(list[i],varObj);
        }
        sendObj.insert("data",dataObj);
        ret = 0;
        return QJsonDocument(sendObj);
    }else if(cmd == "read"){
        if(!obj.contains("data")){
            ret = -3;
            return retdoc;
        }
        QJsonObject sendObj;
        sendObj.insert("cmd",cmd);
        QJsonObject dataObj = obj["data"].toObject();
        // 使用范围循环来遍历 QJsonObject
        for (auto it = dataObj.begin(); it != dataObj.end(); ++it) {
            QString name = it.key();
            cBaseVar *var = findCbase(name);
            if(var == nullptr){
                continue;
            }
            it.value().toObject()["value"] = var->toString();
        }
        sendObj.insert("data",dataObj);
        ret = 0;
        return QJsonDocument(sendObj);
    }else if(cmd == "write"){
        if(!obj.contains("data")){
            ret = -3;
            return retdoc;
        }
        QJsonObject sendObj;
        sendObj.insert("cmd",cmd);
        QJsonObject dataObj = obj["data"].toObject();
        QJsonObject dataObjRet;
        // 使用范围循环来遍历 QJsonObject
        for (auto it = dataObj.begin(); it != dataObj.end(); ++it) {
            QString name = it.key();
            cBaseVar *var = findCbase(name);
            if(var == nullptr){
                continue;
            }
            if(!it.value().toObject().contains("value")){
                continue;
            }
            QString value = it.value().toObject()["value"].toString();
            int ret = var->setByString(value);
            cBaseVar::var_to_db(var->fullname());
            if(ret != 0){
                dataObjRet.insert(var->fullname(),"NG");
            }
            //it.value().toObject()["value"] = var->toString();
        }
        sendObj.insert("data",dataObjRet);
        ret = 0;
        return QJsonDocument(sendObj);
    }else if(cmd == "motioncmd"){

    }else if(cmd == "ethercatstart"){

    }else if(cmd == "ethercatstop"){

    }else if(cmd == "readconfig"){

    }else if(cmd == "writeconfig"){

    }else if(cmd == "readslaves"){

    }else{
        ret = -2;
        return retdoc;
    }
    ret = 0;
    return retdoc;
}


#define BUFFER_SIZE 4096

// 发送数据函数
bool sendData(int socketFd, const QByteArray &data) {
    quint32 dataSize = data.size();
    if (send(socketFd, &dataSize, sizeof(dataSize), 0) == -1) {
        perror("发送数据大小失败");
        return false;
    }

    qint64 bytesWritten = 0;
    while (bytesWritten < dataSize) {
        qint64 chunkSize = qMin<qint64>(BUFFER_SIZE, dataSize - bytesWritten);
        qint64 written = send(socketFd, data.mid(bytesWritten, chunkSize).data(), chunkSize, 0);
        if (written == -1) {
            perror("发送数据失败");
            return false;
        }
        bytesWritten += written;

        // 等待接收方的确认应答
        char ack[4] = {0};
        if (recv(socketFd, ack, sizeof(ack), 0) == -1) {
            perror("接收确认应答失败");
            return false;
        }
        if (std::strcmp(ack, "ACK") != 0) {
            std::cerr << "未收到正确的确认应答" << std::endl;
            return false;
        }
    }

    return true;
}

// 接收数据函数
QJsonDocument receiveData(int socketFd, bool &success) {
    success = false;
    quint32 dataSize = 0;
    if (recv(socketFd, &dataSize, sizeof(dataSize), 0) == -1) {
        perror("接收数据大小失败");
        return QJsonDocument();
    }

    QByteArray jsonData;
    qint64 bytesRead = 0;
    while (bytesRead < dataSize) {
        char buffer[BUFFER_SIZE] = {0};
        qint64 chunkSize = qMin<qint64>(BUFFER_SIZE, dataSize - bytesRead);
        qint64 read = recv(socketFd, buffer, chunkSize, 0);
        if (read == -1) {
            perror("接收数据失败");
            return QJsonDocument();
        }
        jsonData.append(buffer, read);
        bytesRead += read;

        // 发送确认应答
        if (send(socketFd, "ACK", 3, 0) == -1) {
            perror("发送确认应答失败");
            return QJsonDocument();
        }
    }

    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
    if (!jsonDoc.isNull()) {
        success = true;
    }
    return jsonDoc;
}


