#ifndef __LASER_STU__
#define __LASER_STU__

#include <QMutex>
//#pragma pack(16)

#define PG_SZ 4096

#define CLS_CMD_FILE "clsCMD"

#define LS_PARA_FILE "lsparam-"

#define PTH_LIMIT 100
#define PT_SHM "PT_SHM"

#define SIM_MODE 0
#define CH368_MODE 1
#define USB30_MODE 2

#define CMD_HEADLENTH 8

#define MAX_CARD 8
#define MAX_USB 16
#define MAX_LASER 16
#define MAX_ENCODER 16
#define MAX_IO 16

#define MAX_IO_INPUT MAX_IO
#define MAX_IO_OUTPUT MAX_IO_INPUT

#define MAX_INPUT 16
#define MAX_OUTPUT MAX_INPUT

#define PARAM_SIZE (2 * PG_SZ)
#define VIR_SIZE  (128 * 1024)
#define PCI_SIZE  (128 * 1024)
#define KSIN_SIZE (232 * PG_SZ)
#define DATA_SIZE (232 * PG_SZ)

#define CMD_MMAP_SIZE (232 * PG_SZ)

#define LSTYPE 0xFFDF


#define FILE_HEAD 4096
#define FILE_JPLIST 5120
#define FILE_JPDATA 12288


#define MAX_CHECK_TIMES 1000

#define REAL_MAX 0x00ffffff






struct ch368Mem;
struct fileDevice;
struct Claser;
struct ksinStu;
struct laserPDO;
struct ch368Mem;
struct  bCut_Point
{
    double x;
    double y;
};
struct  bCut_Line
{
    double param[20];
    double power[20];
};

struct  ls_distortion
{
        // 幅面参数
        double size_x;
        double size_y;
        // 功率参数
        double power;
        double freq;
        double vel;
        // 线性畸变参数
        int line_distortion_enbale;
        double line_distortion_x;
        double line_distortion_y;
        double line_distortion_xf;
        double line_distortion_yf;
        // 九点矫正参数
        int distortion_9_enbale;
        double distortion_9_param_x[3][3];
        double distortion_9_param_y[3][3];
        double distortion_9_delta_x[3][3];
        double distortion_9_delta_y[3][3];
        // 二十五点矫正参数
        int distortion_25_enbale;
        double distortion_25_param_x[5][5];
        double distortion_25_param_y[5][5];
        double distortion_25_delta_x[5][5];
        double distortion_25_delta_y[5][5];
        //视觉矫正
        int distortion_vision_enbale;
        //菱形梯形矫正
        int distortion_diatarp_enable;
        double distortion_diamond_x;
        double distortion_tarpe_x;
        double distortion_diamond_y;
        double distortion_tarpe_y;
        //多点矫正
        int distortion_xx_enbale;
        int distortion_xx_size;



                unsigned int pwmheigh;
                unsigned int pwmlow;
};
struct  laserInput_bCut
{
        double setVel;
        double setFreq;
        double setPower;
        double setWait;
        int lineNum;
        int execute;

        double size_w;
        double size_h;


        double set_size_h;
        double set_size_w;

        double set_size_mark_w;
        double set_size_mark_h;

        int set_size_w_num;
        int set_size_h_num;

        double set_size_d;
        double set_size_wait;
        double set_size_freq;
        double set_size_power;
        double set_size_vel;

        int set_size_execute;


        int map_xi;
        int map_yi;
        double map_x;
        double map_y;


        double set_ls_off_wait;




