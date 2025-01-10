#include "ethercartest.h"
/****************************************************************************/

// Application parameters
#define FREQUENCY 4000
#define CLOCK_TO_USE CLOCK_MONOTONIC
#define MEASURE_TIMING

#define TARGET_POSITION         0 /*target position*/
#define CYCLIC_POSITION            8   /*Operation mode for 0x6060:0*/

/****************************************************************************/

#define NSEC_PER_SEC (1000000000L)
#define PERIOD_NS (NSEC_PER_SEC / FREQUENCY)    /*本次设置周期PERIOD_NS为1ms*/

#define DIFF_NS(A, B) (((B).tv_sec - (A).tv_sec) * NSEC_PER_SEC + \
        (B).tv_nsec - (A).tv_nsec)

#define TIMESPEC2NS(T) ((uint64_t) (T).tv_sec * NSEC_PER_SEC + (T).tv_nsec)

/****************************************************************************/

// EtherCAT
static ec_master_t *master = NULL;
static ec_master_state_t master_state = {};

static ec_domain_t *domain1 = NULL;
static ec_domain_state_t domain1_state = {};

static ec_slave_config_t *sc = NULL;
static ec_slave_config_state_t sc_state = {};

/****************************************************************************/

// process data
static uint8_t *domain1_pd = NULL;

#define PANASONIC        0,0                        /*EtherCAT address on the bus*/

#define VID_PID          0x00100000,0x000c0108      /*Vendor ID, product code*/

/*Offsets for PDO entries*/
static struct{
    unsigned int ctrl_word;
    unsigned int operation_mode;
    unsigned int target_position;
    unsigned int touch_probe_function;
    unsigned int target_torque;
    unsigned int error_code;
    unsigned int status_word;
    unsigned int modes_of_operation_display;
    unsigned int position_actual_value;
    unsigned int touch_probe_status;
    unsigned int touch_probe_pos1_pos_value;
    unsigned int following_error_actual_value;
    unsigned int digital_inputs;
}offset;

const static ec_pdo_entry_reg_t domain1_regs[] = {
    {PANASONIC, VID_PID, 0x6040, 0, &offset.ctrl_word},
    {PANASONIC, VID_PID, 0x6060, 0, &offset.operation_mode },
    {PANASONIC, VID_PID, 0x607A, 0, &offset.target_position},
    {PANASONIC, VID_PID, 0x60B8, 0, &offset.touch_probe_function},
    {PANASONIC, VID_PID, 0x6071, 0, &offset.target_torque},
    {PANASONIC, VID_PID, 0x603F, 0, &offset.error_code},
    {PANASONIC, VID_PID, 0x6041, 0, &offset.status_word},
    {PANASONIC, VID_PID, 0x6061, 0, &offset.modes_of_operation_display},
    {PANASONIC, VID_PID, 0x6064, 0, &offset.position_actual_value},
    {PANASONIC, VID_PID, 0x60B9, 0, &offset.touch_probe_status},
    {PANASONIC, VID_PID, 0x60BA, 0, &offset.touch_probe_pos1_pos_value},
    {PANASONIC, VID_PID, 0x60F4, 0, &offset.following_error_actual_value},
    {PANASONIC, VID_PID, 0x60FD, 0, &offset.digital_inputs},
    {}
};

/***************************************************************************/
/*Config PDOs*/
static ec_pdo_entry_info_t device_pdo_entries[] = {
    /*RxPdo 0x1600*/
    {0x6040, 0x00, 16},
    {0x6060, 0x00, 8 },
    {0x607A, 0x00, 32},
    {0x60B8, 0x00, 16},
    {0x6071, 0x00, 16},
    /*TxPdo 0x1A00*/
    {0x603F, 0x00, 16},
    {0x6041, 0x00, 16},
    {0x6061, 0x00, 8},
    {0x6064, 0x00, 32},
    {0x60B9, 0x00, 16},
    {0x60BA, 0x00, 32},
    {0x60F4, 0x00, 32},
    {0x60FD, 0x00, 32},
};

static ec_pdo_info_t device_pdos[] = {
    //RxPdo
    {0x1600, 5, device_pdo_entries + 0 },
    //TxPdo
    {0x1A00, 8, device_pdo_entries + 5 }
};

