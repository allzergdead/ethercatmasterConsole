#include "widStu.h"

QHash<QString,QHash<QString,cBaseVar>> qMemHash;
//QHash<QString,cBaseVar> cBaseHash;
QMutex cBaseMtx;

QList<cLogInfo> *infoList;
QMutex cInfoMtx;


QSqlDatabase cBaseVar::db;
QMutex cBaseVar::dbMtx;
QMutex cBaseVar::tostrMtx;
QString cBaseVar::db_lastError;
#ifdef GUIMODE
cbase_signals *cBaseVar::base_signals = nullptr;
#endif
QHash<QString , cBaseVar *> cBaseVar::connector;
QMutex cBaseVar::connector_mtx;
QVector<cBaseVar> cBaseVar::vartypes;
sysErrorList cBaseVar::errorList;



bool cBaseVar::connect_check = true;//深检查
bool cBaseVar::connect_run = false;//深检查
void* (*cBaseVar::connect_fun)(void *arg) = nullptr;
void *cBaseVar::connect_arg = nullptr;
void* (*cBaseVar::connect_fun_tail)(void *arg) = nullptr;
void *cBaseVar::connect_arg_tail = nullptr;
int gvlLevel = 1000;
int protectLevel = 100;
QString cBaseVar::lastDB = "";


QString btn_grey = "background: rgb(150,150,150);\n  border: 0px solid #333333;\n   ";
QString btn_red = "background: rgb(250,50,50);\n  border: 0px solid #333333;\n   ";
QString btn_green = "background: rgb(50,250,50);\n  border: 0px solid #333333;\n   ";
QString btn_blue = "background: rgb(50,50,250);\n  border: 0px solid #333333;\n   ";
QString btn_darkyellow = "background: rgb(200,200,0);\n  border: 0px solid #333333;\n   ";
QString btn_yellow = "background: rgb(250,250,10);\n  border: 0px solid #333333;\n   ";


long long diff_tv(struct timespec start, struct timespec end) {
    return (end.tv_sec-start.tv_sec)*1000000000+(end.tv_nsec-start.tv_nsec);
}
void timespec_add_us(struct timespec *it , long _ns)
{
    it->tv_sec = it->tv_sec + (_ns / 1000000000);
    it->tv_nsec = it->tv_nsec + (_ns % 1000000000);
    if (it->tv_nsec >= 1000000000){
        it->tv_sec = it->tv_sec + 1;
        it->tv_nsec = it->tv_nsec % 1000000000;
    }
}
void initktask(struct kenrel_TaskInfo *stask)
{
    stask->task_countor = 0;
    //unsigned long task_cost_target;
    stask->task_cost = 0;
    stask->task_cost_avg = 0;
    stask->task_cost_max = 0;
    stask->task_cost_min = 100000000;
    stask->task_cost_sum = 0;
    //stask->task_period = interval;
    stask->task_period_avg = 0;
    stask->task_period_max = 0;
    stask->task_period_min = 100000000;
    stask->task_period_sum = 0;
    stask->task_outof_sync = 0;
    stask->task_interrupt_delay = 0;
    stask->clear = 0;
}

QString getStr(QWidget *self,QString title,QString notice)
{
    #ifdef GUIMODE
    bool ok;
    QString text = QInputDialog::getText(self, title,notice, QLineEdit::Normal,"", &ok);
    if (ok && !text.isEmpty())
    {
        return text;
    }
    if (!ok){
        return "";
    }else if(text.isEmpty()){
        return "";
    }
    return "";
    #else
    return "";
    #endif
}
void cBaseVar::initSelf(QString sname,cTYPE stype,cDIR sdir,int slevel,int ssize)
{
    #ifdef GUIMODE
    name = sname;
    type = stype;
    dir = sdir;
    level = slevel;
    size = ssize;
    mem = nullptr;
    real_mem = nullptr;
    if(base_signals == nullptr){
        base_signals = new cbase_signals();
    }
    #else
    return;
    #endif
}
cBaseVar::cBaseVar()
{
    return;
}
cBaseVar::cBaseVar(QString sname,cTYPE stype,cDIR sdir,int slevel)
{
    int ssize = 0;
    if (stype == C_INT || stype == C_UINT ||stype == C_FLOAT){
        ssize = 4;
    }else if(stype == C_LONG || stype == C_ULONG ||stype == C_DOUBLE){
        ssize = 8;
    }else if(stype == C_BOOL || stype == C_CHAR){
        ssize = 1;
    }else if(stype == C_SHORT || stype == C_USHORT){
        ssize = 2;
    }else{
        ssize = 0;
    }
    initSelf(sname,stype,sdir,slevel,ssize);
}
int cBaseVar::varChange(void *var)
{
    if (var == nullptr){
        throw father + "." + name + u8"空指针赋值错误";
        return -1;
    }else{
        mem = var;
        real_mem = var;
        connect = false;
        return 0;
    }
}
cBaseVar::cBaseVar(QString sname,cTYPE stype,cDIR sdir,int slevel,int ssize)
{
    if (stype != C_BYTEARY){
        mem = nullptr;
        return;
    }
    initSelf(sname,stype,sdir,slevel,ssize);
    return;
}
cBaseVar &cBaseVar::operator= (const cBaseVar& x)
{
    //memcpy(this,&x,sizeof(cBaseVar));
    this->name = x.name;
    this->father = x.father;
    this->dir = x.dir;
    this->mem = x.mem;
    this->real_mem = x.mem;
    this->bind = x.bind;
    this->size = x.size;
    this->level = x.level;
    this->type = x.type;
    this->connect = false;
    return *this;
}
int cBaseVar::emitSignal(QStringList info)
{
    #ifdef GUIMODE
    if(cBaseVar::base_signals == nullptr){
        insertLog(-1,"变量管理器","信号中心初始化失败");
        return -1;
    }
    cBaseVar::base_signals->input_signals(info);
    #else
    insertLog(-1,"变量管理器","非GUI模式，不支持此功能");
    return -1;
    #endif
}
bool cBaseVar::isNull(void)
{
    if (mem == nullptr){
        return true;
    }else{
        return false;
    }
}
bool cBaseVar::isINT(void)
{
    if (type == C_INT && !isNull()){
        return true;
    }else{
        return false;
    }
}
bool cBaseVar::isUINT(void)
{
    if (type == C_UINT && !isNull()){
        return true;
    }else{
        return false;
    }
}
bool cBaseVar::isLONG(void)
{
    if (type == C_LONG && !isNull()){
        return true;
    }else{
        return false;
    }
}
bool cBaseVar::isULONG(void)
{
    if (type == C_ULONG && !isNull()){
        return true;
    }else{
        return false;
    }
}
bool cBaseVar::isFLOAT(void)
{
    if (type == C_FLOAT && !isNull()){
        return true;
    }else{
        return false;
    }
}
bool cBaseVar::isDOUBLE(void)
{
    if (type == C_DOUBLE && !isNull()){
        return true;
    }else{
        return false;
    }
}
bool cBaseVar::isBOOL(void)
{

    if (type == C_BOOL && !isNull()){
        return true;
    }else{
        return false;
    }
}
bool cBaseVar::isBYTEARY(void)
{
    if (type == C_BYTEARY && !isNull()){
        return true;
    }else{
        return false;
    }
}
bool cBaseVar::isCHAR(void)
{
    if (type == C_CHAR && !isNull()){
        return true;
    }else{
        return false;
    }
}
bool cBaseVar::isSHORT(void)
{
    if (type == C_SHORT && !isNull()){
        return true;
    }else{
        return false;
    }
}
bool cBaseVar::isUSHORT(void)
{
    if (type == C_USHORT && !isNull()){
        return true;
    }else{
        return false;
    }
}
bool cBaseVar::isQSTRING(void)
{
    if (type == C_QSTRING && !isNull()) {
        return true;
    }
    else {
        return false;
    }
}

bool  cBaseVar::isCVMAT(void)
{
    if (type == C_CVMAT && !isNull()) {
        return true;
    }
    else {
        return false;
    }
}
bool  cBaseVar::isHOBJ(void)
{
    if (type == C_HOBJ && !isNull()) {
        return true;
    }
    else {
        return false;
    }
}
bool  cBaseVar::isFUNC(void)
{
    if (type == C_FUNC && !isNull()) {
        return true;
    }
    else {
        return false;
    }
}
bool  cBaseVar::isAXIS(void)
{
    if (type == C_AXIS && !isNull()) {
        return true;
    }
    else {
        return false;
    }
}

bool cBaseVar::isACTION(void)
{
    #ifdef GUIMODE
    if (type == C_ACTION && !isNull()) {
        return true;
    }
    else {
        return false;
    }
    #else
    return false;
    #endif
}

