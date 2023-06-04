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
#include <time.h>
#include <sys/wait.h>


int mia_random(int n){
    int casuale;
    casuale = rand() % n;
    return casuale;
}

int main(void){
    srand(time(NULL));
    int pid_cur = getpid();
    printf("Il PID del processo corrente e' : %d\n", pid_cur);

    int pid, status, exit_s;

    if((pid=fork()) < 0){
        printf("Errore in fork\n");
        exit(1);
    }

    if (pid==0){
        int pid_padre = getppid();
        int pid_figlio = getpid();
        printf("Il PID del padre e' : %d\n", pid_padre);
        printf("Il PID del figlio e' : %d\n", pid_figlio);

        int n_random = mia_random(100);

        exit(n_random);
    }
    if(wait(&status) < 0){
        printf("ERRORE in wait\n");
        exit(2);
    }
    if((status & 0xFF) != 0){
        printf("ERRORE in status\n");
    }else{
        exit_s = status >> 8;
        exit_s &= 0xFF;
        printf("Per il figlio %d lo stato di EXIT e' : %d\n", pid, exit_s);
    }
    exit(0);
}