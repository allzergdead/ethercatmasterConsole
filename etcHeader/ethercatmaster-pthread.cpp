#include "ethercatmaster.h"
//#include "ui_ethercatmaster.h"
#include <sys/inotify.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <cstring>
#include <fstream>

#include <QElapsedTimer>
#define MEASURE_TIMING

void initktask(struct kenrel_TaskInfo_etc *stask)
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

void cmd_2_cmd0(axis_Cmd *dest , axis_Cmd *src)
{
    memcpy((void *)dest,(void *)src,sizeof(axis_Cmd) - 200);
}
void cmd_2_cmd(axis_Cmd *dest , axis_Cmd *src)
{
    //memcpy((void *)dest,(void *)src,sizeof(axis_Cmd) - 200);
    //vel param
    dest->vel.acc = src->vel.acc;
    dest->vel.dec = src->vel.acc;
    dest->vel.vel = src->vel.acc;
    dest->vel.jerk = src->vel.acc;

    //abs param
    dest->abs.acc = src->abs.acc;
    dest->abs.dec = src->abs.dec;
    dest->abs.pos = src->abs.pos;
    dest->abs.vel = src->abs.vel;
    dest->abs.jerk = src->abs.jerk;

    //stop param
    dest->stop.dec = src->stop.dec;
    dest->stop.jerk = src->stop.jerk;

    //bu5_tc param


    //setpos param
    dest->setpos.pos = src->setpos.pos;

    //dev_home param
    dest->dev_home.timeOut = src->dev_home.timeOut;

    //jog param
    dest->fjog_acc = src->fjog_acc;
    dest->fjog_dec = src->fjog_dec;
    dest->fjog_vel = src->fjog_vel;
    dest->fjog_jerk = src->fjog_jerk;
    //dest->fmanual_dir = src->fmanual_dir;


    //setpos param
    dest->setpos.pos = src->setpos.pos;

    //reset param
    dest->xReset = src->xReset;

    //real param
    dest->freal_acc = src->freal_acc;
    dest->freal_dec = src->freal_dec;
    dest->freal_pos = src->freal_pos;
    dest->freal_vel = src->freal_vel;
    dest->freal_jerk = src->freal_jerk;

    //other param
    dest->act_Now = src->act_Now;
    dest->xpower_on = src->xpower_on;
    dest->xpower_drvstart = src->xpower_drvstart;

    //wtmc
    dest->type_Runing = src->type_Runing;

    //


}
void EtherCatMaster::cyclic_task()
{

    struct timespec wakeupTime, time;
#ifdef MEASURE_TIMING
    struct timespec startTime, endTime, lastStartTime = {};
    uint32_t period_ns = 0, exec_ns = 0, latency_ns = 0,
             latency_min_ns = 0, latency_max_ns = 0,
             period_min_ns = 0, period_max_ns = 0,
             exec_min_ns = 0, exec_max_ns = 0;
#endif

    // get current time
    clock_gettime(CLOCK_MONOTONIC, &wakeupTime);
    int fd = open("/dev/chrtimer", O_WRONLY);
    char IOTest = 0xff;
    QElapsedTimer wakeTime;
    wakeTime.start();
    ulong timens;// = wakeTime.nsecsElapsed();
    ulong timeus;// = timens / 1000;
    ulong timeusOld = 0;// = timens / 1000;
    while(1) {
        if (xCloseSig){
            ecrt_release_master(master);
            return ;
        }
        if (!start){
            ecrt_release_master(master);
            return;
        }

        timespec wakeupTime0 = timespec_add(wakeupTime, cycletime0);
        wakeupTime = timespec_add(wakeupTime, cycletime);

//        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &wakeupTime, NULL);
        //timespec wakeupTime0 = ;
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &wakeupTime0, NULL);
        while(true){
            timespec wakeupTimeTest;
            clock_gettime(CLOCK_MONOTONIC, &wakeupTimeTest);
            if(wakeupTimeTest.tv_sec > wakeupTime.tv_sec){
                break;
            }else{
                if(wakeupTimeTest.tv_nsec > wakeupTime.tv_nsec){
                    break;
                }
            }
        }
        QElapsedTimer timer0;
        timer0.start();

//        struct chrtimer_req req;
//        req.clock_id = CLOCK_MONOTONIC;
//        req.flags = TIMER_ABSTIME; // 或TIMER_ABSTIME
//        req.time = wakeupTime;



//        write(fd, &req, sizeof(req));
        //::close(fd);*/
        // Write application time to master
        //
        // It is a good idea to use the target time (not the measured time) as
        // application time, because it is more stable.
        //
        ecrt_master_application_time(master, TIMESPEC2NS(wakeupTime));

#ifdef MEASURE_TIMING
        clock_gettime(CLOCK_MONOTONIC, &startTime);
        latency_ns = DIFF_NS(wakeupTime, startTime);
        period_ns = DIFF_NS(lastStartTime, startTime);
        exec_ns = DIFF_NS(lastStartTime, endTime);
        lastStartTime = startTime;

        if (latency_ns > latency_max_ns) {
            latency_max_ns = latency_ns;
        }
        if (latency_ns < latency_min_ns) {
            latency_min_ns = latency_ns;
        }
        if (period_ns > period_max_ns) {
            period_max_ns = period_ns;
        }
        if (period_ns < period_min_ns) {
            period_min_ns = period_ns;
        }
        if (exec_ns > exec_max_ns) {
            exec_max_ns = exec_ns;
        }
        if (exec_ns < exec_min_ns) {
            exec_min_ns = exec_ns;
        }
        ecm->task_info.task_countor++;
        ecm->task_info.task_cost = exec_ns;
        ecm->task_info.task_period = period_ns;
        ecm->task_info.task_cost_sum = ecm->task_info.task_cost_sum + ecm->task_info.task_cost;
        if (ecm->task_info.task_cost > ecm->task_info.task_cost_max){
            ecm->task_info.task_cost_max = ecm->task_info.task_cost;
        }
        if (ecm->task_info.task_cost < ecm->task_info.task_cost_min){
            ecm->task_info.task_cost_min = ecm->task_info.task_cost;
        }
        ecm->task_info.task_cost_avg = ecm->task_info.task_cost_sum / ecm->task_info.task_countor;

        ecm->task_info.task_period_sum = ecm->task_info.task_period_sum +  ecm->task_info.task_period;
        if (ecm->task_info.task_period > ecm->task_info.task_period_max){
            ecm->task_info.task_period_max = ecm->task_info.task_period;
        }
        if (ecm->task_info.task_period < ecm->task_info.task_period_min){
            ecm->task_info.task_period_min = ecm->task_info.task_period;
        }
        ecm->task_info.task_period_avg = ecm->task_info.task_period_sum / ecm->task_info.task_countor;

        if (ecm->task_info.clear == 1){
            initktask(&ecm->task_info);
            ecm->task_info.clear = 0;
        }
        ecm->task_info.cpu = sched_getcpu();

//         period_max_ns = 0;
        //            period_min_ns = 0xffffffff;
        //            exec_max_ns = 0;
        //            exec_min_ns = 0xffffffff;
        //            latency_max_ns = 0;
        //            latency_min_ns = 0xffffffff;

        //IOTest = ~IOTest;
        //EC_WRITE_U8(domain1_pd + ecm->pdoconf[0].offset[1],IOTest);
        //EC_WRITE_U8(domain1_pd + ecm->pdoconf[0].offset[1],0x0f);
#endif

        // receive process data
        ecrt_master_receive(master);
        ecrt_domain_process(domain1);


        // check process data state (optional)
        //check_domain1_state();

        if (counter) {
            counter--;
        } else { // do this at 1 Hz
            counter = NSEC_PER_SEC / period_ns;
            //qDebug() << ecm->axis[0].iSetPos;
            // check for master state (optional)
            //check_master_state();
            // check for slave configuration state(s)
            //check_slave_config_states();
#ifdef MEASURE_TIMING
//            qDebug() << "-------------------------------------------------------------------------------------------------";
//            qDebug() << "latency_ns" <<latency_ns <<  "period_ns" << period_ns << "exec_ns" << exec_ns;
//            qDebug() << "latency_max" <<latency_max_ns <<  "period_max" << period_max_ns << "exec_max" << exec_max_ns;
//            qDebug() << "-------------------------------------------------------------------------------------------------";

//            qDebug() << "factpos" << hex << ecm->axis[0].fActPos << "lactaxis" << hex << ecm->axis[0].lActPos << "iactaxis"<< hex << ecm->axis[0].iActPos;

//            qDebug() << "fsetpos" <<  hex << ecm->axis[0].fSetPos << "lsetaxis" << hex <<  ecm->axis[0].lSetPos << "isetaxis"<< hex << ecm->axis[0].iSetPos;
//            qDebug() << "syncstate" << ecm->axis[0].syncState << "status_word" << ecm->axis[0].status_word;
//            qDebug() << "c_state" << c_state <<"axis_state" << ecm->axis[0].state << "axis_step" << ecm->axis[0].syncState;
//            qDebug() << "posequaCount_p" << ecm->axis[0].posequaCount_p <<  "posequaCount" <<ecm->axis[0].posequaCount;
            //qDebug() << "etc pthread run";
//            if (shm_axdata.isAttached()){
//                axis_data *data = (axis_data *)shm_axdata.data();
//                qDebug() << "data num" << data->num;
//            }
            //qDebug() << ecm->axis[0].cmd.bu5_tc.act_1s_count << "/S";
            for(int i = 0;i < ecm->axis_Num;i++){
                ecm->axis[i].posequaCount = 0;
                ecm->axis[i].posequaCount_p = 0;
                ecm->axis[i].cmd.bu5_tc.act_1s_count = 0;
                //qDebug() <<"axis" << i<<" : " <<ecm->axis[i].status_word;
            }

            //actCount = 0;
            //check_domain1_state();
            //check_master_state();
            //check_slave_config_states();
            period_max_ns = 0;
            period_min_ns = 0xffffffff;
            exec_max_ns = 0;
            exec_min_ns = 0xffffffff;
            latency_max_ns = 0;
            latency_min_ns = 0xffffffff;

            //qDebug() <<"I_CH1_OFFSET : " << ecm->pdoconf[0].offset[2] << "value " << I_CH1 ;
            //qDebug() <<"I_CH2_OFFSET : " << ecm->pdoconf[0].offset[3] << "value " << I_CH2 ;
            //qDebug() <<"testCount : " << testCount;
            testCount = testCount + 1;
            //EC_WRITE_U8(domain1_pd + ecm->pdoconf[0].offset[0],testCount);
#endif


            blink = !blink;
        }

        //lseek(ETCWritefd, 0, SEEK_SET);
        //read(ETCWritefd,ecwrite,sizeof(cec_shm_write));
        //msync(ecwrite, sizeof(cec_shm_write), MS_SYNC); // 强制刷新映射区域
        if(ecwrite->ecNo != ecm->ecNo){
            if(ecwrite->beUse1 == 1){
                //cmd_2_cmd(ecm->axis->cmd);
                for(int i = 0;i < MAX_AXIS;i++){
                    cmd_2_cmd(&ecm->axis[i].cmd,&ecwrite->cmd1[i]);
                    ecm->axis[i].target_mode = ecwrite->target_mode1[i];
                    //ecm->axis[i].target_mode = ecwrite->cmd1[i]
                }
                ecwrite->beUse1 = 0;
                ecwrite->beUse2 = 0;

            }else if(ecwrite->beUse2 == 1){
                for(int i = 0;i < MAX_AXIS;i++){
                    cmd_2_cmd(&ecm->axis[i].cmd,&ecwrite->cmd2[i]);
                    ecm->axis[i].target_mode = ecwrite->target_mode2[i];
                }
                ecwrite->beUse1 = 0;
                ecwrite->beUse2 = 0;
            }else{

            }
        }


//        for(int i = 0;i < MAX_AXIS;i++){
//            //ecwrite->cmd1[i].abs.acc = 666;
//            cmd_2_cmd(&ecm->axis[i].cmd,&ecwrite->cmd1[i]);
//        }
//        //ecm->axis[0].cmd.act_Now = 777;
//        ecm->axis[0].cmd.act_Now = ecwrite->cmd1[0].act_Now;

        for(int i = 0 ; i<ecm->pdo_Num ; i++){
            syncPod_Read(&ecm->pdoconf[i]);
        }
        for(int i = 0;i < ecm->axis_Num ;i++){
            //tc_sync_call_680N(&ecm->axis[i],&tc_jmc_ary[i]);
            syncAxis_Ref(&ecm->axis[i]);
        }
        //IOTest = ~IOTest;
        //ecm->pdoconf[0].pdo_data[2] = IOTest;
        for(int i = 0 ; i<ecm->pdo_Num ; i++){
            syncPod_Write(&ecm->pdoconf[i]);
        }


        //EC_WRITE_U8(domain1_pd + ecm->pdoconf[0].offset[0],(uchar)IO_counter);


        //Q_CH1 = (uchar *)&ecm->pdoconf[0].pdo_data[0];
        //I_CH1 = (uchar *)&ecm->pdoconf[0].pdo_data[2];


            //cmd_2_cmd(ecm->axis->cmd);
        for(int i = 0;i < MAX_AXIS;i++){
            //cmd_2_cmd(&ecwrite->cmd1[i],&ecm->axis[i].cmd);
            //cmd_2_cmd(&ecwrite->cmd2[i],&ecm->axis[i].cmd);
            memcpy(&ecwrite->cmd1[i],&ecm->axis[i].cmd,sizeof (axis_Cmd));
            memcpy(&ecwrite->cmd2[i],&ecm->axis[i].cmd,sizeof (axis_Cmd));
            for(int i = 0;i < MAX_AXIS;i++){
                //cmd_2_cmd(&ecm->axis[i].cmd,&ecwrite->cmd1[i]);
                ecwrite->target_mode1[i] = ecm->axis[i].target_mode;
                ecwrite->target_mode2[i] = ecm->axis[i].target_mode;
                //ecm->axis[i].target_mode = ecwrite->cmd1[i]
            }
        }
        //ecwrite->beUse2++;
        ecm->ecNo = ecwrite->ecNo;

        //fsync(ETCfd);
        double timens = timer0.nsecsElapsed();
        double timeus = timens / 1000.0;
        if(timeus > 150){
            //qDebug() << "timeus" << timeus;
        }
        if (sync_ref_counter) {
            sync_ref_counter--;
        } else {
            sync_ref_counter = 1; // sync every cycle

            clock_gettime(CLOCK_MONOTONIC, &time);
            ecrt_master_sync_reference_clock_to(master, TIMESPEC2NS(time));
        }
        ecrt_master_sync_slave_clocks(master);

        // send process data
        ecrt_domain_queue(domain1);
        ecrt_master_send(master);
        ecm->etc_task_counter++;
#ifdef MEASURE_TIMING
        clock_gettime(CLOCK_MONOTONIC, &endTime);
#endif
    }
}
int EtherCatMaster::etc_run_cycle()
{
    cpu_set_t mask;
    CPU_ZERO(&mask);
    kv_pair keyAry[MAX_KV_PAIRS];
    getCoreConf(keyAry);
    //CPU_SET(0,&mask);
    //CPU_SET(1,&mask);
    //CPU_SET(2,&mask);
    //CPU_SET(3,&mask);
    //CPU_SET(4,&mask);
    //CPU_SET(5,&mask);
    for(int i = 0;i < MAX_KV_PAIRS;i++){
        qDebug() << keyAry[i].key << keyAry[i].value;
        if(keyAry[i].key == "ethercat-0"){
            CPU_SET(keyAry[i].value,&mask);
        }
    }
    int pid = pthread_self();
    sched_getaffinity(pid,sizeof(mask),&mask);
    pthread_setaffinity_np(pthread_self(),sizeof(mask),&mask);

    for(int i=0;i < MAX_AXIS;i++){
        ecm->axis[i].axNo = i;
    }

    // 设置调度策略为 SCHED_FIFO（实时优先级调度）
     struct sched_param param;
        int policy = SCHED_FIFO;

        // 获取当前调度策略的最高优先级
        int max_priority = sched_get_priority_max(policy);
        if (max_priority == -1) {
            perror("sched_get_priority_max");
            return EXIT_FAILURE;
        }

        // 设置线程的调度优先级为最高
        param.sched_priority = max_priority;

        if (pthread_setschedparam(pthread_self(), policy, &param) != 0) {
            perror("pthread_setschedparam");
            return EXIT_FAILURE;
        }

        printf("Thread is now running on CPU 3 with the highest priority (%d) under SCHED_FIFO policy.\n", max_priority);
    if (mlockall(MCL_CURRENT | MCL_FUTURE) == -1) {
        MESSAGE_INFO("error","mlockall error!");
        start = false;
        return -1;
    }
    //ecrt_release_master(master);
    qDebug() << "ecrt_startup_master!";

// call etc config!
    //oneinotest();
    read_config_by_json();


    master = ecrt_request_master(0);
    if (!master){
        MESSAGE_INFO("error","ecrt_request_master error!");
        start = false;
        return -1;
    }
    mem_insertLog(1,"EtherCatMaster","初始化主站成功");
    qDebug() << "ecrt_request_master done!";
    domain1 = ecrt_master_create_domain(master);
    if (!domain1){
        MESSAGE_INFO("error","ecrt_master_create_domain error!");
        ecrt_release_master(master);
        start = false;
        return -1;
    }
    //mem_insertLog(1,"EtherCatMaster","初始化主站成功");
    qDebug() << "ecrt_master_create_domain done!";
    for(int i = 0;i < ecm->pdo_Num;i++){
        ecm->pdoconf[i].sc = ecrt_master_slave_config(master, ecm->pdoconf[i].alias,ecm->pdoconf[i].position,
                                      ecm->pdoconf[i].vid,ecm->pdoconf[i].pid);
        if(!ecm->pdoconf[i].sc){
            MESSAGE_INFO("error","ecrt_master_slave_config slave-" + QString::number(i) + " error!");
            ecrt_release_master(master);
            start = false;
            return -1;
        }
    }
    //int pdo_num = ecm->pdo_Num;
    qDebug() << "ecrt_master_slave_config done!";
    mem_insertLog(1,"EtherCatMaster","初始化PDO成功");
    ecrt_master_reset(master);
    for(int i = 0;i < ecm->pdo_Num;i++){
        ec_sync_info_t sync_info[5];
        memcpy(sync_info,ecm->pdoconf[i].device_syncs,5 * sizeof(ec_sync_info_t));
        int ret = ecrt_slave_config_pdos(ecm->pdoconf[i].sc,EC_END,ecm->pdoconf[i].device_syncs);
        if (ret)
        {
            MESSAGE_INFO("error","ecrt_slave_config_pdos slave-" + QString::number(i) + " error! errorid-" + QString::number(ret));
            ecrt_release_master(master);
            start = false;
            return -1;
        }else{
             qDebug() << "ecrt_slave_config_pdos slave-" + QString::number(i) + " done!";
        }

    }

    int num = ecm->pdo_Num;
    for(int i = 0;i < ecm->pdo_Num;i++){
        pod_config see_pdo_conf = ecm->pdoconf[0];
        int pdoSize = ecm->pdoconf[i].pdoSize;
        ec_pdo_entry_reg_t see_pdo_entry[pdoSize];
        memcpy(see_pdo_entry,ecm->pdoconf[i].domain1_regs, pdoSize * sizeof(ec_pdo_entry_reg_t));

        int indexSize = ecm->pdoconf[i].indexSize;
        ec_pdo_info_t see_pdo_info[indexSize];
        memcpy(see_pdo_info,ecm->pdoconf[i].device_pdos, indexSize * sizeof(ec_pdo_info_t));

        int syncSize = 5;
        ec_sync_info_t see_ec_sync[5];
        memcpy(see_ec_sync,ecm->pdoconf[i].device_syncs, 5 * sizeof(ec_sync_info_t));
        int ret = ecrt_domain_reg_pdo_entry_list(domain1, ecm->pdoconf[i].domain1_regs);
        //ecrt_slave_config_reg_pdo_entry()
        //ecrt_slave_config_reg_pdo_entry();
        //for(int pdo_entry_i = 0; pdo_entry_i < ecm->pdoconf[i].pdoSize;pdo_entry_i++){
            //pod_config *pdo = &(ecm->pdoconf[i]);
            //int  ret = ecrt_slave_config_reg_pdo_entry(ecm->pdoconf[i].sc,pdo->domain1_regs[pdo_entry_i].index,pdo->domain1_regs[pdo_entry_i].subindex,
            //                                      domain1,pdo->domain1_regs[pdo_entry_i].bit_position);

            if (ret < 0)
            {
                MESSAGE_INFO("error","ecrt_domain_reg_pdo_entry_list slave-" + QString::number(i) + " error! errorid-" + QString::number(ret));
                ecrt_release_master(master);
                start = false;
                return -1;
            }else{
                qDebug() << "ecrt_domain_reg_pdo_entry_list slave-" + QString::number(i) + " done!";
            }
        //}
    }
    qDebug() << "period_ns" << period_ns;
    mem_insertLog(0,"EtherCatMaster","主站周期 - " + QString::number(period_ns / 1000) + "us");
    for(int i = 0;i < ecm->pdo_Num;i++){
        ecm->pdoconf[i].assign = 0x300;
        ecrt_slave_config_dc(ecm->pdoconf[i].sc, ecm->pdoconf[i].assign, period_ns, 0, 0, 0);
    }
    //ecrt_master_sync_reference_clock(master);
    //ecrt_master_sync_slave_clocks(master);
    if (ecrt_master_activate(master)){
        MESSAGE_INFO("error","ecrt_master_activate  error!" );
        ecrt_release_master(master);
        start = false;
        return -1;
    }
    qDebug() << "ecrt_master_activate done!";

    if (!(domain1_pd = ecrt_domain_data(domain1))){
        MESSAGE_INFO("error","ecrt_domain_data  error!" );
        ecrt_release_master(master);
        start = false;
        return -1;
    }
     qDebug() << "ecrt_domain_data done!";

    /* Set priority */

//    struct sched_param param = {};
//    param.sched_priority = 99;//sched_get_priority_max(SCHED_FIFO);

//    printf("Using priority %i.", param.sched_priority);
//    if (sched_setscheduler(0, SCHED_FIFO, &param) == -1) {
//        MESSAGE_INFO("error","sched_setscheduler  error!" );
//    }
    axis_hash.clear();
//      appendAxisHash(ecm->axis[0].name,&ecm->axis[0]);
    for(int i = 0;i < ecm->axis_Num ;i++){
        int ret = ec::appendAxisHash(ecm->axis[i].name,&ecm->axis[i]);
        if (ret != 0){
            MESSAGE_INFO("error","appendAxisHash  error!" );
            ecrt_release_master(master);
            start = false;
            return -1;
        }
    }
    //return 0;
    ecm->etc_task_counter = 0;
    if(!test){
        mem_insertLog(1,"EtherCat","主站启动成功");
        mem_insertLog(100,"EtherCat","rereadDB");
    }
    //mem_insertLog(1,"EtherCat","主站启动成功");
    //mem_insertLog(100,"EtherCat","rereadDB");
    cyclic_task();

    return 0;
}
/*****************************************************************************/