bool cBaseVar::isVALUE(void)
{
    if ( type <= C_USHORT && type > C_UNTYPE && !isNull()) {
        return true;
    }else {
        return false;
    }
}
void cBaseVar::setFather(QString sfather)
{
    father = sfather;
}
QString cBaseVar::toString(void)
{
    if (isNull()){
        return "NULL_VAR";
    }
    if (isINT()){
        return QString::number(*(int *)mem,showBase);
    }else if(isUINT()){
        return QString::number(*(unsigned int *)mem,showBase);
    }else if(isLONG()){
        return QString::number(*(long *)mem,showBase);
    }else if(isULONG()){
        return QString::number(*(unsigned long *)mem,showBase);
    }else if(isFLOAT()){
        return QString::number(*(float *)mem,'f',accur);
    }else if(isDOUBLE()){
        return QString::number(*(double *)mem,'f',accur);
    }else if(isBOOL()){
        if (*(bool *)mem){
            return "1";
        }else{
            return "0";
        }
    }else if(isCHAR()){
        return QString::number(*(char *)mem,showBase);
    }else if(isBYTEARY()){
        return "BYTE_ARY : " + name;
    }else if(isSHORT()){
        return QString::number(*(short *)mem,showBase);
    }else if(isUSHORT()){
        return QString::number(*(unsigned short *)mem,showBase);
    }else if (isQSTRING()){
        return *(QString *)mem;
    }else if (isCVMAT()){
        return fullname();
    }else if (isHOBJ()) {
        return fullname();
    }else if (isFUNC()) {
        return fullname();
    }else if(isAXIS()){
        axis_ref *axis = (axis_ref *)mem;
        std::string axstr = axis->name;
        QString axName = QString::fromStdString(axstr);
        return axName;
    }else if (isACTION()) {
        return fullname();
    }

    else{
        return "UNDEFINED_TYPE";
    }
}
QString cBaseVar::fullname(void)
{
    return father + "." + name;
}
int cBaseVar::setByString(QString str)
{
    if (isNull()){
        return -1;
    }

    bool ok = false;
    if (isINT()){
        int temp;
        temp = str.toInt(&ok);

        if (ok){
            oldValue =  toString();
            *(int *)mem = temp;
        }
    }else if(isUINT()){
        unsigned int temp;
        temp = str.toUInt(&ok);
        if (ok){
            oldValue =  toString();
            *(unsigned int *)mem = temp;
        }
    }else if(isLONG()){
        long temp;
        temp = str.toLong(&ok);
        if (ok){
            oldValue =  toString();
            *(long *)mem = temp;
        }
    }else if(isULONG()){
        unsigned long temp;
        temp = str.toULong(&ok);
        if (ok){
            oldValue =  toString();
            *(unsigned long *)mem = temp;
        }
    }else if(isFLOAT()){
        float temp;
        temp = str.toFloat(&ok);
        if (ok){
            oldValue =  toString();
            *(float *)mem = temp;
        }
    }else if(isDOUBLE()){
        double temp;
        temp = str.toDouble(&ok);
        if (ok){
            oldValue =  toString();
            *(double *)mem = temp;
        }
    }else if(isBOOL()){
        int temp;
        temp = str.toInt(&ok);
        if (ok){
            if(temp == 0){
                oldValue =  toString();
                *(bool *)mem = false;
            }else{
                oldValue =  toString();
                *(bool *)mem = true;
            }
        }
    }else if(isBYTEARY()){
        return -3;
    }else if(isCHAR()){
        int temp;
        temp = str.toUInt(&ok);
        char ctemp;
        if (temp > 255 || temp < 0){
            return -2;
        }
        ctemp = temp;
        if (ok){
            oldValue =  toString();
            *(char *)mem = ctemp;
        }
    }else if(isSHORT()){
        short temp;
        temp = str.toShort(&ok);
        if (ok){
            oldValue =  toString();
            *(short *)mem = temp;
        }
    }else if(isUSHORT()){
        unsigned short temp;
        temp = str.toShort(&ok);
        if (ok){
            oldValue =  toString();
            *(unsigned short *)mem = temp;
        }
    }else if (isQSTRING()) {
        oldValue =  toString();
        *((QString *)mem) = str;
    }else{
        return -4;
    }
    if (ok){
        return 0;
    }else{
        return -5;
    }
}
QString cBaseVar::dirToStr(void)
{
    return dirStr(this);
}
QString cBaseVar::typeToStr(void)
{
    //return "";
    return typeStr(this);
}

int cBaseVar::checkMapping(int adr)
{
    if (adr < 0){
        return 0;
    }
    if (adr > 10000){
        insertLog(-1, u8"变量管理器",QString::number(adr) + u8" 超出映射范围 ");
        return -2;
    }
    int times = 0;
    GROUP_HASH::iterator it;
    for(it = qMemHash.begin();it != qMemHash.end();it++){
        VAR_HASH* group = &qMemHash[it.key()];
        VAR_HASH::iterator varit;
        for(varit = group->begin();varit != group->end();varit++){
            cBaseVar *var = &(*group)[varit.key()];
            if (var->mapping == adr){
                insertLog(0, u8"变量管理器",var->fullname() + u8"占用映射 ");
                times--;
                //return -1;
            }
        }
    }
    return times;
}
int cBaseVar::setMapping(int adr)
{
    if(cBaseVar::checkMapping(adr) != 0){
        return -1;
    }
    mapping = adr;
    insertLog(1, u8"变量管理器",fullname() + u8" 映射至 " + QString::number(adr) + u8"成功");
    return 0;
}
int cBaseVar::setQString()
{
    #ifdef GUIMODE
    if (isNull()) {
        return -1;
    }
    if (QMessageBox::Yes == QMessageBox::information(nullptr, fullname(), u8"是否查找文件路径", QMessageBox::Yes|QMessageBox::No)){
        QFileDialog dialog;
        QString path = dialog.getOpenFileName();
        if (path == "") {
            return 0;
        }
        setByString(path);
        return 0;
    }
    QString str = getStr(nullptr, fullname(), toString());
    if (str != "") {
        setByString(str);
    }
    return 0;
    #else
    return -1;
    #endif
}
int cBaseVar::mappingAll(mappingInfo_sub *mem)
{
    mem->inputSize = 0;
    mem->outputSize = 0;
    GROUP_HASH::iterator it;
    for(it = qMemHash.begin();it != qMemHash.end();it++){
        VAR_HASH* group = &qMemHash[it.key()];
        VAR_HASH::iterator varit;
        for(varit = group->begin();varit != group->end();varit++){
            cBaseVar *var = &(*group)[varit.key()];
            int mappRet = cBaseVar::checkMapping(var->mapping);
            if (mappRet != -1){
                if (mappRet == 0){
                    continue;
                }
                insertLog(-1, u8"变量管理器",var->fullname() + u8" 映射表错误! ");
                return -1;
            }else{
                if (var->mapping < 5000){
                    memcpy(mem->inputInfo[mem->inputSize].name,var->name.toLocal8Bit().data(),64);
                    mem->inputInfo[mem->inputSize].varAdr = var;
                    mem->inputInfo[mem->inputSize].mappingAdr = var->mapping;
                    mem->inputInfo[mem->inputSize].dir = 1;
                    mem->inputSize++;
                    if(mem->inputSize >= MAX_INPUT_MAAPING){
                        insertLog(-1, u8"变量管理器", " 输入映射表超出长度 ");
                        return -1;
                    }
                }else{
                    memcpy(mem->outputInfo[mem->outputSize].name,var->name.toLocal8Bit().data(),64);
                    mem->outputInfo[mem->outputSize].varAdr = var;
                    mem->outputInfo[mem->outputSize].mappingAdr = var->mapping;
                    mem->outputInfo[mem->outputSize].dir = 0;
                    mem->outputSize++;
                    if(mem->outputSize >= MAX_OUTPUT_MAAPING){
                        insertLog(-1, u8"变量管理器", " 输出映射表超出长度 ");
                        return -1;
                    }
                }
            }
        }
    }
    return 0;
}

QString  cBaseVar::typeStr(cBaseVar *var)
{
    if (var->isNull()){
        return "NULL_VAR";
    }
    if (var->isINT()){
        return "INT";
    }else if(var->isUINT()){
        return "UINT";
    }else if(var->isLONG()){
        return "LONG";
    }else if(var->isULONG()){
        return "ULONG";
    }else if(var->isFLOAT()){
        return "FLOAT";
    }else if(var->isDOUBLE()){
        return "DOUBLE";
    }else if(var->isBOOL()){
        return "BOOL";
    }else if(var->isBYTEARY()){
        return "BYTE_ARY";
    }else if(var->isCHAR()){
        return "CHAR";
    }else if(var->isSHORT()){
        return "SHORT";
    }else if(var->isUSHORT()){
        return "USHORT";
    }else if (var->isQSTRING()) {
        return "QSTRING";
    }else if (var->isCVMAT()) {
        return "CVMAT";
    }else if (var->isHOBJ()) {
        return "HOBJ";
    }else if (var->isFUNC()) {
        return "FUNC";
    }else if (var->isAXIS()){
        return "AXIS";
    }else if (var->isACTION()){
        return "ACTION";
    }
    else {
        return "UNDEFINED_TYPE";
    }
}
QString  cBaseVar::dirStr(cBaseVar *var)
{
    if (var->isNull()){
        return "NULL_VAR";
    }
    if (var->dir == C_INPUT){
        return "INPUT";
    }else if(var->dir == C_OUTPUT){
        return "OUTPUT";
    }else if(var->dir == C_PARAM){
        return "PARAM";
    }else{
        return "UNDEFINED_DIR";
    }
}

int cBaseVar::insertValueChange()
{
    QJsonDocument jsonDoc;
    jsonDoc = QJsonDocument::fromJson(jsonValue.toLatin1());
    //jsonDoc.fromJson(jsonValue.toLatin1());
    QJsonObject jsonObj = jsonDoc.object();
    bool ref = jsonObj.contains("valueChange");
    QJsonArray jsonAry;
    if(ref){
        //QJsonArray jsonAry;
        jsonAry = jsonObj["valueChange"].toArray();
        if(jsonAry.size() > 20){
            jsonAry.erase(jsonAry.begin());
        }
        QDateTime current_time = QDateTime::currentDateTime();
        QString time = current_time.toString("yyyy-MM-dd hh:mm:ss.zzz");
        jsonAry.append(time + "---" + oldValue + "->" + toString());
    }else{
        //QJsonArray jsonAry;
        QDateTime current_time = QDateTime::currentDateTime();
        QString time = current_time.toString("yyyy-MM-dd hh:mm:ss.zzz");
        jsonAry.append(time + "---" + oldValue + "->" + toString());
    }
    jsonObj["valueChange"] = jsonAry;
    //QJsonDocument jsonDoc0(jsonObj);
    jsonDoc.setObject(jsonObj);
    jsonValue = jsonDoc.toJson();
}

