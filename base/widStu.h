#ifndef WIDSTU_H
#define WIDSTU_H

//#define WINDOWS_CORE
#include <QString>
#include <QList>
#ifdef GUIMODE
#include <QInputDialog>
#include <QFileDialog>
#include <QWidget>
#include <QMdiArea>
#include <QMessageBox>
#include <QAction>
#include "cbase_signals.h"

#endif
//#include "../dev/claseruserdev.h"
#include "../dev/studef.h"

#include <QMutex>

#include <QDateTime>
#include <QTime>

#include <QSql>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSettings>
// QT JSON
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
//#include <QInputDilog>



// 额外支持轴和PDO等
#include "../etcHeader/etcheader.h"

#include <QDebug>
#ifdef WINDOWS_CORE
    #include "opencv2\opencv.hpp"
    #include "HalconCpp.h"

    using namespace cv;
    using namespace HalconCpp;
#endif
#define RESTREE_NULL                0x00000000// 空页面
#define RESTREE_HARDWARE            0x01000000// 硬件配置页面
#define RESTREE_CORES               0x02000000// CPU核心配置页面
#define RESTREE_PCIE                0x03000000// PCIE总页
#define RESTREE_PCIEHSL             0x03010000// PCIE硬件数据页面
#define RESTREE_USB30               0x04000000// USB3.0总页
#define RESTREE_USB30HSL            0x04010000// USB3.0硬件数据页面

#define RESTREE_ETHERCAT            0x05000000// ETHERCAT 总页
#define RESTREE_ETCSLAVE            0x05010000
#define RESTREE_ETCPDO              0x05010100
#define RESTREE_ETCAXIS             0x05020000
#define RESTREE_AXISREF             0x05020100

#define RESTREE_ETHERNET            0x06000000
#define RESTREE_TCP                 0x06010000
#define RESTREE_UDP                 0x06020000
#define RESTREE_ADS                 0x06030000
#define RESTREE_FINS                0x06040000

#define RESTREE_TASKS               0x07000000
#define RESTREE_KENRELTASK          0x07010000
#define RESTREE_USERTASK            0x07020000
#define RESTREE_OUTTASK             0x07030000
#define RESTREE_ETCTASK             0x07040000
#define RESTREE_MAPPINGTASK         0x07050000
#define RESTREE_OTHERTTASK          0x07060000

#define RESTREE_PROJECT             0x08000000
#define RESTREE_PJSELECT            0x08010000
#define RESTREE_LASER               0x08010100
#define RESTREE_ENCODER             0x08010200
#define RESTREE_IO                  0x08010300

#define RESTREE_LSHSL               0x08010101
#define RESTREE_ENCHSL              0x08010201
#define RESTREE_IOHSL               0x08010301

#define RESTREE_USERHMI             0x09000000
#define RESTREE_HMIPAGE             0x09010000

#define RESTREE_MOTIONSTRUCT        0x0A000000
#define RESTREE_TENSIONV            0x0A010000
#define RESTREE_TENSIONT            0x0A020000
#define RESTREE_LSTRAC              0x0A030000
#define RESTREE_RECTIFY             0x0A040000
#define RESTREE_FOLLOWTRAC          0x0A050000
#define RESTREE_NORMALTRAC          0x0A060000
#define RESTREE_NORMACILYDER        0x0A070000
#define RESTREE_MSSYNCHAL           0x0A080000



#define RESTREE_FlOWCHART           0x0B000000
#define RESTREE_CHARTPAGE           0x0B010000
#define ICON_PATH "/home/icon/"

#define EPOLL_LISTEN_CNT 100
long long diff_tv(struct timespec start, struct timespec end);
void timespec_add_us(struct timespec *it , long _ns);
void initktask(struct kenrel_TaskInfo *stask);

