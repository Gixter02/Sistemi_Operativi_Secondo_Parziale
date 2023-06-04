# Wiki Sistemi operativi

Per il finito si fa :

```jsx
/* VARIABILI GLOBALI */
int *finito;    /* array dinamico per indicare i figli che sono terminati */
                /* la semantica di questo array e' che ogni elemento vale 0 se il corrisponde
		 * nte processo NON e' finito, altrimenti vale 1 */
int N;          /* numero di processi figli (N.B. nome indicato dal testo) */

int finitof()
{
/* questa funzione verifica i valori memorizzati nell'array finito: appena trova un elemento uguale a 0 vuole dire che non tutti i processi figli sono finiti e quindi torna 0; tornera' 1 se e solo se tutti gli elementi dell'array sono a 1 e quindi tutti i processi sono finiti */
  int i;
	for (i=0; i < N; i++)
		if (!finito[i])
		/* appena ne trova uno che non ha finito */
			return 0; /* ritorna falso */
 	return 1;
}

POI NEL MAIN
finito = (int *) malloc(sizeof(int) * N);

/* ... e inizializzazione a 0: all'inizio nessun figlio e' finito */
memset(finito, 0, N*sizeof(int));

NEL PADRE ALLA FINE SI SCRIVE:

while(!finitof())
{
	for (n=0; n < N; n++)
	{
		/* tentiamo di leggere i caratteri dal figlio i-esimo: contestualmente viene aggiornato il valore del corrispondente elemento dell'array finito */
		nr=read(piped[n][0], chs, 2);  
		finito[n] = (nr == 0);  
		/* nel caso la read torni un valore 0, il corrispondente valore di finito viene settato a 1 dato che la condizione e' vera! */
		if (!finito[n])
	        {
			write(fdw, chs, nr);
			/* il padre scrive sul file: ATTEZNIONE SI DEVE USARE nr e NON 2 */
		}
	}
}
```

## Parziali

- 8 giugno 2022, Cerca i nomi di file presente nel primo file nei file degli altri file
    
    TESTO PARTE C: ATTENZIONE LEGGERE ANCHE LA NOTA SEGUENTE AL TESTO!
    
    La parte in C accetta un numero variabile N di parametri maggiore o uguale a 3 (da controllare) che
    rappresentano nomi assoluti di file F1, ... FN: gli N file hanno le linee tutte della stessa lunghezza
    (MSGSIZE, compreso il terminatore di linea). Ognuno degli N file (come fosse un file temporaneo
    riempito da uno script shell) contiene in ogni linea il nome di un file: come già detto, tali nomi hanno tutti
    la stessa lunghezza data da MSGSIZE-1 (da non controllare).
    
    Il processo padre deve generare N processi figli: i processi figli Pn sono associati agli N file Fh (con h=
    n+1). Ognuno di tali figli, a parte il primo, deve creare a sua volta un processo nipote PPn ogni volta che
    serve (si veda nel seguito).
    Il primo figlio P0 deve leggere via via le linee dal proprio file associato F1 e, una volta trasformata ogni
    linea (linea) in stringa, deve mandare linea (che rappresenta il nome di un file) via via a tutti gli altri
    fratelli (P1 .. PN-1).
    
    Gli altri processi P1 .. PN-1 devono ricevere via via i nomi inviati dal figlio P0 e, per ogni nome ricevuto
    (buffer), devono via via leggere i nomi presenti nel proprio file associato (linea): per ogni coppia buffer-
    linea il processo Pn (con n!=0) deve creare un processo nipote. Ogni processo nipote PPi esegue
    concorrentemente e deve confrontare i file di nome buffer e linea, usando in modo opportuno il comando
    diff di UNIX/Linux.
    
    Ogni processo figlio Pn (con n!=0) deve aspettare ogni nipote creato e, sulla base del valore di ritorno,
    deve stampare su standard output se il contenuto del file di nome buffer è uguale al contenuto del file di
    nome linea (esattamente in questo ordine): ad esempio
    I file p1 e f1 sono uguali
    
    Al termine, ogni processo figlio Pn deve ritornare al padre il proprio numero d’ordine (n) e il padre deve
    stampare su standard output il PID di ogni figlio e il valore ritornato.
    
    NOTA BENE NEL FILE C main.c SI USI OBBLIGATORIAMENTE:
    
    - una variabile di nome N per il numero di file;
    - una variabile di nome n per l’indice dei processi figli;
    - una costante di nome MSGSIZE per la lunghezza delle linee (compreso il terminatore di linea); N.B.
        
        Per provare la soluzione si deve chiaramente scegliere un valore per tale costante!
        
    - una variabile di nome linea (che rappresenta il nome di un file) per memorizzare la linea letta dai figli
        
        dal file associato;
        
    - una variabile di nome buffer per leggere il nome inviato dal figlio P0 ai fratelli.
    
    ```jsx
    /* Soluzione della Prova d'esame del 8 Giugno 2022 - SOLO Parte C */
    #include <stdio.h>
    #include <string.h>
    #include <fcntl.h>
    #include <sys/stat.h>
    #include <unistd.h>
    #include <stdlib.h>
    #include <sys/wait.h>
    #define MSGSIZE 3	/* COSTANTE CON NOME INDICATO DAL TESTO E CON IL MINIMO VALORE POSSIBILE: 2 caratteri piu' il terminatore di linea/stringa */
    
    typedef int pipe_t[2];
    
    int main(int argc, char **argv) 
    {
       /* -------- Variabili locali ---------- */
       int pid;			/* process identifier per le varie fork() */
       int N; 			/* numero di file passati sulla riga di comando (uguale al numero di processi figli) */
       int Npipe; 			/* numero di pipe */
       int fd; 			/* file descriptor per apertura file */
       int status;			/* variabile di stato per la wait */
       pipe_t *piped;		/* array dinamico di pipe descriptors per comunicazioni figli-padre */
       int n;			/* indice per i figli */
       int i, j;			/* indici per i cicli */
       char linea[MSGSIZE];		/* array di caratteri per memorizzare la linea: come indicato dal testo si puo' supporre una lunghezza di ogni linea di MSGSIZE caratteri compreso il terminatore di linea */
       char buffer[MSGSIZE];	/* array di caratteri per memorizzare la stringa ricevuta dal primo figlio (stessa dimensione di linea) */
       int ritorno; 		/* variabile che viene ritornata da ogni figlio al padre e che contiene la lunghezza media delle linee (supposta strettamente minore di 255); questa variabile viene usata anche dal padre per recuperare appunto il valore tornato dai singoli figli */
       /* ------------------------------------ */
    
    	printf("DEBUG-Sono il padre con pid %d\n", getpid());
    	
    	/* Controllo sul numero di parametri */
    	if (argc < 4) /* Meno di tre parametri */  
    	{
    		printf("Errore nel numero dei parametri dato che argc = %d\n", argc);
    		exit(1);
    	}
    
    	/* Calcoliamo il numero di file passati */
    	N = argc - 1;
    
    	/* Calcoliamo il numero di pipe: ATTENZIONE CI VOGLIONO N-1 pipe! */
    	Npipe = N - 1;
    	
    	/* Allocazione dell'array di Npipe pipe descriptors */
    	piped = (pipe_t *) malloc (Npipe*sizeof(pipe_t));
    	if (piped == NULL)
    	{
    		printf("Errore nella allocazione della memoria\n");
    		exit(2);
    	}
    	
    	/* Creazione delle Npipe pipe figli-padre */
    	for (i=0; i < Npipe; i++)
    	{
    		if(pipe(piped[i]) < 0)
    		{
    			printf("Errore nella creazione della pipe\n");
    			exit(3);
    		}
    	}
    
    	/* Ciclo di generazione dei figli */
    	for (n=0; n < N; n++)
    	{
    		if ( (pid = fork()) < 0)
    		{
    			printf("Errore nella fork del figlio %d-esimo\n", n);
    			exit(4);
    		}
    		
    		if (pid == 0) 
    		{
    			/* codice del figlio */
    			printf("DEBUG-Sono il processo figlio di indice %d e pid %d e sono associato al file %s\n", n, getpid(), argv[n+1]);
    
    			if (n == 0) /* se e' il primo figlio il codice e' differente */
    			{
    				/* Chiusura delle pipe non usate nella comunicazione */
    				for (j=0; j < Npipe; j++)
    				{
    					close(piped[j][0]);
    				}
    			}
    			else /* se siamo negli altri figli a parte il primo */
    			{
    
    				/* Chiusura delle pipe non usate nella comunicazione */
    				for (j=0; j < Npipe; j++)
    				{
    					close(piped[j][1]);
    					if ((n-1) != j) close(piped[j][0]);
    				}
    			}
    
    			/* TUTTI i figli devono aprire il file associato */
    			if ((fd=open(argv[n+1], O_RDONLY)) < 0)
    			{
                                    printf("Errore nella open del file %s\n", argv[n+1]);
                                    exit(N); /* in caso di errore nei figli decidiamo di tornare un valore crescente da N in poi */
                           	}
    			if (n == 0) /* se e' il primo figlio il codice e' differente */
    			{
    				/* il primo figlio legge dal file una linea alla volta: N.B. la lettura si puo' fare direttamente di MSGSIZE caratteri! */
    		        	while (read(fd, linea, MSGSIZE))
    				{
    					/* dobbiamo trasformare la linea in una stringa */
    					linea[MSGSIZE-1] = '\0'; 
    					//printf("DEBUG-Sono il PRIMO processo figlio di indice %d e pid %d e ho letto dal file %s\n", n, getpid(), linea);
    					for (j=0; j < Npipe; j++)
    					{
    						write(piped[j][1], linea, MSGSIZE);	/* il primo figlio deve mandare ogni linea (trasformata in stringa) a tutti i fratelli! */
                           			}
                           		}
                           	}
    			else /* se siamo negli altri figli a parte il primo */
    			{
    				/* dobbiamo ricevere le stringhe inviate dal primo figlio */
    				while (read(piped[n-1][0], buffer, MSGSIZE))
    				{
    			           	/* dato che il primo figlio ha inviato delle stringhe, il figlio corrente le puo' usare direttamente */
    					//printf("DEBUG-Sono il processo figlio di indice %d e pid %d e ho letto dalla pipe %s\n", n, getpid(), buffer);
    					/* anche gli altri figli devono leggere il proprio file una linea alla volta */
    	                               	while (read(fd, linea, MSGSIZE))
    					{
    						/* dobbiamo trasformare la linea in una stringa */
    						linea[MSGSIZE-1] = '\0';
    						printf("DEBUG-Sono il processo figlio di indice %d e pid %d e ho letto dal file %s\n", n, getpid(), linea);
    
    						/* ora dobbiamo creare un processo nipote */ 
                            			if ( (pid = fork()) < 0)	/* dobbiamo creare un nipote per ogni coppia di nomi di file */
    			                       	{
    			                       		printf("Errore nella fork di creazione del nipote\n");
    			                                exit(N+1);
    			                        }
    			                        if (pid == 0)
    			                        {
    			                        	/* codice del nipote: attenzione all'indice da usare per associare ogni nipote al giusto file! */
    			                                printf("DEBUG-Sono il processo nipote del figlio di indice %d e ho pid %d e faro' il diff fra i file %s e %s\n", i, getpid(), linea, buffer);
    			                                /* chiusura della pipe non usata */
    			                                close(piped[n-1][0]);
    
    							/* ridirezionamo lo standard output su /dev/null perche' ci interessa solo se il comando diff ha successo o meno */
    							close(1);
    							open("/dev/null", O_WRONLY);
    							/* ridirezionamo anche lo standard error su /dev/null perche' ci interessa solo se il comando diff ha successo o meno */
    							close(2);
    							open("/dev/null", O_WRONLY);
    
    							execlp("diff", "diff", buffer, linea, (char *)0); /* attenzione prima il nome ricevuto e poi il proprio */
    
    						 	/* si esegue l'istruzione seguente SOLO in caso di fallimento della execlp */
    							/* ATTENZIONE SE LA EXEC FALLISCE NON HA SENSO NE' FARE printf("Errore in execlp\n"); ne' perror */
    							exit(-1); /* torniamo un -1 che sara' interpretato come 255 e quindi identificato come errore */
    			                        }
    					 	/* il figlio deve aspettare ogni nipote. Non e' richiesta la stampa la inseriamo lo stesso per controllo: stampiaamo il suo pid con il valore ritornato */
    						if ((pid = wait(&status)) < 0)
    						{
    							printf("Errore in wait\n");
    							exit(-1);
    						}
    						if ((status & 0xFF) != 0)
    							printf("Nipote con pid %d terminato in modo anomalo\n", pid);
    						else
    						{
    							/* ATTENZIONE: stampa non richiesta */
    							printf("DEBUG-Il nipote con pid=%d ha ritornato %d\n", pid, ritorno=(status >> 8) & 0xFF);
    							/* omettiamo il controllo se il nipote NON riesce ad eseguire la exec! */
    							if (ritorno == 0)
    								printf("I file %s e %s sono uguali\n", buffer, linea); 	/* attenzione prima il nomee ricevuto e poi il proprio */
    							else	/* la stampa seguente e' solo di DEBUG dato che il testo chiedeva solo di indicare se i file erano uguali! */
    								printf("DEBUG-I file %s e %s sono diversi\n", buffer, linea);
    
    			              		}
    			              	}
    					/* finito di confrontare il nome letto dalla pipe, bisogna tornare all'inizio del file capo per confrontare il prossimo nome con tutti gli altri */
    					lseek(fd, 0L, 0);
    				}
    			}
    			/* torniamo il valore richiesto dal testo */
    			exit(n);
    		}
    	}
    	
    	/* Codice del padre */
    	/* Il padre chiude tutti i lati delle pipe dato che non le usa */
    	for (i=0; i < Npipe; i++)
    	{
    		close(piped[i][0]);
    		close(piped[i][1]);
    	}
    
    	/* Il padre aspetta tutti i figli */
    	for (i=0; i < N; i++)
    	{
    		pid = wait(&status);
    		if (pid < 0)
    		{
    		printf("Errore in wait\n");
    		exit(5);
    		}
    
    		if ((status & 0xFF) != 0)
        		printf("Figlio con pid %d terminato in modo anomalo\n", pid);
        		else
    		{ 
    			ritorno=(int)((status >> 8) &	0xFF); 
    			printf("Il figlio con pid=%d ha ritornato %d (se 255 significa che il figlio e' terminato con un errore)\n", pid, ritorno);
    		}
    	}
    	exit(0);
    }
    ```
    
