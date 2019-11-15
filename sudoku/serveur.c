#include <sys/types.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <errno.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <arpa/inet.h>
#include <unistd.h>

union semun{
	int val;
	struct semid_ds *buf;
	unsigned short *array;
	struct seminfo *__buf;
};

typedef struct Param Param;
struct Param{
	int * adresseSegment;
	int idSemaphore;
	int dSClient;
	int numClient;
};

int send_tcp(int socket, char * message, int nbOctets){
	int totalEnvoye = 0, s;

	while(totalEnvoye != nbOctets){
		s = send(socket, message+totalEnvoye, nbOctets-totalEnvoye, 0);
		if(s < 0){
			perror(" Erreur lors du send ");
			return -1;
		}
		else if(s == 0){
			printf(" Deconnexion du joueur\n");
			return -2;
		}
		totalEnvoye += s;
	}
	return 0;
}

int recv_tcp(int socket, char * message, int nbOctets){
	int totalRecu = 0, r;

	while(totalRecu != nbOctets){
		r = recv(socket, message+totalRecu, nbOctets-totalRecu, 0);
		if(r < 0){
			perror(" Erreur à la réception du message : ");
			return -1;
		}
		else if(r == 0){
			printf(" Le joueur s'est déconnecté\n");
			return -2;
		}
		totalRecu += r;
	}
	return 0;
}

int supprimerSegment(int id){
	if(shmctl(id, IPC_RMID, NULL) == -1){
		perror(" Erreur à la destruction du segment mémoire ");
		return -1;
	}
	printf(" - Segment mémoire supprimé proprement\n");
	return 0;
}

int supprimerSemaphore(int id){
	if(semctl(id, 0, IPC_RMID) == -1){
		perror(" Erreur à la destruction du tableau de sémaphores ");
		return -1;
	}
	printf(" - Tableau de sémaphores supprimé proprement\n");
	return 0;
}

int fermerSocket(int dSock){
	if(close(dSock) == -1){ // On ferme la socket
		perror(" Erreur à la fermeture de la socket ");
		return -1;
	}
	printf(" - Socket fermée\n");
	return 0;
}

// On demande à obtenir le sémaphore qui protège le segment mémoire
int demandeRessourceSegment(int idSemaphore){
	struct sembuf opp =  {1, -1, SEM_UNDO};
	if(semop(idSemaphore, &opp, 1) == -1){
		perror(" Erreur avec semop, dans demandeRessourceSegment ");
		return -1;
	}
	return 0;
}

// On redonne le sémaphore qui protège le segment mémoire
int redonneRessourceSegment(int idSemaphore){
	struct sembuf opv = {1, +1, SEM_UNDO};
	if(semop(idSemaphore, &opv, 1) == -1){
		perror(" Erreur avec semop, dans redonneRessourceSegment ");
		return -1;
	}
	return 0;
}

int avertirAutrePusServeur(int idSemaphore){

	/*
	Dans l'ordre :
	- Bloque le sémaphore 3
	- Met le sémaphore 2 à 0 pour indiquer un changement/débloquer les autres processusServeur
	- Attend que tous les processusServeur prennent compte de ce changement.
	*/

	struct sembuf op[] = {
		{3, +1, SEM_UNDO},
		{2, -1, SEM_UNDO}
	};

	if(semop(idSemaphore, op, 2) == -1){
		perror(" Erreur avec semop, dans avertirAutrePusServeur ");
		return -1;
	}

	struct sembuf opz = {0, 0, 0};

	if(semop(idSemaphore, &opz, 1) == -1){
		perror(" Erreur avec semop, dans avertirAutrePusServeur ");
		return -1;
	}
	return 0;
}

int prepareLaProchaineMaj(int idSemaphore){
	struct sembuf op[] = {
		{2, +1, SEM_UNDO},
		{3, -1, SEM_UNDO}
	};

	if(semop(idSemaphore, op, 2) == -1){
		perror(" Erreur avec semop, dans prepareLaProchaineMaj ");
		return -1;
	}
	return 0;
}

// Initialisation du sudoku dans le segment de mémoire partagé
int initSegmentSudoku(int * sudoku){

	FILE* fichier = NULL;

	fichier = fopen("./sudoku.txt", "r");
	if (fichier == NULL){
		perror(" Problème à l'ouverture du fichier ");
		return -1;
	}

	char ligne[200];

	fgets(&ligne[0], 200, fichier);
	fgets(&ligne[81], 200, fichier);

	if (fclose(fichier) != 0){
		perror(" Problème à la fermeture du fichier ");
		return -1;
	}

	for(int i=0; i<162; i++){ sudoku[i+3]=ligne[i]-'0'; }
	for(int i=0; i<81; i++){ sudoku[i+165] = ligne[i]-'0'; }

	sudoku[0] = 1; // pour indiquer qu'une partie est en cours

	return 0;
}