struct pageinfo
{
    int type;
    QString name;
    QString icon;
};
enum MStructState{
    NOT_INIT,
    ERROR,
    INITING,
    IDEL,
    STARTING,
    EXECUTE,
    HOLDING

};
struct usertask
{
  void *(*fun)(void* arg);
  void *arg;
  std::string name;

};
struct userpthread
{

    std::mutex *mtx = nullptr;
    std::vector<usertask> tasklist;
    std::string name;
    kenrel_TaskInfo taskinfo;
    struct sigevent evp;
    struct itimerspec it;
    unsigned int count = 0;
    unsigned int cyctime_ns;
    unsigned int priority;
    struct timespec time_in_last;
    struct timespec time_in;
    struct timespec time_out;
    timer_t id;

    int timerfd;

    pthread_t pth;
};
#define INIT_FILE "init.ini"
//QHash<QString,QHash<QString,cBaseVar>> qMemHash;
#define GROUP_HASH QHash<QString,QHash<QString,cBaseVar>>
#define VAR_HASH QHash<QString,cBaseVar>
#define MAX_INPUT_MAAPING 5000
#define MAX_OUTPUT_MAAPING 5000

class cBaseVar;
class trigger;

enum cDIR
{
    C_UNSET,
    C_INPUT,
    C_OUTPUT,
    C_PARAM
};
enum cTYPE
{
    C_UNTYPE,
    C_INT,
    C_UINT,
    C_LONG,
    C_ULONG,
    C_FLOAT,
    C_DOUBLE,
    C_BOOL,
    C_BYTEARY,
    C_CHAR,
    C_SHORT,
    C_USHORT,
    C_QSTRING,
    C_CVMAT,
    C_HOBJ,
    C_FUNC,
    C_AXIS,
    C_ACTION
};
struct cbaseFunc
{
    int(* f)(void *) = nullptr;
    void* arg = nullptr;
    QString funcName;
};
struct pjmember
{
    QString name;
    int iNo;
    int filefd;
    void *PDO = nullptr;
    QWidget *form = nullptr;
    void * mdi = nullptr;
    unsigned int type = 0;
};
struct pjLaser
{
    QString name;
    int iNo;
    int filefd;
    void *PDO = nullptr;
    QWidget *form = nullptr;
};
struct pjEnc
{
    QString name;
    int iNo;
    void *PDO = nullptr;
    QWidget *form = nullptr;
};
struct pjIO{
    QString name;
    int iNo;
    void *PDO = nullptr;
    QWidget *form = nullptr;
};
struct cproJect
{
    QString name;
    QList<pjLaser> lsList;
    QList<pjEnc> encList;
    QList<pjIO> ioList;
};
struct mappingInfo{
    char name[64];
    int dir = 0;
    cBaseVar *varAdr = nullptr;
    int mappingAdr = -1;
};
struct mappingInfo_sub
{
    int inputSize = 0;
    int outputSize = 0;

    mappingInfo inputInfo[5000];
    mappingInfo outputInfo[5000];

};
struct mappingMemery
{

    unsigned long input[5000];
    unsigned long output[5000];

};


struct cBaseError
{
    cBaseVar *var = nullptr;
    cBaseVar *fun = nullptr;
    bool xUsed = false;
    bool xError = false;
    int type = 0;
    QString name;
    QString info;

};

struct sysErrorList
{
    QMutex lock;
    QVector<cBaseError> sysErrors;
    QVector<cBaseError> userErrors;
    bool xError = false;
    bool xEtherCATError = true;
    bool xEtherCATNeed = true;
    cBaseVar *cEtherCATFun = nullptr;

    QVector<cBaseError> curErrors;
};



