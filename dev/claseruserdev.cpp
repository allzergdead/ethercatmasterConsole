#include "claseruserdev.h"



class claserUserDev laserDev;
using namespace  std;

claserUserDev::claserUserDev()
{
    nullls.input = new laserInPut;
    nullls.output = new laserOutPut;
    cmdStu = &nullcmdStu;
    for(int i = 0;i < MAX_LASER;i++){
        lsStu[i] = new laserPDO;
        lsStu[i]->input = nullls.input;
        lsStu[i]->output = nullls.output;
    }
    //pthvector.clear();
}

int claserUserDev::writePf(void *data, int size,int iNo)
{
    //return write(cmdfd,data,size);
    unsigned int*adr = (unsigned int*)data;
    *(adr) = 0xFFEEFFEE;
    *(adr+1) = 1;
    return write(lsfd[iNo],data,size);

}
int claserUserDev::readPf(void *data,int iNo)
{
    //return write(cmdfd,data,size);
    if(data == nullptr){return 0;}
    //if(iNo<0||iNo>=MAX_LASER){return;}
    qDebug()<<"lsfd[iNo] = "<<lsfd[iNo]<<" iNo = "<<iNo;
    return read(lsfd[iNo],data,4096*232*4);
}
int claserUserDev::writePfPower(void *data, int size,int iNo)
{
    //return write(cmdfd,data,size);
    unsigned int*adr = (unsigned int*)data;
    *(adr) = 0xFFEEFFEE;
    *(adr+1) = 2;
    return write(lsfd[iNo],data,size);
}
int claserUserDev::writePcieMem(int PcieNo,int memAddr,void*data)
{
    //return write(cmdfd,data,size);
    char cmd[50];
    memset(cmd,0,sizeof (cmd));
    cmd[0] = 10;
    *(int*)((&cmd[0])+CMD_HEADLENTH) = PcieNo;
    *((int*)((&cmd[0])+CMD_HEADLENTH)+1) = memAddr;
    memcpy((&cmd[0])+CMD_HEADLENTH+4*2,data,4);
    return write(cmdfd,cmd,sizeof (cmd));
}
/**********************************************************************************************
 * last Change : 2022-4-25
 * $vision : 0.1$
 * $Author : Tom Chang $
 * $Init PCIE card and write sin and cos function to kernel dev
 * $return : 0 : sucessed
 * $         others : error
 * ********************************************************************************************/
int claserUserDev::init(void)
{

    //pid_t pid = getpid();
    //gDebug("pci dev pid : %d",pid);
    //struct sched_param param;
    //param.sched_priority = sched_get_priority_max(SCHED_FIFO);   // 也可用SCHED_RR
    //sched_setscheduler(pid, SCHED_RR, &param);                   // 设置当前进程
    //etc_main();
    string s1,s2,path;
    s1 = "/dev/";
    s2 = CLS_CMD_FILE;
    path = s1 + s2;
    cmdfd = open(path.c_str(), O_RDWR);
    if (cmdfd < 0){
        return 1001;
    }
    initDone = true;

    epfd = epoll_create(EPOLL_LISTEN_CNT);
    qDebug() << epfd;
    if (epfd < 0){
        return 1001;
    }

    //void *memTest = (mappingInfo_sub *)malloc(1024 * 1024 * 3);
    close(mpfd);
    mpfd = open("/dev/mappingshm", O_RDWR);
    //qDebug() << QString::fromStdString(path.str()) << "lsfd-"<< i << "="<< lsfd[i];
    if (mpfd < 0){
        close(mpfd);
        return 1003;
    }
    void * memTest = static_cast<void *>(mmap(NULL,ETC_SHM_SIZE,PROT_READ|PROT_WRITE,MAP_SHARED,mpfd,0));
    if ((ulong)memTest == 0xffffffffffffffff){
        close(mpfd);
        return 1002;
    }else{

    }
    mapping_mem = (mappingInfo_sub *)memTest;
    //mappingshm

    mapping_mem->inputSize = 0;
    mapping_mem->outputSize = 0;

    //pthread_create()
    newPthread("100ms_user_task",1000 * 1000 * 100);
    push_task(0,_100ms_task,(void *)this,"心跳线程");

    newPthread("1ms_mapping_task",1000 * 1000 * 1);
    push_task(1,_1ms_task,(void *)this,"映射线程");

    pthread_create(&eppth,NULL,epool_pth,this);
    return 0;
}
/**********************************************************************************************
 * last Change : 2022-5-6
 * $vision : 0.1$
 * $Author : Tom Chang $
 * $Init PCIE card and write sin and cos function to kernel dev
 * $return : 0 : sucessed
 * $         others : error
 * ********************************************************************************************/