// Opération V sur le sémaphore 0
int incrementeClient(int idSemaphore){
	struct sembuf opv = {0, +1, SEM_UNDO};

	if(semop(idSemaphore, &opv, 1) == -1){
		perror(" Erreur avec semop, dans incrementeClient ");
		return -1;
	}
	return 0;
}

// Opération Z sur le sémaphore 2
int attendMajAutreClient(int idSemaphore){
	struct sembuf opz = {2, 0, 0};
	if(semop(idSemaphore, &opz, 1) == -1){
		perror(" Erreur avec semop, dans attendMajAutreClient ");
		return -1;
	}
	return 0;
}

// Permet d'attendre que tous les sémaphores aient les bonnes valeurs avant
// de recommencer la boucle
int attendreSemaphoreCorrect(int idSemaphore){
	struct sembuf opz = {3, 0, 0};
	if(semop(idSemaphore, &opz, 1) == -1){
		perror(" Erreur avec semop, dans demandeRessourceSegment ");
		return -1;
	}
	return 0;
}

int rdvPusServeur(int idSemaphore){
	struct sembuf opp = {0, -1, SEM_UNDO};
	struct sembuf opz = {0, 0, 0};

	if(semop(idSemaphore, &opp, 1) == -1){
		perror(" Erreur avec semop, dans demandeRessourceSegment ");
		return -1;
	}

	if(semop(idSemaphore, &opz, 1) == -1){
		perror(" Erreur avec semop, dans demandeRessourceSegment ");
		return -1;
	}
	return 0;
}

void verifVictoire(int * sudoku){
	int i =0;
	int faux = 0;
	while(i < 81 && !faux){
		if(sudoku[i+3] != sudoku[i+3+81]){
			faux = 1;
		}
		i++;
	}

	if(!faux){
		sudoku[0] = 0;
	}
}

void * reponseClient(void * par){
	Param * p = (Param *) par;

	// Récupération des paramètres
	int dSClient = p->dSClient;
	int * sudoku = p->adresseSegment;
	int idSemaphore = p->idSemaphore;
	int numClient = p->numClient;

	int clientConnecte = 1;
	int reponse[2];

	while(clientConnecte){

		int r = recv_tcp(dSClient, (char*)reponse, 2*sizeof(int));
		if(r == -1)	pthread_exit((void *)-1);
		if(r == -2) clientConnecte = 0;


		//VÉRIFIER LA VALIDITÉ DES INFOS REÇUES OU/ET DANS LE CLIENT CE QUI EST ENVOYÉ

		// On demande à obtenir le sémaphore qui protège le segment mémoire
		if(demandeRessourceSegment(idSemaphore) == -1) pthread_exit(NULL);

		if(clientConnecte == 0){
			sudoku[1] = numClient;
			sudoku[2] = -2; // Pour indiquer aux autres clients que le joueur s'est déconnecté
		}
		else{
			// On écrit le changement dans le segment de mémoire partagé
			sudoku[1] = numClient;
			sudoku[2] = reponse[0]; // case changée
			sudoku[reponse[0]+3] = reponse[1]; // valeur changée
			verifVictoire(sudoku);
		}

		// On réveille les autres processusFilsServeur pour qu'ils envoient
		// la maj aux autres joueurs et on attend qu'ils terminent tous.
		if(avertirAutrePusServeur(idSemaphore) == -1) pthread_exit(NULL);

		/*
		Si on est ici c'est que tous les autres processusFilsServeur ont envoyé
		la maj aux autres joueurs

		On remet les sémaphores de synchronisation (2 et 3) à jour
		pour recommencer les boucles normalement
		*/
		if(prepareLaProchaineMaj(idSemaphore) == -1) pthread_exit(NULL);

		// On redonne le sémaphore qui protège le segment mémoire
		if(redonneRessourceSegment(idSemaphore) == -1) pthread_exit(NULL);
	}
	pthread_exit(NULL);
}