class cBaseVar
{
public:
    cBaseVar();
    cBaseVar(QString sname,cTYPE stype,cDIR sdir,int slevel);
    cBaseVar(QString sname,cTYPE stype,cDIR sdir,int slevel,int ssize);
    cBaseVar &operator= (const cBaseVar& x);//{memcpy(this,&x,sizeof(cBaseVar));return *this};
    void setFather(QString sfather);
    int varChange(void *var);
    bool isNull(void);
    bool isINT(void);
    bool isUINT(void);
    bool isLONG(void);
    bool isULONG(void);
    bool isFLOAT(void);
    bool isDOUBLE(void);
    bool isBOOL(void);
    bool isBYTEARY(void);
    bool isCHAR(void);
    bool isSHORT(void);
    bool isUSHORT(void);
    bool isQSTRING(void);
    bool isCVMAT(void);
    bool isHOBJ(void);
    bool isFUNC(void);
    bool isAXIS(void);
    bool isACTION(void);


    bool isVALUE(void);


    QString toString(void);
    QString fullname(void);
    QString dirToStr(void);
    QString typeToStr(void);

    int setByString(QString str);
    static int checkMapping(int adr);
    int setMapping(int adr);
    int setQString();
    //int set(char *v);
    QString name;
    cTYPE type = C_UNTYPE;
    int level = 0;
    cDIR dir = C_UNSET;
    int size = 0;
    void *mem = nullptr;
    void *real_mem = nullptr;
    int bind = 0;
    int ptlevel = 200;
    QString father = "default";
    QString info = "";
    bool connect = false;
    cBaseVar* connect_Var = nullptr;
    QString connect_Name = "";
    static bool connect_check;//深检查
    static bool connect_run;//深检查
    static void *(*connect_fun)(void *arg);
    static void *connect_arg;
    static void *(*connect_fun_tail)(void *arg);
    static void *connect_arg_tail;
    double max = 99999999999;
    double min = -99999999999;
    bool byuser = false;
    static QString  typeStr(cBaseVar *var);
    static QString  dirStr(cBaseVar *var);
    bool *enable = nullptr;
    bool retain = false;
    int accur = 3;
    int needUpdate = 1;

    int beConnectTimes = 0;

    int mapping = -1;

    QString jsonValue;
    static int mappingAll(mappingInfo_sub *mem);

    QString oldValue;

    int showBase = 10;
    int insertValueChange();
    //void changeValue(QString oldValue);
    // db

    static QSqlDatabase db;
    static QMutex dbMtx;
    static QMutex tostrMtx;
    static QString lastDB;
    static int openDB(QString path);
    static int newDB(QString path);

    static int rereadDB();

    static int all_db_to_vars();
    static int all_vars_to_db();

    static int db_to_var(QString fullname);
    static int db_to_var(QString group,QString name);
    static int db_to_var(cBaseVar *var);
    static int var_to_db(QString fullname);
    static int var_to_db(QString group,QString name);
    static int var_to_db(cBaseVar *var);


    // connector 

    static int var_connect(QString input, QString output);
    static int var_connect(cBaseVar* input , cBaseVar* output);
    static int var_disconnect(QString input);
    static int var_disconnect(cBaseVar* input);
    static QMutex connector_mtx;
    static QHash<QString , cBaseVar *> connector;
    static QString db_lastError;
    static int set_connect_fun(void *(*fun)(void *arg),void *arg);
    static int set_connect_fun_tail(void *(*fun)(void *arg),void *arg);
    #ifdef GUIMODE
    static cbase_signals *base_signals;
    #endif
    static int emitSignal(QStringList info);

    static QVector<cBaseVar> vartypes;



    //选项卡相关
    QStringList boxList;
    int addCheckBox(QString value,QString info);
    bool isCheckBox();
    QString getCheckBoxString();
    //static canDelGroup();
    bool xAskRetain = false;

    //错误信息相关
    static sysErrorList errorList;
    static int appendSysError(QString var,QString func);
    static int appendUserError(QString var,QString func);
    static int appendSysError(cBaseVar *var,cBaseVar *func);
    static int appendUserError(cBaseVar *var,cBaseVar *func);
    static int syncErrors();
private:
    void initSelf(QString sname,cTYPE stype,cDIR sdir,int slevel,int ssize);

};