void *claserUserDev::etcinit(void)
{
    std::stringstream path;
    path << "/dev/" << ETC_FILE;
    close(ETCfd);
    ETCfd = open(path.str().c_str(), O_RDWR);
    //qDebug() << QString::fromStdString(path.str()) << "lsfd-"<< i << "="<< lsfd[i];
    if (ETCfd < 0){
        close(ETCfd);
        return nullptr;
    }
    void *etcmem;
    etcmem = static_cast<void *>(mmap(NULL,ETC_SHM_SIZE,PROT_READ|PROT_WRITE,MAP_SHARED,ETCfd,0));
    if ((ulong)etcmem == 0xffffffffffffffff){
        ecm = nullptr;
        close(ETCfd);
        return nullptr;
    }else{
        ecm = (cec_shm_pdo *)etcmem;
        return etcmem;
    }
}
/**********************************************************************************************
 * last Change : 2022-5-6
 * $vision : 0.1$
 * $Author : Tom Chang $
 * $Init ksin mem in kenrel
 * $return : 0 : sucessed
 * $         others : error
 * ********************************************************************************************/
int claserUserDev::ksinInit(void)
{
    int i;
    void *cmdPtr;
    struct ksinStu *ksin_usrPtr;
    cmdPtr = (void *)malloc(sizeof(struct ksinStu) + CMD_HEADLENTH);
    if (cmdPtr == 0){
        return 1002;
    }
    ksin_usrPtr = (struct ksinStu *)((long)cmdPtr + CMD_HEADLENTH);
    qDebug() << " cmd " << cmdPtr <<"ksin_usrPtr " << ksin_usrPtr;
    float div = 0.0001;
    for (i = 0;i<10000;i++){
        float value = atan(div * (float)i);
        ksin_usrPtr->arctan[i] = value;
    }
    div = PI / 2.0 / 20000.0;
    ksin_usrPtr->scdiv = div;
    for (i = 0;i<20000;i++){
        float value = sin(div * (float)i);
        ksin_usrPtr->sin[i] = value;
    }
    for (i = 0;i<20000;i++){
        float value = cos(div * (float)i);
        ksin_usrPtr->cos[i] = value;
    }
    ksin_usrPtr->init = 1;
    char *cmd = (char *)cmdPtr;
    cmd[0] = 100;
    int ret = write(cmdfd,(void *)cmdPtr,sizeof(struct ksinStu) + CMD_HEADLENTH);
    free(cmdPtr);
    ksinDone = true;
    return ret;
}
/**********************************************************************************************
 * last Change : 2022-5-9
 * $vision : 0.1$
 * $Author : Tom Chang $
 * $kcmd mmap init
 * ********************************************************************************************/
int claserUserDev::kcmdInit(void)
{
    cmd_file_stu *temp;
    cmdStu = &nullcmdStu;
    temp = static_cast<cmd_file_stu *>(mmap(NULL,CMD_MMAP_SIZE,PROT_READ|PROT_WRITE,MAP_SHARED,cmdfd,0));
    if ((ulong)temp == 0xffffffffffffffff){
        cmdStu = &nullcmdStu;
        return -1;
    }else{
        cmdStu = temp;
    }
    for(int i = 0;i < MAX_LASER;i++){
        std::stringstream path;
        path << "/dev/" << LS_PARA_FILE << i;

        lsfd[i] = open(path.str().c_str(), O_RDWR);
        qDebug() << QString::fromStdString(path.str()) << "lsfd-"<< i << "="<< lsfd[i];
        if (lsfd[i] < 0){
            clearls();
            return -2;
        }
        char *lstemp;
        lstemp = (char*)mmap(NULL,CMD_MMAP_SIZE,PROT_READ|PROT_WRITE,MAP_SHARED,lsfd[i],0);
        if ((ulong)lstemp == 0xffffffffffffffff){
            clearls();
            return -3;
        }else{
            lsStu[i]->input = (laserInPut*)(lstemp);
            lsStu[i]->output = (laserOutPut*)(lstemp+2*4096);
            //qDebug()<<(ulong)lstemp;
        }
    }
    eStop();
    delConnect();

    return 0;
}
/**********************************************************************************************
 * last Change : 2022-4-25
 * $vision : 0.1$
 * $Author : Tom Chang $
 * $
 * ********************************************************************************************/