static ec_sync_info_t device_syncs[] = {
    { 0, EC_DIR_OUTPUT, 0, NULL, EC_WD_DISABLE },
    { 1, EC_DIR_INPUT, 0, NULL, EC_WD_DISABLE },
    { 2, EC_DIR_OUTPUT, 1, device_pdos + 0, EC_WD_ENABLE },
    { 3, EC_DIR_INPUT, 1, device_pdos + 1, EC_WD_DISABLE },
    { 0xFF}
};

static unsigned int counter = 0;
static unsigned int blink = 0;
static unsigned int sync_ref_counter = 0;
const struct timespec cycletime = {0, PERIOD_NS};

/*****************************************************************************/

struct timespec timespec_add(struct timespec time1, struct timespec time2)
{
    struct timespec result;

    if ((time1.tv_nsec + time2.tv_nsec) >= NSEC_PER_SEC) {
        result.tv_sec = time1.tv_sec + time2.tv_sec + 1;
        result.tv_nsec = time1.tv_nsec + time2.tv_nsec - NSEC_PER_SEC;
    } else {
        result.tv_sec = time1.tv_sec + time2.tv_sec;
        result.tv_nsec = time1.tv_nsec + time2.tv_nsec;
    }

    return result;
}

/*****************************************************************************/

void check_domain1_state(void)
{
    ec_domain_state_t ds;

    ecrt_domain_state(domain1, &ds);

    if (ds.working_counter != domain1_state.working_counter)
        printf("Domain1: WC %u.\n", ds.working_counter);
    if (ds.wc_state != domain1_state.wc_state)
        printf("Domain1: State %u.\n", ds.wc_state);

    domain1_state = ds;
}

/*****************************************************************************/

void check_master_state(void)
{
    ec_master_state_t ms;

    ecrt_master_state(master, &ms);

    if (ms.slaves_responding != master_state.slaves_responding)
        printf("%u slave(s).\n", ms.slaves_responding);
    if (ms.al_states != master_state.al_states)
        printf("AL states: 0x%02X.\n", ms.al_states);
    if (ms.link_up != master_state.link_up)
        printf("Link is %s.\n", ms.link_up ? "up" : "down");

    master_state = ms;
}

/****************************************************************************/