//extern QHash<QString,cBaseVar> cBaseHash;
extern QHash<QString,QHash<QString,cBaseVar>> qMemHash;
extern int gvlLevel;
extern int protectLevel;



QString getStr(QWidget *self,QString title,QString notice);

int newCVar(void *mem,QString name,cTYPE type,cDIR dir,int level,QString father,QString info);
int newCVar(void *mem,QString name,cTYPE type,cDIR dir,int level,int size,QString father,QString info);
int reconfCVar(void *mem,QString name,cTYPE type,cDIR dir,int level,QString father,QString info);

int newCVar(bool *mem,QString name,cDIR dir,int level,QString father,QString info);
int newCVar(char *mem,QString name,cDIR dir,int level,QString father,QString info);
int newCVar(short *mem,QString name,cDIR dir,int level,QString father,QString info);
int newCVar(ushort *mem,QString name,cDIR dir,int level,QString father,QString info);
int newCVar(int *mem,QString name,cDIR dir,int level,QString father,QString info);
int newCVar(uint *mem,QString name,cDIR dir,int level,QString father,QString info);
int newCVar(long *mem,QString name,cDIR dir,int level,QString father,QString info);
int newCVar(ulong *mem,QString name,cDIR dir,int level,QString father,QString info);
int newCVar(float *mem,QString name,cDIR dir,int level,QString father,QString info);
int newCVar(double *mem,QString name,cDIR dir,int level,QString father,QString info);
int newCVar(QString* mem, QString name, cDIR dir, int level, QString father, QString info);
#ifdef GUIMODE
int newCVar(QAction* mem, QString name, cDIR dir, int level, QString father, QString info);
#endif
#ifdef WINDOWS_CORE
int newCVar(Mat* mem, QString name, cDIR dir, int level, QString father, QString info);
int newCVar(HObject* mem, QString name, cDIR dir, int level, QString father, QString info);
#endif
int newCVar(cbaseFunc *mem, QString name, cDIR dir, int level, QString father, QString info);
int newCVar(axis_ref* mem, QString name, cDIR dir, int level, QString father, QString info);

cBaseVar *newCVar(QString name,cTYPE type,cDIR dir,int level,QString father,QString info);


int reconfCVar(bool *mem,QString name,cDIR dir,int level,QString father,QString info);
int reconfCVar(char *mem,QString name,cDIR dir,int level,QString father,QString info);
int reconfCVar(short *mem,QString name,cDIR dir,int level,QString father,QString info);
int reconfCVar(ushort *mem,QString name,cDIR dir,int level,QString father,QString info);
int reconfCVar(int *mem,QString name,cDIR dir,int level,QString father,QString info);
int reconfCVar(uint *mem,QString name,cDIR dir,int level,QString father,QString info);
int reconfCVar(long *mem,QString name,cDIR dir,int level,QString father,QString info);
int reconfCVar(ulong *mem,QString name,cDIR dir,int level,QString father,QString info);
int reconfCVar(float *mem,QString name,cDIR dir,int level,QString father,QString info);
int reconfCVar(double *mem,QString name,cDIR dir,int level,QString father,QString info);
int reconfCVar(QString* mem, QString name, cDIR dir, int level, QString father, QString info);
#ifdef GUIMODE
int reconfCVar(QAction* mem, QString name, cDIR dir, int level, QString father, QString info);
#endif
#ifdef WINDOWS_CORE
int reconfCVar(Mat* mem, QString name, cDIR dir, int level, QString father, QString info);
int reconfCVar(HObject* mem, QString name, cDIR dir, int level, QString father, QString info);
#endif
int reconfCVar(cbaseFunc* mem, QString name, cDIR dir, int level, QString father, QString info);
int reconfCVar(axis_ref* mem, QString name, cDIR dir, int level, QString father, QString info);




QJsonObject hashToJson();

