#include <stdio.h>		// Includo la libreria per la funzione printf e BUFSIZ
#include <stdlib.h>		// Includo la libreria per la funzione exit
#include <unistd.h>		// Includo la libreria per la funzione close, fork, famiglia exec, read, write, lseek, famiglia get, pipe,
#include <fcntl.h>		// Includo la libreria per la funzione open, creat e le relative macro
#include <sys/wait.h>	// Includo la libreria per la funzione wait
#include <errno.h>

//definisco il tipo pipe_t
typedef int pipe_t[2];

int main(int argc, char** argv) {
    int N; /* numero di file */
    int n; /* indice dei processi figli */
    char linea[200]; /* variabile per memorizzare la linea sia per il padre che per i figli */
    int pid;	// memorizza il valore di ritorno della funzione fork
    pipe_t p;
    int numero_linee = 0; /* numero di linee dei file */
    pipe_t* piped;
    int j = 0, i; /* contatore */
    int fd; /* variabile per contenere il file descriptor */
    int ritorno, status;
    int pidPrimo;

    if (argc < 2 + 1) /* controllo sul numero di parametri: devono essere in numero maggiore o uguale a 2*/
    {
        printf("Errore: numero di argomenti sbagliato dato che argc = %d\n", argc);
        exit(1);
    }

    N = argc -1;

    //creo il file file
    int file = 0; // variabile che conterra il file descriptor del file file */
    if((file = creat("/tmp/MattiaMontanari", 0644)) < 0){		/* ERRORE se non si riesce a creare il file */
        printf("Errore in creazione del file %s dato che file = %d\n", "/tmp/MattiaMontanari", file);
        exit(2);
    }
    
    if (pipe(p) < 0 )
    {	printf("Errore creazione pipe\n");
        exit(3);
    }
    
    if ((pidPrimo = fork()) < 0)	/* Il processo padre crea un figlio */
    {	/* In questo caso la fork e' fallita */
        printf("Errore durante la fork\n");
        exit(4);
    }
    
    if (pidPrimo == 0)
    {	/* processo figlio */
        //apro il file argv[1]
        int fd_speciale = 0; // variabile che conterra' il file descriptor del file argv[1]
        if((fd_speciale = open(argv[1], O_RDONLY)) < 0){		/* ERRORE se non si riesce ad aprire il file */
            printf("Errore in apertura file %s dato che fd = %d\n", argv[1], fd_speciale);
            exit(-1);
        }
        close(0);
        dup(fd_speciale);
        close(p[0]);
        close(1);
        dup(p[1]);
        close(p[1]);
        execlp("wc","wc","-l",(char*)0);
        perror("Errore, nella wc\n"); 
        exit(-1);
    }
    /* processo padre */
    close(p[1]);
    j=0;
    while (read(p[0], &(linea[j]), 1))
    {
    	j++;
    }
    /* converto l'array di char in stringa sostituendo allo '\n' il terminatore di stringa */
    if (j!=0) /*se il padre ha letto qualcosa */
    {
    	linea[j-1]='\0';
    }
    numero_linee = atoi(linea);
    close(p[0]);
    //apro N pipes
    piped = (pipe_t*)malloc(sizeof(pipe_t) * (N));
    if (piped == NULL) {
        printf("Errore nell'allocazione della memoria\n");
        exit(5);
    }
    
    for(int i = 0; i < N; i++){
        if(pipe(piped[i])<0){
            printf("Errore nella creazione della pipe numero:%d\n",i);
            exit(6);
        }
    }

    for(n = 0; n < N; n++){
        
        if ((pid = fork()) < 0)	/* Il processo padre crea un figlio */
        {	/* In questo caso la fork e' fallita */
            printf("Errore durante la fork\n");
            exit(7);
        }
        
        if (pid == 0)
        {	/* processo figlio */
            //chiudo tutte le pipe in lettura e scrittura tranne la i-esima pipe, aperta in lettura
            for(int k = 0; k < N; k++){
                close(piped[k][0]);
                if (k != n) {
                    close(piped[k][1]);
                }
            }

            if((fd = open(argv[n+1], O_RDONLY)) < 0){		/* ERRORE se non si riesce ad aprire il file */
                printf("Errore in apertura file %s dato che fd = %d\n", argv[n+1], fd);
                exit(8);
            }
            j = 0;
            while (read(fd, &linea[j], sizeof(char)) > 0)	/* ciclo di lettura fino a che riesco a leggere un carattere da file */
            {
                if( linea[j] == '\n'){
                    j++;
                    int nw;
                    nw = write(piped[n][1], &j, sizeof(int));
                    if(nw != sizeof(int)){
                        printf("Errore, nella write sulla pipe\n"); 
                        exit(9);
                    }
                    nw = write(piped[n][1], linea, j*sizeof(char));
                    if(nw != j*sizeof(char)){
                        printf("Errore, nella write sulla pipe\n"); 
                        exit(10);
                    }
                    ritorno = j;
                    j = 0;
                }else{
                    j++;
                }

            }
            exit(ritorno);
        }
    }
    /* processo padre */
    //chiudo tutte le pipe in lettura e scrittura tranne la i-esima pipe, aperta in lettura
    for(int k = 0; k < N; k++){
        close(piped[k][1]);
    }
    for(i = 0; i < numero_linee; i++){
        int lunghezza_linea;
        int nread;              // variabile che memorizzera' il ritorno della read
        int k;
        for(k = 0; k < N; k++){
            nread = read(piped[k][0], &lunghezza_linea, sizeof(int));
            if(nread != sizeof(int)){
                printf("Errore, lettura dalla pipe\n"); 
                exit(11);
            }
            nread = read(piped[k][0], linea, lunghezza_linea*sizeof(char));
            if(nread != lunghezza_linea*sizeof(char)){
                printf("Errore, lettura dalla pipe\n"); 
                exit(12);
            }
            write(file, linea, lunghezza_linea*sizeof(char));
        }
        
    }
    /* Il padre aspetta tutti i figli (quelli normali e lo speciale) e quindi N+1 */
	for (i=0; i < N+1; i++)
	{
		pid = wait(&status);
		if (pid < 0)
		{
		printf("Errore in wait\n");
		exit(13);
		}

		if ((status & 0xFF) != 0)
    		printf("Figlio con pid %d terminato in modo anomalo\n", pid);
    		else
		{ 
			ritorno=(int)((status >> 8) &	0xFF); 
			if (pid == pidPrimo) /* facciamo una stampa differenziata per il figlio speciale */
				printf("Il figlio che ha calcolato la lunghezza in linee dei file con pid=%d ha ritornato %d (se 255 significa che il figlio e' terminato con un errore)\n", pid, ritorno);
		  
			else	
				printf("Il figlio con pid=%d ha ritornato %d (se 255 significa che il figlio e' terminato con un errore)\n", pid, ritorno);
		}
	}
    exit(0);
}