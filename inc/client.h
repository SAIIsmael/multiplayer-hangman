#ifndef __CLIENT_H
#define __CLIENT_H

/* includes */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <pthread.h>
#include <fcntl.h>
#include <semaphore.h>
/* end of includes */


int tcpConnect();
int tcpSend();
int tcpRecv();



#endif