int processusFils(int idSemaphore, int * sudoku, int dSClient, int numClient){

	// On demande à obtenir le sémaphore qui protège le segment mémoire
	if(demandeRessourceSegment(idSemaphore) == -1) return -1;

	// Si une partie est finie on réinitialise le sudoku pour une nouvelle partie
	if(sudoku[0] == 0){
		if(initSegmentSudoku(sudoku) == -1){ return -1; }
		printf(" - Nouvelle partie chargée dans le segment partagé\n");
	}

	// On envoie le sudoku en cours
	if(send_tcp(dSClient, (char *)&(sudoku[3]), 81*sizeof(int)) < 0) return -1;

	// On envoie le sudoku original
	if(send_tcp(dSClient, (char *)&(sudoku[165]), 81*sizeof(int)) <  0) return -1;

	// On prévient les autres joueurs qu'un nouveau joueur est là
	sudoku[1] = numClient;
	sudoku[2] = -1; // Pour indiquer aux autres clients qu'un joueur s'est connecté

	if(avertirAutrePusServeur(idSemaphore) == -1) return -1;
	if(prepareLaProchaineMaj(idSemaphore) == -1) return -1;

	// On incrémente le sémaphore 0 pour indiquer qu'un nouveau client est présent
	if(incrementeClient(idSemaphore) == -1) return -1;

	// On redonne le sémaphore qui protège le segment mémoire
	if(redonneRessourceSegment(idSemaphore) == -1) return -1;

	Param p = {sudoku, idSemaphore, dSClient, numClient};
	Param * aP = &p;

	pthread_t idTh;

	// Création du thread qui va attendre une modification du client
	if(pthread_create(&idTh, NULL, reponseClient, (void *)aP) != 0){
		printf("Erreur lors de la création du thread reponseClient\n");
		return -1;
	}

	int toutEstBon = 1;
	while(toutEstBon){

		// On se met en attente sur le sémaphore 2
		if(attendMajAutreClient(idSemaphore) == -1) return -1;

		int infosAEnvoyer[3];
		infosAEnvoyer[0] = sudoku[1]; // Le numJoueur qui a joué
		infosAEnvoyer[1] = sudoku[2]; // -1 = connecté, -2 = déconnecté, >=0 = case modifiée
		if(sudoku[2] != -1 && sudoku[2] != -2){
			infosAEnvoyer[2] = sudoku[sudoku[2]+3]; // La valeur changée
		}
		else{
			infosAEnvoyer[2] = 0;
		}
		if(infosAEnvoyer[0] == numClient && infosAEnvoyer[1] == -2){
			toutEstBon = 0;
		}
		else{
			int s = send_tcp(dSClient, (char *)infosAEnvoyer, 3*sizeof(int));
			if(s == -1) return -1;
			if(s == -2) toutEstBon = 0;

			if(sudoku[0] == 0){
				infosAEnvoyer[1] = -3;

				s = send_tcp(dSClient, (char *)infosAEnvoyer, 3*sizeof(int));
				if(s == -1) return -1;

				printf(" Partie terminée : déconnexion du serveur pour le joueur %d\n", numClient);
				toutEstBon = 0;
			}
		}

		// On décrémente le sémaphore 0 et on attend que tous les clients aient traité la maj
		if(rdvPusServeur(idSemaphore) == -1) return -1;

		// On attend que tous les sémaphores aient les bonnes valeurs avant de recommencer la boucle
		if(attendreSemaphoreCorrect(idSemaphore) == -1) return -1;

		// On remet le sémaphore 0 comme avant, une fois que tout le monde a fini
		if(incrementeClient(idSemaphore) == -1) return -1;
	}

	// Pas besoin de pthread_join

	return 0;
}


