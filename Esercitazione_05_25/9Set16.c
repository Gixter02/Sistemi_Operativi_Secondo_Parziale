#include <stdio.h>	// Includo la libreria per la funzione printf e BUFSIZ
#include <stdlib.h>	// Includo la libreria per la funzione exit
#include <unistd.h>	// Includo la libreria per la funzione close, fork, exec, read, write
#include <fcntl.h>	// Includo la libreria per la funzione open e le relative macro
#include <sys/wait.h>	// Includo la libreria per la funzione wait

typedef int pipe_t[2];

typedef struct{
    char c;
    long int numero_occorrenze;
} Strut;

void bubbleSort(Strut *v, int dim)
{
   int i;
   Strut tmp;
   int ordinato = 0;
	while(dim > 0 && !ordinato)
	{
		ordinato = 1;
		for(i=0; i < dim-1; i++)
		{
			if(v[i].numero_occorrenze > v[i+1].numero_occorrenze)
			{
				tmp = v[i];
				v[i] = v[i+1];
				v[i+1] = tmp;
				ordinato = 0;
			}
		}
		dim--;
	}
}

int main(int argc, char** argv) {
    int i;
    int pid[26];
    int pidFiglio, status, ritorno;
    int fd = 0; /*variabile che conterra il file descriptor del file che stiamo per aprire */
    Strut s[26];

    pipe_t piped[26];

    char caratteri[26]={'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z'};

    if(argc != 2){
        printf("ERRORE nel numero di parametri in quanto sono: %d", argc);
        exit(1);
    }
    
    for(int i = 0;i < 26;i++){
        if(pipe(piped[i])<0){
            printf("Errore nella creazione della pipe numero:%i\n",i);
            exit(2);
        }
    }
    for(i=0; i < 26; i++){
        
        if ((pid[i] = fork()) < 0)	/* Il processo padre crea un figlio */
        {	/* In questo caso la fork e' fallita */
            printf("Errore durante la fork\n");
            exit(3);
        }
        
        if (pid[i] == 0)
        {	/* processo figlio */
            //controllo se il file e' accedibile
            
            if((fd = open(argv[1], O_RDONLY)) < 0){		/* ERRORE se non si riesce ad aprire in LETTURA il file */
                printf("Errore in apertura file %s dato che fd = %d\n", argv[1], fd);
                exit(4);
            }
        for(int j = 0 ; j< 26; j++){
            if(j != i){
                close(piped[j][1]);
            }
            if(j != i-1 || i==0){
                close(piped[j][0]);
            }

        }
        
        char buffer;
        int numero_occorrenze=0;
        
        while (read(fd, &buffer, 1) > 0)	/* ciclo di lettura fino a che riesco a leggere un carattere da file */
        {
            if(buffer == caratteri[i]){
                numero_occorrenze++;
            }
        }
        if (i != 0)
			{
				/* lettura da pipe dell'array di strutture per tutti i figli a parte il primo */
				int nr=read(piped[i-1][0],s,sizeof(s));
				if (nr!=sizeof(s)) 
	 			{ 
					printf("Errore in lettura da pipe[%d]\n", i);
					exit(-1);
				}
			}
        s[i].numero_occorrenze=numero_occorrenze;
        s[i].c=caratteri[i];

        int nw = write(piped[i][1], s, sizeof(s));
        if(nw < sizeof(s)){
            printf("ERRORE di scrittura nella pipe\n");
            exit(-1);
        }
        exit(buffer);  
        }  
            
    }
    /* processo padre */
        //chiudo tutte le pipe in lettura e scrittura tranne la i-esima pipe, aperta in scrittura
        for(int k = 0; k < 26; k++){
            close(piped[k][1]);
            if (k != 25) {
                close(piped[k][0]);
            }
    int nr = read(piped[25][0], s, sizeof(s));
    if (nr < sizeof(s)){
        printf("Errore in lettura da pipe[%d]\n", i);
		exit(-1);
    }
    bubbleSort(s, 26);

    for(i=0;i<26;i++)
	{
		printf("Il figlio di indice %d e pid %d ha trovato %ld occorrenze del carattere %c\n", s[i].c-'a', pid[(s[i].c-'a')], s[i].numero_occorrenze, s[i].c);
		/* N.B. l'indice del figlio NON e' i dato che l'array e' stato ordinato e quindi per individuare l'indice si deve usare l'informazione relativa al carattere cui il conteggio di riferisce sottraendo 'a'; chiaramente questo calcolo dell'indice va usato anche per selezionare il pid giusto dall'array pid */ 
	}	 
	}

	/* Il padre aspetta i figli */
	for (i=0; i < 26; i++)
	{
        	if ((pidFiglio = wait(&status)) < 0)
        	{
                	printf("Errore in wait\n");
                	exit(5);
        	}

        	if ((status & 0xFF) != 0)
                	printf("Figlio con pid %d terminato in modo anomalo\n", pidFiglio);
        	else
                { 
			ritorno=(int)((status >> 8) & 0xFF);
                  	printf("Il figlio con pid=%d ha ritornato il carattere %c (in decimale %d, se 255 problemi)\n", pidFiglio, ritorno, ritorno);
                }
	}

    exit(0);
}