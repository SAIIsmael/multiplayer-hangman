#include "../inc/functions.h"

#define BUFF_SIZE 256
#define SA struct sockaddr

/*! \struct clientCInfos
 * \brief Connection informations
 *
 *  This structure gives connections informations for the client side thread
 */

struct connInfosCli {
        int sockfd; /*!< socket of the server  */
        struct gamestate * gs; /*!< game status  */
        char * buff; /*!< reception buffer */
        char * ip; /*!< server ip */
        int port; /*!< server port */
        struct printStatus *ps; /*!< print status */
        pthread_mutex_t *lock; /*!< mutex synchro print and update */
        pthread_cond_t *cond; /*!<  cond  synchro print and update*/

};

/*!
 *  \brief recvAll
 *
 *  TCP recv all
 *  \param int socket : socket of the server
 *  \param char* buf : reception buffer 
 *  \param int len : size of the message
 *  \param char* ip : server ip
 *  \param int port : server port 
 *  \return int : 0 if succeed
 */
int recvAll(int socket, char *buf, int len, char* ip, int port) { // recvAll function
        int remaining = len;

        while (remaining) {
                int received = recv(socket, buf, remaining, 0);
                if (received <= 0) { printf(MAG "[Quit] Server %s:%d disconnected !\n", ip, port); exit(1);}
                buf += received;
                remaining -= received;
        }
        return 0;
}

/*!
 *  \brief recv with size 
 *
 *  TCP recv size than recv message 
 *  \param int socket : socket of the server
 *  \param char* data : reception buffer 
 *  \param char* ip : server ip
 *  \param int port : server port 
 *  \return int : 0 if succeed
 */
int recvWithSize(int sock, char* data, char* ip, int port){
        char sizeToRecv[4];
        if(recvAll(sock, sizeToRecv, sizeof(sizeToRecv), ip, port) == 1) {return -1;};
        if(recvAll(sock, data, *((int*) sizeToRecv ), ip, port) == 1) {return -1;};
        return *((int*) sizeToRecv );
}

/*!
 *  \brief sendall
 *
 *  TCP send all
 *  \param int socket : socket of the current client
 *  \param const char* message : message to send
 *  \param int size : size of the message
 *  \return bytessend : number of bytes sent
 */
int sendall(int sock, const char* data, int data_length){
        int bytessend = 0;
        while ( bytessend < data_length) {
                int result = send(sock, data + bytessend, data_length - bytessend, 0);
                if ( result == -1) { perror("send error"); exit(1);} // not exit if errno == EAGAIN
                bytessend += result;
        }
        return bytessend;
}
/*!
 *  \brief sendWithSize
 *
 *  TCP send size than send message
 *  \param int socket : socket of the current client
 *  \param const char* message : message to send
 *  \param int size : size of the message
 *  \return int : 1 if succeed
 */
int sendWithSize(int sock, const char* data, int data_length){
        char sizeToSend[4];
        *((int*) sizeToSend ) = data_length;
        sendall(sock, sizeToSend,sizeof(sizeToSend) );
        sendall(sock, data,data_length);
        return 1;
}

/*!
 *  \brief recvStruct
 *
 *  TCP receive all the structure
 *  \param int socket : socket of the current client
 *  \param struct gamestate* : struct to send
 *  \param char * ip : ip of the sender
 *  \param int port  : port of the sender
 */
void recvStruct(int sockfd, struct gamestate *gs, char* buff, char * ip, int port){
        recvWithSize(sockfd, buff, ip, port);
        gs->error = atoi(buff);
        bzero(buff, BUFF_SIZE);

        recvWithSize(sockfd, buff, ip, port);
        gs->win = atoi(buff);
        bzero(buff, BUFF_SIZE);

        recvWithSize(sockfd, buff, ip, port);
        gs->size = atoi(buff);
        bzero(buff, BUFF_SIZE);

        for (size_t i = 0; i < gs->size; i++) {
                recvWithSize(sockfd, buff, ip, port);
                gs->alreadyFound[i] = atoi(buff);
                bzero(buff, BUFF_SIZE);
        }

        recvWithSize(sockfd, buff, ip, port);
        for (size_t i = 0; i < strlen(buff); i++) {
                gs->word_found[i]=buff[i];
        }
        bzero(buff, BUFF_SIZE);

        recvWithSize(sockfd, buff, ip, port);
        strcpy(gs->word_to_find, buff);
        bzero(buff, BUFF_SIZE);

        for (size_t i = 0; i < gs->size; i++) {
                recvWithSize(sockfd, buff, ip, port);
                gs->errormsg[i] = atoi(buff);
                bzero(buff, BUFF_SIZE);
        }
}