int cBaseVar::set_connect_fun(void *(*fun)(void *arg),void *arg)
{
    if(cBaseVar::connect_run){
        insertLog(-1, u8"变量管理器", u8"映射程序运行中，无法修改");
        return -1;
    }
    cBaseVar::connect_fun = fun;
    cBaseVar::connect_arg = arg;
    return 0;
}
int cBaseVar::set_connect_fun_tail(void *(*fun)(void *arg),void *arg)
{
    if(cBaseVar::connect_run){
        insertLog(-1, u8"变量管理器", u8"映射程序运行中，无法修改");
        return -1;
    }
    cBaseVar::connect_fun_tail = fun;
    cBaseVar::connect_arg_tail = arg;
    return 0;
}
int canDelGroup()
{
    if(cBaseVar::connect_run){
        insertLog(-1, u8"变量管理器", u8"映射程序运行中，无法修改");
        return -1;
    }
    return 0;
}
int newGroup(QString name)
{
    if (qMemHash.find(name) != qMemHash.end()){
        throw name + u8"  变量管理器：该变量组已经被申请";
        return -1;
    }else{
        qMemHash[name] = QHash<QString,cBaseVar>();
        return 0;
    }
}
int delGroup(QString name)
{
    if(cBaseVar::connect_run){
        insertLog(-1, u8"变量管理器", u8"映射程序运行中，无法修改");
        return -1;
    }
    if (qMemHash.find(name) != qMemHash.end()){

//        connector_mtx.lock();
//        cBaseVar::connector.insert(input->fullname(),input);
//        connector_mtx.unlock();

        GROUP_HASH::iterator it = qMemHash.find(name);
        //VAR_HASH::iterator varit;

        qMemHash.erase(it);
        return 0;
    }else{
        return -1;
    }

}

bool hasGroup(QString name)
{
    if (qMemHash.find(name) != qMemHash.end()){
        //throw name + "  变量管理器：该变量组已经申请完成";
        return true;
    }else{
        //qMemHash[name] = QHash<QString,cBaseVar>();
        return false;
    }
}
int newCVar(void *mem,QString name,cTYPE type,cDIR dir,int level,QString father,QString info)
{
    cBaseMtx.lock();
    bool hasType = false;
    QString fullname = father + "." + name;
    cBaseVar *temp = new cBaseVar(name,type,dir,level);
    if (qMemHash.find(father) == qMemHash.end()){
        insertLog(0,u8"变量管理器",u8"组名  " + father + u8"  未创建，清检查");
        throw father + u8"  变量管理器：组名未创建";
        goto errorout;
    }
    if (qMemHash[father].find(name) != qMemHash[father].end()){
        insertLog(0,u8"变量管理器","变量名  " + fullname + u8"  重复,申明失败");
        throw fullname + u8"  变量管理器：变量名重复";
        goto errorout;
    }
    qMemHash[father][name] = *temp;
    
    qMemHash[father][name].father = father;
    qMemHash[father][name].mem = mem;
    qMemHash[father][name].real_mem = mem;
    qMemHash[father][name].info = info;
    qMemHash[father][name].enable = nullptr;
    delete temp;
    cBaseMtx.unlock();
    insertLog(1,u8"变量管理器",u8"变量名  " + fullname + u8"  申请成功");

    for(int i = 0;i < cBaseVar::vartypes.size();i++){
        if(qMemHash[father][name].type == cBaseVar::vartypes[i].type){
            hasType = true;
            break;
        }
    }
    if(!hasType){
        cBaseVar::vartypes.push_back(qMemHash[father][name]);
    }
    return 0;
errorout:
    delete temp;
    cBaseMtx.unlock();
    return -1;
}
int reconfCVar(void *mem,QString name,cTYPE type,cDIR dir,int level,QString father,QString info)
{
    QString fullname = father + "." + name;
    //cBaseVar *temp = new cBaseVar(name,type,dir,level);
    if (qMemHash.find(father) == qMemHash.end()){
        insertLog(0, u8"变量管理器", u8"组名  " + father + u8"  未创建，清检查");
        throw father + u8"  变量管理器：组名未创建";
        goto errorout;
    }
    if (qMemHash[father].find(name) == qMemHash[father].end()){
        insertLog(0, u8"变量管理器", u8"变量名  " + fullname + u8"  不存在，无法重配置");
        throw fullname + u8"  变量管理器：变量名不存在";
        goto errorout;
    }
    if (qMemHash[father][name].type != type){
        insertLog(0, u8"变量管理器","变量名  " + fullname + u8"  重配变量类型不同");
        throw fullname + u8"  变量管理器：重配变量类型不同";
        goto errorout;
    }
    qMemHash[father][name].varChange(mem);
    qMemHash[father][name].info = info;
errorout:
    return -1;
}
int newCVar(void *mem,QString name,cTYPE type,cDIR dir,int level,int size,QString father,QString info)
{
    cBaseMtx.lock();
    QString fullname = father + "." + name;
    cBaseVar *temp = new cBaseVar(name,type,dir,level,size);
    if (qMemHash.find(father) == qMemHash.end()){
        insertLog(0, u8"变量管理器","组名  " + father + u8"  未创建，清检查");
        throw father + u8"  变量管理器：组名未创建";
        goto errorout;
    }
    if (qMemHash[father].find(name) != qMemHash[father].end()){
        insertLog(0, u8"变量管理器","变量名  " + fullname + u8"  重复,申明失败");
        throw fullname + u8"  变量管理器：变量名重复";
        goto errorout;
    }
    qMemHash[father][name] = *temp;
    delete temp;
    qMemHash[father][name].father = father;
    qMemHash[father][name].mem = mem;
    qMemHash[father][name].info = info;
    qMemHash[father][name].enable = nullptr;
    cBaseMtx.unlock();
    insertLog(1, u8"变量管理器","变量名  " + fullname + u8"  申请成功");
    return 0;
errorout:
    delete temp;
    cBaseMtx.unlock();
    return -1;
}
void updayePtLevel()
{
    protectLevel = 0;
//    if (!laserDev.isRuning() && !laserDev.cmdStu->runing){
//        protectLevel = 0;
//    }else if(!laserDev.isRuning() && laserDev.cmdStu->runing){
//        protectLevel = 100;
//    }else if(laserDev.isRuning() && laserDev.cmdStu->runing){
//        protectLevel = 200;
//    }
}
void insertLog(int type , QString title , QString info)
{
    cInfoMtx.lock();
    if (infoList == nullptr){
        infoList = new QList<cLogInfo>();
    }
    cLogInfo log;
    log.type = type;
    log.title = title;
    log.info = info;
    QDateTime current_time = QDateTime::currentDateTime();
    log.time = current_time.toString("yyyy-MM-dd hh:mm:ss.zzz");
    infoList->append(log);
    if (infoList->size() > 1000){
        infoList->erase(infoList->begin());
    }
    cInfoMtx.unlock();
}
QList<cLogInfo> *popLog()
{
    cInfoMtx.lock();
    QList<cLogInfo> *pop = infoList;
    infoList = nullptr;
    cInfoMtx.unlock();
    return pop;

}
bool levelCheck(cBaseVar *var)
{
    if (var == nullptr){
        insertLog(0, u8"变量管理器","变量未绑定");
        return false;
    }
    if (gvlLevel < var->level){
        insertLog(0, u8"变量管理器",var->fullname() + u8": 用户权限不足,当前权限等级" + QString::number(gvlLevel) + u8" 需求权限等级" + QString::number(var->level));
        return false;
    }
    if (protectLevel > var->ptlevel){
        insertLog(0, u8"变量管理器",var->fullname() + u8": 保护等级不足,当前设备保护等级" + QString::number(protectLevel) + u8" 参数保护等级" + QString::number(var->ptlevel));
        return false;
    }
    if (var->dir == C_INPUT && var->connect) {
        insertLog(0, u8"变量管理器", var->fullname() + u8": 变量已经被绑定");
        return false;
    }
    if(var->enable != nullptr){
        if (!var->enable){
            insertLog(0, u8"变量管理器",var->fullname() + u8": 自定义安全条件不满足");
            return false;
        }
    }

    return true;
}
bool levelCheck_nolog(cBaseVar *var)
{
    if (var == nullptr){
        return false;
    }
    if (gvlLevel < var->level){
        return false;
    }
    if (protectLevel > var->ptlevel){
        return false;
    }
    if (var->dir == C_INPUT && var->connect) {
        return false;
    }
    if(var->enable != nullptr){
        if (!var->enable){
            return false;
        }
    }
    return true;
}



