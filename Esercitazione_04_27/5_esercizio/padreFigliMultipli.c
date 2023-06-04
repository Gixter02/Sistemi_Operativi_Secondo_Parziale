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
#include <string.h>

int main(int argc, char** argv){
    int N=atoi(argv[1]);
    if (N<0 || N> 255){
        printf("ERRORE nell'inserimento del numero\n");
        exit(1);
    }
    int pid;
    int i;
    for(i=0; i < N; i++){
        if((pid=fork()) < 0){
            printf("ERRORE nella fork\n");
            exit(2);
        }
        if(pid==0){
            printf("Sono il processo %d e sono il numero %d\n", getpid(),i);
            exit(i);
        }
    }
    int pidFiglio, status, ritorno;
        for(i=0; i < N; i++){
            pidFiglio = wait(&status);
            if(pidFiglio < 0){
                printf("ERRORE nella wait\n");
                exit(3);
            }
            if((status & 0xFF)!= 0){
                printf("Figlio con PID %d terminato in modo anomalo\n", pidFiglio);
            }else{
                ritorno=(int)((status >> 8) & 0xFF);
                printf("Il figlio con PID=%d ha ritornato %d\n", pidFiglio, ritorno);
            }
        }
    exit(0);    
}