- 25 gennaio 2023, leggere due caratteri alla volta, ci sono indicazioni utili nel caso un figlio mandi un numero di byte diverso da 2
    
    La parte in C accetta un numero variabile di parametri **N+1** (con **N** maggiore o uguale a **2)** che rappresentano nomi di file
    (**F1, ...FN, FN+1**).
    Il processo padre deve, per prima cosa, aprire il file **FN+1** in scrittura e quindi deve generare un numero di **processi figli**
    pari a **N**: ogni processo figlio **Pn** è associato ad uno dei primi **N** file **F1, ...FN** (*in ordine*). Ognuno di tali processi figli **Pn**
    esegue concorrentemente e legge il proprio file associato, come specificato in seguito.
    Ogni figlio **Pn** deve leggere 2 caratteri alla volta dal proprio file associato, fino alla fine di tale file, e deve mandare i
    caratteri correntemente letti al padre; il padre deve ricevere i caratteri inviati via via dai figli: prima i 2 caratteri inviati dal
    figlio **P0**, poi i 2 caratteri inviati dal figlio **P1** e via via fino ai 2 caratteri inviati dal figlio **PN-1, per poi ricominciare a
    ricevere altri** 2 caratteri inviati dal figlio **P0**, e così via fino a che non ci saranno più caratteri da ricevere dai figli; fare
    attenzione che se la dimensione del file associato ad un figlio non è un multiplo intero di 2, tale figlio manderà nell’ultimo
    invio un numero minore di 2 caratteri al padre! Il padre deve scrivere tutti i caratteri ricevuti dai figli alla fine del file **FN+1.**
    Al termine dell’esecuzione, ogni figlio **Pn** ritorna al padre il numero totale di caratteri inviati (*supposto minore di 255*); il
    padre deve stampare su standard output il PID di ogni figlio e il valore ritornato.
    
    **NOTA BENE NEL FILE C main.c SI USI OBBLIGATORIAMENTE:**
    - una variabile di nome **N** per il numero di processi figli;
    - una variabile di nome **n** per l’indice dei processi figli;
    - una variabile **fdw** per il file descriptor del file aperto in scrittura;
    
    - una variabile di nome **chs** per l’array da passare dai figli al padre.
    
    ```jsx
    
    while(!finitof())
    {
    	for (n=0; n < N; n++)
    	{
    		/* tentiamo di leggere i caratteri dal figlio i-esimo: contestualmente viene aggiornato il valore del corrispondente elemento dell'array finito */
    		nr=read(piped[n][0], chs, 2);  
    		finito[n] = (nr == 0);  
    		/* nel caso la read torni un valore 0, il corrispondente valore di finito viene settato a 1 dato che la condizione e' vera! */
    		if (!finito[n])
    	        {
    			write(fdw, chs, nr);
    			/* il padre scrive sul file: ATTEZNIONE SI DEVE USARE nr e NON 2 */
    		}
    	}
    }
    ```
    
- 22 febbraio 2023, convertire i caratteri numerici in numeri di un file senza scriverli sul file
    
    La parte in C accetta un numero variabile di parametri N maggiore o uguale a 2 che rappresentano nomi di file (F1, ...FN):
    si assuma (senza bisogno di effettuare alcun controllo) che il formato di tali file sia tale che i caratteri in posizione pari
    (offset interno al file 0, 2, 4, etc.) siano caratteri numerici, mentre i caratteri di posizione dispari siano caratteri ‘spazio’
    (cioè blank)*.
    Il processo padre deve generare un numero di processi figli pari a N: ogni processo figlio Pn è associato ad uno dei file F1,
    ...FN (in ordine). Ognuno di tali processi figli Pn esegue concorrentemente e legge tutti i caratteri del proprio file associato
    operando una opportuna selezione come indicato nel seguito.
    Ogni processo figlio Pn deve convertire i caratteri numerici nei corrispondenti numeri interi e calcolare via via la loro
    somma: tale somma deve essere memorizzata in una variabile di tipo long int (somma). Al termine della lettura del proprio
    file associato, ogni processo figlio Pn deve comunicare al processo padre somma (chiaramente in formato long int). Il
    processo padre deve ricevere, rispettando l'ordine dei file F1, ...FN, da ogni figlio i valori long int che rappresentano le
    somme calcolate dai figli e deve stamparle su standard output insieme con l’indice del processo che ha comunicato tale
    valore e il nome del file cui tale somma si riferisce.
    Al termine dell’esecuzione, ogni figlio Pn ritorna al padre il numero (garantito dalla parte shell essere minore di 255) di
    caratteri numerici trovati nel file associato; il padre deve stampare su standard output il PID di ogni figlio e il valore
    ritornato.
    
    NOTA BENE NEL FILE C main.c SI USI OBBLIGATORIAMENTE:
    - una variabile di nome N per il numero di processi figli;
    - una variabile di nome n per l’indice dei processi figli;
    - una variabile di nome car per il carattere letto correntemente dai figli dal proprio file;
    - una variabile di nome somma per il valore long int che i figli devono comunicare al padre.
    
    ```jsx
    #include <stdio.h>	// Includo la libreria per la funzione printf e BUFSIZ
    #include <stdlib.h>	// Includo la libreria per la funzione exit
    #include <unistd.h>	// Includo la libreria per la funzione close, fork, exec, read, write
    #include <fcntl.h>	// Includo la libreria per la funzione open e le relative macro
    #include <sys/wait.h>	// Includo la libreria per la funzione wait
    #include <ctype.h>
    
    //definisco il tipo pipe_t
    typedef int pipe_t[2];
    
    int main(int argc, char** argv) {
    
        /*******************/
        int N=argc-1;
        long int somma;
        int occ;
        char car;
        int pid;	// memorizzo il valore di ritorno della funzione fork
        int fd = 0; /*variabile che conterra il file descriptor del file che stiamo per aprire */
        int pidChild;
    
        int ritorno, status;
        /*******************/
    
        if (argc < 3) /* controllo sul numero di parametri: devono essere in numero maggiore o uguale a 2*/
        {
            printf("Errore: numero di argomenti sbagliato dato che argc = %d\n", argc);
            exit(1);
        }    
    
        //apro N pipes
        pipe_t* piped=malloc(sizeof(pipe_t)*N);
        for(int i=0;i<N;i++){
            if(pipe(piped[i])<0){
                printf("Errore nella creazione della pipe numero:%i",i);
                exit(2);
            }
        }
    
        for (int n = 0; n < N; n++)
        {
            /* code */
            
            if ((pid = fork()) < 0)	/* Il processo padre crea un figlio */
            {	/* In questo caso la fork e' fallita */
                printf("Errore durante la fork\n");
                exit(3);
            }
            
            if (pid == 0)
            {	/* processo figlio */
                
                //chiudo tutte le pipe in lettura e scrittura tranne la i-esima pipe, aperta in scrittura
                for(int k = 0; k < N; k++){
                    close(piped[k][0]);
                    if (k != n) {
                        close(piped[k][1]);
                    }
                }
    
                //controllo se il file e' accedibile
                if((fd = open(argv[n+1], O_RDONLY)) < 0){		/* ERRORE se non si riesce ad aprire in LETTURA il file */
                    printf("Errore in apertura file %s dato che fd = %d\n", argv[n+1], fd);
                    exit(4);
                }
                somma=0;
                occ=0;
                while(read(fd, &car, 1)) {
                    if(isdigit(car)) {
                        somma += atoi(&car);
                        occ++;
                    }
                }
                ritorno=occ;
                write(piped[n][1], &somma, sizeof(long int));
                exit(ritorno);
            }
            
    
        }
        /* processo padre */
        
        //chiudo tutte le pipe in lettura e scrittura tranne la i-esima pipe, aperta in scrittura
        for(int k = 0; k < N; k++){
            close(piped[k][1]);
        }
    
        for (int n = 0; n < N; n++)
        {
            /* code */
            read(piped[n][0], &somma, sizeof(long int));
    
            printf("Il figlio di indice %d ha ottenuto la somma: %li dal file %s\n", n, somma, argv[n+1]);
    
        }
        for (int i = 0; i < N; i++)
        {
            /* code */
        
            
        if ((pidChild = wait(&status)) < 0) {
            printf("Non e' stato creato nessun processo figlio\n");
            exit(4);
        }
        
        if ((status & 0xFF) != 0)
        {
            printf("Il processo figlio è stato terminato in modo anomalo\n");
            exit(5);
        } else {
            ritorno = (status >> 8) & 0xFF;
            printf("il figlio pid=%i ed ha ritornato=%i\n",pidChild,ritorno);
        }
        }
        exit(0);
    }
    ```
    
- 9 giugno 2021, uso di wc e scrittura sulla pipe di 2 parametri senza usare struct
    
    La parte in C accetta un numero variabile N di parametri maggiore o uguale a 2 (da controllare) che
    rappresentano nomi assoluti di file F1, ... FN (che hanno la stessa lunghezza in linee che non va
    controllata). Il processo padre deve, per prima cosa, creare nella directory di sistema /tmp un file fcreato
    con nome corrispondente alla concatenazione (senza spazi) del proprio nome e del proprio cognome (in
    caso di più nomi o più cognomi se ne usi solo uno, inserendo un opportuno commento).
    
    Quindi deve generare un processo figlio ‘speciale’ che deve calcolare la lunghezza in linee del file F1
    (che sarà poi la lunghezza in linee di tutti i file) usando in modo opportuno il comando wc di UNIX/Linux.
    Il padre deve convertire in termini di valore intero quanto scritto in termini di caratteri sullo standard
    output dal comando wc eseguito dal figlio speciale: tale valore servirà al padre per recuperare le
    informazioni inviate dagli ulteriori figli.
    
    Il processo padre poi deve generare N processi figli: i processi figli Pn sono associati agli N file Ff (con
    f= n+1). Ognuno di tali figli esegue concorrentemente leggendo dal file associato tutte le linee (ogni
    linea si può supporre che abbia una lunghezza massima di 200 caratteri, compreso il terminatore di linea):
    ogni figlio Pn, per ogni linea letta, deve comunicare al padre prima la lunghezza (come int) della linea
    corrente compreso il terminatore di linea e quindi la linea stessa. Il padre deve ricevere rispettando
    l’ordine delle linee e quindi l'ordine dei file, utilizzando in modo opportuno le informazioni inviate dai
    figli: il padre deve scrivere tutte le linee inviate sul file fcreato. Quindi in tale file al termine si dovrà
    trovare una sorta di merge linea-linea dei file passati come parametri e quindi si dovrà trovare la prima
    linea inviata dal processo figlio P0 (letta dal file F1), quindi la prima linea inviata dal processo figlio P1
    (letta dal file F2) e così via.
    
    Al termine, ogni processo figlio Pn deve ritornare al padre la lunghezza dell’ultima linea inviata al padre
    compreso il terminatore di linea e il padre deve stampare su standard output il PID di ogni figlio e il
    valore ritornato.
    
    NOTA BENE NEL FILE C main.c SI USI OBBLIGATORIAMENTE:
    
    - una variabile di nome N per il numero di file/processi figli;
    - una variabile di nome n per l’indice dei processi figli;
    - una variabile di nome linea per memorizzare la linea (sia da parte dei figli che da parte del padre).
    
    ```jsx
    /* Soluzione della Prova d'esame del 9 Giugno 2021 - SOLO Parte C */
    #include <stdio.h>
    #include <string.h>
    #include <fcntl.h>
    #include <sys/stat.h>
    #include <unistd.h>
    #include <stdlib.h>
    #include <sys/wait.h>
    
    typedef int pipe_t[2];
    
    int main(int argc, char **argv) 
    {
    	/* -------- Variabili locali ---------- */
    	int pidPrimo;			/* process identifier per la prima fork() */
    	int pid;			/* process identifier per le altre fork() */
    	int N; 				/* numero di file passati sulla riga di comando (uguale al numero di processi figli normali) */
    	int X; 				/* numero che sara' la lunghezza in linee dei file passati sulla riga di comando */
    	int fdout; 			/* file descriptor per creazione file da parte del padre */
    	int fd; 			/* file descriptor per apertura file */
    	int status;			/* variabile di stato per la wait */
    	pipe_t	p;			/* pipe fra figlio che calcola lunghezza e padre */
    	pipe_t *piped;			/* array dinamico di pipe descriptors per comunicazioni figli-padre */
    	int n;				/* indice per i figli */
    	int i, j;			/* indici per i cicli */
    	char linea[200];		/* array di caratteri per memorizzare la linea: come indicato dal testo si puo' supporre una lunghezza massima di ogni linea di 200 caratteri compreso il terminatore di linea */
    	int valore; 			/* variabile che viene usata dal padre per recuperare il valore comunicato da ogni figlio e che contiene la lunghezza della linea selezionata */
    	int ritorno; 			/* variabile che viene ritornata da ogni figlio al padre e che contiene la lunghezza media delle linee (supposta strettamente minore di 255) */
    	/* ------------------------------------ */
    
    	printf("Sono il padre con pid %d\n", getpid());
    	
    	/* Controllo sul numero di parametri */
    	if (argc < 3) /* Meno di due parametri */  
    	{
    		printf("Errore nel numero dei parametri\n");
    		exit(1);
    	}
    
    	/* Calcoliamo il numero di file passati */
    	N = argc - 1;
    	
    	/* PER PRIMA COSA creazione file in /tmp con mio nome e cognome */
    	if ((fdout=creat("/tmp/LetiziaLeonardi", 0644)) < 0)  
    	{
                  	printf("Errore nella creat del file %s\n", "/tmp/LetiziaLeonardi");
                    exit(2);
    	}
     
    	/* Creazione pipe fra figlio speciale che calcola lunghezza in linee e padre */
    	if(pipe(p) < 0)
            {
                   	printf("Errore nella creazione della pipe con figlio speciale\n");
                    exit(3);
            }
    
    	/* generiamo il processo figlio speciale che deve calcolare la lunghezza dei file: dato che i file e' noto che abbiano tutti la stessa lunghezza in linee questo figlio usera' il file argv[1], come indicato nel testo*/
    	pidPrimo = fork();
    	if (pidPrimo < 0)
    	{       /* fork fallita */
            	printf("Errore nella prima fork\n");
            	exit(4);
    	}
    
    	if (pidPrimo == 0)
    	{       /* figlio speciale che calcola lunghezza in linee di argv[1] */
            	printf("Esecuzione di wc -l da parte del figlio con pid %d\n", getpid());
            	/* ridirezionamo lo standard input in modo da leggere dal file passato come primo parametro */
            	close(0);
            	if (open(argv[1], O_RDONLY))
            	{       printf("Errore in apertura file %s\n", argv[1]);
                    	exit(-1); /* ritorniamo -1 al padre che verra' interpretato come 255 e quindi come errore */
            	}
            	/* dobbiamo implementare il piping e quindi lo standard output lo dobbiamo agganciare al lato di scrittura della pipe p */
            	close(1);
            	dup(p[1]);
    		close(p[0]); /* dobbiamo chiudere il lato di lettura che non viene usato */
    		close(p[1]); /* possiamo chiudere il lato di scrittura che verra' usato tramite il fd 1 */
            	execlp("wc", "wc", "-l", (char *)0); /* passiamo come parametro solo l'opzione -l */
    
            	/* si esegue l'istruzione seguente SOLO in caso di fallimento della execlp */
            	/* ATTENZIONE SE LA EXEC FALLISCE NON HA SENSO FARE printf("Errore in execlp\n"); DATO CHE LO STANDARD OUTPUT E' AGGANCIATO ALLA pipe p ma dobbiamo usare perror*/
    		perror("Errore nella esecuzone di wc -l");
            	exit(-1); /* torniamo al padre un -1 che sara' interpretato come 255 e quindi identificato come errore */
    	}	
    
    	/* padre chiude il lato che non usa di p */
    	close(p[1]);
    	/* il padre deve recuperare il valore calcolato dal wc -l dalla pipe leggendo tutti i caratteri (numerici) che saranno inviati dal figlio: poichÃ¨ non si sa quanti caratteri saranno inviati il padre deve leggerli tutti; il padre puo' usare l'array linea */
    	j=0;
            while (read(p[0], &(linea[j]), 1))
            {
            	j++;
            }
            /* converto l'array di char in stringa sostituendo allo '\n' il terminatore di stringa */
            if (j!=0) /*se il padre ha letto qualcosa */
            {
           		linea[j-1]='\0';
    		X=atoi(linea);	/* dobbiamo convertire la stringa in numero */
    		/* stampa di debugging NON richiesta */
    		printf("Padre ricevuto valore %d\n", X);
    	}
    	else
          	{
    		printf("Errore nel calcolo numero di linee da parte del figlio speciale\n");
    		exit(5); /* se il figlio speciale NON ha inviato nulla NON si possono creare i figli normali e quindi il padre termina */
            }
    	
    	/* padre prima di creeare gli altri figli chiude da pipe usata con il figlio speciale */
    	close(p[0]);
    
    	/* Allocazione dell'array di N pipe descriptors*/
    	piped = (pipe_t *) malloc (N*sizeof(pipe_t));
    	if (piped == NULL)
    	{
    		printf("Errore nella allocazione della memoria\n");
    		exit(6);
    	}
    	
    	/* Creazione delle N pipe figli-padre */
    	for (i=0; i < N; i++)
    	{
    		if(pipe(piped[i]) < 0)
    		{
    			printf("Errore nella creazione della pipe\n");
    			exit(7);
    		}
    	}
    
    	/* Ciclo di generazione dei figli normali */
    	for (n=0; n < N; n++)
    	{
    		if ( (pid = fork()) < 0)
    		{
    			printf("Errore nella fork del figlio %d-esimo\n", n);
    			exit(8);
    		}
    		
    		if (pid == 0) 
    		{
    			/* codice del figlio */
    			printf("Sono il processo figlio di indice %d e pid %d e sono associato al file %s\n", n, getpid(), argv[n+1]);
    			/* Chiusura delle pipe non usate nella comunicazione con il padre */
    			for (j=0; j < N; j++)
    			{
    				close(piped[j][0]);
    				if (n != j) close(piped[j][1]);
    			}
    
    			if ((fd=open(argv[n+1], O_RDONLY)) < 0)
    			{
                                    printf("Errore nella open del file %s\n", argv[n+1]);
                                    exit(-1); /* in caso di errore nei figli decidiamo di tornare -1 che corrispondera' per il padre al valore 255 che supponiamo non essere un valore accettabile di ritorno */
                           	}
    			/* adesso il figlio legge dal file una linea alla volta */
    			j=0; /* azzeriamo l'indice della linea */
    		        while (read(fd, &(linea[j]), 1))
    			{
    				if (linea[j] == '\n') 
    			 	{ 
    					/* dobbiamo mandare al padre la lunghezza della linea selezionata  compreso il terminatore di linea (come int) e quindi incrementiamo j */
    				   	j++;
    				   	write(piped[n][1], &j, sizeof(j));
    				   	/* e quindi la linea stessa; N.B. SBAGLIATO convertirla in stringa! */
    				   	write(piped[n][1], linea, j);
    					ritorno=j; /* salviamo la lunghezza corrente dato che dovremo ritornare al padre la lunghezza dell'ultima linea */
    				   	j=0; /* azzeriamo l'indice per le prossime linee */
    				}
    				else j++; /* continuiamo a leggere */
    			}
    			/* ogni figlio deve ritornare al padre il valore corrispondente alla lunghezza dell'ultima linea inviata al padre */
    			exit(ritorno);
      			}	
    		}
    	
    	/* Codice del padre */
    	/* Il padre chiude i lati delle pipe che non usa */
    	for (i=0; i < N; i++)
    		close(piped[i][1]);
    
    	/* Il padre recupera le informazioni dai figli: prima in ordine di linee e quindi in ordine di indice */
            for (j=1; j <= X; j++)
    		for (n=0; n < N; n++)
    		{ 
    			/* il padre recupera prima la lunghezza della linea da ogni figlio */
    			read(piped[n][0], &valore, sizeof(valore));
    			/* quindi recupera la linea selezionata dal ogni figlio */
    			read(piped[n][0], linea, valore);
    			/* adesso va scritta sul file; N.B. la scriviamo come linea e quindi SBAGIATO convertirla in stringa! */
    			write(fdout, linea, valore);
    		}	
    
    	/* Il padre aspetta tutti i figli (quelli normali e lo speciale) e quindi N+1 */
    	for (i=0; i < N+1; i++)
    	{
    		pid = wait(&status);
    		if (pid < 0)
    		{
    		printf("Errore in wait\n");
    		exit(9);
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
    ```
    
