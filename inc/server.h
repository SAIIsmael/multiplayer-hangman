#ifndef __SERVER_H
#define __SERVER_H

/* includes */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <pthread.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <errno.h>
/* end of includes */


/* colors define */
#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"
/* end of color define */


int tcpSend();
int tcpRecv();

key_t keyGenerator(int);
int semConnect(key_t , int);
int semGetVal(int);
int semSetIntVal(int, int);
int semLock(int);
int semUnlock(int);

int shmConnect();
int shmSend();
int shmRecv();

void* shmMaj(void*);


#endif
