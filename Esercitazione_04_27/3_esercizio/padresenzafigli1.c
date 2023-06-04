#include <stdio.h> /* Standard Input-Output */
/* include necessario per usare printf e altre; inoltre
definisce la costante BUFSIZ */
#include <stdlib.h> /* Standard Library */
/* include necessario per usare la primitiva exit */
#include <unistd.h> /* UNIX Standard */
/* include necessario per usare la primitiva read, write
e close */
#include <sys/wait.h>
/* include necessario per la primitiva wait */

int main(void){
    int pid_cur = getpid();
    printf("Il PID del processo corrente e' : %d\n", pid_cur);
    int status;
    if(wait(&status) < 0){
        printf("ERRORE in wait\n");
    }
    if(wait(&status) < 0){
        printf("ERRORE in wait\n");
        exit(2);
    }
    exit(0);
}