int claserUserDev::clearls()
{
    for(int i = 0;i < MAX_LASER;i++){
        if (lsfd[i] >0 ){
            close(lsfd[i]);
        }
        lsStu[i]->input = nullls.input;
        lsStu[i]->output = nullls.output;
    }
}
/**********************************************************************************************
 * last Change : 2022-5-9
 * $vision : 0.1$
 * $Author : Tom Chang $
 * $change new a kenrel laser dev
 * ********************************************************************************************/
int claserUserDev::newLaser(void)
{
    if (!isInitDone()){
        return -1;
    }

    return 0;
}
/**********************************************************************************************
 * last Change : 2022-4-25
 * $vision : 0.1$
 * $Author : Tom Chang $
 * $laser is runing
 * ********************************************************************************************/
bool claserUserDev::isRuning()
{
    bool runing;
    for(int i = 0;i < MAX_LASER;i++){
        if (lsStu[i]->output->output_config.lsState == 0 || lsStu[i]->output->output_config.lsState == 20000){
            //return
        }else{
            return true;
        }
    }
    return false;
}
/**********************************************************************************************
 * last Change : 2022-5-9
 * $vision : 0.1$
 * $Author : Tom Chang $
 * $change return init dong dor not
 * ********************************************************************************************/
bool claserUserDev::isInitDone(void)
{
    return initDone && ksinDone;
}
/**********************************************************************************************
 * last Change : 2022-4-25
 * $vision : 0.1$
 * $Author : Tom Chang $
 * $Init PCIE card and write sin and cos function to kernel dev
 * $return : 0 : sucessed
 * $         others : error
 * ********************************************************************************************/
 std::string claserUserDev::err2Str(int err)
 {
     switch (err) {
        case 0:     return "无错误";
        case 1001:    return "初始化错误：打开ksin驱动失败";
        case 1002:    return "初始化错误：ksin内存失败";
        case 1003:    return "初始化错误：ksin写入失败";
        case 1004:    return "初始化错误：打开data驱动失败";
        default: return "无此错误码";
     }
 }
 /**********************************************************************************************
  * last Change : 2022-5-9
  * $vision : 0.1$
  * $Author : Tom Chang $
  * $change config kenrel by confstu
  * ********************************************************************************************/
 int claserUserDev::reConfig(struct hardwareConf conf)
 {
     void *cmdPtr;
     char *cmd;
     cmdPtr = (void *)malloc(sizeof(struct hardwareConf) + CMD_HEADLENTH);
     memcpy((void *)((unsigned long)cmdPtr + CMD_HEADLENTH),&conf,sizeof(struct hardwareConf));
     cmd = (char *)cmdPtr;
     cmd[0] = 101;
     int ret = write(cmdfd,(void *)cmdPtr,sizeof(struct hardwareConf) + CMD_HEADLENTH);
     return ret;
 }
 /**********************************************************************************************
  * last Change : 2022-4-25
  * $vision : 0.1$
  * $Author : Tom Chang $
  * $new a pthread by itimerspec and return a Vector to insert tasks
  * ********************************************************************************************/
int claserUserDev::newPthread(std::string name,unsigned int cyctime_ns)
 {
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(1,&mask);
    int tret;
    //
    //tret = pthread_setaffinity_np(pthread_self(),sizeof(mask),&mask);
    //struct sched_param param;
    //param.sched_priority = 99;

    //pthread_setschedparam(pthread_self(),SCHED_FIFO,&param);
    userpthread newpth;
    pthvector.push_back(newpth);
    userpthread *temp;
    temp = &pthvector[pthvector.size() - 1];
    qDebug() << "pthvector.size() = "  <<pthvector.size();
    temp->mtx = new std::mutex();
    pthmtx.lock();
    temp->name = name;
    memcpy((void *)temp->taskinfo.name,(void *)name.c_str(),sizeof(temp->name));
    temp->cyctime_ns = cyctime_ns;
    if (cyctime_ns > 1000000000){
        temp->it.it_interval.tv_sec = cyctime_ns / 1000000000;
        temp->it.it_interval.tv_nsec = cyctime_ns % 1000000000;
    }else{
        temp->it.it_interval.tv_sec = 0;
        temp->it.it_interval.tv_nsec = cyctime_ns;
    }
    temp->it.it_value = temp->it.it_interval;

    int ret = 0;

    temp->timerfd = timerfd_create(CLOCK_MONOTONIC,0);
    if(temp->timerfd < 0){
        qDebug() << "timerfd_create error";
    }
    qDebug() << "temp->timerfd" << temp->timerfd;
    clock_gettime(CLOCK_MONOTONIC, &temp->it.it_value);
    timespec_add_us(&temp->it.it_value,cyctime_ns);
    memset(&temp->it.it_interval,0,sizeof(temp->it.it_interval));

    ret = timerfd_settime(temp->timerfd,TFD_TIMER_ABSTIME, &temp->it, NULL);
    if(ret != 0){
         qDebug() << "timerfd_settime error";
    }
    struct epoll_event event;

    memset(&event, 0, sizeof(event));
    event.data.fd = temp->timerfd;
    event.data.ptr = (void *)pthvector.size() - 1;
    event.events = EPOLLIN | EPOLLET;
    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, temp->timerfd, &event);
    if(ret < 0) {
        qDebug() << "epoll_ctl error";
    }
    initktask(&temp->taskinfo);

    qDebug() << ret;
    pthmtx.unlock();
    qDebug() << "new done" ;
    qDebug() << "pthvector.size" << pthvector.size();
    return pthvector.size();
 }
