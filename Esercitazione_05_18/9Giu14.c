#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>

typedef int pipe_t[2];

int main(int argc, char** argv){
    int pid, pidfiglio, status, ritorno, fd;
    pipe_t *piped;			/* array dinamico di pipe descriptors per comunicazioni figli-padre  */
    pipe_t p;				/* una sola pipe per ogni coppia figlio-nipote */ 
    char numero[11];			/* array di caratteri per memorizzare la stringa corrispondente al numero di righe: ci vogliono 10 char per rappresentare un intero di 16 bit e poi ci sara' il carattere di "a capo" */

    if (argc<3){
        printf("ERRORE nel numero di parametri in quanto sono %d\n", argc-1);
        exit(1);
    }
    int N = argc - 1;

    piped = (pipe_t *) malloc (N*sizeof(pipe_t));
	if (piped == NULL)
	{
		printf("Errore nella allocazione della memoria\n");
		exit(2);
	}
    /* Creazione delle N pipe figli-padre */
	for (int i=0; i < N; i++)
	{
		if(pipe(piped[i]) < 0)
		{
			printf("Errore nella creazione della pipe\n");
			exit(3);
		}
	}
     /* Il padre chiude i lati delle pipe che non usa */
	    for (int i=0; i < N; i++)
		close(piped[i][1]);

    for(int i = 0; i < N; i++){
        if((pid=fork())<0){
            printf("ERRORE nella fork\n");
            exit(4);
        }
       

        if(pid == 0){
            /* Chiusura delle pipe non usate nella comunicazione con il padre  */
			for (int j=0; j < N; j++){
				close(piped[j][0]); /*chiudo i lati di lettura della pipe*/
				if (i != j) close(piped[j][1]); /*chiudo tutti i lati di scrittura della pipe che non sto usando*/
			}


            if ((fd=open(argv[i+1], O_RDONLY))< 0){
                printf("ERRORE nell'apertura del file\n");
                exit(5);
            }
            if(pipe(p) < 0)
		    {
			printf("Errore nella creazione della pipe\n");
			exit(3);
		    }
            close(p[1]);
            if((pidfiglio=fork())<0){
                printf("ERRORE nella fork\n");
                exit(4);
            }
            if(pidfiglio==0){
                close(1);
                dup(p[1]); /*apro al posto dello standard output la pipe della scrittura duplicando al chiave*/
                close(p[0]);
                close(p[1]); /*chiudo entrambi i lati in quanto la exec scrive sulla pipe tramite lo standard output*/
                execlp("wc", "wc", "-l", argv[i+1], (char*)0);
                exit(-1);
            }
            int j=0;
            while (read(p[0], &numero[j], 1) != 0){
                j++;
            }
            int numeroletto=0;
            if(j!=0){
                numero[j-1]='\0';
                numeroletto = atoi(numero);
            }

            write(piped[i][1], &numeroletto, sizeof(int));

            ritorno=-1;
			if ((pidfiglio = wait(&status)) < 0)
			{	
				printf("Errore in wait\n");
				exit(ritorno);
			}
			if ((status & 0xFF) != 0)
    				printf("Nipote con pid %d terminato in modo anomalo\n", pid);
    			else
				/* stampa non richiesta che volendo si puo' commentare: SOLO DEBUGGING */
				printf("DEBUG-Il nipote con pid=%d ha ritornato %d\n", pidfiglio, ritorno=(int)((status >> 8) & 0xFF));
			exit(ritorno);
        }
        int valore=0;
        int somma=0;
        for(int i = 0; i < N; i++){
            read(piped[i][0], &valore,sizeof(int));
            printf("DEBUG-Il figlio di indice %d ha convertito il valore %d per il file %s\n", i, valore, argv[i+1]);
		    somma = somma + (long int)valore;
        }
        printf("La somma risultante derivante dai valori comunicati dai figli e' %ld\n", somma); 

	/* Il padre aspetta i figli */
	for (i=0; i < N; i++)
	{
		if ((pid = wait(&status)) < 0)
		{
		printf("Errore in wait\n");
		exit(5);
		}

		if ((status & 0xFF) != 0)
    			printf("Figlio con pid %d terminato in modo anomalo\n", pid);
    		else
		{ 	ritorno=(int)((status >> 8) &	0xFF); 
		  	if (ritorno==255)
 				printf("Il figlio con pid=%d ha ritornato %d e quindi vuole dire che il figlio ha avuto dei problemi oppure il nipote non � riuscito ad eseguire il wc oppure � terminato in modo anormale\n", pid, ritorno);
		  	else  	printf("Il figlio con pid=%d ha ritornato %d\n", pid, ritorno);
		}
	}

	exit(0);
    }
}