void EtherCatMaster::check_domain1_state(void)
{
    ec_domain_state_t ds;

    ecrt_domain_state(domain1, &ds);

    if (ds.working_counter != domain1_state.working_counter)
        qDebug() << "Domain1: WC " << ds.working_counter;
        //printf("Domain1: WC %u.\n", ds.working_counter);
    if (ds.wc_state != domain1_state.wc_state)
        qDebug() << "Domain1: State " << ds.wc_state;
        //printf("Domain1: State %u.\n", ds.wc_state);

    domain1_state = ds;
}

/*****************************************************************************/

void EtherCatMaster::check_master_state(void)
{
    ec_master_state_t ms;

    ecrt_master_state(master, &ms);

    if (ms.slaves_responding != master_state.slaves_responding)
        //printf("%u slave(s).\n", ms.slaves_responding);
        qDebug() << ms.slaves_responding << "slave(s).";
    if (ms.al_states != master_state.al_states)
        //printf("AL states: 0x%02X.\n", ms.al_states);
        qDebug() << "AL states: " << hex << ms.al_states;
    if (ms.link_up != master_state.link_up)
        //printf("Link is %s.\n", ms.link_up ? "up" : "down");
        qDebug() << "Link is " << hex << ms.link_up;
    master_state = ms;
}