- 16 giugno 2021, schema di piping a ring
    
    La parte in C accetta un numero variabile di parametri N maggiore o uguale a 2 che rappresentano nomi di file (F1,
    ...FN). Il processo padre deve, per prima cosa, creare nella directory corrente un file fcreato con nome corrispondente
    al proprio cognome (in caso di più cognomi se ne usi solo uno, inserendo un opportuno commento).
    Il processo padre deve generare un numero di processi figli pari a N: ogni processo figlio Pn è associato ad uno dei file
    F1, ...FN (in ordine); la lunghezza in linee di tutti i file è uguale e non deve essere controllata.
    Ognuno di tali processi figli Pn esegue concorrentemente e legge tutte le linee del proprio file associato: le linee lette
    devono essere scritte nel file fcreato seguendo le indicazioni fornite nel seguito.
    I processi figli Pn devono usare uno schema di comunicazione a ring: la comunicazione deve prevedere l’invio di un
    array di grandezza N e in cui ogni elemento dell’array corrisponde alla linea corrente (supposta lunga 250 caratteri
    compreso il terminatore di linea1) letta dal corrispondente processo figlio Pn. Quindi, il generico processo Pn, dopo
    aver letto la linea corrente, deve ricevere dal figlio precedente l’array di linee e, dopo aver inserito la linea corrente nella
    posizione giusta dell’array di linee, deve inviare l’array di linee al figlio successivo, con PN-1 che manda a P0; il figlio
    PN-1, prima di riprendere il ciclo di lettura, deve scrivere le linee correnti sul file fcreato usando chiaramente l’array
    di linee corrente. Per semplicità, il primo ciclo può essere attivato dal padre che manda un array di linee (senza
    informazioni significative) al primo figlio P0.
    Al termine dell’esecuzione, ogni figlio Pn ritorna al padre la lunghezza dell’ultima linea letta dal proprio file compreso
    il terminatore di linea (sicuramente minore di 255); il padre deve stampare su standard output il PID di ogni figlio e il
    valore ritornato.
    
    ```jsx
    /* Soluzione della parte C del compito del 16 Giugno 2021 */
    #include <stdio.h>
    #include <stdlib.h>
    #include <unistd.h>
    #include <sys/wait.h> 
    #include <sys/stat.h>
    #include <string.h>
    #include <fcntl.h>
    #include <ctype.h>
    #define PERM 0644
    
    typedef int pipe_t[2];
    typedef char L[250]; /* tipo array statico: per ogni linea letta dai figli dal proprio file come indicato dal testo basta 250 caratteri per linea e terminatore di linea */
    
    int main (int argc, char **argv)
    {
    int N; 			/* numero di file/processi */
    /* ATTENZIONE NOME N imposto dal testo! */
    int pid;                /* pid per fork */
    pipe_t *pipes;          /* array di pipe usate a ring da primo figlio, a secondo figlio .... ultimo figlio, padre e poi a primo figlio: ogni processo legge dalla pipe n
     e scrive sulla pipe (n+1) */
    int n,j,i,k; 		/* indici */
    /* ATTENZIONE NOME n imposto dal testo! */
    int fd, fdw; 		/* file descriptor */
    int pidFiglio, status, ritorno;	/* per valore di ritorno figli */
    L linea;        	/* linea corrente */
    /* ATTENZIONE NOME linea imposto dal testo! */
    L *tutteLinee;        	/* array dinamico di linee */
    /* ATTENZIONE NOME tutteLinee imposto dal testo! */
    int nr,nw;              /* variabili per salvare valori di ritorno di read/write da/su pipe */
    
    /* controllo sul numero di parametri almeno 2 file */
    if (argc < 3)
    {
    	printf("Errore numero di parametri\n");
    	exit(1);
    }
    
    N = argc-1;
    printf("Numero di processi da creare %d\n", N);
    
    /* allocazione pipe */
    if ((pipes=(pipe_t *)malloc(N*sizeof(pipe_t))) == NULL)
    {
    	printf("Errore allocazione pipe\n");
    	exit(2); 
    }
    
    /* creazione pipe */
    for (n=0;n<N;n++)
    	if(pipe(pipes[n])<0)
    	{
    		printf("Errore creazione pipe\n");
    		exit(3);
    	}
    
    /* allocazione array di linee */
    if ((tutteLinee=(L *)malloc(N*sizeof(L))) == NULL)
    {
    	printf("Errore allocazione array di linee\n");
    	exit(4); 
    }
    
    /* creiamo il file nella directory corrente avente come nome il mio cognome */
    if ((fdw=creat("Leonardi", PERM)) < 0)
    {
            printf("Errore nella creat del file %s\n", "Leonardi");
            exit(5);
    }
    
    /* creazione figli */
    for (n=0;n<N;n++)
    {
    	if ((pid=fork())<0)
    	{
    		printf("Errore creazione figlio\n");
    		exit(6);
    	}
    	if (pid == 0)
    	{ /* codice figlio */
    		printf("Sono il figlio %d e sono associato al file %s\n", getpid(), argv[n+1]);
    		/* nel caso di errore in un figlio decidiamo di ritornare il valore -1 che sara' interpretato dal padre come 255 (valore NON ammissibile) */
    
    		/* chiusura pipes inutilizzate */
    		for (j=0;j<N;j++)
    		{	/* si veda commento nella definizione dell'array pipes per comprendere le chiusure */
    			if (j!=n)
    				close (pipes[j][0]);
    			if (j != (n+1)%N)
    				close (pipes[j][1]);
    		}	
    	
    		/* apertura file */
    		if ((fd=open(argv[n+1],O_RDONLY))<0)
    		{
    			printf("Impossibile aprire il file %s\n", argv[n+1]);
    			exit(-1);
    		}
    	
    		/* inizializziamo l'indice dei caratteri letti per ogni singola linea */
    		j = 0;
    		/* con un ciclo leggiamo tutte le linee, come richiede la specifica */
            	while(read(fd,&(linea[j]),1) != 0)
            	{
              		if (linea[j] == '\n') /* siamo a fine linea */
                    	{
    				/* dobbiamo aspettare l'array dal figlio precedente o dal padre per mandare avanti */
                    		nr=read(pipes[n][0],tutteLinee,N*sizeof(L));
            			/* per sicurezza controlliamo il risultato della lettura da pipe */
                    		if (nr != N*sizeof(L))
                    		{
                           			printf("Figlio %d ha letto un numero di byte sbagliati %d\n", n, nr);
                           			exit(-1);
                    		}
    				/* a questo punto si deve inserire la linea letta nel posto giusto */
    				for (k=0; k <=j ; k++)
                            	{
    					/* ricordarsi che non si possono copiare direttamente gli array */
    					/* fare attenzione ad includere nella copia anche lo '\n' e quindi il controllo su k e' k <= j */
    					tutteLinee[n][k]=linea[k];
                            	}
    
    				/* ora si deve mandare l'array in avanti */
            			nw=write(pipes[(n+1)%N][1],tutteLinee,N*sizeof(L));
            			/* anche in questo caso controlliamo il risultato della scrittura */
            			if (nw != N*sizeof(L))
            			{
                   				printf("Figlio %d ha scritto un numero di byte sbagliati %d\n", n, nw);
                   				exit(-1);
            			}
    				/* NOTA BENE: nell'ultima iterazione l'ultimo figlio mandera' un OK al primo figlio che pero' non verra' ricevuto, ma non creera' alcun problema a patto che il padre mantenga aperto il lato di lettura di pipes[0]: in questo modo, l'ultimo figlio non incorrera' nel problema di scrivere su una pipe che non ha lettori */
    				/* l'ultimo figlio deve scrivere le linee sul file creato dal padre */
    				if (n == N-1)
                            	{
    					/* printf("ULTIMO FIGLIO indice %d e pid= %d\n", n, getpid()); */
    					for (i=0;i<N;i++)
    					{	
    						for (k=0; k<250; k++)
    	        				{
    							/* fino a che non incontriamo il fine linea scriviamo sul file creato */
    							write(fdw, &(tutteLinee[i][k]), 1);
    							if (tutteLinee[i][k] == '\n')
                            				{
    								/* quando troviamo il terminatore di linea, ... */
    								break; /* usciamo dal ciclo for piu' interno */
    							}	
    						}
    					}		
    				}	
    
    				/* si deve azzerare l'indice della linea e il conteggio dei caratteri numerici, quest'ultimo dopo averlo salvato e incrementato per poterlo tornare correttamente */
    					ritorno = j+1;
    					j = 0;
            		}	
    			else
                    	{
    				j++; /* incrementiamo sempre l'indice della linea */
            		}		
            	}		
    		/* ogni figlio deve tornare il numero di caratteri dell'ultima linea */
    		exit(ritorno);
    	}
    } /* fine for */
    
    /* codice del padre */
    /* chiusura di tutte le pipe che non usa, a parte la prima perche' il padre deve dare il primo OK al primo figlio. N.B. Si lascia aperto sia il lato di scrittura che viene usato (e poi in effetti chiuso) che il lato di lettura (che non verra' usato ma serve perche' non venga inviato il segnale SIGPIPE all'ultimo figlio che terminerebbe in modo anomalo)  */
    for (n=1;n<N;n++) 
    /* l'indice lo facciamo partire nuindi da 1! */
    {
    	close (pipes[n][0]);
    	close (pipes[n][1]); 
    }
    
    /* ora si deve mandare l'array tutteLinee per innescare il ring: notare che non serve azzerarlo */
    nw=write(pipes[0][1],tutteLinee,N*sizeof(L));
    /* anche in questo caso controlliamo il risultato della scrittura */
    if (nw != N*sizeof(L))
    {
           printf("Padre ha scritto un numero di byte sbagliati %d\n", nw);
           exit(7);
    }
    
    /* ora possiamo chiudere anche il lato di scrittura, ma ATTENZIONE NON QUELLO DI LETTURA! */
    close(pipes[0][1]);
    /* OSSERVAZIONE SU NON CHIUSURA DI pipes[0][0]: se si vuole procedere con la chiusura di tale lato nel padre, bisognerebbe introdurre del codice ulteriore solo nel prim
    o figlio che vada a fare la lettura dell'ultimo OK prima di terminare! */
    
    /* Il padre aspetta i figli */
    for (n=0; n < N; n++)
    {
            pidFiglio = wait(&status);
            if (pidFiglio < 0)
            {
                    printf("Errore in wait\n");
                    exit(8);
            }
            if ((status & 0xFF) != 0)
                    printf("Figlio con pid %d terminato in modo anomalo\n", pidFiglio);
            else
            { 
    		ritorno=(int)((status >> 8) & 0xFF);
            	printf("Il figlio con pid=%d ha ritornato %d (se 255 problemi)\n", pidFiglio, ritorno);
            } 
    }
    exit(0);
    }
    ```
    