cBaseVar *findCbase(QString father,QString name)
{
    cBaseVar *var;
    try{
        var = findCbase(father + "." + name);
    }catch(QString e){
        //throw e;
        return nullptr;
    }
    return var;
}
QJsonObject hashToJson()
{
    QJsonObject obj;
    GROUP_HASH::iterator it;
    for(it = qMemHash.begin();it != qMemHash.end();it++){
        VAR_HASH* group = &qMemHash[it.key()];
        VAR_HASH::iterator varit;
        for(varit = group->begin();varit != group->end();varit++){
            cBaseVar *var = &(*group)[varit.key()];
            obj[var->fullname()] = var->toString();
        }
    }
    return obj;
}
cBaseVar *findCbase(QString fullname)
{

    QStringList list = fullname.split(".");
    if (list.size() < 2){
        //throw "变量名" + fullname + "不合规";
        return nullptr;
    }
    QString father = list[0];
    list.removeAt(0);
    QString name = list.join(".");
    if(qMemHash.find(father) == qMemHash.end()){
        //throw "未找到变量组 " + father;
        return nullptr;
    }
    if(qMemHash[father].find(name) == qMemHash[father].end()){
        //throw "在 " + father + " 变量组，未找到变量 " + name;
        return nullptr;
    }
    return &qMemHash[father][name];
}
int setptLevel(QString father,QString name,int level)
{
    QString fullname = father + "." + name;
    try{
        cBaseVar *var = findCbase(father,name);
        if (var != nullptr){
            var->ptlevel = level;
        }
    }catch (QString e){
        throw e;
    }
    return 0;

}

bool check_US_level(cBaseVar *var)
{

    if (var == nullptr){
        return false;
    }
    if (gvlLevel < var->level){
        return false;
    }
    return true;
}
bool check_PT_level(cBaseVar *var)
{

    if (var == nullptr){
        return false;
    }
    if (protectLevel > var->ptlevel){
        return false;
    }
    return true;
}
int getuslevel()
{
    return gvlLevel;
}
int getptlevel()
{
    return protectLevel;
}


int newCVar(bool *mem,QString name,cDIR dir,int level,QString father,QString info)
{
    return newCVar((void *)mem, name,cTYPE::C_BOOL, dir, level, father, info);
}
int newCVar(char *mem,QString name,cDIR dir,int level,QString father,QString info)
{
    return newCVar((void *)mem, name,cTYPE::C_CHAR, dir, level, father, info);
}
int newCVar(short *mem,QString name,cDIR dir,int level,QString father,QString info)
{
    return newCVar((void *)mem, name,cTYPE::C_SHORT, dir, level, father, info);
}
int newCVar(ushort *mem,QString name,cDIR dir,int level,QString father,QString info)
{
    return newCVar((void *)mem, name,cTYPE::C_USHORT, dir, level, father, info);
}
int newCVar(int *mem,QString name,cDIR dir,int level,QString father,QString info)
{
    return newCVar((void *)mem, name,cTYPE::C_INT, dir, level, father, info);
}
int newCVar(uint *mem,QString name,cDIR dir,int level,QString father,QString info)
{
    return newCVar((void *)mem, name,cTYPE::C_UINT, dir, level, father, info);
}
int newCVar(long *mem,QString name,cDIR dir,int level,QString father,QString info)
{
    return newCVar((void *)mem, name,cTYPE::C_LONG, dir, level, father, info);
}
int newCVar(ulong *mem,QString name,cDIR dir,int level,QString father,QString info)
{
    return newCVar((void *)mem, name,cTYPE::C_ULONG, dir, level, father, info);
}
int newCVar(float *mem,QString name,cDIR dir,int level,QString father,QString info)
{
    return newCVar((void *)mem, name,cTYPE::C_FLOAT, dir, level, father, info);
}
int newCVar(double *mem,QString name,cDIR dir,int level,QString father,QString info)
{
    return newCVar((void *)mem, name,cTYPE::C_DOUBLE, dir, level, father, info);
}
int newCVar(QString* mem, QString name, cDIR dir, int level, QString father, QString info)
{
    return newCVar((void*)mem, name, cTYPE::C_QSTRING, dir, level, father, info);
}
#ifdef GUIMODE
int newCVar(QAction* mem, QString name, cDIR dir, int level, QString father, QString info)
{
    return newCVar((void*)mem, name, cTYPE::C_ACTION, dir, level, father, info);
}
#endif
#ifdef WINDOWS_CORE
int newCVar(Mat* mem, QString name, cDIR dir, int level, QString father, QString info)
{
    return newCVar((void*)mem, name, cTYPE::C_CVMAT, dir, level, father, info);
}
int newCVar(HObject* mem, QString name, cDIR dir, int level, QString father, QString info)
{
    return newCVar((void*)mem, name, cTYPE::C_HOBJ, dir, level, father, info);
}
#endif
int newCVar(cbaseFunc* mem, QString name, cDIR dir, int level, QString father, QString info)
{
    return newCVar((void*)mem, name, cTYPE::C_FUNC, dir, level, father, info);
}
int newCVar(axis_ref* mem, QString name, cDIR dir, int level, QString father, QString info)
{
    return newCVar((void*)mem, name, cTYPE::C_AXIS, dir, level, father, info);
}

int reconfCVar(bool *mem,QString name,cDIR dir,int level,QString father,QString info)
{
    return reconfCVar((void *)mem, name,cTYPE::C_BOOL, dir, level, father, info);
}
int reconfCVar(char *mem,QString name,cDIR dir,int level,QString father,QString info)
{
    return reconfCVar((void *)mem, name,cTYPE::C_CHAR, dir, level, father, info);
}
int reconfCVar(short *mem,QString name,cDIR dir,int level,QString father,QString info)
{
    return reconfCVar((void *)mem, name,cTYPE::C_SHORT, dir, level, father, info);
}
int reconfCVar(ushort *mem,QString name,cDIR dir,int level,QString father,QString info)
{
    return reconfCVar((void *)mem, name,cTYPE::C_USHORT, dir, level, father, info);
}
int reconfCVar(int *mem,QString name,cDIR dir,int level,QString father,QString info)
{
    return reconfCVar((void *)mem, name,cTYPE::C_INT, dir, level, father, info);
}
int reconfCVar(uint *mem,QString name,cDIR dir,int level,QString father,QString info)
{
    return reconfCVar((void *)mem, name,cTYPE::C_UINT, dir, level, father, info);
}
int reconfCVar(long *mem,QString name,cDIR dir,int level,QString father,QString info)
{
    return reconfCVar((void *)mem, name,cTYPE::C_LONG, dir, level, father, info);
}
int reconfCVar(ulong *mem,QString name,cDIR dir,int level,QString father,QString info)
{
    return reconfCVar((void *)mem, name,cTYPE::C_ULONG, dir, level, father, info);
}
int reconfCVar(float *mem,QString name,cDIR dir,int level,QString father,QString info)
{
    return reconfCVar((void *)mem, name,cTYPE::C_FLOAT, dir, level, father, info);
}
int reconfCVar(double *mem,QString name,cDIR dir,int level,QString father,QString info)
{
    return reconfCVar((void *)mem, name,cTYPE::C_DOUBLE, dir, level, father, info);
}
int reconfCVar(QString* mem, QString name, cDIR dir, int level, QString father, QString info)
{
    return reconfCVar((void*)mem, name, cTYPE::C_QSTRING, dir, level, father, info);
}
#ifdef GUIMODE
int reconfCVar(QAction* mem, QString name, cDIR dir, int level, QString father, QString info)
{
    return newCVar((void*)mem, name, cTYPE::C_ACTION, dir, level, father, info);
}
#endif
#ifdef WINDOWS_CORE
int reconfCVar(Mat* mem, QString name, cDIR dir, int level, QString father, QString info)
{
    return reconfCVar((void*)mem, name, cTYPE::C_CVMAT, dir, level, father, info);
}
int reconfCVar(HObject* mem, QString name, cDIR dir, int level, QString father, QString info)
{
    return reconfCVar((void*)mem, name, cTYPE::C_HOBJ, dir, level, father, info);
}
#endif
int reconfCVar(cbaseFunc* mem, QString name, cDIR dir, int level, QString father, QString info)
{
    return reconfCVar((void*)mem, name, cTYPE::C_FUNC, dir, level, father, info);
}
int reconfCVar(axis_ref* mem, QString name, cDIR dir, int level, QString father, QString info)
{
    return reconfCVar((void*)mem, name, cTYPE::C_AXIS, dir, level, father, info);
}
// db
//static QSqlDatabase db;
//static QMutex dbMtx;
//static int openDB(QString path);
//static int newDB(QString path);

//static int all_db_to_vars();
//static int all_vars_to_db();

//static int db_to_var(QString fullname);
//static int db_to_var(QString group,QString name);
//static int db_to_var(cBaseVar *var);
//static int var_to_db(QString fullname);
//static int var_to_db(QString group,QString name);
//static int var_to_db(cBaseVar *var);
//static QString db_lastError;
int cBaseVar::openDB(QString path)
{
    int ref = 0;
    dbMtx.lock();
    if (db.isOpen()){
        db.close();
    }
    if(QSqlDatabase::contains("qt_sql_default_connection")){
        db = QSqlDatabase::database("qt_sql_default_connection");
    }else{
        db = QSqlDatabase::addDatabase("QSQLITE");
    }

    db.setDatabaseName(path);
    //打开数据库
    if(!db.open()){
        db_lastError = u8"打开数据库 " + path + u8" 失败";
        insertLog(0, u8"数据库",db_lastError);
        ref = -1;
        goto error_out;
    }
    dbMtx.unlock();
    insertLog(0, u8"数据库", u8"打开数据库 " + path + u8" 成功");
    lastDB = path;
    return all_db_to_vars();

error_out:
    dbMtx.unlock();
    return ref;
}
int cBaseVar::newDB(QString path)
{
    int ref = 0;
    dbMtx.lock();

    if (db.isOpen()){
        db.close();
    }
    if(QSqlDatabase::contains("qt_sql_default_connection")){
        db = QSqlDatabase::database("qt_sql_default_connection");
    }else{
        db = QSqlDatabase::addDatabase("QSQLITE");
    }

    db.setDatabaseName(path);
    db.open();
    QSqlQuery query(db);

    const QString sql_check = "SELECT COUNT(*) FROM sqlite_master where type ='table' and name ='CBaseVars'";
    const QString sql=R"(
                      CREATE TABLE IF NOT EXISTS  CBaseVars(
                      id   INTEGER   PRIMARY KEY AUTOINCREMENT NOT NULL,
                      name TEXT UNIQUE NOT NULL,
                      type INTEGER,
                      dir INTEGER,
                      var TEXT,
                      level INTEGER,
                      ptlevel INTEGER,
                      max REAL,
                      min REAL,
                      info TEXT,
                      retain INTEGER,
                      accur INTEGER,
                      mapping INTEGER,
                      userStr TEXT,
                      connect_Name TEXT,
                      jsonValue TEXT
                      );)";

    //打开数据库
    if(!db.isOpen()){
        db_lastError = u8"打开数据库 " + path + u8" 失败";
        insertLog(0, u8"数据库",db_lastError);
        ref = -1;
        goto error_out;
    }

    if(!query.exec(sql)){
        db_lastError = path + u8" 执行创建表命令失败";
        insertLog(0, u8"数据库",db_lastError);
        ref = -2;
        goto error_out;
    }
    dbMtx.unlock();
    insertLog(0, u8"数据库", u8"创建数据库 " + path + u8" 成功");

    lastDB = path;
    return all_vars_to_db();