void check_slave_config_states(void)
{
    ec_slave_config_state_t s;
    ecrt_slave_config_state(sc, &s);
    if (s.al_state != sc_state.al_state)
    {
        printf("slave: State 0x%02X.\n", s.al_state);
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

/****************************************************************************/

void cyclic_task()
{
    int pos;
    static uint16_t command=0x004F;//用来帮助判断状态字的值
    uint16_t    status; //读取伺服状态

    struct timespec wakeupTime, time;
#ifdef MEASURE_TIMING
    struct timespec startTime, endTime, lastStartTime = {};
    uint32_t period_ns = 0, exec_ns = 0, latency_ns = 0,
             latency_min_ns = 0, latency_max_ns = 0,
             period_min_ns = 0, period_max_ns = 0,
             exec_min_ns = 0, exec_max_ns = 0;
#endif

    // get current time
    clock_gettime(CLOCK_TO_USE, &wakeupTime);

    while(1) {
        wakeupTime = timespec_add(wakeupTime, cycletime);
        clock_nanosleep(CLOCK_TO_USE, TIMER_ABSTIME, &wakeupTime, NULL);

        // Write application time to master
        //
        // It is a good idea to use the target time (not the measured time) as
        // application time, because it is more stable.
        //
        ecrt_master_application_time(master, TIMESPEC2NS(wakeupTime));

#ifdef MEASURE_TIMING
        clock_gettime(CLOCK_TO_USE, &startTime);
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
#endif

        // receive process data
        ecrt_master_receive(master);
        ecrt_domain_process(domain1);

        // check process data state (optional)
        //check_domain1_state();

        if (counter) {
            counter--;
        } else { // do this at 1 Hz
            counter = FREQUENCY;

        // check for master state (optional)
        //check_master_state();
        // check for slave configuration state(s)
        //check_slave_config_states();
        qDebug() << "actpos" <<pos << "offset " << offset.position_actual_value;
        qDebug() << "latency_ns" <<latency_ns <<  "period_ns" << period_ns << "exec_ns" << exec_ns;
        qDebug() << "latency_max" <<latency_max_ns <<  "period_max" << period_max_ns << "exec_max" << exec_max_ns;
#ifdef MEASURE_TIMING
            // output timing stats
            /*printf("period     %10u ... %10u\n",
                    period_min_ns, period_max_ns);
            printf("exec       %10u ... %10u\n",
                    exec_min_ns, exec_max_ns);
            printf("latency    %10u ... %10u\n",
                    latency_min_ns, latency_max_ns);*/
            period_max_ns = 0;
            period_min_ns = 0xffffffff;
            exec_max_ns = 0;
            exec_min_ns = 0xffffffff;
            latency_max_ns = 0;
            latency_min_ns = 0xffffffff;
#endif

            // calculate new process data
            blink = !blink;
        }
        /*Read state*/
        status = EC_READ_U16(domain1_pd + offset.status_word);//读取状态字
        pos = EC_READ_S32(domain1_pd + offset.position_actual_value);
        // write process data
        //DS402 CANOpen over EtherCAT status machine
        /*if( (status & command) == 0x0040 )
        {
            EC_WRITE_U16(domain1_pd + offset.ctrl_word, 0x0006 );
            EC_WRITE_S8(domain1_pd + offset.operation_mode, CYCLIC_POSITION);
        //set control mode
            command = 0x006F;
        }

        else if( (status & command) == 0x0021)
        {
            EC_WRITE_U16(domain1_pd + offset.ctrl_word, 0x0007 );
            command = 0x006F;
        }

        else if( (status & command) == 0x0023)
        {
            EC_WRITE_U16(domain1_pd + offset.ctrl_word, 0x000f );
            command = 0x006F;
        }
        //operation enabled

        else if( (status & command) == 0x0027)
        {
            EC_WRITE_U16(domain1_pd + offset.ctrl_word, 0x001f );
        }*/

        if (sync_ref_counter) {
            sync_ref_counter--;
        } else {
            sync_ref_counter = 1; // sync every cycle

            clock_gettime(CLOCK_TO_USE, &time);
            ecrt_master_sync_reference_clock_to(master, TIMESPEC2NS(time));
        }
        ecrt_master_sync_slave_clocks(master);

        // send process data
        ecrt_domain_queue(domain1);
        ecrt_master_send(master);

#ifdef MEASURE_TIMING
        clock_gettime(CLOCK_TO_USE, &endTime);
#endif
    }
}

/****************************************************************************/

int etc_main()
{
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(3,&mask);
    int tret;
    //
    //tret = pthread_setaffinity_np(pthread_self(),sizeof(mask),&mask);
    if (mlockall(MCL_CURRENT | MCL_FUTURE) == -1) {
        perror("mlockall failed");
        return -1;
    }

    master = ecrt_request_master(0);
    if (!master)
        return -1;
    qDebug() << "etc_main 1";
    domain1 = ecrt_master_create_domain(master);
    if (!domain1)
        return -1;

    // Create configuration for bus coupler
    sc = ecrt_master_slave_config(master, PANASONIC, VID_PID);
    if (!sc)
        return -1;
    ecrt_master_reset(master);
    printf("Configuring PDOs...\n");
    if (ecrt_slave_config_pdos(sc, EC_END, device_syncs))
    {
        fprintf(stderr, "Failed to configure slave PDOs!\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("*Success to configuring slave PDOs*\n");
    }

    if (ecrt_domain_reg_pdo_entry_list(domain1, domain1_regs))
    {
        fprintf(stderr, "PDO entry registration failed!\n");
        exit(EXIT_FAILURE);
    }

    // configure SYNC signals for this slave
    ecrt_slave_config_dc(sc, 0x0300, PERIOD_NS, 0, 0, 0);

    printf("Activating master...\n");
    if (ecrt_master_activate(master))
        return -1;

    if (!(domain1_pd = ecrt_domain_data(domain1))) {
        return -1;
    }

    /* Set priority */

    struct sched_param param = {};
    param.sched_priority = sched_get_priority_max(SCHED_FIFO);

    printf("Using priority %i.", param.sched_priority);
    if (sched_setscheduler(0, SCHED_FIFO, &param) == -1) {
        perror("sched_setscheduler failed");
    }

    printf("Starting cyclic function.\n");
    cyclic_task();

    return 0;
}

/****************************************************************************/
