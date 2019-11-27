#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <netinet/in.h>
#include <string.h>

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

#define BUFF_SIZE 256
#define SA struct sockaddr




struct gamestate {
        int error;
        int win;
        char word_to_find[4];
        int alreadyFound[4];
        int size;
        char word_found[4];
        int clients[4];
        int currentClient;
        int errormsg;

};
typedef struct dataGame dataG;

struct connInfos {
        int connfd;
        struct gamestate * gs;
};

int semP(short PVal, int idSem, int numSem){
        struct sembuf op[] = {
                { numSem, -PVal, SEM_UNDO } //lock
        };
        return semop(idSem, op, 0);

}

void semV(short VVal, int idSem, int numSem){
        struct sembuf op[] = {
                { numSem, VVal, SEM_UNDO }, //unlock
        };
        semop(idSem, op, 0);

}

void semZ(int idSem, int numSem){
        struct sembuf op[] = {
                { numSem, 0, SEM_UNDO }, //unlock
        };
        semop(idSem, op, 0);
}

char* getRandomWord(){
        char* words[16]= {"COIN",
                          "DONS",
                          "BANC",
                          "FEUX",
                          "HERO",
                          "GARE",
                          "DUEL",
                          "AVEC",
                          "CLEF",
                          "COIN",
                          "DOJO",
                          "DORS",
                          "EAUX",
                          "FEVE",
                          "FOUS",
                          "FORT"};
        srand(time(NULL));
        char * wordRet = words[(rand() % 15)];
        //char * wordRet = words[11];
        return wordRet;
}

void initGame(struct gamestate* gs){
        gs->size = 4;
        gs->error = 0;
        for (size_t i = 0; i < 4; i++) {
                gs->word_to_find[i] = getRandomWord()[i];
                gs->alreadyFound[i] = 0;
                gs->word_found[i] = '_';
                gs->clients[i] =-1;

        }
        gs->win = 0;
        gs->currentClient = -1;
        gs->errormsg = 0;
        printf("[Parent] Word to find : %s\n",gs->word_to_find );
        //  printf("%s\n",gs->word_found );

}

int sendall(int sock, const char* data, int data_length){
        int bytessend = 0;
        while ( bytessend < data_length) {
                int result = send(sock, data + bytessend, data_length - bytessend, 0);
                if ( result == -1) { perror("send error"); exit(1);} // not exit if errno == EAGAIN
                bytessend += result;
        }
        return bytessend;
}

int sendWithSize(int sock, const char* data, int data_length){
        char sizeToSend[4];
        *((int*) sizeToSend ) = data_length;
        sendall(sock, sizeToSend,sizeof(sizeToSend) );
        sendall(sock, data,data_length);
        return 1;
}

int recvAll(int socket, char *buf, int len, char* ip, int port) { // recvAll function
        int remaining = len;
        key_t keysem = ftok("shmfile", 10);
        int idSem = semget(keysem, 5, IPC_CREAT|0666);

        while (remaining) {
                int received = recv(socket, buf, remaining, 0);
                if (received <= 0) { semctl(idSem,5,SETVAL,semctl(idSem,5,GETVAL) -1); printf(MAG "[Quit] Client %s:%d disconnected !\n", ip, port);
                                     return 1;}
                buf += received;
                remaining -= received;
        }
        return 0;
}

int recvWithSize(int sock, char* data, char* ip, int port){
        char sizeToRecv[4];
        if(recvAll(sock, sizeToRecv, sizeof(sizeToRecv), ip, port) == 1) {return -1;};
        if(recvAll(sock, data, *((int*) sizeToRecv ), ip, port) == 1) {return -1;};
        printf("received : %s\n", data);
        return *((int*) sizeToRecv );
}

// Driver function

void sendStruct(int connfd, struct gamestate *gs){

        char intToSend[4];
        sprintf(intToSend, "%d", gs->error);
        printf("%s\n", intToSend);
        sendWithSize(connfd, intToSend, strlen(intToSend));
        bzero(intToSend, 4);

        sprintf(intToSend, "%d", gs->win);
        sendWithSize(connfd, intToSend, strlen(intToSend));
        bzero(intToSend, 4);

        sprintf(intToSend, "%d", gs->size);
        sendWithSize(connfd, intToSend, strlen(intToSend));
        bzero(intToSend, 4);

        for (size_t i = 0; i < gs->size; i++) {
                sprintf(intToSend, "%d", gs->alreadyFound[i]);
                sendWithSize(connfd, intToSend, strlen(intToSend));
                bzero(intToSend, 4);
        }

        char *wf = &gs->word_found[0];
        sendWithSize(connfd, wf, strlen(gs->word_found));
        sendWithSize(connfd, gs->word_to_find, strlen(gs->word_to_find));

        sprintf(intToSend, "%d", gs->errormsg);
        sendWithSize(connfd, intToSend, strlen(intToSend));
        bzero(intToSend, 4);

}

