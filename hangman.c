#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

char* words[16]= {"ANGLE","ARMOIRE","BANC","BUREAU","CABINET","CARREAU","CHAISE","CLASSE","CLEF","COIN","COULOIR","DOSSIER","EAU","ECOLE","ENTRER","ESCALIER"};


struct gamestate {
        int error;
        int win;
        char* word_to_find;
        int* alreadyFound;
        int size;
        char word_found[];

};


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

char* getRandomWord(){
        srand(time(NULL));
        //    char * wordRet = words[(rand() % 15)];
        char * wordRet = words[11];
        return wordRet;
}

void initGame(struct gamestate* gs){
        gs->error = 0;
        gs->word_to_find = (char*) getRandomWord();
        gs->size = strlen(gs->word_to_find);
        gs->alreadyFound = malloc(gs->size*sizeof(int));
        for (size_t i = 0; i <= strlen(gs->word_to_find); i++) {
                gs->alreadyFound[i] = 0;
                gs->word_found[i] ='_';
                if ( i == 0) { gs->word_found[i] = gs->word_to_find[i];}
                if ( i == strlen(gs->word_to_find) ) { gs->word_found[i] = '\0';}
        }

        gs->win = 0;
}

void printWF(struct gamestate* gs){
        for (size_t i = 0; i < strlen(gs->word_to_find); i++) {
                printf("%c", gs->word_found[i]);
        }
}
void printGUI(char* s){
        for (size_t i = 0; i < strlen(s); i++) {
                printf("-");
        }
}
void gamePrint(struct gamestate* gs){
        switch (gs->win) {
        case 0:
                printf("\n\n\n"); errorPrint(gs->error);
                printf("\n\n\n");
                printf("+-------------"); printGUI(gs->word_found); printf("-------------+"); printf("\t"); printf("+--------------------+\n");
                printf("|%8s  Word : %s %8s|", "",gs->word_found,""); printf("\t"); printf("|    error : %d/8     |\n",gs->error);
                printf("+-------------"); printGUI(gs->word_found); printf("-------------+"); printf("\t"); printf("+--------------------+\n");


                break;
        case 1:
                errorPrint(gs->error); printf("Word : %s", gs->word_found); printf("\nYou found the word ! Congrats ! \n");
                break;
        case -1:
                errorPrint(gs->error); printf("Word : %s", gs->word_found); printf("\nYou didn't found the word, you're dead !\n" );
        }
}

int containsAtPos( char c, char* s, struct gamestate *gs){
        int pos = -1;
        for (size_t i = 0; i < strlen(s); i++) {
                if ( ( c == s[i] || toupper(c) == s[i] ) && gs->alreadyFound[i] == 0 ) { pos = i;}
        }
        return pos;
}

void executePlay(char *toDecode, struct gamestate* gs){
        char newLetter = *toDecode;
        printf("You chose to put letter %c\n", newLetter);
        if ( containsAtPos(newLetter, gs->word_to_find, gs) != -1 ) {
                gs->word_found[containsAtPos(newLetter, gs->word_to_find, gs)] = newLetter;
                gs->alreadyFound[containsAtPos(newLetter, gs->word_to_find, gs)] = 1;
        }
        if (containsAtPos(newLetter, gs->word_to_find, gs) == -1 ) { gs->error++;}
        if ( strcmp(gs->word_found, gs->word_to_find) == 0 || strcasecmp(gs->word_found,gs->word_to_find) == 0) { gs->win = 1;}
        if (gs->error == 8) { gs->win = -1;}
}

int main(int argc, char const *argv[]) {
        struct gamestate gs;
        initGame(&gs);
        while(1) {
                system("clear");
                gamePrint(&gs);
                printf("\n\n\n");
                printf("Play > ");
                char playToDecode;
                scanf("\n%c", &playToDecode);
                executePlay(&playToDecode, &gs);
                system("clear");
                gamePrint(&gs);
                if ( gs.win == -1 ) { break;}
                if ( gs.win == 1 ) { break;}
        }


        return 0;
}