/**********************************************************************************************
 * last Change : 2022-4-25
 * $vision : 0.1$
 * $Author : Tom Chang $
 * $100ms heart task
 * ********************************************************************************************/
void *claserUserDev::_100ms_task(void *arg)
{
    claserUserDev *self = (claserUserDev *)arg;
    self->cmdStu->heartCount_U++;
    if (self->cmdStu->heartCount_K == self->cmdStu->heartCountOld_K){
        self->cmdStu->heartEqua_K++;
        if (self->cmdStu->heartEqua_K >= self->cmdStu->heartMax_K){
            self->cmdStu->heartEqua_K = self->cmdStu->heartMax_K;
            self->cmdStu->heartAlarm_K = 1;
        }
    }else{
        self->cmdStu->heartEqua_K = 0;
        self->cmdStu->heartAlarm_K = 0;
    }
    if(self->ecm != nullptr){
        self->ecm->CRTtools_counter++;
    }
    self->cmdStu->heartCountOld_K = self->cmdStu->heartCount_K;

    return nullptr;
}
/**********************************************************************************************
 * last Change : 2022-4-25
 * $vision : 0.1$
 * $Author : Tom Chang $
 * $100ms heart task
 * ********************************************************************************************/
void *claserUserDev::_1ms_task(void *arg)
{
    claserUserDev *self = (claserUserDev *)arg;
    void *memTest = (void *)self->mapping_mem;
    if (laserDev.cmdStu->runing == 0){
        return nullptr;
    }
    if (memTest == nullptr){
        return nullptr;
    }

    unsigned long *mmap_mem = (unsigned long *)(memTest + 1024 * 1024);
    //unsigned long *output_mem = (void *)input_mem + 5000 * sizeof(unsigned long);
    int adr;
    cBaseVar *var;
    for(int i = 0; i < self->mapping_mem->inputSize;i++){
        adr = self->mapping_mem->inputInfo[i].mappingAdr;
        if (adr < 0 || adr > 5000){
            continue;
        }
        var = self->mapping_mem->inputInfo[i].varAdr;
        if (var == nullptr){
            continue;
        }
        memcpy(&mmap_mem[adr],var->mem,var->size);
        //memcpy(var->mem,&mmap_mem[adr],var->size);
    }
    //qDebug() <<
    for(int i = 0; i < self->mapping_mem->outputSize;i++){
        adr = self->mapping_mem->outputInfo[i].mappingAdr;
        if (adr < 5000 || adr > 10000){
            continue;
        }
        var = self->mapping_mem->outputInfo[adr - 5000].varAdr;
        if (var == nullptr){
            continue;
        }
        ushort value;
        value = *(ushort *)&mmap_mem[adr];
        memcpy(var->mem,&mmap_mem[adr],var->size);
        //memcpy(&mmap_mem[adr],var->mem,var->size);
        //qDebug() << var->fullname() << " ->" << value;
    }
    if(!cBaseVar::connect_run){
        return nullptr;
    }
    if(cBaseVar::connect_fun != nullptr){
        cBaseVar::connect_fun(cBaseVar::connect_arg);
    }
    if(cBaseVar::connect_run){
        bool try_lock = cBaseVar::connector_mtx.try_lock();
        if(try_lock){
            QHash<QString,cBaseVar*>::iterator it;
            for(it = cBaseVar::connector.begin();it != cBaseVar::connector.end() ; it++){
                updateParam(cBaseVar::connector[it.key()]);
            }
            cBaseVar::connector_mtx.unlock();
        }
    }
    if(cBaseVar::connect_fun_tail != nullptr){
        cBaseVar::connect_fun_tail(cBaseVar::connect_arg_tail);
    }
    //qDebug() << "memcpy !";
    return nullptr;
}
 /**********************************************************************************************
  * last Change : 2022-4-25
  * $vision : 0.1$
  * $Author : Tom Chang $
  * $static pthread func
  * ********************************************************************************************/
