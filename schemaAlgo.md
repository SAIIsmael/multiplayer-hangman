# Serveur :

InitShm();

-   InitSemaphores();
-   InitServeur();
-   Ecoute(socket);
-   si serveur accepte une connexion :
    -   fils = fork()
    -   si (fils ) :
        -   thread 1 :
            \-Reception données;
            \-semaphore lock sur la mémoire partagé
        -   mets à jour la mémoire partagé
            \-incrémente le semaphore mise à jour
            \-sempahore unlock la mémoire partagé
    -   thread principal :
        -   si une mise à jour dispo via le semaphore maj:
            -   emet les mises à jours aux CLIENTS
-   fin algo

# Client :

-   creation thread.
-   initClient()
-   connexionServeur()
    -   thread 1 :
        -   reçoit mise à jour du serveur
    -   thread principal :
        -   demande la saisi de données
        -   envoie les données au serveurConnecte
-   fin algo
