#include <stdio.h>	// Includo la libreria per la funzione printf e BUFSIZ
#include <stdlib.h>	// Includo la libreria per la funzione exit
#include <unistd.h>	// Includo la libreria per la funzione close, fork, exec, read, write
#include <fcntl.h>	// Includo la libreria per la funzione open e le relative macro
#include <sys/wait.h>	// Includo la libreria per la funzione wait

//definisco il tipo pipe_t
typedef int pipe_t[2];
typedef struct {
        int pid_nipote;         /* campo c1 del testo */
        int lung_linea;         /* campo c2 del testo */
        char linea_letta[250];  /* campo c3 del testo */
	} Strut;

int main(int argc, char** argv) {
    if (argc < 3){
        printf("ERRORE nel numero di parametri in quanto sono %d", argc);
        exit(1);
    }
    pipe_t *piped;			/* array dinamico di pipe descriptors per comunicazioni figli-padre  */
    pipe_t p;				/* una sola pipe per ogni coppia figlio-nipote */ 
    Strut s;                /* struttura per la recezione dei dati dal figlio */
    int pidFiglio;
    int status;	// La variabile usata per memorizzare quanto ritornato dalla primitiva wait
    int ritorno;	// La variabile usata per memorizzare il valore di ritorno del processo figlio
    int N = argc -1;

    piped = malloc(N * sizeof(pipe_t));
    /*si puo controllare che la malloc fallsica */

    for(int i =0; i< N; i++){
        if((pipe(piped[i])) < 0){               /* creo le pipe per la comunicazione padre-figlio*/
            printf("Errore creazione pipe\n");
        exit(2);
        }
    }

    int pidFiglio;	// memorizzo il valore di ritorno della funzione fork
    
    for (int i = 0 ; i < N; i++){

        if ((pidFiglio = fork()) < 0)	/* Il processo padre crea un figlio */
        {	/* In questo caso la fork e' fallita */
            printf("Errore durante la fork\n");
            exit(3);
        }

        if (pidFiglio == 0)
        {	/* processo figlio */

            /* Chiusura delle pipe non usate nella comunicazione con il padre  */
			for (int j=0; j < N; j++){
				close(piped[j][0]); /*chiudo i lati di lettura della pipe*/
				if (i != j) close(piped[j][1]); /*chiudo tutti i lati di scrittura della pipe che non sto usando*/
			}

            int fd = 0; /*variabile che conterra il file descriptor del file che stiamo per aprire */
            if((fd = open(argv[i+1], O_RDONLY)) < 0){		/* ERRORE se non si riesce ad aprire in LETTURA il file */
                printf("Errore in apertura file %s dato che fd = %d\n", argv[i+1], fd);
                exit (4);
            }
            /*creazione della pipe con il nipote */
            if(pipe(p) < 0){
			printf("Errore nella creazione della pipe\n");
			exit(5);
		    }
            int pidnipote;
            if ((pidnipote = fork()) < 0){
                printf("ERRORE durante la fork\n");
                exit(6);
            }
            if(pidnipote == 0){
                /*processo nipote*/
                close(1);
                dup(p[1]);
                close(p[0]);
                close(p[1]);

                execlp("sort", "sort", "-f", argv[i+1], (char *) 0);

                exit(-1);
            }
            close(p[1]);
            /* adesso il figlio legge dalla pipe la PRIMA linea */
            s.pid_nipote=pidnipote; /* inizializziamo il campo con il pid del processo nipote */
            int j=0; /* inizializziamo l'indice della linea */

            while (read(p[0], &s.linea_letta[j] , 1) > 0)	/* ciclo di lettura fino a che riesco a leggere un carattere da file */
            {
                if(s.linea_letta[j] == '\n'){
                    s.lung_linea++;
                    break;
                }
                s.lung_linea++;
                j++;
            }

            write(piped[i][1], &s, sizeof(s));    
            
            if ((pidFiglio = wait(&status)) < 0) {
                printf("Non e' stato creato nessun processo figlio\n");
                exit(7);
            }
            
            if ((status & 0xFF) != 0)
            {
                printf("Il processo figlio è stato terminato in modo anomalo\n");
                exit(8);
            }
            exit(s.lung_linea - 1);
        }
        /* processo padre */
        for(int j = 0; j < N; ++j){ /* chiudiamo i lati della pipe che il padre non usa */
            close(piped[j][1]);
        }
        /* Il padre recupera le informazioni dai figli: in ordine di indice */
        for (i=0; i<N; i++)
        {
         /* si legge la struttura inviata  dal figlio i-esimo */
            int nr = read(piped[i][0], &s, sizeof(s));
            if (nr != 0)
            {
            	/* Nota bene: la stampa della linea con il formato %s richiede che il padre inserisca il terminatore di stringa nel posto giusto e cioe' DOPO il terminatore di linea */
                s.linea_letta[s.lung_linea]='\0'; /* dato che lung_linea rappresenta la lunghezza della linea, compreso il terminatore, per avere l'indice giusto SENZA SOVRASCRIVERE lo '\n' dobbiamo usare tale valore come indice */
                printf("Il nipote con pid %d ha letto dal file %s questa linea '%s' che ha lunghezza (compreso il terminatore) di %d caratteri:\n",  s.pid_nipote, argv[i+1], s.linea_letta,s.lung_linea);
             }
         }

        for(i = 0; i < N; i++){
            
            if ((pidFiglio = wait(&status)) < 0) {
                printf("Non e' stato creato nessun processo figlio\n");
                exit(9);
            }
            
            if ((status & 0xFF) != 0)
            {
                printf("Il processo figlio è stato terminato in modo anomalo\n");
                exit(10);
            } else {
                ritorno = (status >> 8) & 0xFF;
                printf("Il figlio con pid=%d ha ritornato %d (se 255 problemi nel figlio o nel nipote)\n", pidFiglio, ritorno);
            }
        }

    }
    exit(0);
}