void executePlay( int nLetter, char letter, struct gamestate* gs){
        if( gs->errormsg > 0 ) { gs->errormsg = 0;}
        printf("User chose to put letter %c at place %d \n", letter, nLetter);
        if(gs->word_to_find[nLetter-1] == letter) {
                gs->word_found[nLetter-1] = letter;
        }
        else{ gs->error++;}
        if ( gs->error == 8) { gs->win = -1;}
        char wfound[4];

        for (int i = 0; i < 4; ++i)
        {
                wfound[i] = gs->word_found[i];
        }

        if ( strcmp(gs->word_to_find, wfound) == 0) { gs->win = 1;}
        printf("%s\n", wfound);
        printf("%s\n", gs->word_to_find);

}


void* updateThread(void* param){
        struct connInfos *c = (struct connInfos *) param;
        struct sembuf opP[] = {
                {4, -1, SEM_UNDO},
                {4, 0, SEM_UNDO}
        };
        key_t keysem = ftok("shmfile", 10);
        int idSem = semget(keysem, 5, IPC_CREAT|0666);

        while (1) {
                if ( semctl(idSem, 4, GETVAL) > 0 ) {
                        //  printf("update needed -> %d \n", semctl(idSem, 5, GETVAL));
                        semop(idSem, opP, 1);
                        sendStruct(c->connfd, c->gs);
                        printf("waiting for all clients updated (%d/%d )\n",semctl(idSem, 4, GETVAL), semctl(idSem, 5, GETVAL) );
                        semop(idSem, opP+1, 1);
                }

        }
}

void sendNextSTep(int connfd, char* ip, int port, struct gamestate* gs){
        key_t keysem = ftok("shmfile", 10);
        int idSem = semget(keysem, 5, IPC_CREAT|0666);
        for (size_t i = 0; i < 6; i++) {
                printf("idSem at %zu -> %d \n", i, semctl(idSem, i, GETVAL) );
        }
        struct sembuf opP[] = {
                {0, -1, SEM_UNDO},
                {1, -1, SEM_UNDO},
                {2, -1, SEM_UNDO},
                {3, -1, SEM_UNDO}
        };

        struct sembuf opV[] = {
                {0, 1, SEM_UNDO},
                {1, 1, SEM_UNDO},
                {2, 1, SEM_UNDO},
                {3, 1, SEM_UNDO}
        };

        while(1) {
                char intRec;
                int letterNum;
                printf("waiting user letter num ..");
                recvWithSize(connfd, &intRec, ip, port);
                letterNum = intRec - '0';
                printf("letter from user : %d\n", letterNum);
                printf("%d",semctl(idSem, letterNum-1, GETVAL));

                if ( semctl(idSem, letterNum-1, GETVAL) == 0 ) {
                        printf(MAG "you can't edit this letter for the moment \n");
                        gs->errormsg = letterNum;
                        sendStruct(connfd, gs);
                        bzero(&intRec,1);
                        fflush(stdout);
                        fflush(stdin);
                }

                else {
                        if(gs->errormsg != letterNum) {
                                if ( semop(idSem, opP+(letterNum-1), 1) < 0 ) { perror("op P : "); exit(1);}

                                printf("letter %d lock, semaphore value : %d\n", letterNum-1,semctl(idSem, letterNum-1, GETVAL));

                                char playRec;
                                bzero(&playRec,1);
                                printf("waiting user letter ..");
                                recvWithSize(connfd, &playRec, ip, port);
                                printf("play from user : %c\n", playRec);
                                char letter = playRec;
                                bzero(&playRec,1);

                                executePlay(letterNum,letter, gs);

                                semctl(idSem, 4, SETVAL, semctl(idSem, 5, GETVAL));
                                printf("we need update for %d clients \n", semctl(idSem, 4, GETVAL));
                                if ( semop(idSem, opV+(letterNum-1), 1) < 0 ) { perror("op P : "); exit(1);}
                                printf("letter %d unlock, semaphore value : %d\n", letterNum-1,semctl(idSem, letterNum-1, GETVAL));
                                //sendStruct(connfd, gs);
                        }
                }

        }
}

