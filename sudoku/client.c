#include <stdio.h>      //Pour printf
#include <sys/types.h>
#include <sys/socket.h> // Pour socket, send...
#include <arpa/inet.h>  // Pour inet_pton, htons...
#include <stdlib.h>     // Pour fgets
#include <unistd.h>     // Pour close
#include <string.h>     // Pour strlen
#include <pthread.h>

// Couleur pour l'affichage
#define CYAN  "\x1B[36m"
#define RESET "\x1B[0m"

typedef struct Param Param;
struct Param{
	int dSock;
	int * sudokuOriginal;
};

// return < 0 s'il faut arrêter
int send_tcp(int socket, char * message, int nbOctets){
	int totalEnvoye = 0, s;

	while(totalEnvoye != nbOctets){
		s = send(socket, &(message[totalEnvoye]), nbOctets-totalEnvoye, 0);
		if(s < 0){
			perror(" Erreur lors du send ");
			return -1;
		}
		else if(s == 0){
			printf(" Deconnexion du serveur\n");
			return -2;
		}
		totalEnvoye += s;
	}
	return 0;
}

// return < 0 s'il faut arrêter
int recv_tcp(int socket, char * message, int nbOctets){
	int totalRecu = 0, r;

	while(totalRecu != nbOctets){
		r = recv(socket, message+totalRecu, nbOctets-totalRecu, 0);
		if(r < 0){
			perror(" Erreur lors du recv ");
			return -1;
		}
		else if(r == 0){
			printf(" Le serveur s'est déconnecté\n");
			return -2;
		}
		totalRecu += r;
	}
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

void convertirEntree(char * entree, int * res){
	int colonne, ligne, valeur;

	if(entree[0] >= 'a' && entree[0] <= 'i') colonne = entree[0]-'a';
	else colonne = entree[0]-'A';

	ligne = entree[1]-'1';
	valeur = entree[2]-'0';

	res[0] = (ligne * 9) + colonne;
	res[1] = valeur;
}

void convertionInverse(int aConvertir, char * converti){
	int cpt = 0, num = aConvertir;
	while (num >= 0){
		num -= 9;
		cpt ++;
	}

	converti[1] = cpt + '0';

	num+=9;
	switch(num){
		case 0 : converti[0] = 'A'; break;
		case 1 : converti[0] = 'B'; break;
		case 2 : converti[0] = 'C'; break;
		case 3 : converti[0] = 'D'; break;
		case 4 : converti[0] = 'E'; break;
		case 5 : converti[0] = 'F'; break;
		case 6 : converti[0] = 'G'; break;
		case 7 : converti[0] = 'H'; break;
		case 8 : converti[0] = 'I'; break;
		default : printf("Erreur à la conversion\n");
	}
}

void viderBuffer(){
	int c = 0;
	while(c != '\n' && c != EOF){ c = getchar(); }
}

void afficheSudoku(int * sudoku, int * sudokuOriginal){
	int compteur = 1;
	printf("\n     A   B   C   D   E   F   G   H   I \n");
	printf("   ┌───");
	for(int i=1; i<9; i++){ printf("────"); }
	printf("┐\n %i ", compteur);
	for(int i=0; i<81; i++){
		if(i%9 == 0 && i != 0){
			compteur++;
			if(i%27 == 0){
				printf("│\n   │───");
				for(int j=1; j<9; j++){
					if(j%3 == 0 && j != 0) printf("┼───");
					else printf("────");
				}
				printf("│\n %i ", compteur);
			}
			else{
				printf("│\n   │─");
				for(int j=1; j<9; j++){
					if(j%3 == 0 && j != 0) printf("──│─");
					else printf("── ─");
				}
				printf("──│\n %i ", compteur);
			}
		}
		if(sudoku[i] == 0) printf("│   ");
		else{
			if(sudokuOriginal[i] != 0){
				printf("│");
				printf(CYAN " %i " RESET, sudoku[i]);
			}
			else printf("│ %i ", sudoku[i]);
		}
	}
	printf("│\n   └───");
	for(int i=1; i<9; i++){ printf("────");	}

	printf("┘\n");
}

void * joueur(void * par){
	Param * p = (Param *) par;

	// Récupération des paramètres
	int dSock = p->dSock;
	int * sudokuOriginal = p->sudokuOriginal;

	char entree[4];
	int valeurJouee[2];
	int reponseNonValable = 1, jeuEnCours = 1;

	while(jeuEnCours){
		do{
			printf("\n Rentrez la colonne, puis la ligne, puis la valeur - ou OUT pour quitter le jeu.\n Exemple : A19 pour mettre 9 à la case A1\n\n");
			fgets(entree, 4, stdin);
			viderBuffer(); // On vide le buffer d'entrée

			if((entree[0] >= 'a' && entree[0] <= 'i') || (entree[0] >= 'A' && entree[0] <= 'I')){
				if(entree[1] > '0' && entree[1] <= '9' && entree[2] > '0' && entree[2] <= '9')
					reponseNonValable = 0;
			}
			else if(strcmp(entree, "OUT") == 0){
				fermerSocket(dSock);
				exit(0);
			}

			if(reponseNonValable == 1) printf("\n ---- Entrée invalide, réessayez ---- \n");

		}while(reponseNonValable);

		reponseNonValable = 1;

		// Pour avoir valeurJouee[0] = case, valeurJouee[1] = valeur
		convertirEntree(entree, valeurJouee);

		// Si la case a le droit d'être modifiée (pas une case de base)
		if(sudokuOriginal[valeurJouee[0]] == 0){
			// On envoie la réponse
			if(send_tcp(dSock, (char *)valeurJouee, 2*sizeof(int)) < 0) jeuEnCours = 0;
		}
		else printf(" ---- IMPOSSIBLE DE MODIFIER CETTE CASE ! ----\n");
	}
	pthread_exit(NULL);
}

int main(int argc, char **argv){

	printf("────────────────────────────────────────────────────────────\n");
	char ip[16] = "";
	char port[8] = "";

	// On demande l'adresse IP du serveur
	printf(" Quelle est l'adresse IP du serveur ? ");
	fgets(ip, 16, stdin);
	ip[strlen(ip)-1] = '\0';

	// On demande le port
	printf(" Quel est le numéro de port du serveur ? ");
	fgets(port, 8, stdin);
	port[strlen(port)-1] = '\0';
	printf("\n");

	// On crée une socket dans le domaine IPv4, de type stream
	// et utilisant le protocole par défaut (ici TCP)
	int dSock = socket(PF_INET, SOCK_STREAM, 0);
	if(dSock == -1){
		perror(" Erreur à la création de la socket ");
		return -1;
	}

	printf(" - Socket créée\n");

	// Informations sur la socket du serveur
	struct sockaddr_in ad;
	ad.sin_family = AF_INET;
	inet_pton(AF_INET, ip, &(ad.sin_addr)); // convertit l'adresse IP (texte -> binaire)
	ad.sin_port = htons(atoi(port));        // convertit le numéro de port (texte -> binaire)

	printf(" - On demande à se connecter au serveur à l'adresse %s, port %s \n", ip, port);

	// On demande une connexion à la socket du serveur
	if(connect(dSock, (struct sockaddr*) &ad, sizeof(ad)) == -1){
		perror(" Erreur à la demande de connexion ");
		fermerSocket(dSock); // On pense à fermer la socket
		return -1;
	}

	printf(" - Connexion acceptée \n\n");

	int sudoku[81];
	int sudokuOriginal[81];

	// On reçoit la première fois la partie de sudoku en cours sur le serveur
	if(recv_tcp(dSock, (char *)sudoku,  81*sizeof(int)) < 0){
		fermerSocket(dSock); // On pense à fermer la socket
		return -1;
	}

	// On reçoit le sudoku original, utilisé pour l'affichage et les vérifications
	if(recv_tcp(dSock, (char *)sudokuOriginal,  81*sizeof(int)) < 0){
		fermerSocket(dSock); // On pense à fermer la socket
		return -1;
	}

	// Affichage du sudoku
	afficheSudoku(sudoku, sudokuOriginal);

	Param p = {dSock, sudokuOriginal};
	Param * aP = &p;

	pthread_t idTh;
	// Création du thread qui va recevoir et afficher les modifications des autres joueurs
	if(pthread_create(&idTh, NULL, joueur, (void *)aP) != 0){
		printf(" Erreur lors de la création du thread\n");
		fermerSocket(dSock);
		return -1;
	}

	int serveurConnecte = 1;
	int reponse[3];

	while(serveurConnecte){

		int r = recv_tcp(dSock, (char *)reponse,  3*sizeof(int));
		if (r < 0) serveurConnecte = 0;

		if(serveurConnecte != 0){
			if(reponse[1] == -1){
				printf(" Joueur %i s'est connecté\n", reponse[0]); }
			else if(reponse[1] == -2){
				printf(" Joueur %i s'est déconnecté\n", reponse[0]); }
			else if(reponse[1] == -3){
				printf(" VICTOIRE !!!!! Vous avez réussi le sudoku (enfin)\n");
				serveurConnecte = 0; // Pour ne pas recommencer la boucle
			}
			else{
				char affichage[3];
				affichage[2] = '\0';
				convertionInverse(reponse[1], affichage);
				printf("\n - Joueur %i a joué en %s la valeur %i\n", reponse[0], affichage, reponse[2]);
				sudoku[reponse[1]] = reponse[2];
				afficheSudoku(sudoku, sudokuOriginal);
			}
		}
	}
	fermerSocket(dSock);

	// Pas besoin de pthread_join

	return 0;
}