- 14 luglio 2021, schema di piping a pipeline
    
    La parte in C accetta un numero variabile di parametri N+1 maggiore o uguale a 2 che rappresentano i primi N nomi di file (F1,
    ...FN), mentre l’ultimo rappresenta un numero intero strettamente positivo (nroLinee) (da controllare) che indica la lunghezza in
    linee dei file. Il processo padre deve, per prima cosa, creare nella directory corrente un file fcreato con nome corrispondente al
    proprio COGNOME (tutto scritto in maiuscolo, in caso di più cognomi se ne usi solo uno, inserendo un opportuno commento).
    Il processo padre deve generare un numero di processi figli pari a N: ogni processo figlio Pn è associato ad uno dei file F1, ...FN
    (in ordine); la lunghezza in linee di tutti i file è uguale a nroLinee e non deve essere controllata.
    Ognuno di tali processi figli Pn esegue concorrentemente e legge tutte le linee del proprio file associato: le linee lette devono essere
    scritte nel file fcreato seguendo le indicazioni fornite nel seguito.
    I processi figli Pn devono usare uno schema di comunicazione a pipeline: il figlio P0 comunica con il figlio P1 che comunica
    con il figlio P2 etc. fino al figlio PN-1 che comunica con il padre. Questo schema a pipeline deve prevedere l’invio in avanti, per
    ognuna delle nroLinee linee dei file, di un array di grandezza N e in cui ogni elemento dell’array corrisponde alla linea corrente
    (supposta lunga 250 caratteri compreso il terminatore di linea1*) letta dal corrispondente processo figlio Pn. Quindi, il generico
    processo Pn, dopo aver letto la linea corrente, deve ricevere dal figlio precedente (a parte il processo P0) l’array di linee e, dopo
    aver inserito la linea corrente nella posizione giusta dell’array di linee, deve inviare l’array di linee al figlio successivo, con PN-1
    che manda al padre. Quindi al padre deve arrivare, per ognuna delle nroLinee linee un array di grandezza N e in cui ogni elemento
    dell’array corrisponde alla linea corrente letta dai figli Pn: il padre deve scrivere le linee correnti sul file fcreato.
    Al termine dell’esecuzione, ogni figlio Pn ritorna al padre la lunghezza dell’ultima linea letta dal proprio file compreso il
    terminatore di linea (sicuramente minore di 255); il padre deve stampare su standard output il PID di ogni figlio e il valore ritornato.
    
    ```jsx
    /* Soluzione della parte C del compito del 14 Luglio 2021 */
    #include <stdio.h>
    #include <stdlib.h>
    #include <unistd.h>
    #include <sys/wait.h> 
    #include <sys/stat.h>
    #include <string.h>
    #include <fcntl.h>
    #include <ctype.h>
    #define PERM 0644
    
    typedef int pipe_t[2];
    typedef char L[250]; /* tipo array statico: per ogni linea letta dai figli dal proprio file come indicato dal testo bastano 250 caratteri per linea e terminatore di linea */
    
    int main (int argc, char **argv)
    {
    int N; 			/* numero di file/processi */
    /* ATTENZIONE NOME N imposto dal testo! */
    int pid;                /* pid per fork */
    pipe_t *pipes;  /* array di pipe usate a pipeline da primo figlio, a secondo
     figlio .... ultimo figlio e poi a padre: ogni processo (a parte il primo) legge dal
    la pipe i-1 e scrive sulla pipe i */
    int n,j,i,k; 		/* indici */
    /* ATTENZIONE NOME n imposto dal testo! */
    int fd, fdw; 		/* file descriptor */
    int pidFiglio, status, ritorno;	/* per valore di ritorno figli */
    int nroLinee;		/* variabile per salvare la lunghezza in linee dei file */
    /* ATTENZIONE NOME nroLinee imposto dal testo! */
    L linea;        	/* linea corrente */
    /* ATTENZIONE NOME linea imposto dal testo! */
    L *tutteLinee;        	/* array dinamico di linee */
    /* ATTENZIONE NOME tutteLinee imposto dal testo! */
    int nr,nw;              /* variabili per salvare valori di ritorno di read/write da/su pipe */
    
    /* controllo sul numero di parametri: la specifica diceva minimo un solo file e la lunghezza in linee, ma in realta' era una imprecisione del testo e quindi nella soluzione si e' considerato almeno 2 file e la lunghezza in linee (nella correzione delle consegne si e' considerato quello c'era scritto nei testi)*/
    if (argc < 4)
    {
    	printf("Errore numero di parametri\n");
    	exit(1);
    }
    
    nroLinee=atoi(argv[argc-1]);
    if (nroLinee <= 0)
    {
    	printf("Errore nel numero di linee %d\n", nroLinee);
    	exit(2);
    }
    
    N = argc-2;
    printf("Numero di processi da creare %d\n", N);
    
    /* allocazione pipe */
    if ((pipes=(pipe_t *)malloc(N*sizeof(pipe_t))) == NULL)
    {
    	printf("Errore allocazione pipe\n");
    	exit(3); 
    }
    
    /* creazione pipe */
    for (n=0;n<N;n++)
    	if(pipe(pipes[n])<0)
    	{
    		printf("Errore creazione pipe\n");
    		exit(4);
    	}
    
    /* allocazione array di linee */
    if ((tutteLinee=(L *)malloc(N*sizeof(L))) == NULL)
    {
    	printf("Errore allocazione array di linee\n");
    	exit(5); 
    }
    
    /* creiamo il file nella directory corrente avente come nome il mio cognome (TUTTO IN MAIUSCOLO, come specificato nel testo) */
    if ((fdw=creat("LEONARDI", PERM)) < 0)
    {
            printf("Errore nella creat del file %s\n", "LEONARDI");
            exit(6);
    }
    
    /* creazione figli */
    for (n=0;n<N;n++)
    {
    	if ((pid=fork())<0)
    	{
    		printf("Errore creazione figlio\n");
    		exit(7);
    	}
    	if (pid == 0)
    	{ /* codice figlio */
    		printf("Sono il figlio %d e sono associato al file %s\n", getpid(), argv[n+1]);
    		/* nel caso di errore in un figlio decidiamo di ritornare il valore -1 che sara' interpretato dal padre come 255 (valore NON ammissibile) */
    
    		/* chiusura pipes inutilizzate */
    		for (j=0;j<N;j++)
    		{	/* si veda commento nella definizione dell'array pipes per comprendere le chiusure */
                    	if (j!=n)
                    		close (pipes[j][1]);
                    	if ((n == 0) || (j != n-1))
                   			close (pipes[j][0]);
            	}
    	
    		/* apertura file */
    		if ((fd=open(argv[n+1],O_RDONLY))<0)
    		{
    			printf("Impossibile aprire il file %s\n", argv[n+1]);
    			exit(-1);
    		}
    	
    		/* inizializziamo l'indice dei caratteri letti per ogni singola linea */
    		j = 0;
    		/* con un ciclo leggiamo tutte le linee, come richiede la specifica */
            	while(read(fd,&(linea[j]),1) != 0)
            	{
              		if (linea[j] == '\n') /* siamo a fine linea */
                    	{
    				if (n != 0)
    	                	{
    					/* se non siamo il primo figlio, dobbiamo aspettare l'array dal figlio precedente per mandare avanti */
                    			nr=read(pipes[n-1][0],tutteLinee,N*sizeof(L));
            				/* per sicurezza controlliamo il risultato della lettura da pipe */
                    			if (nr != N*sizeof(L))
                    			{
                           				printf("Figlio %d ha letto un numero di byte sbagliati %d\n", n, nr);
                           				exit(-1);
                    			}
                    		}	
    				/* a questo punto si deve inserire la linea letta nel posto giusto */
    				for (k=0; k <= j; k++)
                           		{ /* ricordarsi che non si puo' fare una copia diretta di un array! */
    					tutteLinee[n][k]=linea[k];
                           		}
    
    				/* ora si deve mandare l'array in avanti */
            			nw=write(pipes[n][1],tutteLinee,N*sizeof(L));
            			/* anche in questo caso controlliamo il risultato della scrittura */
            			if (nw != N*sizeof(L))
            			{
                   				printf("Figlio %d ha scritto un numero di byte sbagliati %d\n", n, nw);
                   				exit(-1);
            			}
    				/* si deve azzerare l'indice della linea, dopo averlo salvato (incrementato) per poterlo tornare correttamente */
    				ritorno = j+1;
    				j = 0;
            		}	
    			else
                   		{
    				j++; /* incrementiamo sempre l'indice della linea */
            		}		
            	}		
    		/* ogni figlio deve tornare il numero di caratteri dell'ultima linea */
    		exit(ritorno);
    	}
    } /* fine for */
    
    /* codice del padre */
    /* chiusura di tutte le pipe che non usa */
    for (n=0;n<N;n++) 
    {
    	close (pipes[n][1]);
    	if (n != N-1) close (pipes[n][0]); 
    }
    
    /* il padre deve leggere tutti gli array di linee inviati dall'ultimo figlio */
    for (j=1; j<=nroLinee; j++)
    {
            nr=read(pipes[n-1][0],tutteLinee,N*sizeof(L));
           	/* per sicurezza controlliamo il risultato della lettura da pipe */
            if (nr != N*sizeof(L))
          	{
           		printf("Padre ha letto un numero di byte sbagliati %d\n", nr);
                    exit(8);
            }
    
    	/* il padre deve scrivere le linee sul file creato */
    	for (i=0;i<N;i++)
    	{	
    		for (k=0; k<250; k++)
    		{
    			/* fino a che non incontriamo il fine linea scriviamo sul file creato */
    			write(fdw, &(tutteLinee[i][k]), 1);
    			if (tutteLinee[i][k] == '\n')
                            {
    				/* quando troviamo il terminatore di linea, ... */
    				break; /* usciamo dal ciclo for piu' interno */
    			}	
    		}
    	}	
    }	
    
    /* Il padre aspetta i figli */
    for (n=0; n < N; n++)
    {
            pidFiglio = wait(&status);
            if (pidFiglio < 0)
            {
                    printf("Errore in wait\n");
                    exit(9);
            }
            if ((status & 0xFF) != 0)
                    printf("Figlio con pid %d terminato in modo anomalo\n", pidFiglio);
            else
            { 
    		ritorno=(int)((status >> 8) & 0xFF);
            	printf("Il figlio con pid=%d ha ritornato %d (se 255 problemi)\n", pidFiglio, ritorno);
            } 
    }
    exit(0);
    }
    ```
    
- 8 settembre 2021, uso di doppia pipe e figli che lavorano a coppie
    
    La parte in C accetta un numero variabile di parametri N maggiore o uguale a 1 che rappresentano Q nomi
    di file (F1, ...FN). Il processo padre deve, per prima cosa, creare nella directory corrente un file fcreato con
    nome corrispondente al proprio cognome (tutto scritto in minuscolo, in caso di più cognomi se ne usi solo
    uno, inserendo un opportuno commento) e terminazione log (esempio, leonardi.log).
    
    Il processo padre deve generare un numero di processi figli pari a 2 *N: i processi figli devono essere
    considerati a coppie e ogni coppia è costituita dai processi Pi*2 (detti processi pari) e Pi*2+1 (detti processi
    dispari) (con i che varia da 0 a N-1). Ogni coppia così determinata è associata ad uno dei file F1, ...FN (in
    ordine): quindi ogni processo pari Pn (cioè con indice n pari) identificherà il file associato come Fn/2+1,
    mentre ogni processo dispari Pn (cioè con indice n dispari) identificherà il file associato come F(n+1)/2. Ogni
    processo pari è associato alle linee pari del proprio file e ogni processo dispari è associato alle linee dispari
    del proprio file. ATTENZIONE: Si consideri che la prima linea dei file abbia numero 1 e quindi sia dispari!
    Ognuno di tali processi figli Pn esegue concorrentemente e legge tutte le linee* del proprio file associato: per
    ogni linea dispari, ogni processo figlio dispari Pn calcola la lunghezza della linea corrente compreso il
    terminatore di linea e la invia al padre e per ogni linea pari, ogni processo figlio pari Pn calcola la lunghezza
    della linea corrente compreso il terminatore di linea e la invia al padre. Il padre deve ricevere per ogni file
    F1, ...FN da ognuna delle coppie tutte le lunghezze delle linee dispari/pari e le deve stampare su standard
    output in ordine di linea, come mostrato nell’esempio (riportato sul retro del foglio)!
    
    Al termine dell’esecuzione, ogni figlio Pn ritorna al padre il massimo delle lunghezze calcolate (sicuramente
    minore di 255); il padre deve scrivere sul file fcreato il PID di ogni figlio e il valore ritornato.
    
    NOTA BENE NEL FILE C main.c SI USI OBBLIGATORIAMENTE:
    - una variabile di nome N per il numero di file;
    - una variabile di nome n per l’indice dei processi figli;
    - una variabile di nome linea per la linea corrente (pari/dispari) letta dai figli dal proprio file;
    - una variabile di nome nro per il valore massimo della lunghezza delle linee pari/dispari dei file;
    - una variabile di nome fcreato per il file descriptor del file creato dal padre.
    
    ```jsx
    /* Soluzione della Prova d'esame del 8 Settembre 2021: soluzione con due array di pipe */
    #include <stdio.h>
    #include <string.h>
    #include <fcntl.h>
    #include <sys/stat.h>
    #include <unistd.h>
    #include <stdlib.h>
    #include <sys/wait.h>
    
    typedef int pipe_t[2];
    
    int main(int argc, char **argv) 
    {
    	/* -------- Variabili locali ---------- */
    	int pid;			/* process identifier per le altre fork() */
    	int N; 				/* numero di file passati sulla riga di comando */
    	int nro; 			/* massimo delle lunghezze delle linee calcolate da ogni figlio */
    	int nroLinea; 			/* numero delle linee calcolate da ogni figlio e dal padre */
    	int fcreato; 			/* file descriptor per creazione file da parte del padre */
    	int fd; 			/* file descriptor per apertura file */
    	int status, ritorno;		/* variabili per la wait */
    	pipe_t *pari;			/* array dinamico di pipe descriptors per comunicazioni figli pari-padre */
    	pipe_t *dispari;		/* array dinamico di pipe descriptors per comunicazioni figli dispari-padre */
    	int n;				/* indice per i figli */
    	int i, j;			/* indici per i cicli */
    	char linea[250];		/* array di caratteri per memorizzare la linea: come indicato dal testo si puo' supporre una lunghezza massima di ogni linea di 250 caratteri compreso il terminatore di linea */
    	int nr1, nr2; 			/* variabili che vengono usate dal padre per sapere se i figli hanno inviato qualcosa */
    	int L1, L2; 			/* variabili che vengono usate dal padre per recuperare le lunghezze inviate dai figli */
    	/* ------------------------------------ */
    
    	/* Controllo sul numero di parametri */
    	if (argc < 2) /* Meno di un parametro */  
    	{
    		printf("Errore nel numero dei parametri\n");
    		exit(1);
    	}
    
    	/* Calcoliamo il numero di file passati */
    	N = argc - 1;
    	printf("Sono il padre con pid %d e %d file e creero' %d processi figli\n", getpid(), N, 2*N);
    	
    	/* PER PRIMA COSA creazione file con mio cognome e terminazione log */
    	if ((fcreato=creat("leonardi.log", 0644)) < 0)  
    	{
                  	printf("Errore nella creat del file %s\n", "leonardi.log");
                    exit(2);
    	}
     
    	/* Allocazione dei due array di N pipe descriptors*/
    	pari = (pipe_t *) malloc (N*sizeof(pipe_t));
    	if (pari == NULL)
    	{
    		printf("Errore nella allocazione della memoria per le pipe pari\n");
    		exit(3);
    	}
    	dispari = (pipe_t *) malloc (N*sizeof(pipe_t));
    	if (dispari == NULL)
    	{
    		printf("Errore nella allocazione della memoria per le pipe dispari\n");
    		exit(4);
    	}
    	
    	/* Creazione delle N pipe figli pari-padre e figli dispari-padre*/
    	for (i=0; i < N; i++)
    	{
    		if(pipe(pari[i]) < 0)
    		{
    			printf("Errore nella creazione della pipe pari\n");
    			exit(5);
    		}
    		if(pipe(dispari[i]) < 0)
    		{
    			printf("Errore nella creazione della pipe dispari\n");
    			exit(6);
    		}
    	}
    
    	/* Ciclo di generazione dei figli: NOTA BENE DEVONO ESSERE 2 * N  */
    	for (n=0; n < 2*N; n++)
    	{
    		if ( (pid = fork()) < 0)
    		{
    			printf("Errore nella fork del figlio %d-esimo\n", n);
    			exit(5);
    		}
    		
    		if (pid == 0) 
    		{
    			/* codice del figlio */
    			printf("Sono il processo figlio di indice %d e pid %d e sono associato al file %s\n", n, getpid(), argv[n/2+1]);
    			/* con questa soluzione le chiusure delle pipe si complicano rispetto ad avere un solo array di 2*N pipe */
    			if ((n%2) == 0) /* pari */
    			{
    				/* Chiusura delle pipe non usate nella comunicazione con il padre */
    				for (j=0; j < N; j++)
    				{
    					close(pari[j][0]);
    					if (n/2 != j) close(pari[j][1]);
    					close(dispari[j][0]);
    					close(dispari[j][1]);
    				}
                           	}
    			else /* dispari */
    			{
    				/* Chiusura delle pipe non usate nella comunicazione con il padre */
    				for (j=0; j < N; j++)
    				{
    					close(dispari[j][0]);
    					if (n/2 != j) close(dispari[j][1]);
    					close(pari[j][0]);
    					close(pari[j][1]);
    				}
                           	}
    
    			/* dopo la consegna, ci si e' accorti che i vari processi potevano direttamente individuare il file associato usando l'indice n/2+1 senza bisogn
    o di distinguere caso indice dispari o pari e quindi la soluzione riporta questa semplicificazione; chiaramente le consegne che prevedevano codice differenziato sono st
    ate considerate corrette! */
    			if ((fd=open(argv[n/2+1], O_RDONLY)) < 0)
    			{
                                   	printf("Errore nella open del file %s\n", argv[n/2+1]);
                                   	exit(-1); /* in caso di errore nei figli decidiamo di tornare -1 che corrispondera' per il padre al valore 255 che supponiamo non essere un valore accettabile di ritorno */
                           	}
    
    			/* adesso il figlio legge dal file una linea alla volta */
    			j=0; 		/* azzeriamo l'indice della linea */
    			nroLinea=0; 	/* azzeriamo il numero della linea */
    			nro=-1;		/* settiamo il massimo a -1 */
    		        while (read(fd, &(linea[j]), 1))
    			{
    				if (linea[j] == '\n') 
    			 	{ 
    					/* dobbiamo mandare al padre la lunghezza della linea selezionata compreso il terminatore di linea (come int) e quindi incrementiamo j */
    				   	j++;
    					nroLinea++; /* la prima linea sarÃ  la numero 1! */
    					if ( ((n%2) != 0) && ((nroLinea%2) != 0) ) /* processo associato a linea dispari e numero di linea dispari */
    					{
    				   		write(dispari[n/2][1], &j, sizeof(j));
    						/* verifichiamo e nel caso aggiorniamo il massimo */
    						if (j > nro) 
    							nro=j; 
    				  	}
    					if ( ((n%2) == 0) && ((nroLinea%2) == 0) ) /* processo associato a linea pari e numero di linea pari */
    					{
    				   		write(pari[n/2][1], &j, sizeof(j));
    						/* verifichiamo e nel caso aggiorniamo il massimo */
    						if (j > nro) 
    							nro=j; 
    				  	}
    				   	j=0; /* azzeriamo l'indice per le prossime linee */
    				}
    				else j++; /* continuiamo a leggere */
    			}
    			/* ogni figlio deve ritornare al padre il valore corrispondente al massimo */
    			exit(nro);
      			}	
    		}
    	
    	/* Codice del padre */
    	/* Il padre chiude i lati delle pipe che non usa */
    	for (i=0; i < N; i++)
     	{
    		close(pari[i][1]);
    		close(dispari[i][1]);
    	}
    
    	/* Il padre recupera le informazioni dai figli: prima in ordine di file e quindi di linee */
    	for (i=0; i < N; i++)
    	{
    	       	nroLinea=1; /* la prima linea ha numero 1 */
    	       	printf("Le lunghezze delle linee del file %s sono:\n", argv[i+1]);
    		do
    		{
    			/* il padre recupera le lunghezze delle linee da ogni figlio dispari e pari */
    	       		nr1=read(dispari[i][0], &L1, sizeof(L1));
    			nr2=read(pari[i][0], &L2, sizeof(L2));
    			if (nr1 != 0)
    			{
    				printf("linea numero %d e' lunga %d\n", nroLinea, L1);
    				nroLinea++;		/* incrementiamo il numero di linea */
    			}	
    			if (nr2 != 0)
    			{
    				printf("linea numero %d e' lunga %d\n", nroLinea, L2);
    				nroLinea++;		/* incrementiamo il numero di linea */
    			}	
    
    		} while (nr1||nr2);	
    		/* in alternativa si poteva fare un while(1) ed inserire dei break quando la nr1 o nr2 arrivano a 0 */
    	}	
    
    	/* Il padre aspetta tutti i figli */
    	for (n=0; n < 2*N; n++)
    	{
    		pid = wait(&status);
    		if (pid < 0)
    		{
    		printf("Errore in wait\n");
    		exit(9);
    		}
    
    		if ((status & 0xFF) != 0)
        			printf("Figlio con pid %d terminato in modo anomalo\n", pid);
        		else
    		{ 
    			ritorno=(int)((status >> 8) &	0xFF); 
    			/* dobbiamo preparare la stringa da scrivere sul file creato: N.B. usiamo sprintf e linea che tanto il padre non ha usato */
    			sprintf(linea,"Il figlio con pid=%d ha ritornato %d (se 255 significa che il figlio e' terminato con un errore)\n", pid, ritorno);
    			write(fcreato, linea, strlen(linea));
    		}
    	}
    	exit(0);
    }
    ```
    