/*!
 *  \brief errorPrint
 *
 *  print the hangman ascii art
 *  \param int error : number of error to print the right hangman ascii art
 */
void errorPrint(int error){
        char HANG_STATES[7][10 * 9] =
        {
                "             +         +----     +----     +----     +----     +----     +----     +----  ",
                "             |         |         |   O     |   O     |   O     |   O     |   O     |   O  ",
                "             |         |         |         |   +     | --+     | --+--   | --+--   | --+--",
                "             |         |         |         |   |     |   |     |   |     |   |     |   |  ",
                "             |         |         |         |         |         |         |  /      |  / \\ ",
                "             |         |         |         |         |         |         |         |      ",
                "/*****\\   /*****\\   /*****\\   /*****\\   /*****\\   /*****\\   /*****\\   /*****\\   /*****\\   "
        };

        for (size_t i = 0; i < 7; i++) {
                printf("%.10s \n", &HANG_STATES[i][error*10]);
        }
}

/*!
 *  \brief printWF
 *
 *  print word found 
 *  \param struct gamestate* gs : pointer on the game
 */
void printWF(struct gamestate* gs){
        for (size_t i = 0; i < 4; i++) {
                printf("%c", gs->word_found[i]);
        }
}

/*!
 *  \brief printGUI
 *
 *  print the GUI
 *  \param char* s : string to cover
 */
void printGUI(char* s){
        for (size_t i = 0; i < strlen(s); i++) {
                printf("-");
        }
}

/*!
 *  \brief gamePrint
 *
 *  print the game
 *  \param struct gamestate* gs : pointer to the structure
 *  \param int sockfd : server socket

 */
void gamePrint(struct gamestate* gs, int sockfd){
        switch (gs->win) {
        case 0:
                printf(WHT "\n==========================================\n ");
                printf(WHT "|"CYN "                  HANGMAN               "WHT "|\n");
                printf(WHT "==========================================\n\r");
                printf("\n\n\n"); errorPrint(gs->error);
                printf("\n\n\n");
                printf("+-------------"); printGUI(gs->word_found); printf("-------------+"); printf("\t"); printf("+--------------------+\n");
                printf("|%8s  Word : "MAG "%s "WHT "%8s |", "",gs->word_found,""); printf("\t"); printf(WHT "|    error : "MAG "%d"WHT "/8     |\n",gs->error);
                printf(WHT "+-------------"); printGUI(gs->word_found); printf("-------------+"); printf("\t"); printf("+--------------------+\n");

                break;
        case 1:
                printf(WHT "\n=========================================\n");
                printf(WHT "|"GRN "                  WIN!!!                "WHT "|\n");
                printf(WHT "=========================================\n");
                printf("The word was  : "GRN "%s", gs->word_to_find);
                printf(WHT "\nYou found the word ! " GRN "Congrats ! \n");
                break;
        case -1:
                printf(WHT "\n=========================================\n");
                printf(WHT "|"GRN "                  LOSE!!                "WHT "|\n");
                printf(WHT "=========================================\n");
                printf("The word was  : "MAG "%s", gs->word_to_find);
                printf(WHT "\nYou" MAG "DID NOT" WHT "found the word ! " MAG "Too bad ! \n");
                break;
        }

}

/*!
 *  \brief updateThread
 *
 *  TCP receive update when needed
 *  \param void * params : connections params
 */
void * threadUpdate(void* param){
        while(1) {
                struct connInfosCli * ci = (struct connInfosCli *)param;
                recvStruct(ci->sockfd, ci->gs, ci->buff,ci->ip, ci->port);
                gamePrint(ci->gs, ci->sockfd);
                if ( ci->ps->step == 1){
                 printf("Number > ");
                 fflush(stdout);
                }else{
                    printf("Letter >");
                 fflush(stdout);

                }
                pthread_cond_broadcast(ci->cond);
   

        }
}   
/*!
 *  \brief recvNextStep
 *
 *  get user play and send it to the server
 *  \param int sockfd: server socket
 *  \param char * buff: reception buffer
 *  \param struct gamestate* : game status
 *  \param char * ip : server ip
 *  \param int port: server port
 *  \param struct PrintStatus: print status
 *  \param pthread_mutex_lock *: pointer to mutex 
 *  \param pthread_mutex_cond *: pointer to cond 
 */
