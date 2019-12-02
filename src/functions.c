#include "../inc/functions.h"

#define BUFF_SIZE 256
#define SA struct sockaddr

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
        return wordRet;
}

int getSem(){
        key_t keysem = ftok("../inc/shmfile.txt", 10);
        if ( keysem < 0 ) {
                perror("key generation error\n");
                exit(1);
        }
        int semId = semget(keysem, 6, IPC_CREAT|0666);
        if(semId < 0 ) {
                perror("semget error \n");
                exit(1);
        }
        return semId;
}

int getSharedMemory(){


        key_t key = ftok("../inc/shmfile.txt",65);
        int shmid = shmget(key,sizeof(struct gamestate),0666|IPC_CREAT);
        if(shmid < 0) {
                printf("Error creating shared memory");
                exit(1);
        }
        return shmid;

}
void initGame(struct gamestate* gs){
        gs->size = 4;
        gs->error = 0;
        gs->win = 0;

        for (size_t i = 0; i < 4; i++) {
                gs->word_to_find[i] = getRandomWord()[i];
                gs->alreadyFound[i] = 0;
                gs->word_found[i] = '_';
                gs->errormsg[i] = 0;
        }
        printf("[PROC %d] Word to find : %s\n",getpid(),gs->word_to_find );
}

int sendall(int sock, const char* data, int data_length){
        int bytessend = 0;
        while ( bytessend < data_length) {
                int result = send(sock, data + bytessend, data_length - bytessend, 0);
                if ( result == -1) { perror("send error"); exit(1);}    // not exit if errno == EAGAIN
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

int recvAll(int socket, char *buf, int len, char* ip, int port) {    // recvAll function
        int remaining = len;
  key_t keysem = ftok("shmfile", 10);
        int idSem = semget(keysem, 7, IPC_CREAT|0666); 
        while (remaining) {
                int received = recv(socket, buf, remaining, 0);
                if (received <= 0) {
                        semctl(idSem,5,SETVAL,semctl(idSem,5,GETVAL) -1);
                        printf(MAG "[Quit] Client %s:%d disconnected !\n", ip, port);
                        exit(1);
                }
                buf += received;
                remaining -= received;
        }
        return 0;
}

int recvWithSize(int sock, char* data, char* ip, int port){
        char sizeToRecv[4];
        if(recvAll(sock, sizeToRecv, sizeof(sizeToRecv), ip, port) == 1) {return -1;};
        if(recvAll(sock, data, *((int*) sizeToRecv ), ip, port) == 1) {return -1;};
        return *((int*) sizeToRecv );
}

void sendStruct(int connfd, struct gamestate *gs){
        char intToSend[4];
        sprintf(intToSend, "%d", gs->error);
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

        for (size_t i = 0; i < gs->size; i++) {
                sprintf(intToSend, "%d", gs->errormsg[i]);
                sendWithSize(connfd, intToSend, strlen(intToSend));
                bzero(intToSend, 4);
        }

}

void executePlay( int nLetter, char letter, struct gamestate* gs){
        printf(CYN "[PROC %d] User chose to put letter %c at place %d \n", getpid(),letter, nLetter);
        if(gs->word_to_find[nLetter-1] == letter) {
                
                gs->word_found[nLetter-1] = toupper(letter);
        }
        else{
                gs->error++;
        }
        if( gs->error == 8) {
                gs->win = -1;
        }

        char wfound[4];
        for (int i = 0; i < 4; ++i) {
                wfound[i] = gs->word_found[i];
        }
        if ( strcmp(gs->word_to_find, wfound) == 0) {
                gs->win = 1;
        }
}

void* updateThread(void* param){
        struct connInfos *c = (struct connInfos *) param;

        struct sembuf opP[] = {
                {4, -1, 0},
                {4, 0, 0}
        };

          key_t keysem = ftok("shmfile", 10);
        int idSem = semget(keysem, 7, IPC_CREAT|0666);

        while (1) {
                if ( semctl(idSem, 4, GETVAL) > 0 ) {
                        printf(GRN "[PROC %d ] Update sent ! \n",getpid());
                        semop(idSem, opP, 1);
                        sendStruct(c->connfd, c->gs);
                        printf(YEL "[PROC %d ] waiting for all clients updated (%d/%d )\n",getpid(),semctl(idSem, 4, GETVAL), semctl(idSem, 5, GETVAL) );
                        semop(idSem, opP+1, 1);
                        printf(GRN "[PROC %d ] all client are updated \n", getpid());
                }

        }
}
void sendNextSTep(int connfd, char* ip, int port, struct gamestate* gs){
        key_t keysem = ftok("shmfile", 10);
        int idSem = semget(keysem, 7, IPC_CREAT|0666);
        for (size_t i = 0; i < 6; i++) {
                printf("idSem at %zu -> %d \n", i, semctl(idSem, i, GETVAL) );
        }
        struct sembuf opP[] = {
                {0, -1, SEM_UNDO},
                {1, -1, SEM_UNDO},
                {2, -1, SEM_UNDO},
                {3, -1, SEM_UNDO},
                {6, -1, SEM_UNDO}
        };

        struct sembuf opV[] = {
                {0, 1, SEM_UNDO},
                {1, 1, SEM_UNDO},
                {2, 1, SEM_UNDO},
                {3, 1, SEM_UNDO},
                {6, 1, SEM_UNDO}
        };

        while(1) {
                char intRec;
                int letterNum;
                bzero(&intRec,1);
                printf("waiting user letter num ..");
                recvWithSize(connfd, &intRec, ip, port);
                letterNum = (int) intRec - 48;
                printf("letter from user : %d\n", letterNum);
                bzero(&intRec,1);

                 semop(idSem, opP+4, 1);
                int error = semctl(idSem, letterNum-1, GETVAL);
                 semop(idSem, opV+4, 1);

                if ( error == 0 ) {
                        printf(MAG "you can't edit this letter for the moment \n");
                        gs->errormsg[letterNum-1] = letterNum;
                        printf("error at %d -> %d \n", letterNum-1, letterNum);
                        sendStruct(connfd, gs);
                        fflush(stdout);

                }
                else {
                     sendStruct(connfd, gs);
                        if(gs->errormsg[letterNum-1] == 0) {
                                if ( semop(idSem, opP+(letterNum-1), 1) < 0 ) { perror("op P : "); exit(1);}

                              printf("letter %d lock, semaphore value : %d\n", letterNum-1,semctl(idSem, letterNum-1, GETVAL));
                                if ( connfd <= 0 ){ semop(idSem, opV+(letterNum-1), 1);}
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