error_out:
    dbMtx.unlock();
    return ref;
}
int cBaseVar::rereadDB()
{
    openDB(lastDB);
}

int cBaseVar::var_to_db(cBaseVar *var)
{
    //timespec start;
    //timespec end;
    //long long diff;

    int ref = 0;
    if(!db.isOpen()){
        db_lastError = u8"数据库未打开";
        insertLog(0, u8"数据库",db_lastError);
        return -1;
    }
    dbMtx.lock();
    QSqlQuery query(db);
    bool execRef;
    QString execstr;
    QString fullName = var->father + "." + var->name;

    query.exec(QString(R"(SELECT * FROM CBaseVars WHERE name='%1';)")
               .arg(fullName));
    if (!query.next()){
        execRef = query.exec(QString(R"(INSERT INTO CBaseVars(name,type,dir,var,level,ptlevel,max,min,info,retain,accur,mapping,connect_Name,jsonValue) VALUES('%1',%2,%3,'%4',%5,%6,%7,%8,'%9',%10,%11,%12,'%13','%14');)")
                   .arg(fullName).arg(var->type).arg(var->dir).arg(var->toString()).arg(var->level).arg(var->ptlevel).arg(var->max)
                   .arg(var->min).arg(var->info).arg(var->retain).arg(var->accur).arg(var->mapping).arg(var->connect_Name).arg(var->jsonValue));
        if (!execRef){
            ref = -2;
            db_lastError = u8"在数据库新增变量" + var->father + u8"." + var->name + u8"失败";
            insertLog(0, u8"数据库",db_lastError);
            goto error_out;
        }
    }else{
//        execRef = query.prepare(QString(R"(UPDATE CBaseVars SET type = (:type),dir = (:dir),var = '(:var)',level = (:level),ptlevel = (:ptlevel),
//                                     max = (:max),min = (:min),info = '(:info)';,retain = (:retain),accur = (:accur) WHERE name = '(:name;)')"));
//        query.bindValue(":type",var->type);
//        query.bindValue(":dir",var->dir);
//        query.bindValue(":var",var->toString());
//        query.bindValue(":level",var->level);
//        query.bindValue(":ptlevel",var->ptlevel);
//        query.bindValue(":max",var->max);
//        query.bindValue(":min",var->min);
//        query.bindValue(":info",var->info);
//        query.bindValue(":retain",var->retain);
//        query.bindValue(":accur",var->accur);
//        query.bindValue(":name",var->father + "." + var->name);
//        execRef = query.exec();
        execstr = QString(R"(UPDATE CBaseVars SET type=%2,dir=%3,var='%4',level=%5,ptlevel=%6,max = %7,min=%8,info='%9',retain=%10,accur=%11,mapping=%12,connect_Name='%13',jsonValue='%14' WHERE name='%1')")
                          .arg(fullName).arg(var->type).arg(var->dir).arg(var->toString()).arg(var->level).arg(var->ptlevel).arg(var->max)
                          .arg(var->min).arg(var->info).arg(var->retain).arg(var->accur).arg(var->mapping).arg(var->connect_Name).arg(var->jsonValue);

        //clock_gettime(CLOCK_MONOTONIC, &start);
        execRef = query.exec(execstr);
        //clock_gettime(CLOCK_MONOTONIC, &end);
        //diff = diff_tv(start,end);
        //insertLog(0,"sql_timer",QString::number(diff / 1000));
        if (!execRef){
            ref = -3;
            db_lastError = u8"在数据库修改变量" + var->father + "." + var->name + u8"失败 " ;
            qDebug() << query.lastError();
            goto error_out;
        }
        //db_lastError = "在数据库修改变量" + var->father + "." + var->name + "成功 " ;
        //var->needUpdate++;
        insertLog(0, u8"数据库", u8"修改变量" + var->father + u8"." + var->name + u8" -> " + var->toString() + u8"成功 " );
    }

    dbMtx.unlock();
    return 0;
error_out:
    dbMtx.unlock();
    return ref;
}
int cBaseVar::var_to_db(QString fullname)
{
    int ref = 0;

    bool execRef;
    cBaseVar *var = findCbase(fullname);

    if (var == nullptr){
        ref = -1;
        db_lastError = u8"找不到变量" + fullname;
        insertLog(0, u8"数据库",db_lastError);
        return -1;
    }

    ref = var_to_db(var);
    return ref;
}
int cBaseVar::var_to_db(QString group,QString name)
{
    int ref = 0;
    bool execRef;
    cBaseVar *var = findCbase(group,name);
    QString fullName = group + '.' + name;
    if (var == nullptr){
        ref = -1;
        db_lastError = u8"找不到变量" + group + "." + name;
        insertLog(0, u8"数据库",db_lastError);
        return -1;
    }

    ref = var_to_db(var);
    return ref;
}
int cBaseVar::db_to_var(cBaseVar *var)
{
    int ref = 0;

    if(!db.isOpen()){
        db_lastError = u8"数据库未打开";
        insertLog(0, u8"数据库",db_lastError);
        return -1;
    }
    QSqlQuery query(db);
    bool execRef;
    QString fullName = var->father + "." + var->name;

    query.exec(QString(R"(SELECT * FROM CBaseVars WHERE name='%1';)")
               .arg(fullName));
    if (query.next()){
        //.arg(fullName).arg(var->type).arg(var->dir).arg(var->toString()).arg(var->level).arg(var->ptlevel).arg(var->max)
        //.arg(var->min).arg(var->info).arg(var->retain).arg(var->accur));
        bool ok;
        cBaseVar tempVar;
        tempVar.type = (cTYPE)query.value("type").toInt(&ok);if (!ok){db_lastError = u8" 数据库读取失败  " + var->fullname() + u8"-type";insertLog(-1, u8"数据库",db_lastError);};
        tempVar.dir = (cDIR)query.value("dir").toInt(&ok);if (!ok){db_lastError = u8"数据库读取失败  " + var->fullname() + u8"-dir";insertLog(-1, u8"数据库",db_lastError);};
        tempVar.level = query.value("level").toInt(&ok);if (!ok){db_lastError = u8"数据库读取失败  " + var->fullname() + u8"-level";insertLog(-1, u8"数据库",db_lastError);};
        tempVar.ptlevel = query.value("ptlevel").toInt(&ok);if (!ok){db_lastError = u8"数据库读取失败 - " + var->fullname() + u8"-ptlevel";insertLog(-1, u8"数据库",db_lastError);};
        tempVar.max = query.value("max").toDouble(&ok);if (!ok){db_lastError = u8"数据库读取失败  " + var->fullname() + u8"-max";insertLog(-1, u8"数据库",db_lastError);};
        tempVar.min = query.value("min").toDouble(&ok);if (!ok){db_lastError = u8"数据库读取失败  " + var->fullname() + u8"-min";insertLog(-1, u8"数据库",db_lastError);};
        tempVar.info = query.value("info").toString();if (!ok){db_lastError = u8"数据库读取失败  " + var->fullname() + u8"-info";insertLog(-1, u8"数据库",db_lastError);};
        tempVar.retain = query.value("retain").toBool();if (!ok){db_lastError = u8"数据库读取失败  " + var->fullname() + u8"-retain";insertLog(-1, u8"数据库",db_lastError);};
        tempVar.accur = query.value("accur").toInt(&ok);if (!ok){db_lastError = u8"数据库读取失败  " + var->fullname() + u8"-accur";insertLog(-1, u8"数据库",db_lastError);};
        tempVar.mapping = query.value("mapping").toInt(&ok);if (!ok){db_lastError = u8"数据库读取失败  " + var->fullname() + u8"-mapping";insertLog(-1, u8"数据库",db_lastError);};
        tempVar.connect_Name = query.value("connect_Name").toString(); if (!ok) { db_lastError = u8"数据库读取失败  " + var->fullname() + u8"-connect_Name"; insertLog(-1, u8"数据库", db_lastError); };
        tempVar.jsonValue = query.value("jsonValue").toString(); if (!ok) { db_lastError = u8"数据库读取失败  " + var->fullname() + u8"-jsonValue"; insertLog(-1, u8"数据库", db_lastError); };
        //var->type = tempVar.type;
        //var->dir = tempVar.dir;
        var->level = tempVar.level;
        var->ptlevel = tempVar.ptlevel;
        var->max = tempVar.max;
        var->min = tempVar.min;
        var->info = tempVar.info;
        var->retain = tempVar.retain;
        var->accur = tempVar.accur;
        var->mapping = tempVar.mapping;
        var->jsonValue = tempVar.jsonValue;
        if (tempVar.connect_Name != "") {
            cBaseVar::var_connect(var->fullname(), tempVar.connect_Name);
            var->connect_Name = tempVar.connect_Name;
        }

        //if (var->connect_Name != "") {
        //    var->connect = true;
        //}else{
        //    var->connect = false;
        //}

        if(var->retain){
            QString strValue = query.value("var").toString();
            var->setByString(strValue);
        }
    }
    var->needUpdate++;
    if(var->retain){
        insertLog(0, u8"数据库", u8"读取变量" + var->father + "." + var->name + u8"->" + var->toString() + u8"成功 " );
    }

    return 0;
}
int cBaseVar::db_to_var(QString fullname)
{
    int ref = 0;
    bool execRef;

    cBaseVar *var = findCbase(fullname);

    if (var == nullptr){
        ref = -1;
        db_lastError = u8"找不到变量" + fullname;
        insertLog(0, u8"数据库",db_lastError);
        return -1;
    }

    ref = db_to_var(var);
    return ref;
}