//int reconfCVar(void *mem,QString name,cTYPE type,cDIR dir,int level,int size,QString father,QString info);
int canDelGroup();
int newGroup(QString name);
int delGroup(QString name);
int delVar(QString name);
bool hasGroup(QString name);

template <class param>
class cParam
{
public:
    cParam();
    cParam(param x,bool sISV);
    cParam &operator = (const param &x);
    
    param value;
    QString name;
    int level = 0;
    cDIR dir = C_UNSET;
    int bind = 0;
    bool isValue;
    QString father = "default";

};

struct cLogInfo
{
    QString title;
    int type;
    QString info;
    QString time;
};
cBaseVar *findCbase(QString father,QString name);
cBaseVar *findCbase(QString fullname);
int setptLevel(QString father,QString name,int level);

void updayePtLevel();
void insertLog(int type , QString title , QString info);

QList<cLogInfo> *popLog();
extern QList<cLogInfo> *infoList;
bool levelCheck(cBaseVar *var);
bool levelCheck_nolog(cBaseVar *var);

int getuslevel();
int getptlevel();
bool check_US_level(cBaseVar *var);
bool check_PT_level(cBaseVar *var);

extern QString syncError;

int syncGroup(QString src,QString dest);
int syncParam(QString src,QString dest);
int syncGroup_notMatch(QString src,QString dest);
bool cmpParam(QString src,QString dest);
bool cmpParam(cBaseVar *src,cBaseVar *dest);

// 函数hash表相关
struct cFun{
   int (*fun)(QStringList arg);
   //void *fun;
   QString name;
   QString info;
   QStringList argInfo;
};
extern QHash<QString,cFun> funcHash;
extern QString fun_Error;


int registerFun(QString funName,int (*fun)(QStringList arg),QString info,QStringList argInfo);
int callFun(QString funName,QStringList arg);


int updateGroup(QString group);
//int updateParam(QString fullName);
int updateParam(cBaseVar *var);
// 映射函数相关
//int initMappint(void *mapping_mem);
//int clearMapping(cBaseVar *var,int dir,int adr,void *mapping_mem);
//int checkMapping(cBaseVar *var,int dir,int adr,void *mapping_mem);
//int startMapping(void *mapping_mem);

//class signals_group : public QWidget
//{
//    Q_OBJECT
//public:
//    signals_group(QWidget *parent = 0);
//    ~signals_group();
//public slots:
//    void input_Signal(QString info);
//signals:
//    void output_Signal(QString info);
//};

QString MSState_to_String(MStructState state,int type);
QString MSState_to_Color(MStructState state,int type);
class trigger
{
public :
    void exec(bool execute);
    bool tF = 0;
    bool tR = 0;
private:
    bool xOld;
};

class epool_tasks
{
public:
    int epfd;
    pthread_t eppth;
    std::vector<userpthread> pthvector;
    std::mutex pthmtx;
    uint setcpu = 1;

    //static newTask();
    static void *epool_pth(void *arg);
    void pthread_fn(void *arg);
    int newPthread(std::string name,unsigned int cyctime_ns);
    int push_task(int pthid ,  void *(*fun)(void* arg),void *arg,std::string name);
    int start();
};
void checkRetain(cBaseVar *var);
QStringList findHasStr(QString filter);


bool getBIt(char *var , int pos);
int setBIt(char *var , int pos , bool set);


template<typename T>
bool getBItEx(T *var ,int pos)
{

    bool ret;
    ret = ((*var) & (1 << pos)) == ((1 << pos));
    return ret;
}
template<typename T>
bool setBItEx(T *var , int pos , bool set)
{
    if(set){
        *var = *var | (1 << pos);
    }else{
        *var = *var & ~(1 << pos);
    }


    return 0;
}


QJsonObject byteArrayToJsonObject(const QByteArray &data);

#ifdef WINDOWS_CORE
Mat _cv_HObj_to_Mat(HObject hobj);

HObject _cv_Mat_to_HObj(Mat cvmat);
#endif
#endif
