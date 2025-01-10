/**********************************************************************************************
 * last Change : 2022-4-25
 * $vision : 0.1$
 * $Author : Tom Chang $
 * $
 * $JRS pcie device(CH368 work with laserdev.ko) .h file&
 * $
 * ********************************************************************************************/

#ifndef CLASERUSERDEV_H
#define CLASERUSERDEV_H

#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <math.h>
#include <sys/mman.h>                   //mmap head file
#include <unistd.h>
#include <cstdio>
#include <signal.h>
#include <time.h>
#include <pthread.h>
//#include <sched.h>
#include <sys/time.h>
#include <sys/timerfd.h>
#include <sys/epoll.h>
#include <math.h>
#include <mutex>
#include <sstream>
#include "studef.h"

#include "ethercartest.h"
#include "../../ethercatMaster/etcHeader/etcheader.h"
#include "../widgets/widStu.h"

#define QTDEBUG
#ifdef QTDEBUG
    #include <QDebug>
    #define gDebug qDebug
#else
    #define gDebug printf
#endif

#define PI 3.1415926

#define ETC_SHM_SIZE 1024 * 1024 * 10
#define ETC_FILE "etcshm"
long long diff_tv(struct timespec start, struct timespec end);

typedef struct jp_param_n{
    volatile float fLenth;  //极片长度
    volatile int iNjp;      //极耳个数
    volatile float power;   //间距功率
    volatile float freq;    //间距打点次数
    volatile float duty;    //间距占空比
    volatile float wave;    //间距波形
    volatile float cutPower;   //切入切出功率
    volatile float cutFreq;    //切入切出打点次数
    volatile float cutDuty;    //切入切出占空比
    volatile float cutWave;    //切入切出波形
    volatile float dotTimes;    //打点次数
    volatile float reserveLen;    //预留间距
} volatile jp_param; //极片头部(极片描述数据(4KB-5KB区间))

typedef struct jp_list_n {
    volatile int iPtr;  //极耳数据x偏移量
    volatile float fStart;//此极耳在级片的起始位置
    volatile float fEnd;//此极耳在级片的结束位置
    volatile float fLength;// 未定义
} volatile jp_list;//极片寻址数据(5KB-12KB区间)

typedef struct jp_data_n{
    volatile int iNum;// 极耳标志
    volatile int iCount;//直线、圆弧线段条数
    volatile int iSize;//数据块总大小(tabHead+line*count)
    volatile float fW;//极耳底边长度
    volatile float fL;//极耳图形长度
    volatile float fP;//图形长度/底边长度
} volatile jp_data;//极耳参数数据块(12KB-区间)

typedef struct jp_power_n{
    volatile float fPos;  //起始位置(以线段为起始点)
    volatile float power;//单脉冲能量百分比
    volatile float freq;//打点次数(脉冲个数)
    volatile float duty;//占空比(默认0.5)
    volatile float wave;//脉宽
    volatile float vel;//未用
} volatile jp_power;//线段内参数

typedef struct line_para_n{
    volatile int iNum;  //线段标号
    volatile int iType; //线段类型(0.直线 1.圆弧)
    volatile float param[20];   //线段参数集
    volatile float squap[10];   //算子集
    volatile jp_power pfparam[5];  //线段数据集
} volatile line_para;    //线段参数数据块

/*
param[0]	线段起始位置到极耳起点长度	起点长度
param[1]	线段结束位置到极耳起点长度	终点长度
param[2]	起点x                     起点x
param[3]	起点y                     起点y
param[4]	圆弧半径                   终点x
param[5]	圆弧起始位置                终点y
param[6]	圆弧方向//暂未用,默认逆时针
param[7]	弧度
param[8]
param[9]

squap[0]	x轴底宽起始
squap[1]	x轴底宽结束
squap[2]    设置速度
squap[3]	p/fl  时间占比
squap[4]	基础时间占比 = 线段长度/极耳长度
squap[5]	速度比
squap[6]	基础速度比
squap[7]		*/
//void initktask(struct kenrel_TaskInfo *stask);



class claserUserDev
{
public:
    int cmdfd;
    int ETCfd = 0;
    int epfd = 0;
    int mpfd = 0;
    pthread_t eppth;
    int lsfd[MAX_LASER];
    laserPDO *lsStu[MAX_LASER];
    laserPDO nullls;
    bool initDone = false;
    bool ksinDone = false;
    cmd_file_stu nullcmdStu;
    cmd_file_stu *cmdStu;
    claserUserDev();

    std::vector<userpthread> pthvector;
    std::mutex pthmtx;
    struct mappingInfo_sub *mapping_mem = nullptr;
    cec_shm_pdo *ecm = nullptr;