int cBaseVar::db_to_var(QString group,QString name)
{
    int ref = 0;

    bool execRef;
    cBaseVar *var = findCbase(group,name);
    QString fullName = group + '.' + name;
    if (var == nullptr){
        ref = -1;
        db_lastError = u8"找不到变量" + group + "." + name;
        insertLog(0, u8"数据库",db_lastError);
        return -1;
    }

    ref = db_to_var(var);
    return ref;
}

int cBaseVar::all_db_to_vars()
{
    if(!db.isOpen()){
        db_lastError = u8"数据库未打开";
        insertLog(0, u8"数据库",db_lastError);
        return -1;
    }
    GROUP_HASH::iterator it;
    for(it = qMemHash.begin();it != qMemHash.end();it++){
        VAR_HASH* group = &qMemHash[it.key()];
        VAR_HASH::iterator varit;
        for(varit = group->begin();varit != group->end();varit++){
            cBaseVar *var = &(*group)[varit.key()];
            if (true){
                int ret = db_to_var(var);
                if (ret != 0){
                    insertLog(0, u8"数据库",db_lastError);
                }
            }
        }
    }
    return 0;
}
int cBaseVar::all_vars_to_db()
{
    if(!db.isOpen()){
        db_lastError = u8"数据库未打开";
        insertLog(0, u8"数据库",db_lastError);
        return -1;
    }
    GROUP_HASH::iterator it;
    for(it = qMemHash.begin();it != qMemHash.end();it++){
        VAR_HASH* group = &qMemHash[it.key()];
        VAR_HASH::iterator varit;
        for(varit = group->begin();varit != group->end();varit++){
            cBaseVar *var = &(*group)[varit.key()];
            if (var->dir == C_INPUT){
                int ret = var_to_db(var);
                if (ret != 0){
                    insertLog(0, u8"数据库",db_lastError);
                }
            }
        }
    }
    return 0;
}


int cBaseVar::addCheckBox(QString value,QString info)
{
    boxList.append(value + " : " + info);
}
bool cBaseVar::isCheckBox()
{
    if(boxList.size() != 0){
        return true;
    }
    return false;
}
QString cBaseVar::getCheckBoxString()
{
    QString value = toString();
    for(int i = 0;i < boxList.size();i++){
        QString str = boxList[i];
        QStringList params = str.split(" : ");
        if(params.size() == 2){
            if(value == params[0]){
                return params[1];
            }
        }
    }
    return "null tab";
}
// connector

//
int cBaseVar::var_connect(QString input, QString output)
{
    cBaseVar* inputptr = findCbase(input);
    cBaseVar* outputptr = findCbase(output);
    return var_connect(inputptr, outputptr);
   
}
int cBaseVar::var_connect(cBaseVar* input, cBaseVar* output)
{
    if(connect_run){
        insertLog(-1, u8"变量管理器", u8"映射程序运行中，无法修改");
        return -1;
    }
    if (input == nullptr) {
        insertLog(-1, u8"变量管理器",u8"未找到 input 变量");
        return -1;
    }
    if (output == nullptr) {
        insertLog(-1, u8"变量管理器", u8"未找到 output 变量");
        return -1;
    }
    if (input->dir != C_INPUT) {
        insertLog(-1, u8"变量管理器", u8" input 变量不是输入变量");
        return -1;
    }
    //if (output->dir != C_OUTPUT) {
    //    insertLog(-1, u8"变量管理器", u8" output 变量不是输出变量");
    //    return -1;
    //}
    if (input->type != output->type) {
        insertLog(-1, u8"变量管理器", u8" 变量类型不同");
        return -1;
    }
    input->connect = true;
    input->mem = output->mem;
    input->connect_Var = output;
    input->connect_Name = output->fullname();
    input->needUpdate++;
    #ifdef GUIMODE
    if(input->isACTION()){
        QAction *inputAct,*outputAct;
        inputAct = (QAction *)input->real_mem;
        outputAct = (QAction *)output->real_mem;
        QObject::connect(outputAct,&QAction::triggered,inputAct,&QAction::trigger,Qt::ConnectionType::DirectConnection);//这里是用Dirct模式有一定风险
    }
    #endif
    output->beConnectTimes++;
    connector_mtx.lock();
    connector.insert(input->fullname(),input);
//    if(input->type == C_AXIS){
//        reconfCVar((axis_ref *)output->mem,input->name,input->dir,input->level,input->father,input->info);
//    }
    connector_mtx.unlock();
    var_to_db(input);
    insertLog(0, u8"变量管理器", u8" 绑定 " + output->fullname() + " -> " + input->fullname());
    return 0;
}
int cBaseVar::var_disconnect(QString input)
{
    cBaseVar* inputptr = findCbase(input);
    return var_disconnect(inputptr);
    return 0;
}
int cBaseVar::var_disconnect(cBaseVar* input)
{
    if(connect_run){
        insertLog(-1, u8"变量管理器", u8"映射程序运行中，无法修改");
        return -1;
    }
    if (input == nullptr) {
        insertLog(-1, u8"变量管理器", u8"未找到 input 变量");
        return -1;
    }
    #ifdef GUIMODE
    if(input->isACTION()){
        QAction *inputAct,*outputAct;
        inputAct = (QAction *)input->real_mem;
        cBaseVar *output = findCbase(input->connect_Name);

        if(output != nullptr){
            outputAct = (QAction *)output->real_mem;
            QObject::disconnect(outputAct,&QAction::triggered,inputAct,&QAction::trigger);
            //QObject::disconnect(outputAct,nullptr);
        }
    }
    #endif
    input->mem = input->real_mem;
    input->connect_Var = nullptr;
    input->connect = false;
    input->connect_Name ="";
    input->needUpdate++;

    cBaseVar *output = findCbase(input->connect_Name);
    if(output != nullptr){
        output->beConnectTimes--;
        if(output->beConnectTimes < 0){
            output->beConnectTimes = 0;
        }
    }

    connector_mtx.lock();
    connector.erase(connector.find(input->fullname()));
    connector_mtx.unlock();
    var_to_db(input);
    insertLog(0, u8"变量管理器", u8" 解除绑定 " + input->fullname());
    return 0;
}


QString syncError;
bool cmpParam(QString src,QString dest)
{
    cBaseVar *var0,*var1;
    var0 = findCbase(src);
    var1 = findCbase(dest);
    if(var0 == nullptr || var1 == nullptr){
        return false;
    }
    return cmpParam(var0,var1);
}
bool cmpParam(cBaseVar *src,cBaseVar *dest)
{
    if(src->type != dest->type){
        return false;
    }
    if(src->dir != dest->dir){
        return false;
    }

    return true;
}
int syncGroup(QString src,QString dest)
{
    if (qMemHash.find(src) == qMemHash.end()){
        syncError = u8"无变量组 " + src;
        insertLog(-1, u8"变量同步",syncError);
        return -1;
    }
    if (qMemHash.find(dest) == qMemHash.end()){
        syncError = u8"无变量组 " + dest;
        insertLog(-1, u8"变量同步",syncError);
        return -2;
    }
    QHash<QString,cBaseVar>::iterator it;
    for(it = qMemHash[src].begin();it != qMemHash[src].end();it++){
        cBaseVar *var = &qMemHash[src][it.key()];
        qDebug() << var->name;
        if(qMemHash[dest].find(var->name) == qMemHash[dest].end()){
            syncError = src + u8"与" + dest + u8"构成变量不同";
            insertLog(-1, u8"变量同步",syncError);
            return -3;
        }
        cBaseVar *destVar = &qMemHash[dest][var->name];
        if(!cmpParam(var,destVar)){
            syncError = var->father + u8"." + var->name + u8"与" + dest + "." + var->name + u8" 变量不同";
            insertLog(-1, u8"变量同步",syncError);
            return -3;
        }
    }

    return syncGroup_notMatch( src, dest);
}
int syncGroup_notMatch(QString src,QString dest)
{
    if (qMemHash.find(src) == qMemHash.end()){
        syncError = u8"无变量组 " + src;
        insertLog(-1, u8"变量同步",syncError);
        return -1;
    }
    if (qMemHash.find(dest) == qMemHash.end()){
        syncError = u8"无变量组 " + dest;
        insertLog(-1, u8"变量同步",syncError);
        return -2;
    }
    QHash<QString,cBaseVar>::iterator it;
    for(it = qMemHash[src].begin();it != qMemHash[src].end();it++){
        cBaseVar *var = &qMemHash[src][it.key()];
        if(qMemHash[dest].find(var->name) == qMemHash[dest].end()){
            continue;
        }
        cBaseVar *destVar = &qMemHash[dest][var->name];
        syncParam(var->father +"." + var->name,destVar->father +"." + destVar->name);
    }

    return 0;
}
int syncParam(QString src,QString dest)
{
    cBaseVar *srcVar,*destVar;
    srcVar = findCbase(src);
    destVar = findCbase(dest);
    if (srcVar == nullptr){
        syncError = u8"无变量 " + src;
        insertLog(-1, u8"变量同步",syncError);
        return -1;
    }
    if (destVar == nullptr){
        syncError = u8"无变量 " + dest;
        insertLog(-1, u8"变量同步",syncError);
        return -2;
    }
    destVar->max = srcVar->max;
    destVar->min = srcVar->min;
    destVar->info = srcVar->info;
    destVar->accur = srcVar->accur;
    destVar->level = srcVar->level;
    destVar->retain = srcVar->retain;
    destVar->ptlevel = srcVar->ptlevel;
    destVar->needUpdate++;
    insertLog(-1, u8"变量同步",src + " ==> " + dest);
    cBaseVar::var_to_db(destVar);
    return 0;
}

