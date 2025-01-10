#include <QCoreApplication>

#include "base/widStu.h"
#include "base/cserver.h"
#include "etcHeader/ethercatmaster.h"

EtherCatMaster *etcm0;
cServer *server0;
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    etcm0 = new EtherCatMaster();
    //server0 = new cServer();
    return a.exec();
}