        int set_pnt_enable;
        double set_add_len;
        double set_pnt_power;
        double set_pnt_freq;
        double set_pnt_len;
        double set_pnt_dly;
};
struct  laserOutput_bCut
{
        int cur_line;
        double bcut_x;
        double bcut_y;
        double bcut_x_start;
        double bcut_y_start;
        double bcut_lenth;
        float bcut_lenth_f;
        double bcut_wait;
        double bcut_wait_count;
        double bcut_divpos;
        double bcut_curlenth;
        double bcut_curVel;


};
struct  laserBuff_bCut
{
        struct  bCut_Line *plines;
        int old_execute;
        int cur_line;
        double bcut_x;
        double bcut_y;
        double bcut_x_start;
        double bcut_y_start;
        double bcut_lenth;
        float bcut_lenth_f;
        double bcut_wait;
        double bcut_wait_count;
        double bcut_divpos;
        double bcut_curlenth;
        double bcut_curVel;
        double bcut_curx;
        double bcut_cury;
        double bcut_curArg;

        double bcut_curFreq;
        double bcut_curPower;

        int old_set_size_execute;
        int cur_circle_h;
        int cur_circle_w;
        double circle_x;
        double circle_y;
        //struct  bCut_Line xlines[4096];

        double bcut_setFreq;
        double bcut_setPower;

};

struct  laserLines_Buff
{
        float dTargetX;
        float dTargetY;

        float dLineLenth;

        int iMoveRet;
        int iDelayCounter;


        double dFeedX;

        int iOldStart;

        int iOldScaleStart;
};
struct  laserLines_Input
{
        double dWidth;
        double dInterval;

        uint lineType;
        uint powerType;


        double powerTypes[20];

        double dRectWidth;

        double dSetLineVel;
        double dSetJMPVel;


        int iJmpDelayNs;
        int iLsOpenDelay;
        int iLsCloseDelay;

        int iTestSet;

        double curPower;

        int iStart;

        int iScaleStart;

        int iLaserOn;
        int iEnable;

        double lineFeed;

        double dOuterFeed;
        double dInterFeed;

        int iLaserOnMode;


        double dParamSpeed;
        double dCurSpeed;

        double dAutoYFeed;

        double dTestLenth;
        double dTestInterval;
        int dTestNums;
        double dTestVel;
        double dTestPower;
        int iJmpDelay;


        int iTestStart;


        int iArcFeedEnable;
        double iArcFeedX;
        double iArcFeedY;

        int iRectStart[10];
        int iRectEnd[10];
        int iRectEnable[10];
        double dHeight[10];
        int iRectType[10];
};
struct  laserLines_Output
{
        double dNextPos;
        double dCurPos;
        double dEncPos;
        double dOldPos;
        double dTargetYPos;
        int xTest;

        int iVelOut;
        double dEncDiff;
        long new_sync;

        int rectLines;
        double dLinesLenthPer;

        double dOpenTime;

        double outerSpec;
        double interSpec;
        double curEncSpec;

};


struct hardwareConf{
    //unsigned int laserEnable[MAX_LASER];
    //unsigned int encoderEnable[MAX_ENCODER];
    //unsigned int IOEnable[MAX_INPUT];
    unsigned int laser_num;
    unsigned int encoder_num;
    unsigned int IO_num;
};
struct encoderPDO{
    char name[256];
    volatile int iNo;
    volatile int connectNo;
    volatile unsigned int enable;
    volatile unsigned int newEnc;
    volatile unsigned int oldEnc;
    volatile int deltaEnc;
    volatile unsigned long sumEnc;
    volatile unsigned int sync_us;
    volatile double pos;
    volatile double vel;
    volatile double acc;

};
struct IOPOD{
    char name[256];
    volatile unsigned int iNo;
    volatile unsigned int enable;
    volatile unsigned int input;
    volatile unsigned int output;
    volatile unsigned int inputDelay_ns;
};
struct laserInPut_tabCut //激光模切
{
        //触发信号
    volatile unsigned int xStart;//启动
    volatile unsigned int xHalt;//暂停
    volatile unsigned int xReset;//复位
    volatile unsigned int xResetCut;//复位重切
    volatile unsigned int xCutOut;//切出
    volatile unsigned int xAuto;//手自动 0:手动  1:自动
    volatile unsigned int xReady;//就绪
    volatile unsigned int xWhileSpace;//检测到留白
    volatile unsigned int xPreview;//极耳预览
        volatile unsigned int xRunTest;//模拟运行