// 函数hash表相关
//struct cFun{
//   int (*fun)(QStringList arg);
//   QString info;
//};
//extern QHash<QString,cFun> funcHash;


//int registerFun(QString,int (*)(QStringList arg),QString info);
//int callFun(QString,QStringList arg);
QHash<QString,cFun> funcHash;


int registerFun(QString funName,int (*fun)(QStringList arg),QString info,QStringList argInfo)
{
    if(funcHash.find(funName) != funcHash.end()){
        insertLog(-1, u8"函数执行器",funName + u8" 函数注册失败,已经有该函数");
        return -1;
    }
    cFun tempFun;
    tempFun.fun = fun;
    tempFun.name = funName;
    tempFun.info = info;
    tempFun.argInfo = argInfo;
    funcHash[funName] = tempFun;
    insertLog(1, u8"函数执行器",funName + u8" 函数注册成功");
    return 0;
}

int callFun(QString funName,QStringList arg)
{
    if(funcHash.find(funName) == funcHash.end()){
        insertLog(-1, u8"函数执行器",funName + u8" 无此函数");
        return -1;
    }
    //int (*fun)(QStringList arg);
    //fun = (int (*)(QStringList arg))funcHash[funName].fun;
    //return fun(arg);
    return funcHash[funName].fun(arg);
}


int updateGroup(QString group)
{
    if (!hasGroup(group)) {
        insertLog(-1, u8"变量管理器", group + u8"组不存在");
        return -1;
    }
    QHash<QString, cBaseVar>::iterator it;
    for (it = qMemHash[group].begin(); it != qMemHash[group].end(); it++) {
        cBaseVar* var = &qMemHash[group][it.key()];
        int ret = updateParam(var);
        if (ret != 0) {
            return ret;
        }
    }
    return 0;

}

int updateParam(cBaseVar* var)
{
    if (var->connect == false) {
        return 0;
    }
    if(cBaseVar::connect_check == true){
        if (findCbase(var->connect_Name) == nullptr) {
            insertLog(-1, var->fullname(),u8"输入变量 " + var->connect_Name + u8"不存在，可能被删除");
            var->connect = false;
            var->connect_Var = nullptr;
            var->connect_Name = "";
            var->mem = var->real_mem;
            //cBaseVar::var_disconnect(var);
            return -1;
        }
    }
    if (var->isNull()) {
        return -1;
    }
    var->mem = var->connect_Var->mem;
    if (var->isINT()) {
        *(int*)var->real_mem = *(int*)var->mem;
    }
    else if (var->isUINT()) {
        *(uint*)var->real_mem = *(uint*)var->mem;
    }
    else if (var->isLONG()) {
        *(long*)var->real_mem = *(long*)var->mem;
    }
    else if (var->isULONG()) {
        *(ulong*)var->real_mem = *(ulong*)var->mem;
    }
    else if (var->isFLOAT()) {
        *(float*)var->real_mem = *(float*)var->mem;
    }
    else if (var->isDOUBLE()) {
        *(double*)var->real_mem = *(double*)var->mem;
    }
    else if (var->isBOOL()) {
        *(bool*)var->real_mem = *(bool*)var->mem;
    }
    else if (var->isBYTEARY()) {
        
    }
    else if (var->isCHAR()) {
        *(char*)var->real_mem = *(char*)var->mem;
    }
    else if (var->isSHORT()) {
        *(short*)var->real_mem = *(short*)var->mem;
    }
    else if (var->isUSHORT()) {
        *(ushort*)var->real_mem = *(ushort*)var->mem;
    }
    else if (var->isQSTRING()) {
        *(QString*)var->real_mem = *(QString*)var->mem;
    }
    else if (var->isCVMAT()) {
        #ifdef WINDOWS_CORE
        *(Mat*)var->real_mem = *(Mat*)var->mem;
        #else

        #endif
    }
    else if (var->isHOBJ()) {
        #ifdef WINDOWS_CORE
        *(HObject*)var->real_mem = *(HObject*)var->mem;
        #else

        #endif
    }
    else if (var->isFUNC()) {
       //
    }
    else if (var->isAXIS()) {
       *(long*)var->real_mem =  *(long*)var->mem;
        //if(((axis_ref*)var->real_mem)->axNo != ((axis_ref*)var->mem)->axNo){
            //((axis_ref*)var->real_mem)->axNo = ((axis_ref*)var->mem)->axNo;
        //}
    }
    else {
        return -1;
    }
    return 0;
}

QString MSState_to_String(MStructState state,int type)
{
//    enum MStructState{
//        NOT_INIT,
//        ERROR,
//        INITING,
//        IDEL,
//        STARTING,
//        EXECUTE,
//        HOLDING,
//    };

    switch (state) {
    case NOT_INIT:
        if(type == 0){
            return "未初始化";
        }else{
            return "NOT_INIT";
        }
        break;
    case ERROR:
        if(type == 0){
            return "错误";
        }else{
            return "ERROR";
        }
        break;
    case INITING:
        if(type == 0){
            return "初始化中";
        }else{
            return "INITING";
        }
        break;
    case IDEL:
        if(type == 0){
            return "静止中";
        }else{
            return "IDEL";
        }
        break;
    case STARTING:
        if(type == 0){
            return "启动中";
        }else{
            return "STARTING";
        }
        break;
    case EXECUTE:
        if(type == 0){
            return "运行中";
        }else{
            return "EXECUTE";
        }
        break;
    case HOLDING:
        if(type == 0){
            return "暂停中";
        }else{
            return "HOLDING";
        }
        break;
    default:
        if(type == 0){
            return "无此状态";
        }else{
            return "NOT_A_STATE";
        }
        break;
    }
}
QString MSState_to_Color(MStructState state,int type)
{

    switch (state) {
    case NOT_INIT:
        if(type == 0){
            return btn_grey;
        }
        break;
    case ERROR:
        if(type == 0){
            return btn_red;
        }
        break;
    case INITING:
        if(type == 0){
            return btn_yellow;
        }
        break;
    case IDEL:
        if(type == 0){
            return btn_green;
        }
        break;
    case STARTING:
        if(type == 0){
            return btn_blue;
        }
        break;
    case EXECUTE:
        if(type == 0){
            return btn_green;
        }
        break;
    case HOLDING:
        if(type == 0){
            return btn_blue;
        }
        break;
    default:
        if(type == 0){
            return btn_grey;
        }
        break;
    }
}


void trigger::exec(bool execute)
{
    if (execute && !xOld){
        tR = true;
    }else{
        tR = false;
    }

    if (!execute && xOld){
        tF = true;
    }else{
        tF = false;
    }

    xOld = execute;
}


void epool_tasks::pthread_fn(void *arg)
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
        CPU_SET(setcpu,&mask);
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

void *epool_tasks::epool_pth(void *arg)
{
     epool_tasks *self = (epool_tasks *)arg;
     int i = 0;
     int fd_cnt = 0;
     int sfd;
     struct epoll_event events[EPOLL_LISTEN_CNT];

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
                         self->pthread_fn(ptharg);
                     }

                 }
             }
         }
     }
}

int epool_tasks::newPthread(std::string name,unsigned int cyctime_ns)
 {
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(setcpu,&mask);
    struct epoll_event event;
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
    qDebug() << "pthvector.size() = "  << pthvector.size();
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
        goto error_out;
    }
    qDebug() << "temp->timerfd" << temp->timerfd;
    clock_gettime(CLOCK_MONOTONIC, &temp->it.it_value);
    timespec_add_us(&temp->it.it_value,cyctime_ns);
    memset(&temp->it.it_interval,0,sizeof(temp->it.it_interval));

    ret = timerfd_settime(temp->timerfd,TFD_TIMER_ABSTIME, &temp->it, NULL);
    if(ret != 0){
         qDebug() << "timerfd_settime error";
         goto error_out;
    }


    memset(&event, 0, sizeof(event));
    event.data.fd = temp->timerfd;
    event.data.ptr = (void *)pthvector.size() - 1;
    event.events = EPOLLIN | EPOLLET;
    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, temp->timerfd, &event);
    if(ret < 0) {
        qDebug() << "epoll_ctl error";
        goto error_out;
    }
    initktask(&temp->taskinfo);

    qDebug() << ret;
    pthmtx.unlock();
    qDebug() << "new done" ;
    qDebug() << "pthvector.size" << pthvector.size();
    return pthvector.size() - 1;