/****************************************************************************/

void EtherCatMaster::check_slave_config_states(void)
{
    ec_slave_config_state_t s;
    ecrt_slave_config_state(sc, &s);
    if (s.al_state != sc_state.al_state)
    {
        printf("slave: State 0x%02X.\n", s.al_state);
        //qDebug() <<
    }
    if (s.online != sc_state.online)
    {
        printf("slave: %s.\n", s.online ? "online" : "offline");
    }
    if (s.operational != sc_state.operational)
    {
        printf("slave: %soperational.\n", s.operational ? "" : "Not ");
    }
    sc_state = s;
}
void *EtherCatMaster::call_sync(void *arg)
{
    //return nullptr;
    EtherCatMaster *self = (EtherCatMaster *)arg;
    self->etc_run_cycle();

    return nullptr;
}


#define BUF_LEN (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))
void *EtherCatMaster::cmd_sync(void *arg)
{
    //return nullptr;
    //return nullptr;
    EtherCatMaster *self = (EtherCatMaster *)arg;
    //return 0;
    cpu_set_t mask;
    CPU_ZERO(&mask);
    //CPU_SET(0,&mask);
    //CPU_SET(1,&mask);
    //CPU_SET(2,&mask);
    CPU_SET(4,&mask);
    //CPU_SET(4,&mask);
    //CPU_SET(5,&mask);
    int pid = pthread_self();
    sched_getaffinity(pid,sizeof(mask),&mask);
    pthread_setaffinity_np(pthread_self(),sizeof(mask),&mask);



    // 设置调度策略为 SCHED_FIFO（实时优先级调度）
     struct sched_param param;
        int policy = SCHED_FIFO;

        // 获取当前调度策略的最高优先级
        int max_priority = sched_get_priority_max(policy);
        if (max_priority == -1) {
            perror("sched_get_priority_max");
            return nullptr;
        }

        // 设置线程的调度优先级为最高
        param.sched_priority = 99;

        if (pthread_setschedparam(pthread_self(), policy, &param) != 0) {
            perror("pthread_setschedparam");
            return nullptr;
        }

        printf("Thread is now running on CPU 4 with the highest priority (%d) under SCHED_FIFO policy.\n", max_priority);



    std::stringstream path;
    //path << "/dev/shm/vm_shared/shared_memory";
     path << "/mnt/ramdisk/etc_write";
    int inotify_fd = inotify_init1(IN_NONBLOCK);
    if (inotify_fd < 0) {
        perror("inotify_init1");
        //return;
    }

    int watch_fd = inotify_add_watch(inotify_fd, "/mnt/ramdisk/etc_write", IN_MODIFY);
    if (watch_fd < 0) {
        perror("inotify_add_watch");
        close(inotify_fd);
        //return;
    }

    char buffer[BUF_LEN];

    while(true){
//        lseek(self->ETCWritefd, 0, SEEK_SET);
//        read(self->ETCWritefd,self->ecwrite,sizeof(cec_shm_write));

//        lseek(self->ETCWritefd, 0, SEEK_SET);
//        read(self->ETCWritefd,self->ecwrite,sizeof(cec_shm_write));
        //usleep(500);
        //continue;

        int length = read(inotify_fd, buffer, BUF_LEN);
                if (length < 0 && errno != EAGAIN) {
                    perror("read");
                    break;
                }

                if (length > 0) {
                    for (char* ptr = buffer; ptr < buffer + length;) {
                        struct inotify_event* event = (struct inotify_event*)ptr;

                        if (event->mask & IN_MODIFY) {
                            //std::cout << "File modified: " << file_path << std::endl;

                            // 读取文件内容
                            cec_shm_write temp0;
                            lseek(self->ETCWritefd, 0, SEEK_SET);
                            read(self->ETCWritefd,self->ecwrite,sizeof(cec_shm_write));
                            //read(self->ETCWritefd,&temp0,sizeof(cec_shm_write));
                            //qDebug() << sched_getcpu();
                        }
                            //qDebug() << "file change";
                        ptr += sizeof(struct inotify_event) + event->len;
                    }
                }




        usleep(500);
    }
    return nullptr;
}
//static void *cmd_sync(void *arg);