        volatile unsigned int xClear;//清空数据
        volatile unsigned int xResetError;//故障复位
        volatile unsigned int xLineGradualPower;//开启线段渐变功率

        volatile unsigned int show_jpno;//极耳预览位置
        //volatile float cutDelayPos;//切割延时距离
        volatile unsigned int cutPos;//倒切位置


        //编码器参数
        volatile double encScale;//激光编码器分辨率,每10000个脉冲对应的长度

        //触发选项
        volatile unsigned int xLineOn;//1激光切直线
        volatile unsigned int xErrorLineOn;//1过接带切直线 0切出

        volatile unsigned int xWhileSpaceCut;//留白接带续切 切直线
        volatile unsigned int xOff;//脱机运行
        volatile unsigned int xPCut;//极片倒切
        //volatile unsigned int xIOStyle;//IO
        volatile unsigned int xIOConnect;//激光IO联机信号
        volatile unsigned int xHeart;//心跳信号

        volatile unsigned int xCutOutStyle;//切出触发模式 0:上升沿触发,1:值触发

        //外部输入IO
        volatile unsigned char xLInputIO;//外部IO信号低位
        volatile unsigned char xHInputIO;//外部IO信号高位

        //切直线参数
        volatile float linePower;//切直线功率
        volatile float lineFreq;//切直线频率
        volatile unsigned int lineWave;//切直线脉宽
        volatile float lineDuty;//切直线占空比

        volatile float ErrorLineLength;//过接带直线长度
        volatile float powerEncSpeed;//功率限制速度
        volatile float lsPowerTime;//功率频率滞后时间(ms)

        volatile float compensateSpeed;//补偿参考速度
        volatile float compensatePower;//补偿功率
        volatile float compensateFreq;//补偿频率

        volatile float fInTabH;//指定高度输出
        volatile float fOnTabH;//指定输出高度下限(信号)
        volatile float fInTabDis;//指定极耳前长度信号

};
struct laserOutPut_tabCut //激光模切输出
{
        volatile unsigned int ioInput;//激光输入IO
        volatile unsigned int ioOutput;//激光输出IO

        volatile unsigned int countor_act;//速度周期计数
        volatile int lastjp;//激光切割当前极耳编号
        volatile int lastline;//激光切割当前线段编号
        volatile int xlastjp;//间距上
        volatile int jpCount;//激光切割的极耳数量
        volatile int jpCutCount;//激光切割的黄标切出极耳数量
        volatile int jpErrorCount;//激光切割的复位极耳数量


        volatile float encVel;//激光编码器速度(实时)
        // volatile float curx;//激光振镜X方向实际位置(通过缩放偏移后的位置坐标)
        // volatile float cury;//激光振镜Y方向实际位置(通过缩放偏移后的位置坐标)

        volatile double encdiv;//激光编码器变化长度
        volatile double encpos;//激光编码器运行总长

        volatile double jp_pos;//当前走到的极片位置
        volatile double DelayPos;//切割延迟位置

        volatile float cut_inX;//激光切入宽度，自动运算
        volatile float cut_startX;//激光切入起始位置

        volatile unsigned short xIoInput;//输入IO值
        volatile unsigned char xLCharIoInput;//输入IO值低位
        volatile unsigned char xHCharIoInput;//输入IO值高位
        volatile unsigned int xIoInputArray[MAX_IO_INPUT];

        volatile unsigned short xIoOutput;//输出IO值
        volatile unsigned char xLCharIoOutput;//输出IO值低位
        volatile unsigned char xHCharIoOutput;//输出IO值高位
        volatile unsigned int xIoOutputArray[MAX_IO_OUTPUT];


        volatile unsigned int currentPfNo;//当前使用配方编号
        volatile unsigned int nextPfNo;//下一个使用配方编号


};
struct laserInPut_config //
{
        //信息参数
        char name[256];//名称

