#include <stdio.h>	// Includo la libreria per la funzione printf e BUFSIZ
#include <stdlib.h>	// Includo la libreria per la funzione exit
#include <unistd.h>	// Includo la libreria per la funzione close, fork, famiglia exec, read, write, lseek, famiglia get, pipe,
#include <fcntl.h>	// Includo la libreria per la funzione open, creat e le relative macro
#include <sys/wait.h>	// Includo la libreria per la funzione wait
#include <string.h>


//definisco il tipo pipe_t
typedef int pipe_t[2];

int main(int argc, char** argv) {
    int pid;			/* process identifier per le fork() */
	int N; 				/* numero di file passati sulla riga di comando */
	char Cx;			/* carattere passato come ultimo parametro */
	int fd; 			/* file descriptor per apertura file */
	long int pos; 			/* valore per comunicare la posizione al padre e quindi all'utente */
	int status;			/* variabile di stato per la wait */
	pipe_t *pipedFP;		/* array dinamico di pipe descriptors per comunicazioni figli-padre */
	pipe_t *pipedPF;		/* array dinamico di pipe descriptors per comunicazioni padre-figli */
	int i;			/* indici per i cicli */
	char cx; 			/* variabile che viene usata dal padre per avere dall'utente l'informazione del carattere da sostituire e  dai figli per recuperare il carattere comunicato dal padre */
	char scarto; 			/* variabile che viene usata dal padre per eliminare lo '\n' letto dallo standard input */
	char ch; 			/* variabile che viene usata dai figli per leggere dal file */
	int ritorno=0; 			/* variabile che viene ritornata da ogni figlio al padre e che contiene il numero di caratteri sostituiti nel file (supposto minore di 255 */
  	int nr;     			/* variabile che serve al padre per sapere se non ha letto nulla */
  	int finito;     		/* variabile che serve al padre per sapere se non ci sono piu' posizioni da leggere */
      
    if (argc < 3){
        printf("Errore nel numero dei parametri in quanto sono %d\n", argc); 
        exit(1);
    }
    if(strlen(argv[argc-1])!=1){
        printf("Errore, l'ultimo parametro non e' un carattere\n"); 
        exit(2);
    }
    Cx= argv[argc-1][0];
    N = argc -2;
    //apro Fasi pipes
    pipedFP = (pipe_t*)malloc(sizeof(pipe_t) * (N));
    if (pipedFP == NULL) {
        printf("Errore nell'allocazione della memoria\n");
        exit(3);
    }
    
    for(int i = 0;i < N;i++){
        if(pipe(pipedFP[i])<0){
            printf("Errore nella creazione della pipe numero:%i\n",i);
            exit(4);
        }
    }
    pipedPF = (pipe_t*)malloc(sizeof(pipe_t) * (N));
    if (pipedPF == NULL) {
        printf("Errore nell'allocazione della memoria\n");
        exit(3);
    }
    
    for(int i = 0;i < N;i++){
        if(pipe(pipedPF[i])<0){
            printf("Errore nella creazione della pipe numero:%i\n",i);
            exit(4);
        }
    }
    for(i=0; i< N; i++){
        if ((pid = fork()) < 0)	/* Il processo padre crea un figlio */
        {	/* In questo caso la fork e' fallita */
            printf("Errore durante la fork\n");
            exit(5);
        }

        if (pid == 0)
        {	/* processo figlio */
            /* Chiusura delle pipe non usate nella comunicazione con il padre  */
			for (int j=0; j < N; j++){
				close(pipedFP[j][0]); /*chiudo i lati di lettura della pipe*/
				close(pipedPF[j][1]);
                if (i != j){ 
                    close(pipedFP[j][1]); /*chiudo tutti i lati di scrittura della pipe che non sto usando*/
                    close(pipedPF[j][0]);
                }
            }
            /* apertura del file associato sia in lettura che in scrittura! */
			if ((fd=open(argv[i+1], O_RDWR)) < 0)
			{
                printf("Errore nella open del file %s\n", argv[i+1]);
                exit(-1);
            }
            while(read(fd, &ch, 1)>0){
                if(Cx == ch){   /*ho trovato l'occorrenza*/
                    pos = lseek(fd,0L,SEEK_CUR);
                    write(pipedFP[i][1],&pos,sizeof(pos));

                    read(pipedPF[i][0],&cx,sizeof(cx));
                    if(cx!='\n'){
                        pos=lseek(fd,-1L,SEEK_CUR);
                        write(fd,&cx,sizeof(cx));
                        ritorno++;
                    }
                }
            }

            exit(ritorno);
        }
    }
    /* processo padre */
    //chiudo tutte le pipe in lettura e scrittura tranne la i-esima pipe, aperta in scrittura
    for(i=0; i < N; i++){
        close(pipedFP[i][1]);
        close(pipedPF[i][0]);
    }
    finito = 0;
    while (!finito){
        finito = 1;
        for(i=0; i<N; i++){
            nr=read(pipedFP[i][0], &pos, sizeof(pos));
            if(nr < sizeof(pos)){
                printf("Errore, di lettura dalla pipe\n"); 
                exit(6);
            }
            printf("Il figlio di indice %d ha letto dal file %s nella posizione %ld il carattere %c. Se vuoi sostituirlo, fornisci il carattere altrimenti una linea vuota?\n", i,  argv[i+1], pos, Cx);
            read(0,&cx,sizeof(cx));
            if(cx != '\n'){
                read(0, &scarto, 1); /* se e' stato letto un carattere, bisogna fare una lettura a vuoto per eliminare il carattere corrispondente all'invio */
            }
            write(pipedPF[i][1], &cx, 1);  /* inviamo comunque al figlio */
        }
    }
    /* Il padre aspetta i figli */
	for (i=0; i < N; i++)
	{
		pid = wait(&status);
		if (pid < 0)
		{
			printf("Errore in wait\n");
			exit(7);
		}
		if ((status & 0xFF) != 0)
    			printf("Figlio con pid %d terminato in modo anomalo\n", pid);
    		else
		{ 	ritorno=(int)((status >> 8) &	0xFF); 
		  	printf("Il figlio con pid=%d ha ritornato %d (se 255 problemi nel figlio)\n", pid, ritorno);
		}
	}

    exit(0);
}