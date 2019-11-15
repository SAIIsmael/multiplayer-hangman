all : client serveur

client : client.c
				gcc -Wall client.c -o client -lpthread

serveur : serveur.c
				gcc -Wall serveur.c -o serveur -lpthread

mrproper :
			rm -rf client
			rm -rf serveur