void claserUserDev::pthread_fn(void *arg)
{
    //qDebug() << "pthread";
    userpthread *ptharg;
    ptharg = (userpthread *)arg;

    //qDebug() << "pthread"  << QString::fromStdString(ptharg->name);
    ptharg->time_in_last = ptharg->time_in;
    clock_gettime(CLOCK_MONOTONIC, &ptharg->time_in);
    if (ptharg->count == 0){
        cpu_set_t mask;
        CPU_ZERO(&mask);
        CPU_SET(1,&mask);
        int ret;

        ret = pthread_setaffinity_np(pthread_self(),sizeof(mask),&mask);
        struct sched_param param;
        param.sched_priority = 98;

        //ret = pthread_setschedparam(pthread_self(),SCHED_FIFO,&param);
    }
    pthread_t self_t = pthread_self();
    struct sched_param paa;
    int po;
    pthread_getschedparam(self_t,&po,&paa);
    ptharg->count++;

    ptharg->mtx->lock();

    for(int i = 0;i < ptharg->tasklist.size();i++){
        ptharg->tasklist[i].fun(ptharg->tasklist[i].arg);
    }


    ptharg->mtx->unlock();

    clock_gettime(CLOCK_MONOTONIC, &ptharg->time_out);
    ptharg->taskinfo.task_countor++;
    ptharg->taskinfo.task_cost = diff_tv(ptharg->time_in,ptharg->time_out);
    ptharg->taskinfo.task_period = diff_tv(ptharg->time_in_last,ptharg->time_in);
    ptharg->taskinfo.task_cost_sum = ptharg->taskinfo.task_cost_sum + ptharg->taskinfo.task_cost;
    if (ptharg->taskinfo.task_cost > ptharg->taskinfo.task_cost_max){
        ptharg->taskinfo.task_cost_max = ptharg->taskinfo.task_cost;
    }
    if (ptharg->taskinfo.task_cost < ptharg->taskinfo.task_cost_min){
        ptharg->taskinfo.task_cost_min = ptharg->taskinfo.task_cost;
    }
    ptharg->taskinfo.task_cost_avg = ptharg->taskinfo.task_cost_sum / ptharg->taskinfo.task_countor;

    ptharg->taskinfo.task_period_sum = ptharg->taskinfo.task_period_sum +  ptharg->taskinfo.task_period;
    if (ptharg->taskinfo.task_period > ptharg->taskinfo.task_period_max){
        ptharg->taskinfo.task_period_max = ptharg->taskinfo.task_period;
    }
    if (ptharg->taskinfo.task_period < ptharg->taskinfo.task_period_min){
        ptharg->taskinfo.task_period_min = ptharg->taskinfo.task_period;
    }
    ptharg->taskinfo.task_period_avg = ptharg->taskinfo.task_period_sum / ptharg->taskinfo.task_countor;

    if (ptharg->taskinfo.clear == 1){
        initktask(&ptharg->taskinfo);
        ptharg->taskinfo.clear = 0;
    }
    if (ptharg->count >= 10001){
        ptharg->count = 1;
    }
    ptharg->taskinfo.cpu = sched_getcpu();
 }
/**********************************************************************************************
 * last Change : 2022-4-25
 * $vision : 0.1$
 * $Author : Tom Chang $
 * $push a task into phread
 * ********************************************************************************************/
int claserUserDev::push_task(int pthid ,  void *(*fun)(void* arg),void *arg,std::string name)
{
    if (pthid > pthvector.size()){
        return -1;
    }
    qDebug() << "***************task***************";
    userpthread *ptharg = &pthvector[pthid];
    ptharg->mtx->lock();
    usertask temp;
    temp.fun = fun;
    temp.arg = arg;
    temp.name = name;
    ptharg->tasklist.push_back(temp);
    ptharg->mtx->unlock();
    qDebug() << "ptharg->tasklist.size" <<ptharg->tasklist.size();
    qDebug() << _POSIX_MONOTONIC_CLOCK;
    return 0;
}
/**********************************************************************************************
 * last Change : 2022-4-25
 * $vision : 0.1$
 * $Author : Tom Chang $
 * $
 * ********************************************************************************************/
