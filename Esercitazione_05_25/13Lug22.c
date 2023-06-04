#include <stdio.h>	// Includo la libreria per la funzione printf e BUFSIZ
#include <stdlib.h>	// Includo la libreria per la funzione exit
#include <unistd.h>	// Includo la libreria per la funzione close, fork, famiglia exec, read, write, lseek, famiglia get, pipe,
#include <fcntl.h>	// Includo la libreria per la funzione open, creat e le relative macro
#include <sys/wait.h>	// Includo la libreria per la funzione wait
#include <string.h>

//definisco il tipo pipe_t
typedef int pipe_t[2];

int main(int argc, char** argv) {
    char ok = 1;
    if(argc < 5 ){
        printf("Errore, in quanto i parametri sono %d\n", argc); 
        exit(1);
    }
    char* F = argv[1];
    int L;		/*Dichiarazione varibile che conterra' Param*/
    //controllo utlizzando la funzione atoi se il Param e un numero strettamente positivo
    if((L = atoi(argv[1])) <= 0){
        printf("il parametro %s non un numero positivo\n",argv[1]);
        exit(2);
    }
    for(int j = 3; j < argc; j++){
        if(strlen(argv[j]) > 1){
            printf("Errore, %s non e' un carattere\n", argv[j]);
            exit(9);
        }
    }
    int Q = argc - 3;
    //apro Fasi pipes
    pipe_t* piped = (pipe_t*)malloc(sizeof(pipe_t) * (Q+1));
    if (piped == NULL) {
        printf("Errore nell'allocazione della memoria\n");
        exit(3);
    }
    
    for(int i = 0; i < Q+1; i++){
        if(pipe(piped[i])<0){
            printf("Errore nella creazione della pipe numero:%i\n",i);
            exit(4);
        }
    }
    for(int i = 0; i < Q; i++){
        int pid;	// memorizza il valore di ritorno della funzione fork
        
        if ((pid = fork()) < 0)	/* Il processo padre crea un figlio */
        {	/* In questo caso la fork e' fallita */
            printf("Errore durante la fork\n");
            exit(5);
        }
        
        if (pid == 0){	/* processo figlio */
            //chiudo tutte le pipe in lettura e scrittura tranne la piped-esima pipe, aperta in lettura
            for(int j = 0; j < Q; j++){
                if (j != i) {
                    close(piped[j][0]);
                }
                if(j != i + 1){
                    close(piped[j][1]);
                }
            }
            //controllo se il file e' accedibile
            int fd = 0; /*variabile che conterra il file descriptor del file che stiamo per aprire */
            if((fd = open(F, O_RDONLY)) < 0){		/* ERRORE se non si riesce ad aprire in LETTURA il file */
                printf("Errore in apertura file %s dato che fd = %d\n", F, fd);
                exit(6);
            }
            int n_occorrenze = 0;
            char buffer;
            int L_curr = 1;
            int ritorno = 0;
            while (read(F, &buffer, sizeof(char)) > 0){	/* ciclo di lettura fino a che riesco a leggere un carattere da file */
                if(buffer == '\n'){
                    int nr = 0;
                    if(nr = read(piped[i][0], &ok, sizeof(ok))<0){
                        printf("Errore, errore nel numero di byte letti nella pipe\n"); 
                        exit(7);
                    }

                    printf("Sono il figlio %d e alla riga %d ho trovato %d occorrenze del carattere %c", getpid(), L_curr, n_occorrenze, argv[i+2][0]);
                    write(piped[i+1][0], &ok, sizeof(ok));
                    L_curr++;
                    ritorno = n_occorrenze;
                    n_occorrenze = 0;

                }else{
                    if(buffer == argv[i+2][0]){
                        n_occorrenze++;
                    }
                }
            }

            
            exit(ritorno);
        }
        /* processo padre */
        for(int q=1; q < Q; q++){
            close(piped[q][0]);
            close(piped[q][1]);
        }
        int nw = write(piped[0][1],&ok,sizeof(ok));
        if(nw < sizeof(ok)){
            printf("Errore, nella scrittura sulla pipe\n");
            exit(8);
        }
        for(int l = 1; l < L; l++){
            printf("Linea %d del file %s", l, argv[1]);
            write(piped[0][1], &ok, sizeof(ok));
            read(piped[Q][0], &ok, sizeof(ok));
        }
        for(int q = 0; q < Q; q++){
            int pidFiglio;
            int status;	// La variabile usata per memorizzare quanto ritornato dalla primitiva wait
            int ritorno;	// La variabile usata per memorizzare il valore di ritorno del processo figlio
            if ((pidFiglio = wait(&status)) < 0) {
                printf("Non e' stato creato nessun processo figlio\n");
                exit(10);
            }
            
            if ((status & 0xFF) != 0)
            {
                printf("Il processo figlio è stato terminato in modo anomalo\n");
            } else {
                ritorno = (status >> 8) & 0xFF;
                
            }
        }
    }

    exit(0);
}