- 19 gennaio 2022, figli lavorano a coppie per leggere un file(il primo legge la prima metà, mentre il secondo la restante parte
    
    La parte in C accetta un numero variabile di parametri **N+1** con **N** maggiore o uguale a **1**: i primi **N**
    rappresentano nomi di file (**F1, ...FN**), mentre l’ultimo parametro **C** rappresenta un numero intero
    strettamente positivo e **dispari** (*da controllare*): si può ipotizzare che la lunghezza di tutti i file sia uguale,
    pari e multiplo intero di **C** (senza verificarlo). Il processo padre deve generare **2*N** processi figli (**P0** ... **P2*N-
    1**); tali processi figli costituiscono **N** coppie di processi: ogni coppia **Ci** è composta dal processo **Pi** (primo
    processo della coppia) e dal processo **Pi+N** (secondo processo della coppia), con **i** variabile da **0 a N-1**. Ogni
    coppia di processi figli **Ci** è associata ad uno dei file **Fi+1***. Il secondo processo della coppia deve creare un
    file il cui nome risulti dalla concatenazione del nome del file associato alla coppia con la stringa **.mescolato**
    (ad esempio se il file associato è /tmp/pippo.txt, il file creato si deve chiamare /tmp/pippo.txt.mescolato). Tutte
    le coppie **Ci** di processi figli eseguono concorrentemente leggendo il proprio file associato: in particolare, il
    primo processo di ogni coppia deve leggere la prima metà del file associato, mentre il secondo processo la
    seconda metà del file; inoltre, per entrambi i processi di ogni coppia la lettura deve avvenire a ***blocchi*** di dati
    di grandezza uguale a **C** byte. Il secondo processo di ogni coppia, dopo la lettura di ogni blocco di dati B2
    (con un’unica read!) della sua seconda metà del file, lo scrive (con un'unica write!) sul file creato; quindi deve
    ricevere (con un’unica read!) dal primo processo della coppia il suo corrispondente blocco di dati B1 e quindi
    deve scriverlo (sempre con un'unica write!) sul file creato; viceversa, il primo processo di ogni coppia, dopo
    la lettura di ogni blocco di dati B1 (con un’unica read!) della sua prima metà del file, lo comunica (con un'unica
    write!) al secondo processo della coppia (si veda un esempio riportato sul retro del foglio). Al termine, ogni
    processo di ogni coppia deve ritornare al padre il **numero di blocchi** (**nro**) letti dalla propria metà del file. Il
    padre, dopo che i figli sono terminati, deve stampare su standard output i PID di ogni figlio con il
    corrispondente valore ritornato.
    
    **NOTA BENE NEL FILE C main.c SI USI OBBLIGATORIAMENTE:**- una variabile di nome **N** per il numero di file;
    - una variabile di nome **i** per l’indice dei processi figli;
    - una variabile di nome **b** per il blocco corrente (B1 o B2) letto dai figli dal file;
    - una variabile di nome **nro** per il numero di blocchi letti dalla propria metà del file;
    - una variabile di nome **fcreato** per il file descriptor del file creato dal secondo processo di ogni coppia. Creazione del file unendo il nome di un file già esistente ad una estensione.
    
    ```jsx
    /* soluzione parte C esame del 19 Gennaio 2022: la comunicazione in ogni coppia va dal primo processo della coppia al secondo processo della coppia ed e' il secondo processo della coppia deve creare il file con terminazione ".mescolato" sul quale poi deve scrivere */
    #include <stdio.h> 
    #include <unistd.h> 
    #include <stdlib.h> 
    #include <sys/wait.h> 
    #include <string.h> 
    #include <sys/types.h> 
    #include <sys/stat.h> 
    #include <fcntl.h> 
    
    #define PERM 0644
    
    typedef int pipe_t[2]; 		/* tipo di dato per contenere i file descriptors di una pipe */
    
    int main(int argc, char **argv) 
    {
    	int N; 			/* numero di file: i processi figli saranno il doppio! */
    	/* N nome specificato nel testo */
    	int C; 			/* numero intero positivo dispari */
    	int pid;		/* variabile per fork */
    	pipe_t *pipe_ps;	/* array di pipe per la comunicazione dai figli primi della coppia ai figli secondi della coppia */
    	int fd;			/* variabile per open */
    	char *FCreato;		/* variabile per nome file da creare da parte dei processi figli primi della coppia */
    	int fcreato;		/* variabile per creat */
    	/* fcreato nome specificato nel testo */
    	char *b;		/* variabile per leggere dai figli */
    	/* b nome specificato nel testo */
    	int nroTotale;		/* variabile per tenere traccia del numero di blocchi presenti nei file */
    	int nro;		/* variabile per tenere traccia del numero di blocchi leti dalla propria meta' del file */
    	/* nro nome specificato nel testo */
    	int status, pidFiglio, ritorno;	/* per wait */
    	int i, j;		/* indici per cicli */
    	/* i nome specificato nel testo */
    	int nr, nw;		/* per controllo su read/write */
    
    /* Controllo sul numero di parametri: N deve essere maggiore o uguale a 1 */
    if (argc < 3) 
    {
    	printf("Errore numero parametri dati che argc=%d\n", argc);
    	exit(1);
    }
    
    /* controllo sull'ultimo parametro: numero dispari C */
    C = atoi(argv[argc-1]);
    if ( (C <= 0) || (C % 2 == 0) )
    {
    	printf("Errore numero C %s\n", argv[argc-1]);
    	exit(2);
    }
    
    /* calcoliamo il numero dei file */
    N = argc - 2; 
    
    printf("DEBUG-Numero processi da creare %d con C=%d\n", 2*N, C);
    
    /* allocazione memoria dinamica per buffer */
    b=(char *)malloc(C*sizeof(char));
    if (b == NULL)
    {
            printf("Errore nella malloc per buffer b\n");
            exit(3);
    }
    
    /* allocazione memoria dinamica per pipe_ps. NOTA BENE: servono un numero di pipe che e' la meta' del numero di figli! */
    pipe_ps=(pipe_t *)malloc(N*sizeof(pipe_t));
    if (pipe_ps == NULL)
    {
            printf("Errore nella malloc per le pipe\n");
            exit(4);
    }
    
    /* creazione delle pipe: ATTENZIONE VANNO CREATE solo N pipe */
    for (i=0; i < N; i++) 
    {
    	if (pipe(pipe_ps[i])!=0) 
    	{
    		printf("Errore creazione delle pipe\n"); 
    		exit(5);
    	}
    }
    
    /* creazione dei processi figli: ne devono essere creati 2*N */
    for (i=0; i < 2*N; i++) 
    {
    	pid=fork();
     	if (pid < 0)  /* errore */
       	{
    		printf("Errore nella fork con indice %d\n", i);
          		exit(6);
       	}
    	if (pid == 0) 
    	{
     		/* codice del figlio: in caso di errore torniamo 0 che non e' un valore accettabile (per quanto risulta dalla specifica della parte shell) */
          		if (i < N) /* siamo nel codice dei figli primi della coppia */
    		{
    			/* stampa di debugging */
    			printf("DEBUG-PRIMO DELLA COPPIA-Figlio di indice %d e pid %d associato al file %s\n",i,getpid(),argv[i+1]);
          			/* chiudiamo le pipe che non servono */
          			/* ogni figlio PRIMO della coppia scrive solo sulla pipe_ps[i] */
          			for (j=0;j<N;j++)
          			{
            			close(pipe_ps[j][0]);
            			if (j!=i) 	
            			{
            				close(pipe_ps[j][1]);
            			}
          			}
    			
    			/* ogni figlio deve aprire il suo file associato */
    			fd=open(argv[i+1], O_RDONLY);
    			if (fd < 0) 
    			{
    				printf("Impossibile aprire il file %s\n", argv[i+1]);
    				exit(0); /* in caso di errore, decidiamo di tornare 0 che non e' un valore accettabile */
    			}
    
    			/* calcoliamo la lunghezza in blocchi del file */
    			nroTotale = lseek(fd, 0L, 2) / C;
    			/* bisogna riportare l'I/0 pointer all'inizio del file */
    			nro=nroTotale/2;	/* ogni figlio legge meta' del file */
    			lseek(fd, 0L, 0);
    	 		for (j=0;j<nro;j++)
    			{
    				read(fd, b, C); 
    				/* ogni blocco letto dal PRIMO processo della coppia deve essere inviato al processo SECONDO della coppia */
    				nw=write(pipe_ps[i][1], b, C);
    				if (nw != C) 
    				{
    					printf("Errore in scrittura su pipe %d\n", i);
    					exit(0); 
    				}
    			}
    		}
    		else /* siamo nel codice dei figli secondi della coppia */
    		{
    			/* stampa di debugging */
    			printf("DEBUG-SECONDO DELLA COPPIA-Figlio di indice %d e pid %d associato al file %s\n",i,getpid(),argv[i-N+1]);
    			/* i figli secondi della coppia devono creare il file specificato */
    			FCreato=(char *)malloc(strlen(argv[i-N+1]) + 11); /* bisogna allocare una stringa lunga come il nome del file associato + il carattere '.' + i caratteri della parola mescolato (9) + il terminatore di stringa: ATTENZIONE ALL'INDICE PER INDIVIDUARE IL FILE */
    			if (FCreato == NULL) 
    			{
    				printf("Errore nelle malloc\n");
    				exit(0);
    			}
    			/* copiamo il nome del file associato */
    			strcpy(FCreato, argv[i-N+1]);
    			/* concateniamo la stringa specificata dal testo */
    			strcat(FCreato,".mescolato");
    			fcreato=creat(FCreato, PERM);
     			if (fcreato < 0)
                    	{	
                            	printf("Impossibile creare il file %s\n", FCreato);
                            	exit(0); 
                    	}
    
                            /* chiudiamo le pipe che non servono */
                            /* ogni figlio SECONDO della coppia legge solo da pipe_ps[i-N] */
                            for (j=0;j<N;j++)
                            {
                                    close(pipe_ps[j][1]);
                                    if (j!= i-N)	 /* ATTENZIONE ALL'INDICE CHE DEVE ESSERE USATO */
                                    {
                                            close(pipe_ps[j][0]);
                                    }
                            }
    
    			/* ogni figlio deve aprire il suo file associato: siamo nei figli secondi della coppia e quindi attenzione all'indice */
    			fd=open(argv[i-N+1], O_RDONLY);
    			if (fd < 0) 
    			{
    				printf("Impossibile aprire il file %s\n", argv[i-N+1]);
    				exit(0); 
    			}
    
    			/* calcoliamo la lunghezza in blocchi del file */
    			nroTotale = lseek(fd, 0L, 2) / C;
    			nro=nroTotale/2;	/* ogni figlio legge meta' del file */
    			/* bisogna posizionare l'I/0 pointer a meta' del file */
    			lseek(fd, (long)nro * C, 0);
    	 		for (j=0;j<nro;j++)
    			{
    				read(fd, b, C); 
    				/* ogni blocco letto dal processo SECONDO della coppia, bisogna scriverlo sul file */
    				write(fcreato, b, C);
    				/* dobbiamo a questo punto aspettare il blocco dal processo PRIMO della coppia: attenzione all'indice */
    				nr=read(pipe_ps[i-N][0], b, C);
    				if (nr != C) 
    				{
    					printf("Errore in lettura da pipe %d\n", i-N);
    					exit(0); 
    				}
    				/* ogni blocco ricevuto dal processo PRIMO della coppia, bisogna scriverlo sul file */
    				write(fcreato, b, C);
            		}
    		}	
    		exit(nro); /* torniamo il numero di blocchi letti (supposto <= di 255) */
    	}
    }
    
    /*codice del padre*/
    /* chiudiamo tutte le pipe, dato che le usano solo i figli */
    for (i=0;i<N;i++)
    {
       close(pipe_ps[i][0]);
       close(pipe_ps[i][1]);
    }
    
    /* Attesa della terminazione dei figli */
    for(i=0;i<2*N;i++)
    {
       pidFiglio = wait(&status);
       if (pidFiglio < 0)
       {
          printf("Errore wait\n");
          exit(7);
       }
       if ((status & 0xFF) != 0)
                    printf("Figlio con pid %d terminato in modo anomalo\n", pidFiglio);
       else
       {
                    ritorno=(int)((status >> 8) & 0xFF);
                    printf("Il figlio con pid=%d ha ritornato %d (se 0 problemi)\n", pidFiglio, ritorno);
       }
    }
    exit(0);
    }/* fine del main */
    ```
    
- 16 febbraio 2022, array di strutture
    
    La parte in C accetta un numero variabile di parametri **N+1** con **N** maggiore o uguale a **2**: i primi **N**
    rappresentano nomi di file (**F1, ...FN**), mentre l’ultimo parametro **C** rappresenta un singolo carattere
    *alfabetico minuscolo* (*da controllare*). Il processo padre deve generare **N** processi figli (**P0** ... **PN-1**): i
    processi figli **Pi (**con **i** variabile da **0 a N-1)** sono associati agli **N** file **Fk** (con k= **i** +1). Ogni processo figlio
    **Pi** deve leggere i caratteri del file associato **Fk** cercando il carattere **C**. I processi figli e il processo padre
    devono attenersi a questo **schema di comunicazione a pipeline**: il figlio **P0** comunica con il figlio **P1** che
    comunica con il figlio **P2** etc. fino al figlio **PN-1** che comunica con il **padre**. Questo schema a pipeline deve
    prevedere l’invio in avanti di un array di **strutture** dati ognuna delle quali deve contenere due campi: 1) *c1*,
    di tipo int, che deve contenere il pid di un processo; 2) *c2*, di tipo long int, che deve contenere il numero di
    occorrenze del carattere **C** calcolate dal corrispondente processo. *Gli array di strutture DEVONO essere creati
    da ogni figlio della dimensione minima necessaria per la comunicazione sia in ricezione che in spedizione.*
    Quindi la comunicazione deve avvenire in particolare in questo modo: il figlio **P0** passa in avanti (cioè
    comunica *con una singola write*) un array di strutture **A1**, che contiene una sola struttura con *c1* uguale al
    proprio pid e con *c2* uguale al numero di occorrenze del carattere **C** trovate da **P0** nel file **F1**; il figlio seguente
    **P1**, dopo aver calcolato numero di occorrenze del carattere **C** nel file **F2**, deve leggere (*con una singola read*)
    l’array **A1** inviato da **P0** e quindi deve confezionare l’array **A2** che corrisponde all’array **A1** aggiungendo
    all’ultimo posto la struttura con i propri dati e la passa (*con una singola write*) al figlio seguente **P2**, etc. fino
    al figlio **PN-1**, che si comporta in modo analogo, ma passa al **padre**. Quindi, il processo padre deve allocare
    l’array **AN** per ricevere quanto inviato dall’ultimo figlio e cioè l’array di **N** strutture (uno per ogni processo
    **P0** ... **PN-1)**. Il padre deve leggere (*con una singola read*) l’array **AN** e, quindi, deve riportare i dati di ognuna
    delle **N** strutture su standard output insieme al numero d’ordine del processo corrispondente, al nome del file
    associato a tale processo e al carattere **C**.
    
    Al termine, ogni processo figlio **Pi** deve ritornare al padre il valore intero corrisponde al proprio indice
    d’ordine (**i**); il padre deve stampare su standard output il PID di ogni figlio e il valore ritornato.
    
    **NOTA BENE NEL FILE C main.c SI USI OBBLIGATORIAMENTE:**
    - una variabile di nome **N** per il numero di file;
    - una variabile di nome **i** per l’indice dei processi figli;
    - una variabile di nome **ch** per il carattere letto dai file dai figli;
    
    - una variabile di nome **cur** per l’array dinamico creato da ogni figlio (della dimensione minima necessaria) e dal padre.
    
    ```jsx
    /* Soluzione della parte C del compito del 16 Febbraio 2022 */
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
            int Fpid;         	/* pid figlio (campo c1 del testo) */
            long int occ;   	/* numero occorrenze (campo c2 del testo) */
                    } s_occ;
    
    int main (int argc, char **argv)
    {
    	int N;          /* numero di file (NOME STABILITO DAL TESTO!) */
    	int pid;        /* pid per fork */
    	pipe_t *pipes;  /* array di pipe usate a pipeline da primo figlio, a secondo figlio .... ultimo figlio e poi a padre: ogni processo (a parte il primo) legge dalla pipe i-1 e scrive sulla pipe i. N.B. Nel caso di schema a pipeline e' molto importante che read e write abbiano SUCCESSO! */
    	int i,j;        /* contatori (i NOME STABILITO DAL TESTO!) */
    	int fd;         /* file descriptor */
    	int pidFiglio, status, ritorno; /* per valore di ritorno figli */
    	char Cx, ch;    /* carattere da cercare e carattere letto da linea (ch NOME STABILITO DAL TESTO!) */
    	s_occ *cur;     /* array di strutture usate dal figlio corrente (NOME STABILITO DAL TESTO!) */
    	int nr;         /* variabile per salvare valori di ritorno di read su pipe */
    
    /* controllo sul numero di parametri almeno 2 file e un carattere */
    if (argc < 4)
    {
            printf("Errore numero di parametri: infatti argc e' %d\n", argc);
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
    if (! islower(Cx)) /* N.B. per usare questa funzione bisogna includere string.h */
    {
            printf("Errore ultimo parametro non alfabetico minuscolo\n");
            exit(3);
    }
    /* stampa di debugging */
    printf("DEBUG-Carattere da cercare %c\n", Cx);
    
    N = argc-2;
    /* stampa di debugging */
    printf("DEBUG-Numero di processi da creare %d\n", N);
    
    /* allocazione pipe */
    if ((pipes=(pipe_t *)malloc(N*sizeof(pipe_t))) == NULL)
    {
            printf("Errore allocazione pipe\n");
            exit(4);
    }
    
    /* creazione pipe */
    for (i=0;i<N;i++)
            if(pipe(pipes[i])<0)
            {
                    printf("Errore creazione pipe\n");
                    exit(5);
            }
    
    /* creazione figli */
    for (i=0;i<N;i++)
    {
            if ((pid=fork())<0)
            {
                    printf("Errore creazione figli\n");
                    exit(6);
            }
            if (pid==0)
            {       /* codice figlio */
                    printf("Sono il figlio di indice %d e pid %d associato al file %s\n", i, getpid(), argv[i+1]);
                    /* nel caso di errore in un figlio decidiamo di ritornare un valore via via crescente rispetto al massimo valore di i (cioe' rispetto N-1) */
                    /* chiusura pipes inutilizzate */
                    for (j=0;j<N;j++)
                    {
                            if (j!=i)
                                    close (pipes[j][1]);
                            if ((i == 0) || (j != i-1))
                                    close (pipes[j][0]);
                    }
    
                    /* allocazione dell'array di strutture specifico di questo figlio */
                    /* creiamo un array di dimensione i+1 anche se leggeremo i strutture, dato che poi ci servira' riempire la i+1-esima struttura! */
                    if ((cur=(s_occ *)malloc((i+1)*sizeof(s_occ))) == NULL)
                    {
                            printf("Errore allocazione cur\n");
                            exit(N);
                    }
                    /* inizializziamo l'ultima struttura (quella di posto i+1 che pero' ha chiaramente indice i) che e' quella specifica del figlio corrente (nel caso del primo figlio sara' l'unica struttura */
                    cur[i].Fpid = getpid();
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
                                    /*
                                    printf("Sono il figlio di indice %d e pid %d e ho trovato %ld occorrenze del carattere %c\n", cur[i].id, getpid(), cur[i].occ, Cx);
                                    */
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
                            /*
                            for(j=0;j<i;j++)
                                    printf("HO ricevuto da figlio di indice %d trovato %ld occorrenze del carattere %c\n", cur[j].id, cur[j].occ, Cx);
                            */
                    }
    
                    /* tutti i figli mandano in avanti, l'ultimo figlio manda al padre un array di strutture (i ricevute dal processo precedente e la i+1-esima che e' la propria */
                    /*
                    printf("Sto per mandare al figlio seguente %d strutture che sono: \n", i+1);
                    for(j=0;j<i+1;j++)
                            printf("Sto per mandare processo %d trovato %ld occorrenze del carattere %c\n", cur[j].id, cur[j].occ, Cx);
                    */
                    write(pipes[i][1],cur,(i+1)*sizeof(s_occ));
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
            exit(7);
    }
    
    /* il padre deve leggere l'array di strutture che gli arriva dall'ultimo figlio */
    nr=read(pipes[N-1][0],cur,N*sizeof(s_occ));
    if (nr != N*sizeof(s_occ))
    {
            printf("Padre ha letto un numero di strutture sbagliate %d\n", nr);
            exit(8);
    }
    nr=nr/sizeof(s_occ);
    printf("Padre ha letto un numero di strutture %d\n", nr);
    /* il padre deve stampare i campi delle strutture ricevute insieme con tutte le altre informazioni richieste dal testo (carattere e nome file) */
    for(i=0;i<N;i++)
            printf("Il figlio di indice %d e pid %d ha trovato %ld occorrenze del carattere %c nel file %s\n", i, cur[i].Fpid, cur[i].occ, Cx, argv[i+1]);
    
    /* Il padre aspetta i figli */
    for (i=0; i < N; i++)
    {
            pidFiglio = wait(&status);
            if (pidFiglio < 0)
            {
                    printf("Errore in wait\n");
                    exit(9);
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
    ```
    
- 15 luglio 2020, uso di struct e figli che agiscono sullo stesso file
    
    La parte in C accetta due parametri F e L che rappresentano rispettivamente le seguenti informazioni: un nome
    assoluto di file e un numero strettamente positivo e minore o uguale di 255 (da controllare) che rappresenta il
    numero di linee del file. Il processo padre deve generare un numero di processi figli pari a L: ogni processo
    figlio Pq è associato ad una linea di F: la numerazione delle linee deve essere fatta partire da 1. Ognuno di
    tali processi figli Pq esegue concorrentemente e legge la propria linea associata (supposta avente lunghezza
    massima di 250 caratteri, compreso il terminatore di linea). Dopo la lettura della propria linea, ogni processo
    figlio Pq seleziona il secondo e il penultimo carattere della linea (considerando il terminatore di linea come
    ultimo carattere) e comunica al padre una struttura contenente tre campi: c1 che deve contenere il pid del
    processo figlio; c2 che deve contenere il secondo carattere e c3 che deve contenere il penultimo carattere della
    linea. Il padre deve ricevere, rispettando l’ordine delle linee, ognuna delle strutture inviate dai figli e per
    ognuna deve controllare se i caratteri corrispondenti ai campi c2 e c3 sono uguali: in tal caso, deve stampare
    su standard output tutti i campi della struttura corrente indicando esplicitamente il loro significato e, inoltre,
    deve riportare anche il numero d’ordine del processo figlio che ha inviato l’informazione, il numero della linea
    cui tale processo era associato e il nome del file F; in caso contrario, il padre non deve stampare nulla.
    Al termine dell’esecuzione, ogni figlio Pq ritorna al padre il numero della propria linea associata; il padre
    deve stampare su standard output il PID di ogni figlio e il valore ritornato.
    
    NOTA BENE NEL FILE C main.c SI USI OBBLIGATORIAMENTE:
    
    - una variabile di nome L per il numero di processi figli;
    - una variabile di nome q per l’indice dei processi figli;
    - una variabile di nome linea per memorizzare la linea associata e quindi letta da ogni figli
    
    ```jsx
    /* Soluzione della Prova del 15 Luglio 2020 */
    #include <stdio.h>
    #include <string.h>
    #include <fcntl.h>
    #include <sys/stat.h>
    #include <unistd.h>
    #include <stdlib.h>
    #include <sys/wait.h>
    
    typedef int pipe_t[2];
    typedef struct {
            int pid;       		/* campo c1 del testo */
            int cardue;	        /* campo c2 del testo */
            char carpen;  		/* campo c3 del testo */
    	} Strut;
    
    int main(int argc, char **argv)
    {
            /* -------- Variabili locali ---------- */
            int pid;                        /* process identifier per le fork() */
            int L;                          /* numero di linee del file e numero di processi */
            int status;                     /* variabile di stato per la wait */
            pipe_t *piped;                  /* array dinamico di pipe descriptors per comunicazioni figli-padre  */
            int q, i, j;                    /* indici per i cicli */
    	int fd; 			/* per la open nei figli */
    	char linea[250]; 		/* per leggere una linea (il testo indica che al massimo puo' essere lunga 250 compreso il terminatore di linea */
    	int nr;                      	/* variabile per valore di ritorno della read */
            int ritorno;                    /* variabile che viene ritornata da ogni figlio al padre */
            Strut S;                        /* struttura usata dai figli e dal padre */
    
            /* ------------------------------------ */
    
            /* Controllo sul numero di parametri */
            if (argc != 3 ) /* Devono essere passati esattamente due parametri */
            {
                    printf("Errore nel numero dei parametri\n");
                    exit(1);
            }
    
            /* Calcoliamo il numero passato */
            L = atoi(argv[2]);
    	if ((L <= 0) || (L > 255))
    	{	
            	printf("Errore: Il secondo parametro non e' un numero strettamente maggiore di 0 e minore/uguale a 255\n");
            	exit(2);
    	}
    
            /* Allocazione dell'array di L pipe descriptors */
            piped = (pipe_t *) malloc (L*sizeof(pipe_t));
            if (piped == NULL)
            {
                    printf("Errore nella allocazione della memoria\n");
                    exit(3);
            }
    
            /* Creazione delle L pipe figli-padre */
            for (q=0; q < L; q++)
            {
                    if (pipe(piped[q]) < 0)
                    {
                            printf("Errore nella creazione della pipe\n");
                            exit(4);
                    }
            }
    
            /* Ciclo di generazione dei figli */
            for (q=0; q < L; q++)
            {
                    if ( (pid = fork()) < 0)
                    {
                            printf("Errore nella fork\n");
                            exit(6);
                    }
    
                    if (pid == 0)
                    {
                            /* codice del figlio */
                            /* in caso di errori nei figli decidiamo di tornare 0 dato che il testo indica che le linee possono essere <= 255 e che la loro numerazione parte da 1 e quindi non puo' essere un valore accettabile di ritorno */
    			printf("Sono il figlio di indice %d e pid %d\n", q, getpid());
    
                            /* Chiusura delle pipe non usate nella comunicazione con il padre */
                            for (j=0; j < L; j++)
                            {
                                    close(piped[j][0]);
                                    if (q != j) close(piped[j][1]);
                            }
                                    
    			
    			/* apriamo il file in lettura: nota bene tutti i figli aprono lo stesso file perche' devono avere l'I/O pointer separato! */
    			if ((fd = open(argv[1], O_RDONLY)) < 0)
    			{
    				printf("Errore: FILE %s NON ESISTE\n", argv[1]);
    				exit(0);
    			}
    
    			i = 1; /* inizializzo il conteggio delle linee a 1 */
    			j = 0; /* valore iniziale dell'indice della linea */
    			while (read (fd, &(linea[j]), 1) != 0)
    			{ 	if (linea[j] == '\n') 
    				{ 
    					if (q+1 == i) /* trovata la linea che deve selezionare */
    					{ 	
    						/* il figlio comunica al padre */
    						S.pid=getpid();
    						S.cardue=linea[1];
    						S.carpen=linea[j-1];
                                    		write(piped[q][1], &S, sizeof(S));
    						break; /* usciamo dal ciclo di lettura */
    					}
    					else
      					{       
    						j = 0; 	/* azzeriamo l'indice per la prossima linea */
    		  				i++; 	/* se troviamo un terminatore di linea incrementiamo il conteggio delle linee */
                    			}
          				}
    				else j++;
    			}	
    
    			/* il figlio ritorna il numero della linea analizzata */
    			ritorno=q+1;
                            exit(ritorno); 
                    }
            }
    
    /* Codice del padre */
    /* Il padre chiude i lati delle pipe che non usa */
            for (q=0; q < L; q++)
                    close(piped[q][1]);
    
    /* Il padre recupera le informazioni dai figli: in ordine di indice */
            for (q=0; q < L; q++)
            {
             /* si legge la struttura inviata  dal figlio q-esimo */
                nr = read(piped[q][0], &S, sizeof(S));
                if (nr != 0)
                {
                    if (S.cardue == S.carpen) 
    			printf("Il figlio di indice %d e pid %d ha trovato che il secondo carattere (%c) e il penultimo carattere (%c) della linea %d-esima del file %s sono UGUALI\n", q, S.pid, S.cardue, S.carpen, q+1, argv[1]);
    		else
    			/* rispetto al testo caricato questa stampa non era da effettuare, ma visto che alcuni testi avevano l'indicazione di stampare se i caratteri erano diversi, si e' introdotta anche questa stampa per verificare il funzionamento */
                    	printf("Il figlio di indice %d e pid %d ha trovato che il secondo carattere (%c) e il penultimo carattere (%c) della linea %d-esima del file %s sono DIVERSI\n", q, S.pid, S.cardue, S.carpen, q+1, argv[1]);
    
                 }
             }
    
            /* Il padre aspetta i figli */
            for (q=0; q < L; q++)
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
                    { ritorno=(int)((status >> 8) & 0xFF);
                      printf("Il figlio con pid=%d ha ritornato %d (se 0 problemi nel figlio)\n", pid, ritorno);
                    }
            }
            exit(0);
    }
    ```
    
- 12 settembre 2018, modifica di un file
    
    Con un editor, scrivere un programma in C 12Set18.c che risolva la parte C dell’Esame del **12
    Settembre 2018** che si riporta qui di seguito per comodità.
    La parte in C accetta un numero variabile **N** di parametri (con **N** maggiore o uguale a **2**, da controllare) che
    rappresentano **N** nomi di file (**F1, F2. ... FN**).
    
    Il processo padre deve generare **N processi figli Pi** (**P0** ... **PN-1**): i processi figli **Pi (con i che varia da 0 a N-
    1)** sono associati agli **N** file **Ff** (con **f**= **i+1**). Ognuno di tali processi figli deve creare a sua volta un **processo
    nipote PPi (PP0 ... PPN-1**) associato sempre al corrispondente file **Ff**. Ogni processo figlio **Pi** e ogni nipote
    **PPi** esegue concorrentemente andando a cercare nel file associato **Ff** tutte le occorrenze dei caratteri
    **numerici** per il figlio e tutte le occorrenze dei caratteri **alfabetici minuscoli** per il nipote. Ognuno dei
    processi figlio e nipote deve operare una modifica del file **Ff:** in specifico, ogni *nipote* deve trasformare
    ogni carattere alfabetico minuscolo nel corrispondente carattere alfabetico maiuscolo, mentre ogni *figlio*
    deve trasformare ogni carattere numerico nel carattere spazio. Una volta terminate le trasformazioni, sia
    i processi figli **Pi** che i processi nipoti **PPi** devono comunicare al padre il numero (in termini di ***long int***) di
    trasformazioni effettuate. Il padre ha il compito di stampare su standard output, rispettando l'ordine dei
    file, il numero di trasformazioni ricevute da ogni figlio **Pi** e da ogni nipote **PPi**, riportando opportuni
    commenti esplicativi, che devono includere anche il nome del file che è stato interessato dalle
    trasformazioni.
    
    Al termine, ogni processo nipote **PPi** deve ritornare al figlio **Pi** un opportuno codice ed analogamente ogni
    processo figlio **Pi** deve ritornare al padre un opportuno codice; il codice che ogni nipote **PPi** e ogni figlio
    **Pi** deve ritornare è:
    
    1. **a) 0** se il numero di trasformazioni attuate è minore di 256;
    2. **b) 1** se il numero di trasformazioni attuate è maggiore o uguale a 256, ma minore di 512;
    3. **c) 2** se il numero di trasformazioni attuate è maggiore o uguale a 512, ma minore di 768;
    4. **d)** etc.
    
    Sia ogni figlio **Pi** e sia il padre devono stampare su standard output il PID di ogni nipote/figlio e il valore
    ritornato.
    
    **OSSERVAZIONE1:** In questo caso, devono essere previsti due array di pipe, uno per la comunicazione **figli-
    padre** e uno per la comunicazione **nipoti-padre** e queste pipe le deve *TUTTE* creare il padre prima di creare
    i figli!**OSSERVAZIONE2:** La chiusura delle pipe nipoti-padre da parte di ogni figlio va assolutamente fatta *SOLO*
    dopo la creazione del proprio nipote!
    
    Sistemi Operativi e Lab. (Prof.ssa Letizia Leonardi)
    
    **OSSERVAZIONE3:** Come nel programma sostituisciCar.c dell’esercitazione del 20 Aprile 2023,
    fare attenzione alla modalità di apertura del file associato, che deve essere aperto sia nel figlio che nel
    nipote, e di come si debba operare la trasformazione.**OSSERVAZIONE4:** Fare attenzione che, nonostante ogni figlio debba aspettare il proprio nipote per
    stampare come richiesto PID e valore ritornato, i figli NON devono tornare al padre tale valore, ma devono
    tornare un proprio valore, calcolato come indicato nel testo!
    
    ```jsx
    /* Soluzione della Prova d'esame del 12 Settembre 2018 - Parte C */
    #include <stdio.h>
    #include <string.h>
    #include <fcntl.h>
    #include <sys/stat.h>
    #include <unistd.h>
    #include <stdlib.h>
    #include <sys/wait.h>
    #include <ctype.h>
    
    typedef int pipe_t[2];
    
    int main(int argc, char **argv) 
    {
       /* -------- Variabili locali ---------- */
       int pid;			/* process identifier per le fork() */
       int N; 			/* numero di file passati sulla riga di comando */
       int status;			/* variabile di stato per la wait */
       pipe_t *pipedFP;		/* array dinamico di pipe descriptors per comunicazioni figli-padre  */
       pipe_t *pipedNP;		/* array dinamico di pipe descriptors per comunicazioni nipoti-padre  */
       int fd;			/* file descriptor che serva sia ai figli che ai nipoti */
       int i, j;			/* indici per i cicli */
       char ch; 			/* variabile che serve per leggere i caratteri sia da parte dei figli che dei nipoti */
       long int trasformazioni=0;	/* variabile che mantiene il numero delle trasformazioni effettuate sia dai figli che dai nipoti */
       int ritorno; 		/* variabile che viene ritornata da ogni figlio al padre e da ogni nipote al figlio */
       /* ------------------------------------ */
    	
    	/* Controllo sul numero di parametri (ci devono essere almeno di due parametri */
    	if (argc < 3) /* Meno di due parametri */  
    	{
    		printf("Errore nel numero dei parametri, dato che argc=%d\n", argc);
    		exit(1);
    	}
    
    	/* Calcoliamo il numero di file passati */
    	N = argc - 1;
    	
    	/* Allocazione dell'array di N pipe descriptors figli-padre */
    	pipedFP = (pipe_t *) malloc(N*sizeof(pipe_t));
    	if (pipedFP == NULL)
    	{
    		printf("Errore nella allocazione della memoria pipe figli-padre\n");
    		exit(2);
    	}
    	
    	/* Allocazione dell'array di N pipe descriptors nipoti-padre */
    	/* N.B. In questo caso i nipoti devono comunicare direttamente con il padre e quindi e' il padre che deve creare una pipe per ogni nipote e non il figlio come invece si puo' fare nei testi in cui la comunicazione deve essere nipote-figlio */
    	pipedNP = (pipe_t *) malloc (N*sizeof(pipe_t));
    	if (pipedNP == NULL)
    	{
    		printf("Errore nella allocazione della memoria pipe nipoti-padre\n");
    		exit(3);
    	}
    	
    	/* Creazione delle N pipe figli-padre e delle N pipe nipoti-padre */
    	for (i=0; i < N; i++)
    	{
    		if (pipe(pipedFP[i]) < 0)
    		{
    			printf("Errore nella creazione della pipe %d-esima figli-padre\n", i);
    			exit(4);
    		}
    		if (pipe(pipedNP[i]) < 0)
    		{
    			printf("Errore nella creazione della pipe %d-esima nipoti-padre\n", i);
    			exit(5);
    		}
    	}
    
    	/* Ciclo di generazione dei figli */
    	for (i=0; i < N; i++)
    	{
    		if ( (pid = fork()) < 0)
    		{
    			printf("Errore nella fork %d-esima\n", i);
    			exit(6);
    		}
    		
    		if (pid == 0) 
    		{
    			/* codice del figlio */
    			printf("DEBUG-Sono il processo figlio di indice %d e pid %d sto per creare il nipote che leggera' sempre dal mio stesso file %s\n", i, getpid(), argv[i+1]);
    			/* Chiusura delle pipe non usate nella comunicazione con il padre  */
    			for (j=0; j < N; j++)
    			{
    				close(pipedFP[j][0]);
    				if (i != j) close(pipedFP[j][1]);
    			}
    
    			/* N.B. Le chiusure per le altre pipe vanno fatte assolutamente dopo la creazione del nipote! */
    			if ( (pid = fork()) < 0)
    			{
    				printf("Errore nella fork di creazione del nipote\n");
    				exit(-1); /* decidiamo, in caso di errore, di tornare -1 che verra' interpretato come 255 e quindi un valore NON accettabile */
    			}	
    			if (pid == 0) 
    			{
    				/* codice del nipote */
    				printf("DEBUG-Sono il processo nipote del figlio di indice %d e pid %d\n", i, getpid());
    				/* chiusura della pipe rimasta aperta di comunicazione fra figlio-padre che il nipote non usa */
    				close(pipedFP[i][1]);
    				/* Chiusura delle pipe non usate nella comunicazione con il padre  */
    				for (j=0; j < N; j++)
    				{
    					close(pipedNP[j][0]);
    					if (i != j) close(pipedNP[j][1]);
    				}
    
    				/* sia il figlio che il nipote devono aprire (con due open separate per avere l'I/O pointer separato) lo stesso file in lettura/scrittura, dato che dovranno operare delle trasformazioni */
    				if ((fd=open(argv[i+1], O_RDWR)) < 0)
    				{
                                    	printf("Errore nella open del file %s da parte del nipote\n", argv[i+1]);
    					exit(-1); /* decidiamo, in caso di errore, di tornare -1 che verra' interpretato come 255 e quindi un valore NON accettabile */
                            	}
    		        	while (read(fd, &ch, 1))
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
    					else
    						; /* non si deve fare nulla */
    				}
    				/* il nipote deve inviare al padre il numero di trasformazioni operate */
    				write(pipedNP[i][1], &trasformazioni, sizeof(trasformazioni));
    				/* torniamo il valore richiesto dal testo operando una divisione intera per 256 */
    				ritorno=trasformazioni/256;	
    				exit(ritorno);
    			}
    			/* codice figlio */
    			/* le pipe usate dal nipote vanno chiuse TUTTE */
    			/* ATTENZIONE: SOLO DOPO AVERE CREATO IL NIPOTE! */
    			for (j=0; j < N; j++)
    			{
    				close(pipedNP[j][0]);
    				close(pipedNP[j][1]);
    			}
    
    			/* il figlio ha un codice molto simile al nipote */
    			/* sia il figlio che il nipote devono aprire (con due open separate per avere l'I/O pointer separato) lo stesso file in lettura/scrittura, dato che dovranno operare delle trasformazioni */
    			if ((fd=open(argv[i+1], O_RDWR)) < 0)
    			{
                                   	printf("Errore nella open del file %s da parte del figlio\n", argv[i+1]);
    				exit(-1); /* decidiamo, in caso di errore, di tornare -1 che verra' interpretato come 255 e quindi un valore NON accettabile */
                            }
    		       	while (read(fd, &ch, 1))
    			{	
    				/* controlliamo se abbiamo trovato un carattere numerico */
    				if (isdigit(ch))  
    				{ 	
    					/* questo carattere deve essere trasformato nel carattere spazio e quindi per prima cosa bisogna tornare indietro di una posizione */
    					lseek(fd, -1L, SEEK_CUR);
    				  	ch = ' '; /* trasformiamo il carattere nello spazio */
    				  	write(fd, &ch, 1); /* scriviamolo sul file */
    					/* e aggiorniamo il numero di trasformazioni */
    					trasformazioni++;
    				}
    				else
    					; /* non si deve fare nulla */
    			}
    			/* il figlio deve inviare al padre il numero di trasformazioni operate */
    			write(pipedFP[i][1], &trasformazioni, sizeof(trasformazioni));
    			/* il figlio deve aspettare il nipote e stampare il suo pid con il valore ritornato (come richiesto dal testo) */
    			if ((pid = wait(&status)) < 0)
    			{	
    				printf("Errore in wait\n");
    				exit(-1);
    			}
    			if ((status & 0xFF) != 0)
        				printf("Nipote con pid %d terminato in modo anomalo\n", pid);
        			else
    				printf("Il nipote con pid=%d ha ritornato %d\n", pid, ritorno=(int)((status >> 8) & 0xFF));
    
    			/* torniamo il valore richiesto dal testo operando una divisione intera per 256 */
    			ritorno=trasformazioni/256;	
    			exit(ritorno); 
    		}
    	}
    	
    	/* Codice del padre */
    	/* Il padre chiude i lati delle pipe che non usa */
    	for (i=0; i < N; i++)
    	{
    		close(pipedFP[i][1]);
    		close(pipedNP[i][1]);
     	}
    
    	/* Il padre recupera le informazioni dai figli in ordine di indice */
    	for (i=0; i < N; i++)
    	{
    		read(pipedFP[i][0], &trasformazioni, sizeof(trasformazioni));
    		printf("Il figlio di indice %d ha operato %ld trasformazioni di caratteri numerici in carattere spazio sul file %s\n", i, trasformazioni, argv[i+1]);
    		read(pipedNP[i][0], &trasformazioni, sizeof(trasformazioni));
    		printf("Il nipote di indice %d ha operato %ld trasformazioni di caratteri minuscoli in MAIUSCOLI sullo stesso file %s\n", i, trasformazioni, argv[i+1]);
    	}	
    
    	/* Il padre aspetta i figli */
    	for (i=0; i < N; i++)
    	{
    		if ((pid = wait(&status)) < 0)
    		{
    			printf("Errore in wait\n");
    			exit(7);
    		}
    
    		if ((status & 0xFF) != 0)
        			printf("Figlio con pid %d terminato in modo anomalo\n", pid);
        		else
    		{ 
    			ritorno=(int)((status >> 8) &	0xFF); 
    		  	if (ritorno==255)
     				printf("Il figlio con pid=%d ha ritornato %d e quindi vuole dire che ci sono stati dei problemi\n", pid, ritorno);
    		  	else  	printf("Il figlio con pid=%d ha ritornato %d\n", pid, ritorno);
    		}
    	}
    
    	exit(0);
    }
    ```
    