int claserUserDev::delConnect()
{
    for(int i = 0; i < MAX_LASER ; i++){
        lsStu[i]->input->input_config.iEncNo = -1;
        lsStu[i]->input->input_config.iCH368No = -1;
    }
    for(int i = 0; i < MAX_ENCODER ; i++){
       cmdStu->encoder[i].connectNo = -1;
       cmdStu->encoder[i].enable = 0;
    }
    for(int i = 0; i < MAX_CARD ; i++){
       cmdStu->ch368[i].bind = 0;
       cmdStu->ch368[i].encRead = 0;
    }
    cmdStu->confDone = 0;
    cmdStu->runing = 0;
    return 0;
}
/**********************************************************************************************
 * last Change : 2022-4-25
 * $vision : 0.1$
 * $Author : Tom Chang $
 * $reconnect
 * ********************************************************************************************/
int claserUserDev::reConnect()
{
    std::string info;
    int ret;
    for(int i = 0; i < MAX_CARD ; i++){
       cmdStu->ch368[i].bind = 0;
       cmdStu->ch368[i].encRead = 0;
    }
    for(int i = 0; i < MAX_LASER ; i++){
        if (lsStu[i]->input->input_config.iCH368No >= 0 && lsStu[i]->input->input_config.iCH368No < MAX_CARD){
            if (cmdStu->ch368[lsStu[i]->input->input_config.iCH368No].bind == 0){
                cmdStu->ch368[lsStu[i]->input->input_config.iCH368No].bind++;
            }else{
                info = "板卡多重绑定激光";
                goto errorout;
            }
        }
    }
    for(int i = 0; i < MAX_ENCODER ; i++){
        if (cmdStu->encoder[i].connectNo >= 0 && cmdStu->encoder[i].connectNo < MAX_CARD){
            cmdStu->ch368[cmdStu->encoder[i].connectNo].encRead = 1;
        }
    }
    if(mapping_mem == nullptr){
        info = "映射内存失效";
        goto errorout;
    }
    ret = cBaseVar::mappingAll(mapping_mem);
    if (ret != 0){
        info = "映射失败";
        goto errorout;
    }

    cBaseVar::connect_run = true;
    cmdStu->confDone = 1;
    cmdStu->runing = 1;
    return 0;
errorout:
    //delConnect();
    cmdStu->runing = 0;
    throw info;
    return -1;
}
/**********************************************************************************************
 * last Change : 2022-4-25
 * $vision : 0.1$
 * $Author : Tom Chang $
 * $force close file
 * ********************************************************************************************/
int claserUserDev::eStop()
{
    cmdStu->estop = 1;
    cBaseVar::connect_run = false;
    //nanosleep(1);
    usleep(100 * 1000);
    cmdStu->estop = 0;
    if (isRuning()){
        return -1;
    }else{
        cmdStu->runing = 0;
        return 0;
    }
}
/**********************************************************************************************
 * last Change : 2022-4-25
 * $vision : 0.1$
 * $Author : Tom Chang $
 * $force close file
 * ********************************************************************************************/
int claserUserDev::setEncBindCard(int enc,int card)
{
    std::string info;
    if (!canConfig()){
        info = "运行中，无法配置";
        goto errorout;
    }
    if (enc > MAX_ENCODER || enc < 0){
        info = "编码器号错误";
        goto errorout;
    }
    if (card > MAX_CARD || card < 0){
        info = "板卡号错误";
        goto errorout;
    }
    if (cmdStu->encoder[enc].enable != 1){
        info = "该编码器未使能";
        goto errorout;
    }
    if (cmdStu->ch368[card].enable != 1){
        info = "该板卡未使能";
        goto errorout;
    }
    cmdStu->encoder[enc].connectNo = card;
    return 0;
errorout:
    //throw info;
    return -1;
}
/**********************************************************************************************
 * last Change : 2022-4-25
 * $vision : 0.1$
 * $Author : Tom Chang $
 * $force close file
 * ********************************************************************************************/
int claserUserDev::setlsBindEnc(int ls,int enc)
{
    std::string info;
    if (!canConfig()){
        info = "运行中，无法配置";
        goto errorout;
    }
    if (enc > MAX_ENCODER || enc < 0){
        info = "编码器号错误";
        goto errorout;
    }
    if (ls > MAX_ENCODER || ls < 0){
        info = "激光号错误";
        goto errorout;
    }
    if (cmdStu->encoder[enc].enable != 1){
        info = "该编码器未使能";
        goto errorout;
    }
    lsStu[ls]->input->input_config.iEncNo = enc;
    return 0;
errorout:
    //throw info;
    return -1;
}
/**********************************************************************************************
 * last Change : 2022-4-25
 * $vision : 0.1$
 * $Author : Tom Chang $
 * $force close file
 * ********************************************************************************************/