        //配置参数
        volatile int type;		// 0:模拟模式   1:PCIE 2:USB3.0
        volatile int iEncNo;	//编码器编号
        volatile int iCH368No;  //PCIE编号
        volatile int USB30No;   //USB编号
        volatile int laserType;//激光器类型

        volatile unsigned int countor_set;//速度周期计数设置值

        //调试参数
        volatile float testVel;//调试速度
        volatile float testLen;//激光调焦矩形边长
        volatile float testPower;//激光调试功率
        volatile float testFreq;//激光打点密度

        //振镜参数
        volatile float xZoom;//X方向缩放
        volatile float yZoom;//Y方向缩放
        volatile float xOffset;//X方向偏移
        volatile float yOffset;//Y方向偏移
        volatile float cgX;//X畸变
    volatile float cgY;//Y畸变

    //激光配置参数
        volatile float ratedFreq;//额定频率(中心频率，单位HZ)
        volatile float ratedPower;//额定功率%
        volatile float mjPower;//单脉冲能量
        volatile float minPower;//最小功率%
        volatile float maxPower;//最大功率%
        volatile float minFreq;//最小频率
        volatile float maxFreq;//最大频率
        volatile float lsOnTime;//开光延时时间(ms)
        volatile float lsOffTime;//关光延时时间(ms)

        //振镜参数
        volatile unsigned int xMirrorXaY;//振镜X-Y方向
        volatile unsigned int xMirrorX;//振镜X方向
        volatile unsigned int xMirrorY;//振镜Y方向

        //激光信号
        volatile unsigned int xRedOn; //开红光
        volatile unsigned int xLaserOn; //开激光

    //调试信号
        volatile unsigned int xCheck;//校正
        volatile unsigned int xFocus;//调焦
    volatile unsigned int xEStop;//急停信号
};
struct laserOutPut_config //
{
        volatile unsigned int iNo;//编号
        volatile unsigned int enable;//激光驱动使能
        volatile unsigned int ctrtype;//激光控制模式

        volatile unsigned int lsState;//激光状态步
        volatile float lsVel;//激光光斑速度(实时)
        volatile float lsPower;//激光脉冲能量(实时)
        volatile float lsFreq;//激光脉冲频率(实时)
        volatile float xPos;//激光振镜X方向位置(未计算之前的位置坐标)
        volatile float yPos;//激光振镜Y方向位置(未计算之前的位置坐标)
        volatile unsigned int MOStatue;//MO状态
        volatile unsigned int BSStatue; //BS状态
        volatile unsigned int RedStatue; //红光状态
        volatile float interval_sec;//激光内核线程周期单位S
        volatile unsigned int mirror_x;//激光振镜X方向实际位置(通过缩放偏移畸变后的位置坐标)
        volatile unsigned int mirror_y;//激光振镜Y方向实际位置(通过缩放偏移畸变后的位置坐标)
};


//共享内存(输入参数)
struct laserInPut{
        struct laserInPut_config input_config;
        struct laserInPut_tabCut input_tabCut;
        //板材切割输入
        struct laserInput_bCut bCut_in;
        struct ls_distortion distortion;

        struct  laserLines_Input lsLine_input;
};
//输出
struct  laserOutPut
{
        struct laserOutPut_config output_config;
        struct laserOutPut_tabCut output_tabCut;

        //板材切割输出
        struct laserOutput_bCut bCut_out;

        struct  laserLines_Output lsLine_output;
};

struct laserPDO{
    struct laserInPut* input = nullptr;
    struct laserOutPut* output = nullptr;
};
struct ksinStu{
    volatile unsigned int init;
    volatile float scdiv;
    volatile float sin[20000];
    volatile float cos[20000];
    volatile float arctan[20000];
};
struct Claser{
    volatile unsigned int dataType;
    volatile int enable;
    volatile unsigned int iNo;

    volatile struct fileDevice *fdev;

    volatile int iState;
    volatile int type;                  // 0:模拟模式   1:PCIE 2:USB3.0

