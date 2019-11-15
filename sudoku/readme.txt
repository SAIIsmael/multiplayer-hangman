Pour commencer, lancer la commande "make" dans un terminal. Elle crée deux exécutables : client, serveur.

Sur l'ordinateur qui fait le serveur : 

- Lancer "./serveur" dans un terminal. 
- Sur un second terminal, utiliser la commande "hostname -i" pour récupérer l'adresse IP de l'ordinateur qui héberge le serveur.
- Le numéro de port de l'application est indiqué par le terminal du serveur.

Sur les ordinateurs clients :

- Lancer "./client" dans un terminal.  
- Entrer l'adresse IP du serveur et le numéro de port de l'application (fournis par le serveur -> voir instructions ci-dessus)

Enjoy !

Note : 
- Le serveur ne s'arrête pas quand la partie est finie, il recharge la partie.
- Si le tableau de sémaphores est supprimé, tous les clients sont déconnectés, mais le serveur recharge le tableau et une nouvelle partie pour les prochains clients.
- Si un processus fils du serveur meurt, le serveur et la partie continuent sans soucis.
- Le serveur ne s'arrête donc que si vous faites CTRL+C ou qu'une erreur survient.