error_out:
    pthmtx.unlock();
    return -1;
}
int epool_tasks::push_task(int pthid ,  void *(*fun)(void* arg),void *arg,std::string name)
{
    if (pthid > pthvector.size()){
        return -1;
    }
    if(pthvector.size() == 0){
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

int epool_tasks::start()
{
    epfd = epoll_create(EPOLL_LISTEN_CNT);
    qDebug() << epfd;
    if (epfd < 0){
        return -1;
    }
     pthread_create(&eppth,NULL,epool_pth,this);
}

void checkRetain(cBaseVar *var){
    if(var->retain == false){
        //QWidget *wid = QWidget::ac
        if(var->xAskRetain == true){
            return;
        }
        var->xAskRetain = true;
        #ifdef GUIMODE
        QMessageBox box;
        box.setWindowFlag(Qt::WindowStaysOnTopHint);
        if(QMessageBox::Yes == box.information(nullptr,"通知","该变量未保存到数据库，是否保存？",QMessageBox::Yes,QMessageBox::No)){
            var->retain = true;
            cBaseVar::var_to_db(var);
        }
        #else
        cBaseVar::var_to_db(var);
        #endif

    }
}

//#define GROUP_HASH QHash<QString,QHash<QString,cBaseVar>>
//#define VAR_HASH QHash<QString,cBaseVar>

QStringList findHasStr(QString filter)
{
    QStringList varList;
    GROUP_HASH::iterator it;
    for(it = qMemHash.begin();it != qMemHash.end();it++){
        VAR_HASH  *hash = &qMemHash[it.key()];
        VAR_HASH::iterator varIt;
        for(varIt = hash->begin();varIt != hash->end();varIt++){
            cBaseVar *var = &(*hash)[varIt.key()];
            QString fullName = var->fullname();
            if(fullName.indexOf(filter) < 0){
                continue;
            }else{
                varList.append(fullName);
            }
        }
    }
    return varList;
}


bool getBIt(char *var , int pos)
{
    if(pos < 0 ){
        pos = 0;
    }
    if(pos > 7){
        pos = 7;
    }
    bool ret;
    ret = ((*var) & (1 << pos)) == ((1 << pos));
    return ret;
}
int setBIt(char *var , int pos , bool set)
{
    if(pos < 0 ){
        pos = 0;
    }
    if(pos > 7){
        pos = 7;
    }
    if(set){
        *var = *var | (1 << pos);
    }else{
        *var = *var & ~(1 << pos);
    }


    return 0;
}
int cBaseVar::appendSysError(QString var,QString func)
{
    cBaseVar *var0 = findCbase(var);
    cBaseVar *func0 = findCbase(func);
    if(var0 == nullptr){
        return -1;
    }
    return appendSysError(var0,func0);
}
int cBaseVar::appendUserError(QString var,QString func)
{
    cBaseVar *var0 = findCbase(var);
    cBaseVar *func0 = findCbase(func);
    if(var0 == nullptr){
        return -1;
    }
    return appendUserError(var0,func0);
}
int cBaseVar::appendSysError(cBaseVar *var,cBaseVar *func)
{
    if(var == nullptr){
        return -1;
    }
//    if(!var->isBOOL()){
//        return -2;
//    }
    //errorList.lock.lock();
    cBaseError tempError;
    tempError.var = var;
    tempError.type = 0;
    tempError.fun = func;
    tempError.name = var->fullname();
    tempError.info = var->info;
    tempError.xUsed = true;
    errorList.lock.lock();
    errorList.sysErrors.append(tempError);
    errorList.lock.unlock();
    return 0;
}
int cBaseVar::appendUserError(cBaseVar *var,cBaseVar *func)
{
    if(var == nullptr){
        return -1;
    }
//    if(!var->isBOOL()){
//        return -2;
//    }
    //errorList.lock.lock();
    cBaseError tempError;
    tempError.var = var;
    tempError.type = 0;
    tempError.fun = func;
    tempError.name = var->fullname();
    tempError.info = var->info;
    tempError.xUsed = true;
    errorList.lock.lock();
    errorList.userErrors.append(tempError);
    errorList.lock.unlock();
    return 0;
}
int cBaseVar::syncErrors()
{
    if(!errorList.lock.tryLock()){
        return -1;
    }
    bool xError = false;
    for(int i = 0; i < errorList.sysErrors.size();i++){
        errorList.sysErrors[i].var = findCbase(errorList.sysErrors[i].name);
        if(errorList.sysErrors[i].var == nullptr){
            errorList.sysErrors[i].xError = false;
            continue;
        }
        errorList.sysErrors[i].info = errorList.sysErrors[i].var->info;
        if(errorList.sysErrors[i].var->toString().toInt() != 0){
            errorList.sysErrors[i].xError = true;
            xError = true;
        }else{
            errorList.sysErrors[i].xError = false;
        }

    }

    for(int i = 0; i < errorList.userErrors.size();i++){
        errorList.userErrors[i].var = findCbase(errorList.userErrors[i].name);
        if(errorList.userErrors[i].var == nullptr){
            errorList.userErrors[i].xError = false;
            continue;
        }
        errorList.userErrors[i].info = errorList.userErrors[i].var->info;
        if(errorList.userErrors[i].var->toString().toInt() != 0){
            errorList.userErrors[i].xError = true;
            xError = true;
        }else{
            errorList.userErrors[i].xError = false;
        }

    }
    errorList.xError = xError;
    errorList.lock.unlock();
    return 0;
}
//template<typename T>
//bool getBItEx(T *var ,int pos)
//{
//    bool ret;
//    ret = ((*var) & (1 << pos)) == ((1 << pos));
//    return ret;
//}
//template<typename T>
//bool setBItEx(T *var , int pos , bool set)
//{
//    if(set){
//        *var = *var | (1 << pos);
//    }else{
//        *var = *var & ~(1 << pos);
//    }


//    return 0;
//}


QJsonObject byteArrayToJsonObject(const QByteArray &data) {
    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        //qDebug() << "Failed to parse JSON:" << parseError.errorString();
        return QJsonObject(); // 返回空对象
    }

    if (!jsonDoc.isObject()) {
        //qDebug() << "JSON is not an object.";
        return QJsonObject(); // 返回空对象
    }

    return jsonDoc.object();
}
#ifdef WINDOWS_CORE
Mat _cv_HObj_to_Mat(HObject hobj)
{
    HTuple htCh;
    HString cType;
    Mat cvmat;
    ConvertImageType(hobj, &hobj, "byte");
    CountChannels(hobj, &htCh);
    Hlong wid = 0;
    Hlong hgt = 0;
    if (htCh[0].I() == 1)
    {
        HImage hImg(hobj);
        void* ptr = hImg.GetImagePointer1(&cType, &wid, &hgt);
        int W = wid;
        int H = hgt;
        cvmat.create(H, W, CV_8UC1);
        unsigned char* pdata = static_cast<unsigned char*>(ptr);
        memcpy(cvmat.data, pdata, W * H);
    }
    else if (htCh[0].I() == 3)
    {
        void* Rptr;
        void* Gptr;
        void* Bptr;
        HImage hImg(hobj);
        hImg.GetImagePointer3(&Rptr, &Gptr, &Bptr, &cType, &wid, &hgt);
        int W = wid;
        int H = hgt;
        cvmat.create(H, W, CV_8UC3);
        std::vector<cv::Mat> VecM(3);
        VecM[0].create(H, W, CV_8UC1);
        VecM[1].create(H, W, CV_8UC1);
        VecM[2].create(H, W, CV_8UC1);
        unsigned char* R = (unsigned char*)Rptr;
        unsigned char* G = (unsigned char*)Gptr;
        unsigned char* B = (unsigned char*)Bptr;
        memcpy(VecM[2].data, R, W * H);
        memcpy(VecM[1].data, G, W * H);
        memcpy(VecM[0].data, B, W * H);
        cv::merge(VecM, cvmat);
    }
    return cvmat;
}
HObject _cv_Mat_to_HObj(Mat cvmat)
{
    int height = cvmat.rows;
    int width = cvmat.cols;
    HObject hobj;
    if (cvmat.type() == CV_8UC3) {
        std::vector<cv::Mat> imgchannel;
        split(cvmat, imgchannel);
        cv::Mat imgB = imgchannel[0];
        cv::Mat imgG = imgchannel[1];
        cv::Mat imgR = imgchannel[2];
        uchar* dataR = new uchar[height * width];
        uchar* dataG = new uchar[height * width];
        uchar* dataB = new uchar[height * width];
        for (int i = 0; i < height; i++)
        {
            memcpy(dataR + width * i, imgR.data + imgR.step * i, width);
            memcpy(dataG + width * i, imgG.data + imgG.step * i, width);
            memcpy(dataB + width * i, imgB.data + imgB.step * i, width);
        }
        GenImage3(&hobj, "byte", width, height, (Hlong)dataR, (Hlong)dataG, (Hlong)dataB);
        delete[]dataR;
        delete[]dataG;
        delete[]dataB;
        dataR = NULL;
        dataG = NULL;
        dataB = NULL;
    }
    else if (cvmat.type() == CV_8UC1)
    {
        uchar* data = new uchar[height * width];
        for (int i = 0; i < height; i++)
            memcpy(data + width * i, cvmat.data + cvmat.step * i, width);
        GenImage1(&hobj, "byte", width, height, (Hlong)data);
        delete[] data;
        data = NULL;
    }
    return hobj;
}


#endif
