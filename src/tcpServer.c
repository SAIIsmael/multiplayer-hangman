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

};
typedef struct dataGame dataG;

struct dataGame {
        int dSclient;
        struct gamestate *gs;
        int port;
        char* Rdata;
        char* ip;
};

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

        while (remaining) {
                int received = recv(socket, buf, remaining, 0);
                if (received <= 0) { printf(MAG "[Quit] Client %s:%d disconnected !\n", ip, port); return 1;}
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

}

void executePlay( int nLetter, char letter, struct gamestate* gs){
        printf("User chose to put letter %c at place %d \n", letter, nLetter);
        if(gs->word_to_find[nLetter-1] == letter) {
                gs->word_found[nLetter-1] = letter;
        }
        else{ gs->error++;}
}

void sendNextSTep(int connfd, char* ip, int port, struct gamestate* gs){

        while(1) {

                char intRec;
                int letterNum;
                printf("waiting user letter num ..");
                recvWithSize(connfd, &intRec, ip, port);
                letterNum = intRec - '0';
                printf("letter from user : %d\n", letterNum);

                char playRec;
                printf("waiting user letter ..");
                recvWithSize(connfd, &playRec, ip, port);
                printf("play from user : %c\n", playRec);
                char letter = playRec;

                executePlay(letterNum,letter, gs);
                //    sendStruct(connfd, gs);
                for (size_t i = 0; i < 4; i++) {
                        if ( gs->clients[i] != -1) {
                                printf("[client %zu] -> %d sending update \n", i, gs->clients[i]);
                                sendStruct(gs->clients[i], gs);
                                printf("client %zu updated\n", i);
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
                        key_t key = ftok("shmfile",65);
                        int shmid = shmget(key,sizeof(struct gamestate),0666|IPC_CREAT);
                        if(shmid < 0) {
                                printf("Error creating shared memory");
                                exit(1);
                        }

                        ptrToGame = shmat(shmid, NULL, 0);
                        close(sockfd);
                        printf(GRN "[Connection] client %s:%d connected.\n", inet_ntoa(cli.sin_addr),htons(cli.sin_port));
                        char *SelectFun[4];


                        recvWithSize(connfd, buff, inet_ntoa(cli.sin_addr),cli.sin_port);
                        bzero(SelectFun, 4);

                        sendStruct(connfd, ptrToGame);

                        sendNextSTep(connfd, inet_ntoa(cli.sin_addr),cli.sin_port, ptrToGame);
                }
                if ( son == father ) {wait(&son);}


        }

        return 0;
}
