#include <stdio.h> /* Standard Input-Output */
/* include necessario per usare printf e altre; inoltre
definisce la costante BUFSIZ */
#include <fcntl.h> /* File CoNTroL */
/* include necessario per usare la primitiva open e le
relative costanti come O_RDONLY */
#include <stdlib.h> /* Standard Library */
/* include necessario per usare la primitiva exit */
#include <unistd.h> /* UNIX Standard */
/* include necessario per usare la primitiva read, write
e close */
#include <sys/wait.h>
/* include necessario per la primitiva wait */

int main(void){
    int number;
    printf("Inserisci un numero\n");
    scanf("%d", &number);
    if(number != 0){
        execlp("prova","prova", (char *)0);
    }
    exit(0);
}