int main(){

	//Création de la clé
	key_t sesame = ftok("./readme.txt", 10);
	if(sesame == -1){
		perror(" Erreur lors de l'obtention de la clé ");
		return -1;
	}

	// Création du segment de mémoire partagé qui stockera le sudoku
	int idSegment = shmget(sesame, sizeof(int)*250, IPC_CREAT|0666);
	if(idSegment == -1){
		perror(" Erreur lors de la création du segment partagé ");
		return -1;
	}

	printf(" - Segment de mémoire partagé créé\n");

	// Demande d'attachement au segment mémoire
	int * sudoku;
	if((sudoku = (int*)shmat(idSegment, NULL, 0)) == (int*)-1){
		perror(" Erreur lors de l'attachement au segment partagé ");
		supprimerSegment(idSegment);
		return -1;
	}

	sudoku[0] = 0;

	// Création des sémaphores
	int idSemaphore = semget(sesame, 4, IPC_CREAT|0666);
	if(idSemaphore == -1){
		perror(" Erreur lors de la création des sémaphores ");
		supprimerSegment(idSegment);
		return -1;
	}

	printf(" - Tableau de sémaphores créé\n");

	/*
	SEMAPHORE 0 : NOMBRE DE CLIENTS = NOMBRE DE THREADS MIS À JOUR
	SEMAPHORE 1 : PROTÈGE LE SEGMENT = LE SUDOKU
	SEMAPHORE 2 et 3 : POUR DIVERSES SYNCHRONISATIONS
	*/

	// Initialisation des sémaphores
	union semun egCtrl;
	egCtrl.array = malloc(sizeof(unsigned short)*4);
	egCtrl.array[0] = egCtrl.array[3] = 0;
	egCtrl.array[1] = egCtrl.array[2] = 1;

	if(semctl(idSemaphore, 0, SETALL, egCtrl) == -1){
		perror(" Erreur lors de l'initialisation des sémaphores ");
		supprimerSegment(idSegment);
		supprimerSemaphore(idSemaphore);
		return -1;
	}

	printf(" - Tableau de sémaphores initialisé\n");

	// On crée une socket dans le domaine IPv4, de type stream
	// et utilisant le protocole par défaut (ici TCP)
	int dSock = socket(PF_INET, SOCK_STREAM, 0);
	if(dSock == -1){
		perror(" Erreur à la création de la socket ");
		supprimerSegment(idSegment);
		supprimerSemaphore(idSemaphore);
		return -1;
	}

	printf(" - Socket créée\n");

	struct sockaddr_in ad;   // Infos de la socket du serveur
	ad.sin_family = AF_INET;
	ad.sin_addr.s_addr = INADDR_ANY;
	ad.sin_port = 0;

	socklen_t longueur = sizeof(struct sockaddr_in);

	// On nomme la socket
	if(bind(dSock, (struct sockaddr*)&ad, longueur) == -1){
		perror(" Erreur lors du nommage de la socket ");
		supprimerSegment(idSegment);
		supprimerSemaphore(idSemaphore);
		fermerSocket(dSock);
		return -1;
	}

	// On demande un numéro de port disponible
	getsockname(dSock, (struct sockaddr*)&ad, &longueur);
	printf(" Voici le numéro de port : %d \n", ntohs(ad.sin_port));

	printf(" - Socket nommée\n");


	int l = listen(dSock, 100); // On met la socket en mode écoute
	if(l < 0){
		perror(" Erreur lors du passage en mode écoute de la socket ");
		supprimerSegment(idSegment);
		supprimerSemaphore(idSemaphore);
		fermerSocket(dSock);
		return -1;
	}

	printf(" - Attente de nouveaux clients\n");

	int continu = 1, fils, clientNumero = 0;

	while(continu){

		// On attend qu'un client se connecte
		int dSClient = accept(dSock, (struct sockaddr*)&ad, &longueur);
		if(dSClient == -1){
			perror(" Erreur lors de la connexion d'un client ");
			supprimerSegment(idSegment);
			supprimerSemaphore(idSemaphore);
			fermerSocket(dSock);
			return -1;
		}

		// On vérifie que le tableau de sémaphores existe toujours
		int id = semget(sesame, 4, 0666);
		if(id == -1){
			supprimerSegment(idSegment);
			fermerSocket(dSock);
			fermerSocket(dSClient);
			return -1;
		}

		demandeRessourceSegment(idSemaphore);
		if(sudoku[0] == 0) clientNumero = 0;
		redonneRessourceSegment(idSemaphore);

		clientNumero++;

		printf(" - Nouveau client connecté : Joueur %i\n", clientNumero);

		if((fils = fork()) == 0){
			printf(" Dans le serveur fils : ");
			if(fermerSocket(dSock) == -1){
				printf(" Pour le serveur/client num %d\n", clientNumero);
				return -1;
			}

			if(processusFils(idSemaphore, sudoku, dSClient, clientNumero) == -1)
				printf(" Le serveur fils s'est arrêté à cause d'une erreur\n");
			continu = 0;
		}

		printf(" Dans le serveur principal : ");

		// On ferme la socket dSClient dans le père
		if(fermerSocket(dSClient) == -1){
			if(fils != 0){ // si on est dans le père
				supprimerSegment(idSegment);
				supprimerSemaphore(idSemaphore);
			}
			fermerSocket(dSock);
			return -1;
		}
	}
	return 0;
}
