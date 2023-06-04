#include <stdio.h>		// Includo la libreria per la funzione printf e BUFSIZ
#include <stdlib.h>		// Includo la libreria per la funzione exit
#include <unistd.h>		// Includo la libreria per la funzione close, fork, famiglia exec, read, write, lseek, famiglia get, pipe,
#include <fcntl.h>		// Includo la libreria per la funzione open, creat e le relative macro
#include <sys/wait.h>	// Includo la libreria per la funzione wait
#include <string.h>

//definisco il tipo pipe_t
typedef int pipe_t[2];

int main(int argc, char** argv) {
    int Q; /* numero dei file */
    int q; /* variabile per numero processi */
    int L; /* lunghezza in linee del file*/
    pipe_t * piped;
    int i, j; /* contatore */
    int pid;
    int ritorno, status;
    int fd;
    char ok; /* variabile per dare l'ok al figlio successivo */
    char buffer;	/*carattere usato per leggere il contenuto del file*/
    int occorrenze; /* variabile per contare il numero di occorrenze */
    int nr, nw;

    if (argc < 4 + 1) /* controllo sul numero di parametri: devono essere in numero maggiore o uguale a 4*/
    {
        printf("Errore: numero di argomenti sbagliato dato che argc = %d\n", argc);
        exit(1);
    }
    Q = argc - 3;
    //controllo utlizzando la funzione atoi se il argv[2] e un numero strettamente positivo
    if((L = atoi(argv[2])) <= 0){
        printf("il parametro %s non un numero positivo\n",argv[2]);
        exit(2);
    }
    for(i=3; i < argc; i++){
        //controlla che la stringa argv[i] contenga un singolo carattere
        if (strlen(argv[i]) != 1) {	// #include <string.h>
            printf("Errore, la stringa %s non e' un singolo carattere\n", argv[i]);
            exit(3);
        }
    }
    //apro Q piped
    piped = (pipe_t*)malloc(sizeof(pipe_t) * (Q));
    if (piped == NULL) {
        printf("Errore nell'allocazione della memoria\n");
        exit(4);
    }
    
    for(int i = 0; i < Q; i++){
        if(pipe(piped[i])<0){
            printf("Errore nella creazione della pipe numero:%d\n",i);
            exit(5);
        }
    }
    for(q = 0; q < Q; q++){
        if ((pid = fork()) < 0)	/* Il processo padre crea un figlio */
        {	/* In questo caso la fork e' fallita */
            printf("Errore durante la fork\n");
            exit(6);
        }
        
        if (pid == 0)
        {	/* processo figlio */
            for(j=0;j<Q;j++){
                if(j!=q)
                    close(piped[j][0]);
                if(j!=(q+1)%Q)
                    close(piped[j][1]);
            }
            if((fd = open(argv[1], O_RDONLY)) < 0){		/* ERRORE se non si riesce ad aprire il file */
                printf("Errore in apertura file %s dato che fd = %d\n", argv[1], fd);
                exit(7);
            }
            
            while (read(fd, &buffer, sizeof(char)) > 0)	/* ciclo di lettura fino a che riesco a leggere un carattere da file */
            {
                if(buffer == '\n'){
                    // ricevere l'ok dal figlio precedente
                    nr = read(piped[q][0],&ok, sizeof(char));
                    if(nr != sizeof(char)){
                        printf("Errore, nella lettura dalla pipe\n"); 
                        exit(8);
                    }
                    printf("Figlio con indice %d e pid %d ha letto %d caratteri  %s dalla linea corrente\n", q, getpid(), occorrenze, argv[q+2]);

                    // mandare l'ok al figlio successivo
                    nw = write(piped[(q+1)%Q][1], &ok, sizeof(char));
                    if(nw != sizeof(char)){
                        printf("Errore, nella scrittura sulla pipe\n"); 
                        exit(9);
                    }
                    ritorno = occorrenze;
                    occorrenze = 0;
                }
                else{
                    if(buffer == argv[q+2][0]){
                        occorrenze++;
                    }
                }
            }

            exit(ritorno);
        }
       
    }
    /* processo padre */    

    for(q=0; q<Q+1; q++) 
	{
		if (q != Q)	close (piped[q][0]);
		if (q != 0) 	close (piped[q][1]); 
	}
	/* N.B. Poiche' in questo caso il padre e' nel ring, non ci sono problemi di dover lasciare aperti lati di pipe che il padre non usa! */

	for (j=0; j<L; j++)	/* per ogni linea del file */
       	{
		/* il padre deve riportare il numero di linea correntemente analizzata dai figli, insieme con il nome del file */
		printf("Linea %d del file %s\n", j+1, argv[1]);	/* il numero di linea deve partire da 1! */
		/* il padre deve inviare un 'segnale' di sincronizzazione al processo di indice 0 */
		nw=write(piped[0][1],&ok,sizeof(char));
		/* anche in questo caso controlliamo il risultato della scrittura */
		if (nw != sizeof(char))
		{
			printf("Padre ha scritto un numero di byte sbagliati %d\n", nw);
	        	exit(7);
		}
		/* il padre quindi deve aspettare che l'ultimo figlio gli invii il 'segnale' di sincronizzazione per fare ripartire il ring */
		nr=read(piped[Q][0],&ok,sizeof(char));
        	/* per sicurezza controlliamo il risultato della lettura da pipe */
                if (nr != sizeof(char))
                {
                	printf("Padre ha letto un numero di byte sbagliati %d\n", nr);
                       	exit(8);
                }
        }

    for(q=1;q<Q;q++){
        int pidFiglio;
        if ((pidFiglio = wait(&status)) < 0) {
            printf("Non e' stato creato nessun processo figlio\n");
            exit(11);
        }
        
        if ((status & 0xFF) != 0)
        {
            printf("Il processo figlio con PID %d è stato terminato in modo anomalo\n", pidFiglio);
        } else {
            ritorno = (status >> 8) & 0xFF;
            printf("Il figlio con pid=%d ha ritornato %d (se 255 problemi)\n", pidFiglio, ritorno);
        }
    }




    exit(0);
}