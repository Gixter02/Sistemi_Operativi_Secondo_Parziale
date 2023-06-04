/* Soluzione della parte C del compito del 12 Febbraio 2016 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>

typedef int pipe_t[2];
typedef struct {
        int id;         /* indice figlio (campo c1 del testo) */
        long int occ;   /* numero occorrenze (campo c2 del testo) */
                } s_occ;

int main (int argc, char **argv)
{
   int N;          /* numero di file */
   int *pid;       /* array di pid per fork */
   		   /* l'array di pid serve perche' il padre deve ricavare il pid dall'indice del array che ricevera' dall'ultimo figlio */
   pipe_t *pipes;  /* array di pipe usate a pipeline da primo figlio, a secondo figlio .... ultimo figlio e poi a padre: ogni processo (a parte il primo) legge dalla pipe i-1 e scrive sulla pipe i */
   int i,j;        /* contatori */
   int fd;         /* file descriptor */
   int pidFiglio, status, ritorno; /* per valore di ritorno figli */
   char Cx, ch;    /* carattere da cercare e carattere letto da linea */
   s_occ *cur;     /* array di strutture usate dai figli e dal padre: ogni processo crea l'array della dimensione minima per le sue esigenze! */
   int nr, nw;     /* variabile per salvare valori di ritorno di read e write su/da pipe */

	/* controllo sul numero di parametri almeno 2 file e un carattere */
	if (argc < 4)
	{
        	printf("Errore numero di parametri, dato che argc=%d\n", argc);
        	exit(1);
	}

	/* controlliamo che l'ultimo parametro sia un singolo carattere */
	if (strlen(argv[argc-1]) != 1)
	{
        	printf("Errore ultimo parametro non singolo carattere\n");
        	exit(2);
	}

	/* individuiamo il carattere da cercare */
	Cx = argv[argc-1][0];
	/* controlliamo che sia un carattere alfabetico minuscolo */
	if (! islower(Cx)) /* N.B. per usare questa funzione bisogna includere ctype.h */
	{
        	printf("Errore ultimo parametro non alfabetico minuscolo\n");
        	exit(3);
	}
	printf("DEBUG-Carattere da cercare %c\n", Cx);

	N = argc-2;
	printf("DEBUG-Numero di processi da creare %d\n", N);

	/* allocazione pid */
	if ((pid=(int *)malloc(N*sizeof(int))) == NULL)
	{
        	printf("Errore allocazione pid\n");
        	exit(4);
	}

	/* allocazione pipe */
	if ((pipes=(pipe_t *)malloc(N*sizeof(pipe_t))) == NULL)
	{
        	printf("Errore allocazione pipe\n");
        	exit(5);
	}

	/* creazione pipe */
	for (i=0;i<N;i++)
        	if(pipe(pipes[i])<0)
        	{
                	printf("Errore creazione pipe\n");
                	exit(6);
        	}

	/* creazione figli */
	for (i=0;i<N;i++)
	{
        	if ((pid[i]=fork())<0)
        	{
                	printf("Errore creazione figli\n");
                	exit(7);
        	}
        	if (pid[i]==0)
        	{       /* codice figlio */
                	printf("DEBUG-Sono il figlio %d e sono associato al file %s\n", getpid(), argv[i+1]);
                	/* nel caso di errore in un figlio decidiamo di ritornare un valore via via crescente rispetto al massimo valore di i (cioe' N-1) */
                	/* chiusura pipes inutilizzate */
                	for (j=0;j<N;j++)
                	{
                        	if (j!=i)
                                	close (pipes[j][1]);
                        	if ((i == 0) || (j != i-1))
                                	close (pipes[j][0]); /* ci serve il lato di lettura del figlio precedente */
                	}

                	/* allocazione dell'array di strutture specifico di questo figlio */
                	/* creiamo un array di dimensione i+1 anche se leggeremo i strutture, dato che poi ci servira' riempire la i+1-esima struttura! */
                	if ((cur=(s_occ *)malloc((i+1)*sizeof(s_occ))) == NULL)
                	{
                        	printf("Errore allocazione cur\n");
                        	exit(N);
                	}
                	/* inizializziamo l'ultima struttura che e' quella specifica del figlio corrente (nel caso del primo figlio sara' l'unica struttura */
                	cur[i].id = i;
                	cur[i].occ= 0;

                	/* apertura file */
                	if ((fd=open(argv[i+1],O_RDONLY))<0)
                	{	
                        	printf("Impossibile aprire il file %s\n", argv[i+1]);
                        	exit(N+1);
                	}

                	while(read(fd,&ch,1)>0)
                	{
                        	/* cerco il carattere */
                        	if (ch == Cx)
                        	{
                                	(cur[i].occ)++;
                        	}
                	}
                	if (i!=0)
                	/* lettura da pipe dell'array di strutture per tutti i figli a parte il primo */
                	{
                        	nr=read(pipes[i-1][0],cur,i*sizeof(s_occ));
                        	if (nr != i*sizeof(s_occ))
                        	{
                                	printf("Figlio %d ha letto un numero di strutture sbagliate %d\n", i, nr);
                                	exit(N+2);
                        	}
                	}

                	/* tutti i figli mandano in avanti, l'ultimo figlio manda al padre un array di strutture (per tutti i figli a parte il primo sono i strutture ricevute dal processo precedente e la i+1-esima che e' la propria) */
                	nw=write(pipes[i][1],cur,(i+1)*sizeof(s_occ));
                        if (nw != (i+1)*sizeof(s_occ))
                        {
                               	printf("Figlio %d ha scritto un numero di strutture sbagliate %d\n", i, nr);
                                exit(N+3);
                        }
                	exit(i); /* ogni figlio deve ritornare al padre il proprio indice */
        	}
	} /* fine for */

	/* codice del padre */
	/* chiusura pipe: tutte meno l'ultima in lettura */
	for(i=0;i<N;i++)
	{
        	close (pipes[i][1]);
        	if (i != N-1) close (pipes[i][0]);
	}

	/* allocazione dell'array di strutture specifico per il padre */
	/* creiamo un array di dimensione N quanto il numero di figli! */
	if ((cur=(s_occ *)malloc(N*sizeof(s_occ))) == NULL)
	{
        	printf("Errore allocazione cur nel padre\n");
        	exit(8);
	}

	/* il padre deve leggere l'array di strutture che gli arriva dall'ultimo figlio */
	nr=read(pipes[N-1][0],cur,N*sizeof(s_occ));
	if (nr != N*sizeof(s_occ))
	{
        	printf("Padre ha letto un numero di strutture sbagliate %d\n", nr);
        	exit(9);
	}
	nr=nr/sizeof(s_occ);
	printf("DEBUG-Padre ha letto un numero %d di strutture\n", nr);
	/* il padre deve stampare i campi delle strutture ricevute */
	for(i=0;i<N;i++)
        	printf("Il figlio di indice %d e pid %d ha trovato %ld occorrenze del carattere %c nel file %s\n", cur[i].id, pid[cur[i].id], cur[i].occ, Cx, argv[(cur[i].id)+1]);

	/* Il padre aspetta i figli */
	for (i=0; i < N; i++)
	{
        	if ((pidFiglio = wait(&status)) < 0)
        	{
                	printf("Errore in wait\n");
                	exit(10);
        	}

        	if ((status & 0xFF) != 0)
                	printf("Figlio con pid %d terminato in modo anomalo\n", pidFiglio);
        	else
        	{
                	ritorno=(int)((status >> 8) & 0xFF);
                	printf("Il figlio con pid=%d ha ritornato %d (se > di %d problemi)\n", pidFiglio, ritorno, N-1);
        	}
	}

	exit(0);
}