#ifndef ETHERCARTEST_H
#define ETHERCARTEST_H

#include "ecrt.h"
#include <QDebug>

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <sys/mman.h>
#include <malloc.h>
#include <sched.h> /* sched_setscheduler() */

/****************************************************************************/

/****************************************************************************/

int etc_main();


class ethercarTest
{
public:
    ethercarTest();
};

#endif // ETHERCARTEST_H
