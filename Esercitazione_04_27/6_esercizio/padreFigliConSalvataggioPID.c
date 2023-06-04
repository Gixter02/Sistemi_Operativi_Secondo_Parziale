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
#include <time.h>

int mia_random(int n){
    int casuale = rand() % n;
    return casuale;
}

int main(int argc, char** argv){
    srand(time(NULL));
    int N=atoi(argv[1]);
    if (N<0 || N> 155){
        printf("ERRORE nell'inserimento del numero\n");
        exit(1);
    }
    int*v = malloc(N*sizeof(int));
    if (v==NULL){
        printf("ERRORE nella malloc");
        exit(2);
    }
    printf("Il processo padre ha PID=%d e il numero inserito e' %d\n", getpid(), N);
    int pid;
    int i;
    for(i=0; i < N; i++){
        if((pid=fork()) < 0){
            printf("ERRORE nella fork\n");
            exit(3);
        }
        if(pid==0){
            printf("Sono il processo %d e sono il numero %d\n", getpid(),i);
            int n_random = mia_random(100+i);
            v[i]=getpid();
            printf("sono v[i]=%d\n", v[i]);
            exit(n_random);
        }
    }
    int pidFiglio, status, ritorno;
        for(i=0; i < N; i++){
            pidFiglio = wait(&status);
            if(pidFiglio < 0){
                printf("ERRORE nella wait\n");
                exit(4);
            }
            if((status & 0xFF)!= 0){
                printf("Figlio con PID %d terminato in modo anomalo\n", pidFiglio);
            }else{
                ritorno=(int)((status >> 8) & 0xFF);
                int j=0;
                for( ;j< N; j++){
                    printf("%d", v[j]);
                    if(v[j]==pidFiglio){
                        printf("Il figlio con PID=%d ha ritornato %d\n", pidFiglio, ritorno);
                    }
                }
            }
        }
    free(v);
    exit(0);    
}