    int writePf(void*data, int size, int iNo);
    int writePfPower(void *data, int size,int iNo);
    int writePcieMem(int PcieNo,int memAddr,void*data);
    int readPf(void *data,int iNo);
    /**********************************************************************************************
     * last Change : 2022-5-6
     * $vision : 0.1$
     * $Author : Tom Chang $
     * $Init PCIE card and write sin and cos function to kernel dev
     * $return : 0 : sucessed
     * $         others : error
     * ********************************************************************************************/
    int init(void);
    /**********************************************************************************************
     * last Change : 2022-5-6
     * $vision : 0.1$
     * $Author : Tom Chang $
     * $Init PCIE card and write sin and cos function to kernel dev
     * $return : 0 : sucessed
     * $         others : error
     * ********************************************************************************************/
    void *etcinit(void);
    /**********************************************************************************************
     * last Change : 2022-5-6
     * $vision : 0.1$
     * $Author : Tom Chang $
     * $Init ksin mem in kenrel
     * $return : 0 : sucessed
     * $         others : error
     * ********************************************************************************************/
    int ksinInit(void);
    /**********************************************************************************************
     * last Change : 2022-5-9
     * $vision : 0.1$
     * $Author : Tom Chang $
     * $kcmd mmap init
     * ********************************************************************************************/
    int kcmdInit(void);
    /**********************************************************************************************
     * last Change : 2022-5-9
     * $vision : 0.1$
     * $Author : Tom Chang $
     * $change new a kenrel laser dev
     * ********************************************************************************************/
    int newLaser(void);
    /**********************************************************************************************
     * last Change : 2022-4-25
     * $vision : 0.1$
     * $Author : Tom Chang $
     * $change return value to err str code
     * ********************************************************************************************/
    std::string err2Str(int err);
    /**********************************************************************************************
     * last Change : 2022-5-9
     * $vision : 0.1$
     * $Author : Tom Chang $
     * $change return init dong dor not
     * ********************************************************************************************/
    bool isInitDone(void);
    /**********************************************************************************************
     * last Change : 2022-5-9
     * $vision : 0.1$
     * $Author : Tom Chang $
     * $change config kenrel by confstu
     * ********************************************************************************************/
    int reConfig(struct hardwareConf conf);
    /**********************************************************************************************
     * last Change : 2022-4-25
     * $vision : 0.1$
     * $Author : Tom Chang $
     * $new a pthread by itimerspec and return a Vector to insert tasks
     * ********************************************************************************************/
    int newPthread(std::string name,unsigned int cyctime_ns);
    /**********************************************************************************************
     * last Change : 2022-4-25
     * $vision : 0.1$
     * $Author : Tom Chang $
     * $push a task into phread
     * ********************************************************************************************/
    int push_task(int pthid ,  void *(*fun)(void* arg),void *arg,std::string name);
    /**********************************************************************************************
     * last Change : 2022-4-25
     * $vision : 0.1$
     * $Author : Tom Chang $
     * $100ms heart task
     * ********************************************************************************************/
    static void *_100ms_task(void *arg);
    /**********************************************************************************************
     * last Change : 2022-4-25
     * $vision : 0.1$
     * $Author : Tom Chang $
     * $static pthread func
     * ********************************************************************************************/
    static void pthread_fn(void *arg);
    /**********************************************************************************************
     * last Change : 2022-4-25
     * $vision : 0.1$
     * $Author : Tom Chang $
     * $
     * ********************************************************************************************/
    static void *_1ms_task(void *arg);
    /**********************************************************************************************
     * last Change : 2022-4-25
     * $vision : 0.1$
     * $Author : Tom Chang $
     * $static pthread func
     * ********************************************************************************************/
    int clearls();
    /**********************************************************************************************
     * last Change : 2022-4-25
     * $vision : 0.1$
     * $Author : Tom Chang $
     * $laser is runing
     * ********************************************************************************************/
    bool isRuning();
    /**********************************************************************************************
     * last Change : 2022-4-25
     * $vision : 0.1$
     * $Author : Tom Chang $
     * $
     * ********************************************************************************************/
    int delConnect();
    /**********************************************************************************************
     * last Change : 2022-4-25
     * $vision : 0.1$
     * $Author : Tom Chang $
     * $reconnect
     * ********************************************************************************************/
    int reConnect();
    /**********************************************************************************************
     * last Change : 2022-4-25
     * $vision : 0.1$
     * $Author : Tom Chang $
     * $force close file
     * ********************************************************************************************/
    int eStop();
    /**********************************************************************************************
     * last Change : 2022-4-25
     * $vision : 0.1$
     * $Author : Tom Chang $
     * $force close file
     * ********************************************************************************************/
    int setEncBindCard(int enc,int card);
    /**********************************************************************************************
     * last Change : 2022-4-25
     * $vision : 0.1$
     * $Author : Tom Chang $
     * $force close file
     * ********************************************************************************************/
    int setlsBindEnc(int ls,int enc);
    /**********************************************************************************************
     * last Change : 2022-4-25
     * $vision : 0.1$
     * $Author : Tom Chang $
     * $force close file
     * ********************************************************************************************/
    int setlsBindCard(int ls,int card);
    /**********************************************************************************************
     * last Change : 2022-4-25
     * $vision : 0.1$
     * $Author : Tom Chang $
     * $force close file
     * ********************************************************************************************/
    bool canConfig();
    /**********************************************************************************************
     * last Change : 2022-4-25
     * $vision : 0.1$
     * $Author : Tom Chang $
     * $force close file
     * ********************************************************************************************/
    static void *epool_pth(void *arg);
    /**********************************************************************************************
     * last Change : 2022-4-25
     * $vision : 0.1$
     * $Author : Tom Chang $
     * $force close file
     * ********************************************************************************************/
    int forceClose();
};

extern class claserUserDev laserDev;

struct point
{
    double x;
    double y;
};
struct segment{
   point start;
   point end;
};
int bCut_across(bCut_Line line1,bCut_Line line2);
int bCut_intersection(bCut_Line line1,bCut_Line line2, bCut_Point  *pnt,double diff);
bool bCut_is_one_line(bCut_Line line1,bCut_Line line2,double diff);
bool bCut_is_one_point(bCut_Point pnt1,bCut_Point pnt2,double diff);
bool bCut_point_in_line(bCut_Point pnt,bCut_Line line,double diff);
double pnt_to_pnt(bCut_Point pnt1,bCut_Point pnt2);
#endif // CLASERUSERDEV_H
