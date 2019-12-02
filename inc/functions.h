#ifndef __FUNCTIONS_H
#define __FUNCTIONS_H


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
#include <sys/types.h>
#include <sys/wait.h>

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

union semun {
        int val;
        struct semid_ds *buf;
        unsigned short * array;
        struct seminfo * _buf;
   };

struct printStatus {
        int step;
        int nLetter;
        int iWas;
};

/*! \struct gamestate
 * \brief game status
 *
 *  This structure gives the status of the game at each turn.
 */
struct gamestate {
        int error; /*!< error counter */
        int win; /*!< win status : -1 lose, 0 in progress, 1 win */
        char word_to_find[4]; /*!< the word user have to find  */
        int alreadyFound[4]; /*!< for each letter, boolean array to check if a letter is already found */
        int size; /*!< size of the arrays */
        char word_found[4]; /*!< the word usersc found  */
        int errormsg[4]; /*!< Error handler if two users try to change the same letter*/

};

/*! \struct connInfos
 * \brief Connection informations
 *
 *  This structure gives connections informations for the server side thread
 */
struct connInfos {
        int connfd; /*!< socket of the current client */
        struct gamestate * gs; /*!< game status at time T */
};

/*! \struct clientCInfos
 * \brief Connection informations
 *
 *  This structure gives connections informations for the client side thread
 */

/*!
 *  \brief getRandomWord
 *
 *  get a random word from a list of 16 words
 *  \return char* word : a random word of four letters
 */
char* getRandomWord();

/*!
 *  \brief getSem
 *
 *  get semaphore
 */
int getSem();


int getSharedMemory();
/*!
 *  \brief initGame
 *
 *  initialize the game status
 *  \param struct gamestate * : pointer on the current game status struct
 */
void initGame(struct gamestate*);

/*!
 *  \brief sendall
 *
 *  TCP send all
 *  \param int socket : socket of the current client
 *  \param const char* message : message to send
 *  \param int size : size of the message
 *  \return bytessend : number of bytes sent
 */
int sendall(int, const char*, int);

/*!
 *  \brief sendWithSize
 *
 *  TCP send size than send message
 *  \param int socket : socket of the current client
 *  \param const char* message : message to send
 *  \param int size : size of the message
 *  \return int : 1 if succeed
 */
int sendWithSize(int, const char*, int);

/*!
 *  \brief recvAll
 *
 *  TCP receive all
 *  \param int socket : socket of the current client
 *  \param const char* message : message to send
 *  \param int size : size of the message
 *  \param char * ip : ip of the sender
 *  \param int port  : port of the sender
 *  \return int : 0 if succeed
 */
int recvAll(int, char*, int, char*, int);

/*!
 *  \brief sendWithSize
 *
 *  TCP receive size than message
 *  \param int socket : socket of the current client
 *  \param const char* message : message to send
 *  \param char * ip : ip of the sender
 *  \param int port  : port of the sender
 *  \return int : 0 if succeed
 */
int recvWithSize(int, char*, char*, int);

/*!
 *  \brief sendStruct
 *
 *  TCP send all the structure
 *  \param int socket : socket of the current client
 *  \param struct gamestate* : struct to send
 */
void sendStruct(int, struct gamestate *);

/*!
 *  \brief executePlay
 *
 *  take the input of the user, check if the letter is the same as the word to find, and set errors and win
 *  \param int place : number of the letter user wants to edit
 *  \param char letter : letter the user wants to set
 *  \param gamestate * : the current game status
 */
void executePlay(int, char, struct gamestate*);

/*!
 *  \brief updateThread
 *
 *  TCP send update when needed
 *  \param void * params : connections params
 */
void* updateThread(void*);

/*!
 *  \brief sendNextSTep
 *
 *  take the user input and send it to executePlay
 *  \param int socket : socket of the current client
 *  \param const char* ip : ip of the client
 *  \param int port  : port of the client
 *  \param gamestate*  : game status
 */
void sendNextSTep(int, char*, int, struct gamestate*);




#endif