int claserUserDev::setlsBindCard(int ls,int card)
{
    std::string info;
    if (!canConfig()){
        info = "运行中，无法配置";
        goto errorout;
    }
    if (ls > MAX_ENCODER || ls < 0){
        info = "编码器号错误";
        goto errorout;
    }
    if (card > MAX_CARD || card < 0){
        info = "板卡号错误";
        goto errorout;
    }

    if (cmdStu->ch368[card].enable != 1){
        info = "该板卡未使能";
        goto errorout;
    }
    lsStu[ls]->input->input_config.iCH368No = card;
    return 0;
errorout:
    //throw info;
    return -1;
}
/**********************************************************************************************
 * last Change : 2022-4-25
 * $vision : 0.1$
 * $Author : Tom Chang $
 * $force close file
 * ********************************************************************************************/
bool claserUserDev::canConfig()
{
    return !(isRuning() || cmdStu->runing);
}
/**********************************************************************************************
 * last Change : 2022-4-25
 * $vision : 0.1$
 * $Author : Tom Chang $
 * $force close file
 * ********************************************************************************************/
void *claserUserDev::epool_pth(void *arg)
{

     int i = 0;
     int fd_cnt = 0;
     int sfd;
     struct epoll_event events[EPOLL_LISTEN_CNT];
     claserUserDev *self = (claserUserDev *)arg;
     //userpthread *ptharg = (userpthread *)arg;
     memset(events, 0, sizeof(events));
     qDebug() << "epoll" << self->epfd;

     while(1)
     {
         /* wait epoll event */
         //qDebug() << "wait epoll event"  ;
         fd_cnt = epoll_wait(self->epfd, events, EPOLL_LISTEN_CNT, -1);
         //qDebug() << "epoll"  << fd_cnt << errno;

         for(i = 0; i < fd_cnt; i++)
         {
             sfd = events[i].data.fd;
             if(events[i].events & EPOLLIN)
             {
                 userpthread *ptharg = &self->pthvector[(long)(events[i].data.ptr)];//(userpthread *)events[i].data.ptr;
                 if (sfd == ptharg->timerfd || true)
                 {
                     timespec_add_us(&ptharg->it.it_value,ptharg->cyctime_ns);
                     memset(&ptharg->it.it_interval,0,sizeof(ptharg-> it.it_interval));
                     int ret;
                     ret = timerfd_settime(ptharg->timerfd,TFD_TIMER_ABSTIME, &ptharg->it, NULL);
                     //qDebug() << " ptharg->timerfd " <<ptharg->timerfd << "ret" << ret;
                     if(ret < 0)
                     {
                         qDebug() << "timerfd_settime error";
                     }else{
                         pthread_fn(ptharg);
                     }

                 }
             }
         }
     }
 }
 /**********************************************************************************************
  * last Change : 2022-4-25
  * $vision : 0.1$
  * $Author : Tom Chang $
  * $force close file
  * ********************************************************************************************/
 int claserUserDev::forceClose()
 {
     close(cmdfd);
     close(ETCfd);
     close(epfd);
     for (int i = 0;i < pthvector.size();i++){
         //timer_delete(pthvector[i].id);
         close(pthvector[i].timerfd);
     }
     clearls();
    // close(data0fd);
 }


 

 double multi(point a,point b,point c)
 {
    return (a.x-c.x)*(b.y-c.y)-(b.x-c.x)*(a.y-c.y);
 }
 int across(segment v1,segment v2)
 {
     if(max(v1.start.x,v1.end.x)>=min(v2.start.x,v2.end.x)&&
        max(v2.start.x,v2.end.x)>=min(v1.start.x,v1.end.x)&&
        max(v1.start.y,v1.end.y)>=min(v2.start.y,v2.end.y)&&
        max(v2.start.y,v2.end.y)>=min(v1.start.y,v1.end.y)&&
        multi(v2.start,v1.end,v1.start)*multi(v1.end,v2.end,v1.start)>0&&
        multi(v1.start,v2.end,v2.start)*multi(v2.end,v1.end,v2.start)>0){
        return 1;
     }
        return 0;
 }
int intersection(segment u,segment v, point  *p)
 {
     //point p;

     if(across(u,v))
     {
         p->x=(multi(v.end,u.end,v.start)*v.start.x-multi(v.start,u.end,u.start)*v.end.x)
         /(multi(v.end,u.end,v.start)-multi(v.start,u.end,u.start));

         p->y=(multi(v.end,u.end,v.start)*v.start.y-multi(v.start,u.end,u.start)*v.end.y)
         /(multi(v.end,u.end,v.start)-multi(v.start,u.end,u.start));

         return 1;
     }
     return 0;
 }
