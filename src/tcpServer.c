#include "../inc/functions.h"

#define BUFF_SIZE 256
#define SA struct sockaddr




int main(int argc, char* argv[])
{
        int father = getpid();
        if ( argc < 2 ) {
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

        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1) {
                printf("socket creation failed...\n");
                exit(0);
        }
        else
                bzero(&servaddr, sizeof(servaddr));

        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        servaddr.sin_port = htons(atoi(argv[1]));

        char * ipServ = inet_ntoa(servaddr.sin_addr);

        if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
                perror("socket bind failed...\n");
                exit(0);
        }

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
        int idSem = semget(keysem, 7, IPC_CREAT|0666);
        union semun egCtrl;
        egCtrl.array = malloc(sizeof(unsigned short)*4);
        for (size_t i = 0; i < 4; i++) {
                egCtrl.array[i] = 1;
        }
        egCtrl.array[4]= 0;
        egCtrl.array[5]=0;
        egCtrl.array[6]=1;
        semctl(idSem, 0, SETALL, egCtrl);

        printf(CYN "\n==========================================\n");
        printf(CYN "|            SERVER %s:%d         |\n", ipServ,htons(servaddr.sin_port));
        printf(CYN "==========================================\n");
        while(1) {
                if ((listen(sockfd, nbClients)) != 0) {
                        printf("Listen failed...\n");
                        exit(0);
                       if ( semctl(idSem, 6, IPC_RMID) < 0){
                        perror("error sem delete");
                        exit(0);
                       }
                       if ( shmctl(shmid, IPC_RMID, NULL) < 0 ){
                        perror("error shm delete");
                        exit(0);
                       }
                       close(sockfd);

                }

                struct sockaddr_in cli;
                int len = sizeof(cli);

                int connfd = accept(sockfd, (SA*)&cli, (socklen_t* ) &len);
                if (connfd < 0) {
                        printf("server acccept failed...\n");
                        //exit(0);
                }

                int son;

                if ( (son = fork()) == 0) {
                        key_t key = ftok("shmfile.txt",65);
                        int shmid = shmget(key,sizeof(struct gamestate),0666|IPC_CREAT);
                        key_t keysem = ftok("shmfile", 10);
                        int idSem = semget(keysem, 7, IPC_CREAT|0666);
                        if(shmid < 0) {

                                perror("Error creating shared memory");
                                printf("Listen failed...\n");
                                exit(0);
                                if ( semctl(idSem, 6, IPC_RMID) < 0){
                                perror("error sem delete");
                                exit(0);
                                }
                                if ( shmctl(shmid, IPC_RMID, NULL) < 0 ){
                                perror("error shm delete");
                                exit(0);
                                 }
                       close(sockfd);

                                exit(1);
                        }

                        semctl(idSem, 5, SETVAL, semctl(idSem, 5, GETVAL) + 1);
                        ptrToGame = shmat(shmid, NULL, 0);

                        close(sockfd);
                        printf(GRN "[Connection] client %s:%d connected.\n", inet_ntoa(cli.sin_addr),htons(cli.sin_port));
                       
                        pthread_t update;
                        struct connInfos c;
                        c.connfd = connfd;
                        c.gs = ptrToGame;


                        sendStruct(connfd, ptrToGame);

                        if (pthread_create(&update, NULL, updateThread, &c) < 0) {
                                fprintf(stderr, "    * [Main] pthread_create error for thread 1 * %s\n",
                                        " ");
                                exit(1);
                                                       printf("Listen failed...\n");
                        exit(0);
                       if ( semctl(idSem, 6, IPC_RMID) < 0){
                        perror("error sem delete");
                        exit(0);
                       }
                       if ( shmctl(shmid, IPC_RMID, NULL) < 0 ){
                        perror("error shm delete");
                        exit(0);
                       }
                       close(sockfd);

                        }

                        sendNextSTep(connfd, inet_ntoa(cli.sin_addr),cli.sin_port, ptrToGame);
                        struct sembuf op[] = {
                                {5, -1, SEM_UNDO}

                        };
                        semop(idSem,op,1);
                        printf("%d\n",semget(idSem,5,GETVAL) );

                }
                if ( son == father ) {wait(&son);
                                       printf("Listen failed...\n");
                        exit(0);
                       if ( semctl(idSem, 6, IPC_RMID) < 0){
                        perror("error sem delete");
                        exit(0);
                       }
                       if ( shmctl(shmid, IPC_RMID, NULL) < 0 ){
                        perror("error shm delete");
                        exit(0);
                       }
                       close(sockfd);

}


        }

        return 0;
}