    volatile unsigned int dataSize;
    void *virMem;
    void *paramMem;
    void *dataMem;
    void *ksinMem;

    struct ch368Mem* ch368;
    void *USB3;
    void *others;

    struct laserPDO PDO;
};
struct ch368Mem{
        char name[256];
        volatile int enable;
        void *pDev;
        void *IO;
        void *Mem;
        void *Conf;
        int bind;

        int encRead;

        unsigned int newEnc;

        bool laserFlag;
        unsigned int laserReg;
        unsigned int freqRegL;
        unsigned int freqRegH;
        bool mirrorFlag;
        unsigned int mirrorReg;

        unsigned int lsDelay;

        unsigned int lsOpenDelay;
        unsigned int lsCloseDelay;
        unsigned int iLsMoTime;
        unsigned int iLsMoMode;

        unsigned int iLStart;
        unsigned int iLEnd;
        unsigned int iLVel;
        unsigned int iLID;
        unsigned int iLFeed;
        unsigned int iLMark;

        int mirrorDiffX;
        int mirrorDiffY;

        int mirrorDiffX0;
        int mirrorDiffY0;
};
struct fileDevice{
    volatile void *ksinAdr;
    struct Claser laser[MAX_LASER];
    struct ch368Mem *ch368;
    struct encoderPDO *encoder;
    struct IOPOD *IO;
};
struct kenrel_TaskInfo{
    int writing;
    char name[256];
    unsigned long task_countor;
    //unsigned long task_cost_target;
    unsigned long task_cost;
    unsigned long task_cost_avg;
    unsigned long task_cost_max;
    unsigned long task_cost_min;
    unsigned long task_cost_sum;

    unsigned long task_period;
    unsigned long task_period_avg = 0;
    unsigned long task_period_max = 1;
    unsigned long task_period_min = 10000000000;
    unsigned long task_period_sum = 0;

    unsigned long task_outof_sync;

    unsigned long task_interrupt_delay;

    unsigned int cpu;
    unsigned int clear;
};

//#pragma pack()

struct taskInfo{
    int no;
    kenrel_TaskInfo info;
    int counter;
    int oldCounter;
    bool run = false;
};

struct taskInfoShm{
    QMutex mtx;
    int size;
    int upudate;
    taskInfo task[PTH_LIMIT];
};
struct cmd_file_stu
{
    char visionInfo[256];
    struct kenrel_TaskInfo ktask;
    struct ch368Mem ch368[MAX_CARD];
    struct encoderPDO encoder[MAX_ENCODER];
    struct IOPOD IO[MAX_IO];

    unsigned int heartCount_K;
    unsigned int heartCountOld_K;
    unsigned int heartMax_K;
    unsigned int heartAlarm_K;
    unsigned int heartEqua_K;

    unsigned int heartCount_U;
    unsigned int heartCountOld_U;
    unsigned int heartMax_U;
    unsigned int heartAlarm_U;
    unsigned int heartEqua_U;

    unsigned int confDone;
    unsigned int runing;

    unsigned int estop;

};
//struct jp_param{
//    volatile float fLenth;
//    volatile int iNjp;
//    volatile float power;
//    volatile float freq;
//    volatile float duty;
//    volatile float wave;
//};
//struct jp_list{
//    volatile int iPtr;
//    volatile float fStart;
//    volatile float fEnd;
//    volatile int iPFPtr;
//};
//struct jp_data{
//    volatile int iNum;
//    volatile int iCount;
//    volatile int iSize;
//    volatile float fW;
//    volatile float fL;
//    volatile float fP;
//};
//struct jp_power{
//    volatile float fPos;
//    volatile float power;
//    volatile float freq;
//    volatile float duty;
//    volatile float wave;
//    volatile float vel;
//    //volatile float setFreq;
//};
//struct line_para{
//    volatile int iNum;
//    volatile int iType;
//    volatile float param[20];
//    volatile float squap[10];
//    volatile struct jp_power pfparam[5];
//};
#endif