int main(int argc, char* argv[])
{
        int father = getpid();
        if ( argc < 3 ) {
                printf(MAG "[Error] Unrecognize command. \n");
                printf(CYN "[Usage] ./tcpServer [portServer][nbClients] \n");
                printf(MAG "portServer:" WHT "Port you want to use for your server\n");
                printf(MAG "nbClients:" WHT "How many simultaneous clients can connect to your server \n");
                printf(MAG "[Error] Program exited. \n");
                exit(1);
        }

        int sockfd;
        int nbClients = atoi(argv[2]);
        struct sockaddr_in servaddr;
        char buff[BUFF_SIZE];

        // socket create and verification
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1) {
                printf("socket creation failed...\n");
                exit(0);
        }
        else
                bzero(&servaddr, sizeof(servaddr));

        // assign IP, PORT
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        servaddr.sin_port = htons(atoi(argv[1]));

        char * ipServ = inet_ntoa(servaddr.sin_addr);
        // Binding newly created socket to given IP and verification
        if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
                perror("socket bind failed...\n");
                exit(0);
        }
        // Now server is ready to listen and verification
        key_t key = ftok("shmfile",65);
        int shmid = shmget(key,sizeof(struct gamestate),0666|IPC_CREAT);
        if(shmid < 0) {
                printf("Error creating shared memory");
                exit(1);
        }
        struct gamestate *ptrToGame;
        ptrToGame = shmat(shmid, NULL, 0);
        initGame(ptrToGame);

        key_t keysem = ftok("shmfile", 10);
        int idSem = semget(keysem, 6, IPC_CREAT|0666);
        union semun egCtrl;
        egCtrl.array = malloc(sizeof(unsigned short)*4);
        for (size_t i = 0; i < 4; i++) {
                egCtrl.array[i] = 1;
        }
        egCtrl.array[4]= 0;
        egCtrl.array[5]=0;
        semctl(idSem, 0, SETALL, egCtrl);

        printf(CYN "\n==========================================\n");
        printf(CYN "|            SERVER %s:%d         |\n", ipServ,htons(servaddr.sin_port));
        printf(CYN "==========================================\n");
        while(1) {
                if ((listen(sockfd, nbClients)) != 0) {
                        printf("Listen failed...\n");
                        exit(0);
                }

                struct sockaddr_in cli;
                int len = sizeof(cli);
                //  int connfd;

                // Accept the data packet from client and verification
                int connfd = accept(sockfd, (SA*)&cli, (socklen_t* ) &len);
                if (connfd < 0) {
                        printf("server acccept failed...\n");
                        exit(0);
                }

                int son;
                ptrToGame->currentClient++;
                ptrToGame->clients[ptrToGame->currentClient] = connfd;

                if ( (son = fork()) == 0) {
                        key_t key = ftok("shmfile.txt",65);
                        int shmid = shmget(key,sizeof(struct gamestate),0666|IPC_CREAT);
                        key_t keysem = ftok("shmfile", 10);
                        int idSem = semget(keysem, 5, IPC_CREAT|0666);
                        if(shmid < 0) {

                                perror("Error creating shared memory");
                                exit(1);
                        }
                        semctl(idSem, 5, SETVAL, semctl(idSem, 5, GETVAL) + 1);
                        ptrToGame = shmat(shmid, NULL, 0);

                        close(sockfd);
                        printf(GRN "[Connection] client %s:%d connected.\n", inet_ntoa(cli.sin_addr),htons(cli.sin_port));
                        char *SelectFun[4];
                        pthread_t update;
                        struct connInfos c;
                        c.connfd = connfd;
                        c.gs = ptrToGame;

                        recvWithSize(connfd, buff, inet_ntoa(cli.sin_addr),cli.sin_port);
                        bzero(SelectFun, 4);

                        sendStruct(connfd, ptrToGame);

                        if (pthread_create(&update, NULL, updateThread, &c) < 0) {
                                fprintf(stderr, "    * [Main] pthread_create error for thread 1 * %s\n",
                                        " ");
                                exit(1);
                        }

                        sendNextSTep(connfd, inet_ntoa(cli.sin_addr),cli.sin_port, ptrToGame);
                        struct sembuf op[] = {
                                {5, -1, SEM_UNDO}

                        };
                        semop(idSem,op,1);
                        printf("%d\n",semget(idSem,5,GETVAL) );

                }
                if ( son == father ) {wait(&son);}


        }

        return 0;
}