void recvNextStep(int sockfd, char* buff, struct gamestate *gs, char* ip, int port, struct printStatus *ps,pthread_mutex_t *lock,pthread_cond_t* cond ){
        while(1) {
                char nLetter;
                char intToSend[4];
                int number;

                ps->step = 1;
                scanf("%c", &nLetter);
                number = (int) nLetter - 48;
                while(number > 4 ) {
                    char ch;
                    while(  ch != '\n' && getchar() != '\n' ){ /* flush to end of input line */ }

                    printf("veuillez  saisir un nombrre entre 1 et 4  \n");
                    scanf("%c", &nLetter);
                    number = (int) nLetter - 48;

                    while(  ch != '\n' && getchar() != '\n' ){ /* flush to end of input line */ }

                }

                sendWithSize(sockfd, &nLetter, 1);
                 ps->step = 2;
                bzero(intToSend, 4);
                number = (int) nLetter - 48;
                 ps->nLetter = number-1;

                pthread_cond_wait(cond, lock);
                char letter;
                printf("%d \n", number);
                 if ( gs->errormsg[number-1] != 0) {

                        printf(MAG "Letter already in use send. Chose another letter. \n"RESET);
                        printf(WHT "(if you just logged in just ignore this message )\n");
                        printf(WHT "Number > ");
                        fflush(stdout);

                    }


                if ( gs->errormsg[number-1] == 0) {
                        scanf("%c", &letter);
                            char ch;
                            while( ch != '\n' && getchar() != '\n' ){ /* flush to end of input line */ }
                            
                            printf("veuillez  saisir une lettre \n");
                        char lettertosend = letter;
                        sendWithSize(sockfd, &lettertosend, 1);
                        bzero(&letter, 1);
                }


                if ( gs->win == -1 ) { break;}
                if ( gs->win == 1 ) { break;}
        }
}

int main(int argc, char* argv[])
{
        int sockfd;
        char buff[BUFF_SIZE];
        bzero(buff, BUFF_SIZE);
        struct sockaddr_in servaddr;
        if ( argc < 3 ) {
                printf(MAG "[Error] Unrecognize command. \n");
                printf(CYN "[Usage] ./tcpClient [ipServer][portServer] \n");
                printf(MAG "[Error] Program exited. \n");
                exit(1);

        }

        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1) {
                printf("socket creation failed...\n");
                exit(0);
        }
        bzero(&servaddr, sizeof(servaddr));

        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = inet_addr(argv[1]);
        servaddr.sin_port = htons(atoi(argv[2]));

        printf(CYN "\n=========================================\n");
        printf(CYN "|                  CLIENT                |\n");
        printf(CYN "=========================================\n");
        if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
                printf("connection with the server failed...\n");
                exit(0);
        }
        else

                printf(GRN "[Connection] connected to the server %s:%d\n", argv[1], atoi(argv[2]));
        struct gamestate gs;
        struct printStatus ps;

        ps.step = 1;
        ps.nLetter = 0;

        pthread_mutex_t lock;
        pthread_mutex_init(&lock, NULL);
        pthread_cond_t cond;
        pthread_cond_init(&cond, NULL);
        struct connInfosCli ci;
        ci.sockfd = sockfd;
        ci.buff = buff;
        ci.gs = &gs;
        ci.ip =  argv[1];
        ci.port = atoi(argv[2]);
        ci.ps = &ps;
        ci.lock =&lock;
        ci.cond = &cond;

        pthread_t updt;



        if (pthread_create(&updt, NULL, threadUpdate, &ci) < 0) {
                fprintf(stderr, "    * [Main] pthread_create error for thread 1 * %s\n",
                        " ");
                exit(1);
        }
        recvNextStep(sockfd,buff, &gs, argv[1], atoi(argv[2]), &ps, &lock, &cond);

        if ( gs.win == -1 || gs.win == 1) {   system("clear");
                                              gamePrint(&gs, sockfd);}
        printf(MAG "[Quit] client closed. Bye bye.\n");
        printf(RESET "\n" );
        return 0;
}