- 10 luglio 2019, uso di coppie di figli ( il primo è collegato all’ultimo e così via) e comunicazione tra coppie di figli
    
    La parte in C accetta un numero variabile N+1 di parametri (con N maggiore o uguale a 2, da controllare)
    che rappresentano i primi N nomi di file (F1, F2. ... FN), mentre l’ultimo rappresenta un singolo carattere
    Cz (da controllare).
    Il processo padre deve generare 2 * N processi figli (P0, P1 .. PN-1, PN, PN+1 ... P2*N-1): i processi
    figli vanno considerati a coppie, ognuna delle quali è associata ad uno dei file Ff (con f = i+1, con i che
    varia da 0 a ... N-1 e che corrisponde al file Fx con x = 2*N-i per i da N a 2*N-1). In particolare, la prima
    coppia è costituita dal processo P0 e dal processo P2*N-1, la seconda dal processo P1 e dal processo
    P2*N-2 e così via fino alla coppia costituita dal processo PN-1 e dal processo PN. Entrambi i processi
    della coppia (nel seguito chiamati primo e secondo processo) devono cercare il carattere Cz nel file
    associato Fi sempre fino alla fine attuando una sorta di staffetta così come illustrato nel seguito. Il primo
    processo della coppia Pi (con i che varia da 0 a ... N-1) deve cominciare a leggere dal file associato Ff
    cercando la prima occorrenza del carattere Cz; appena trovata deve comunicare al secondo processo della
    coppia Pi (con i da N a 2*N-1) la posizione del carattere trovato all’interno del file (in termini di long
    int); quindi il secondo processo della coppia deve partire nello stesso file associato Fx (che
    corrisponde a Ff) con la sua ricerca del carattere Cz dalla posizione seguente a quella ricevuta;
    appena trovata una nuova occorrenza di Cz, deve comunicare al primo processo della coppia la posizione
    del carattere trovato all’interno del file (in termini di long int) che quindi riparte dalla posizione seguente
    a cercare; tale staffetta deve avere termine quando il file è finito*.
    
    Al termine, ogni processo figlio deve ritornare al padre il numero di occorrenze del carattere Cz trovate
    dal singolo processo della coppia (supposto minore o uguale a 255) e il padre deve stampare su standard
    output il PID di ogni figlio e il valore ritornato.
    
    ```jsx
    /* soluzione parte C esame del 10 Luglio 2019 */
    #include <stdio.h> 
    #include <unistd.h> 
    #include <stdlib.h> 
    #include <sys/wait.h> 
    #include <string.h> 
    #include <sys/types.h> 
    #include <sys/stat.h> 
    #include <fcntl.h> 
    
    typedef int pipe_t[2]; 		/* tipo di dato per contenere i file descriptors di una pipe */
    
    int main(int argc, char **argv) 
    {
    	int N; 			/* numero di file: i processi figli saranno il doppio! */
    	int pid;		/* variabile per fork */
    	pipe_t *pipe_f;		/* array di pipe per la comunicazione dai primi N figli agli ultimi N figli */
    	pipe_t *pipe_fbis;	/* array di pipe per la comunicazione dagli ultimi N figli ai primi N figli */
    	/* OSSERVAZIONE: IN ALTERNATIVA POTEVA ESSERE USATO UN SOLO ARRAY DI DIMENSIONE 2 * N: CHIARAMENTE IN QUESTO CASO GLI INDICI DA USARE SAREBBERO DIVERSI RISPETTO A QUESTA SOLUZIONE! */
    	int fd;			/* variabile per open */
    	char ch;		/* variabile per leggere dai figli */
    	char Cz;		/* variabile per tenere traccia del carattere da cercare */
    	int occ;		/* variabile per tenere traccia del numero di occorrenze trovate */
    	long int pos;		/* posizione corrente del carattere trovato: inviamo il valore ricavato dalla lseek decrementato di 1 dato che dopo la lettura l'I/O pointer e' posizionato sul carattere seguente quello letto */
    	long int posLetta;	/* posizione corrente del carattere trovato ricevuta */
    	int status, pidFiglio, ritorno;	/* per wait */
    	int i, j;		/* indici per cicli */
    	int nr, nw;		/* per controlli read e write su/da pipe */
    
    /* Controllo sul numero di parametri */
    if (argc < 4) 
    {
    	printf("Errore numero parametri %d\n", argc);
    	exit(1);
    }
    
    /* calcoliamo il numero dei file: ATTENZIONE BISOGNA TOGLIERE 2 PERCHE' C'E' ANCHE IL CARATTERE Cz */
    N = argc - 2; 
    
    /* Controlliamo se l'ultimo parametro e' un singolo carattere */
    if (strlen(argv[argc-1]) != 1)
    {
            printf("Errore ultimo parametro non singolo carattere %s\n", argv[argc-1]);
            exit(2);
    }
    
    Cz = argv[argc-1][0]; /* isoliamo il carattere che devono cercare i figli */
    printf("Carattere da cercare %c\n", Cz);
    
    /* allocazione memoria dinamica per pipe_f e pipe_fbis */
    pipe_f=malloc(N*sizeof(pipe_t));
    pipe_fbis=malloc(N*sizeof(pipe_t));
    if ((pipe_f == NULL) || (pipe_fbis == NULL))
    {
    	printf("Errore nelle malloc\n");
    	exit(3);
    }
    
    /* creazione delle pipe */
    for (i=0; i < N; i++) 
    {
    	if (pipe(pipe_f[i])!=0) 
    	{
    		printf("Errore creazione delle pipe primi N figli e gli ultimi N\n");
    		exit(4);
    	}
    	if (pipe(pipe_fbis[i])!=0) 
    	{
    		printf("Errore creazione delle pipe ultimi N figli e i primii N\n");
    		exit(5);
    	}
    }
    
    /* creazione dei processi figli: ne devono essere creati 2 * N */
    for (i=0; i < 2*N; i++) 
    {
    	pid=fork();
     	if (pid < 0)  /* errore */
       	{
    		printf("Errore nella fork con indice %d\n", i);
          		exit(6);
       	}
    	if (pid == 0) 
    	{
     		/* codice del figlio */
    		/* stampa di debugging */
          		if (i < N) /* siamo nel codice dei primi N figli */
    		{
    			printf("Figlio di indice %d e pid %d associato al file %s\n",i,getpid(), argv[i+1]);
          			/* chiudiamo le pipe che non servono */
          			/* ogni figlio scrive solo su pipe_f[i] e legge solo da pipe_fbis[i] */
          			for (j=0;j<N;j++)
          			{
            			close(pipe_f[j][0]);
            			close(pipe_fbis[j][1]);
            			if (j!=i)
            			{
            				close(pipe_f[j][1]);
            				close(pipe_fbis[j][0]);
            			}
          			}
    
    			/* per i primi N processi, il file viene individuato come al solito */
    			fd=open(argv[i+1], O_RDONLY);
    			if (fd < 0) 
    			{
    				printf("Impossibile aprire il file %s\n", argv[i+1]);
    				exit(0); /* in caso di errore torniamo 0 che non e' un valore accettabile (per quanto risulta dalla specifica della parte shell) */
    			}
    
    			/* inizializziamo occ */
    			occ=0;
    	 		while (read(fd, &ch, 1)) 
    			{
    				if (ch == Cz) /* se abbiamo trovato il carattere da cercare */
    				{
    					/* incrementiamo occ */
    					occ++;
    					/* calcoliamo la posizione del carattere */
    					/* il valore ricavato dalla lseek lo decrementiamo di 1 dato che dopo la lettura l'I/O pointer e' posizionato sul carattere seguente quello letto */
    					pos=lseek(fd, 0L, SEEK_CUR) - 1;		
    					//printf("DEBUG- VALORE DI pos %ld per processo di indice %d che sto per mandare su pipe_f[i][1] %d\n", pos, i, pipe_f[i][1]);
    					/* inviamo la posizione del carattere all'altro processo della coppia */
    					nw=write(pipe_f[i][1], &pos, sizeof(pos));
    					if (nw != sizeof(pos))
        					{		
                            			printf("Impossibile scrivere sulla pipe per il processo di indice %d\n", i);
                            			exit(0); 
                   	 			}
    					/* aspettiamo dall'altro processo della coppia la nuova posizione da cui si deve riprendere la ricerca */
    					nr=read(pipe_fbis[i][0], &posLetta, sizeof(posLetta));
                            		//printf("DEBUG- VALORE DI nr %d per processo di indice %d\n", nr, i);
    					if (nr != sizeof(posLetta))
        					{		
                            			/* se non mi viene inviato alcuna posizione vuole dire che l'altro processo della coppia NON ha trovato altre occorrenze e quindi si puÃ² terminare la lettura */
    						break;
             		 		}		
    					/* printf("DEBUG- VALORE DI pos %ld per processo di indice %d che ho ricevuto da pipe_fbis[i][0] %d\n", pos, i, pipe_fbis[i][0]); */
    					/* spostiamo l'I/O pointer nella posizione seguente! */
    					lseek(fd, posLetta+1, SEEK_SET);
    				} 
    				else 
    				{  /* nulla, si continua a leggere */
    					;
    				}
    			}
    			exit(occ); /* torniamo il numero di occorrenze trovate (supposto dal testo <= di 255) */
    	}
    	else /* siamo nel codice degli ultimi N figli */
    	{
    		 printf("SECONDA SERIE DI FIGLI-Figlio di indice %d e pid %d associato al file %s\n",i,getpid(), argv[2*N-i]); /* ATTENZIONE ALL'INDICE CHE DEVE ESSERE USATO */
                            /* chiudiamo le pipe che non servono */
                            /* ogni figlio scrive solo su pipe_fbis[i] e legge solo da pipe_f[i] */
                            for (j=0;j<N;j++)
                            {
                                    close(pipe_f[j][1]);
                                    close(pipe_fbis[j][0]);
                                    if (j!= 2*N-i-1)	 /* ATTENZIONE ALL'INDICE CHE DEVE ESSERE USATO */
                                    {
                                            close(pipe_f[j][0]);
                                            close(pipe_fbis[j][1]);
                                    }
                            }
    
    			/* per gli ultimi N processi, il file viene individuato come indicato nel testo! */
                            fd=open(argv[2*N-i], O_RDONLY);
                            if (fd < 0)
                            {
                                    printf("Impossibile aprire il file %s\n", argv[2*N-i]);
                                    exit(0); /* in caso di errore torniamo 0 che non e' un valore accettabile (per quanto risulta dalla specifica della parte shell) */
                            }
    
                            /* inizializziamo occ */
                            occ=0;
                           	/* per prima cosa dobbiamo aspettare la posizione dall'altro figlio */
    			nr=read(pipe_f[2*N-i-1][0], &posLetta, sizeof(posLetta));
                            if (nr != sizeof(posLetta))
                            {
                            	printf("Impossibile leggere dalla pipe per il processo di indice %d (PRIMA LETTURA)\n", i);
                                    exit(0);
                            }
    			/* printf("DEBUG- VALORE DI pos %ld per processo di indice %d che ho ricevuto da pipe_fbis[2*N-i-1][0] %d\n", pos, i, pipe_fbis[2*N-i-1][0]); */
                            /* spostiamo l'I/O pointer nella posizione seguente! */
                            lseek(fd, posLetta+1, SEEK_SET);
                            while (read(fd, &ch, 1))
                            {
    				if (ch == Cz) /* se abbiamo trovato il carattere da cercare */
                                    {
                                            /* incrementiamo occ */
                                            occ++;
                                            /* calcoliamo la posizione del carattere */
                                            /* il valore ricavato dalla lseek lo decrementiamo di 1 dato che dopo la lettura l'I/O pointer e' posizionato sul carattere seguente quello letto */
                                            pos=lseek(fd, 0L, SEEK_CUR) - 1;
    					/* inviamo la posizione del carattere all'altro processo della coppia */
    					/* printf("DEBUG- VALORE DI pos %ld per processo di indice %d che sto per mandare su pipe_f[2*N-i-1][1] %d\n", pos, i, pipe_f[2*N-i-1][1]); */
                                            nw=write(pipe_fbis[2*N-i-1][1], &pos, sizeof(pos));
                                            if (nw != sizeof(pos))
                                            {
                                                    printf("Impossibile scrivere sulla pipe per il processo di indice %d\n", i);
                                                    exit(0);
                                            }
                                            /* aspettiamo dall'altro processo della coppia la nuova posizione da cui si deve riprendere la ricerca */
                                            nr=read(pipe_f[2*N-i-1][0], &posLetta, sizeof(posLetta));
                                            if (nr != sizeof(posLetta))
        					{		
                            			/* se non mi viene inviato alcuna posizione vuole dire che l'altro processo della coppia NON ha trovato altre occorrenze e quindi si puÃ² terminare la lettura */
    						break;
             		 		}		
    					/* printf("DEBUG- VALORE DI pos %ld per processo di indice %d che ho ricevuto da pipe_fbis[i][0] %d\n", pos, i, pipe_fbis[i][0]); */
                            	        /* spostiamo l'I/O pointer nella posizione seguente! */
                                            lseek(fd, posLetta+1, SEEK_SET);
                                    }
                                    else
                                    {  /* nulla, si continua a leggere */
                                            ;
                                    }
                            }
                    	exit(occ); /* torniamo il numero di occorrenze trovate (supposto dal testo <= di 255) */
            }
    }
    }
    
    /*codice del padre*/
    /* chiudiamo tutte le pipe, dato che le usano solo i figli */
    for (i=0;i<N;i++)
    {
       close(pipe_f[i][0]);
       close(pipe_f[i][1]);
       close(pipe_fbis[i][0]);
       close(pipe_fbis[i][1]);
    }
    
    /* Attesa della terminazione dei figli */
    for(i=0;i<2*N;i++)
    {
       pidFiglio = wait(&status);
       if (pidFiglio < 0)
       {
          printf("Errore wait\n");
          exit(7);
       }
       if ((status & 0xFF) != 0)
                    printf("Figlio con pid %d terminato in modo anomalo\n", pidFiglio);
       else
       {
                    ritorno=(int)((status >> 8) & 0xFF);
                    printf("Il figlio con pid=%d ha ritornato %d\n", pidFiglio, ritorno);
       }
    }
    exit(0);
    }/* fine del main */
    ```