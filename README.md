# Utilisation : 

## Pour compiler : 

```bash
cd build
make
```

## Pour executer : 

```bash
./server [port]
./client [ip serveur][port serveur]
```


## Pour supprimer les fichiers binaires et les executables : 

```bash
make fclean
```


# Règles du jeu : 

- Première étape : choisir le numéro de la lettre à deviner
- Seconde étape : choisir la lettre à deviner
- Tentative maximum d'échec = 8 (PERDU si 8 erreurs commises)