segment line_ext(segment line ,double diff)
{
    double len;
    len = sqrt(pow(line.start.x - line.end.x,2.0) + pow(line.start.y - line.end.y,2.0));
    if(len < 0.0001){
        return line;
    }
    segment ret;
    ret.start.x = line.start.x + (line.start.x - line.end.x) / len * diff;
    ret.start.y = line.start.y + (line.start.y - line.end.y) / len * diff;

    ret.end.x = line.end.x + (line.end.x - line.start.x) / len * diff;
    ret.end.y = line.end.y + (line.end.y - line.start.y) / len * diff;
    return ret;
}
int intersection2(segment u0,segment v0, point  *p,double diff)
{
    segment u;segment v;
    u = line_ext(u0,diff);
    v = line_ext(v0,diff);
    if(across(u,v))
    {
        point u1;point u2;point v1;point v2;

        u1 = u.start;u2 = u.end;
        v1 = v.start;v2 = v.end;
        point ret=u1;
        double t=((u1.x-v1.x)*(v1.y-v2.y)-(u1.y-v1.y)*(v1.x-v2.x))
        /((u1.x-u2.x)*(v1.y-v2.y)-(u1.y-u2.y)*(v1.x-v2.x));
        ret.x+=(u2.x-u1.x)*t;
        ret.y+=(u2.y-u1.y)*t;
        p->x = ret.x;
        p->y = ret.y;

        return 1;
    }
    return 0;
}
 segment line_to_segment(bCut_Line line)
 {
     segment v;
     v.start.x = line.param[1];
     v.start.y = line.param[2];
     v.end.x = line.param[3];
     v.end.y = line.param[4];
     return v;
 }
 int bCut_across(bCut_Line line1,bCut_Line line2)
 {
     segment v1,v2;
     v1 = line_to_segment(line1);
     v2 = line_to_segment(line2);
     return across(v1,v2);
 }
 int bCut_intersection(bCut_Line line1,bCut_Line line2, bCut_Point  *pnt ,double diff)
 {
     segment v1,v2;
     v1 = line_to_segment(line1);
     v2 = line_to_segment(line2);
     point p;
     int ref = intersection2(v1,v2,&p,diff);
     if(ref == 1){
         pnt->x = p.x;
         pnt->y = p.y;
         return 1;
     }else{
         return 0;
     }
 }

bool bCut_is_point(bCut_Point pnt1,bCut_Point pnt2,double diff)
{

}

bool bCut_is_one_line(bCut_Line line1,bCut_Line line2,double diff)
{
    segment v1,v2;
    v1 = line_to_segment(line1);
    v2 = line_to_segment(line2);
    bool isone = true;
    if(abs(v1.start.x - v2.start.x) > diff){
        isone = false;
    }
    if(abs(v1.start.y - v2.start.y) > diff){
        isone = false;
    }
    if(abs(v1.end.x - v2.end.x) > diff){
        isone = false;
    }
    if(abs(v1.end.y - v2.end.y) > diff){
        isone = false;
    }

    if(isone == false){
        isone = true;
        if(abs(v1.start.x - v2.end.x) > diff){
            isone = false;
        }
        if(abs(v1.start.y - v2.end.y) > diff){
            isone = false;
        }
        if(abs(v1.end.x - v2.start.x) > diff){
            isone = false;
        }
        if(abs(v1.end.y - v2.start.y) > diff){
            isone = false;
        }
    }
    return isone;
}
bool bCut_is_one_point(bCut_Point pnt1,bCut_Point pnt2,double diff)
{
    if(abs(pnt1.x - pnt2.x) > diff){
        return false;
    }
    if(abs(pnt1.y - pnt2.y) > diff){
        return false;
    }
    return true;
}
double pnt_to_pnt(bCut_Point pnt1,bCut_Point pnt2)
{
    double dd,d;
    dd = (pnt1.x - pnt2.x) * (pnt1.x - pnt2.x) + (pnt1.y - pnt2.y) * (pnt1.y - pnt2.y);
    d = sqrt(dd);
    return d;
}
bool bCut_point_in_line(bCut_Point pnt,bCut_Line line,double diff)
{
    double a_pnt,b_pnt,a_b;
    bCut_Point a,b;
    a.x = line.param[1];
    a.y = line.param[2];
    b.x = line.param[3];
    b.y = line.param[4];
    a_pnt = pnt_to_pnt(a , pnt);
    b_pnt = pnt_to_pnt(b , pnt);
    a_b = pnt_to_pnt(a , b);


    if(abs(a_b - a_pnt - b_pnt) < diff){
        return true;
    }else{
        return false;
    }
}



