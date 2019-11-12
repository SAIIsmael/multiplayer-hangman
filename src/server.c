#include "../inc/server.h"

key_t keyGenerator(int val){
        printf(YEL "      * [keyGenerator] Generating a key. * %s\n", " ");
        key_t key = ftok("../bin/k.txt", val);
        return key;
}

int semConnect(key_t key){
        printf(YEL "      * [semConnect] Connection to the semaphore. * %s\n", " ");
        return semget(key, 1, IPC_CREAT|0666);

}

int semGetVal(int idSem){
        int semVal = semctl(idSem, 0, GETVAL);
        printf(CYN "      * [semGetVal] semaphore %d value : %d . * %s\n\n\n", idSem, semVal," ");
        return semVal;
}

int semSetIntVal(int idSem, int value){
        return semctl(idSem, 0, SETVAL, value);
}

int semLock(int idSem){
        struct sembuf op[] = {
                { 0, -1, SEM_UNDO }, //lock
                { 0, 1, SEM_UNDO }, //unlock
                { 0, 0, SEM_UNDO} // wait sem equals 0
        };
        return semop(idSem,op, 1);
}

int semUnlock(int idSem){
        struct sembuf op[] = {
                { 0, -1, SEM_UNDO }, //lock
                { 0, 1, SEM_UNDO }, //unlock
                { 0, 0, SEM_UNDO} // wait sem equals 0
        };
        return semop(idSem,op+1, 1);
}



int main(int argc, char const *argv[]) {

        printf("Main server \n" );
        int sfd, key, enable;
        struct sockaddr_in srv_addr, clt_addr;
        socklen_t clt_addr_size;

        if (argc != 2) {
                fprintf(stderr, "demarer le serveur au port : %s <port>\n", argv[0]);
                exit(EXIT_FAILURE);
        }

        if ((key = keyGenerator(0)) == -1) {
                perror("ftok");
                exit(1);
        }
        // // Créer la mémoire partagée
        // if ((shmem.shm = shmget(key, BUF_SIZE, IPC_CREAT | 0600)) == -1) {
        //         error("shmget");
        // }

        memset(&srv_addr, 0, sizeof(srv_addr));
        srv_addr.sin_family = AF_UNSPEC;          // Nommage de la famille de socket
        srv_addr.sin_addr.s_addr = INADDR_ANY;    // Adresse de la socket
        srv_addr.sin_port = htons(atoi(argv[1])); // Récupération du numero de port avec conversion en réseau du numero de port grace à htons

        sfd = socket(PF_INET, SOCK_STREAM, 0);
        if (sfd == -1) {
                perror("socket");
        }
        // Pour réutiliser la socket sur le même port
        if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
                perror("setsockopt(SO_REUSEADDR) failed");
        }

        if (bind(sfd, (struct sockaddr*)&srv_addr, sizeof(srv_addr)) == -1) {
                perror("bind");
        }

        if (listen(sfd, 128) == -1) {
                perror("listen");
        }


        printf("\n=========================================\n");
        printf("|                 SERVEUR               |\n");
        printf("=========================================\n");



        return 0;
}
