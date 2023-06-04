#include <stdio.h>	// Includo la libreria per la funzione printf e BUFSIZ
#include <stdlib.h>	// Includo la libreria per la funzione exit
#include <unistd.h>	// Includo la libreria per la funzione close, fork, exec, read, write
#include <fcntl.h>	// Includo la libreria per la funzione open e le relative macro
#include <sys/wait.h>	// Includo la libreria per la funzione wait

//definisco il tipo pipe_t
typedef int pipe_t[2];

int main(int argc, char** argv) {
    if (argc < 3) /* controllo sul numero di parametri: devono essere in numero minore a 3*/
    {
        printf("Errore: numero di argomenti sbagliato dato che argc = %d\n", argc);
        exit(1);
    }
int N = argc - 1;
int fd;
char ch;
int trasformazioni=0;
int ritorno;
//apro Fasi pipes
pipe_t* piped_nipote = (pipe_t*)malloc(sizeof(pipe_t) * (N));
if (piped_nipote == NULL) {
    printf("Errore nell'allocazione della memoria\n");
    exit(2);
}

for(int i = 0;i < N;i++){
    if(pipe(piped_nipote[i])<0){
        printf("Errore nella creazione della pipe numero:%i\n",i);
        exit(3);
    }
}
//apro Fasi pipes
pipe_t* piped = (pipe_t*)malloc(sizeof(pipe_t) * (N));
if (piped == NULL) {
    printf("Errore nell'allocazione della memoria\n");
    exit(2);
}

for(int i = 0;i < N;i++){
    if(pipe(piped[i])<0){
        printf("Errore nella creazione della pipe numero:%i\n",i);
        exit(3);
    }
}
    int i = 0;
    int pid;	// memorizzo il valore di ritorno della funzione fork
    for(i; i < N; i++){
        
        
        if ((pid = fork()) < 0)	/* Il processo padre crea un figlio */
        {	/* In questo caso la fork e' fallita */
            printf("Errore durante la fork\n");
            exit(4);
        }
        
        if (pid == 0)
        {	/* processo figlio */
        for (int j=0; j < N; j++)
			{
				close(piped[j][0]);
				if (i != j) close(piped[j][1]);
			}

            //controllo se il file e' accedibile
            
            int pidFiglio;	// memorizzo il valore di ritorno della funzione fork
            
            if ((pidFiglio = fork()) < 0)	/* Il processo padre crea un nipote */
            {	/* In questo caso la fork e' fallita */
                printf("Errore durante la fork\n");
                exit(5);
            }
            
            if (pidFiglio == 0)
            {	/* processo nipote */
            //controllo se il file e' accedibile
            fd = 0; /*variabile che conterra il file descriptor del file che stiamo per aprire */
            if((fd = open(argv[i], O_RDWR)) < 0){		/* ERRORE se non si riesce ad aprire in LETTURA il file */
                printf("Errore in apertura file %s dato che fd = %d\n", argv[i], fd);
                exit(7);
            }
            /* chiusura della pipe rimasta aperta di comunicazione fra figlio-padre che il nipote non usa */
				close(piped_nipote[i][1]);
				/* Chiusura delle pipe non usate nella comunicazione con il padre  */
				for (int j=0; j < N; j++)
				{
					close(piped_nipote[j][0]);
					if (i != j) close(piped_nipote[j][1]);
				}

                
                exit(0);
            }
            /* processo figlio */
            fd = 0; /*variabile che conterra il file descriptor del file che stiamo per aprire */
            if((fd = open(argv[i], O_RDWR)) < 0){		/* ERRORE se non si riesce ad aprire in LETTURA il file */
                printf("Errore in apertura file %s dato che fd = %d\n", argv[i], fd);
                exit(6);
            }
            {	
					/* controlliamo se abbiamo trovato un carattere alfabetico minuscolo */
					if (islower(ch))  
					{ 	
						/* questo carattere deve essere trasformato nel corrispondente carattere alfabetico MAIUSCOLO e quindi per prima cosa bisogna tornare indietro di una posizione */
						lseek(fd, -1L, SEEK_CUR);
					  	ch = ch - 32; /* trasformiamo il carattere da minuscolo in MAIUSCOLO togliendo 32 */
					  	write(fd, &ch, 1); /* scriviamolo sul file */
						/* e aggiorniamo il numero di trasformazioni */
						trasformazioni++;
					}
                write(piped_nipote[i][1], &trasformazioni, sizeof(trasformazioni));
		    /* torniamo il valore richiesto dal testo operando una divisione intera per 256 */
		    ritorno=trasformazioni/256;	
		    exit(ritorno);
            
        }
        /* processo padre */
        
    }


    exit(0);
}