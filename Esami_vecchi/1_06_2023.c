#include <stdio.h>	// Includo la libreria per la funzione printf e BUFSIZ
#include <stdlib.h>	// Includo la libreria per la funzione exit
#include <unistd.h>	// Includo la libreria per la funzione close, fork, exec, read, write
#include <fcntl.h>	// Includo la libreria per la funzione open e le relative macro
#include <sys/wait.h>	// Includo la libreria per la funzione wait
#include <string.h>

//definisco il tipo pipe_t
typedef int pipe_t[2];

typedef struct {
    int pid;
    char linea[250];
    int len;
} struct_t; //definita la struct 

int main(int argc, char** argv) {
    
    /*-------------------------------------*/
    int H;                      //numero di file
    int h;                      //indice per i figli

    struct_t st;                //Definisco la struct
    char store[250];            //Per leggere dai nipoti
    int pidChild;	// memorizzo il valore di ritorno della funzione fork per il nipote
    int pid;	// memorizzo il valore di ritorno della funzione fork per il figlio



    int j;                      //per il conteggio della read

    pipe_t* piped;              //pipe tra padre padre e figlio
    pipe_t p;                   //pipe nipote figlio

    int ritorno, status, pidFiglio;         //variabili per la wait
    /*-------------------------------------*/

    if (argc < 3) /* controllo sul numero di parametri: devono essere in numero maggiore o uguale a 2*/
    {
        printf("Errore: numero di argomenti sbagliato dato che argc = %d\n", argc);
        exit(1);
    }

    H=argc-1;       //definisco il numero dei file

    //apro H pipes
    piped=malloc(sizeof(pipe_t)*H);         //alloco  la memoria per le pipe

    for(int i=0;i<H;i++){
        if(pipe(piped[i])<0){
            printf("Errore nella creazione della pipe numero:%i",i);
            exit(2);
        }
    }

    for (h=0; h<H; h++) {        
        if ((pid = fork()) < 0)	/* Il processo padre crea un figlio */
        {	/* In questo caso la fork e' fallita */
            printf("Errore durante la fork\n");
            exit(3);
        }
        
        if (pid == 0)
        {	/* processo figlio */
            //chiudo tutte le pipe in lettura e scrittura tranne la i-esima pipe, aperta in scrittura
            for(int k = 0; k < H; k++){
                close(piped[k][0]);
                if (k != h) {
                    close(piped[k][1]);
                }
            }

            //creao la pipe per il nipote
            if (pipe(p) < 0 )
            {	printf("Errore creazione pipe\n");
                exit(4);
            }

            
            if ((pidChild = fork()) < 0)	/* Il processo figlio crea un nipote */
            {	/* In questo caso la fork e' fallita */
                printf("Errore durante la fork\n");
                exit(5);
            }
            
            if (pidChild == 0)
            {	/* processo nipote */
                
                close(p[0]);    //questo lato di pipe non ci serve
                close(0);       //chiudo lo standard input
                //controllo se il file e' accedibile
                if((open(argv[h+1], O_RDONLY)) < 0){		/* ERRORE se non si riesce ad aprire in LETTURA il file */
                    printf("Errore in apertura file %s \n", argv[h+1]);
                    exit(6);
                }           //ho ridirezionato lo std input
                
                close(1);
                dup(p[1]);  //ho ridirezionato lo std output

                execlp("rev", "rev", (char *) 0);   //faccio la exec
                //Se arrivo qui vuol dire che è fallita la exec, quindi esco con errore
                exit(-1);   
            }
            /* processo figlio */
            st.pid=pidChild;        //conservo il valore del pid nipote

            close(p[1]);            

            //faccio la read per carattere
            j=0;
            while(read(p[0], &store[j], 1)) {
                if (store[j] == '\n') {
                    st.len = j+1;
                    //conservo la lunghezza nel caso questa sia l'ultima linea
                    j=0;
                }   else    j++;
            }

            memcpy(st.linea, store, 250);   //copio l'ultima linea letta nella mia struct
 
            write(piped[h][1], &st, sizeof(struct_t));      //mando la struct al padre
            exit(st.len-1);     //ritorno il valore della lunghezza -1
        }
        
    }
    /* processo padre */

    //chiudo tutte le pipe in scrittura 
    for(int k = 0; k < H; k++){
        close(piped[k][1]);
    }       

    for (h=0; h<H; h++) {   //per ogni figlio leggo la struct e poi stampo i dati su std output
        read(piped[h][0], &st, sizeof(struct_t));
        st.linea[st.len]='\0';      //devo mettere il terminatore di stringa
        printf("Il nipote con pid: %d ha letto dal file %s, e l'ultima linea invertita ha lunghezza %d, ed e': %s\n", st.pid, argv[h+1], st.len, st.linea);
    }

    //Il padre aspetta i figli
    for(h=0; h<H; h++) {
        
        if ((pidFiglio = wait(&status)) < 0) {
            printf("Non e' stato creato nessun processo figlio\n");
            exit(7);
        }
        
        if ((status & 0xFF) != 0)
        {
            printf("Il processo figlio è stato terminato in modo anomalo\n");
            exit(8);
        } else {
            ritorno = (status >> 8) & 0xFF;
            printf("Il padre ha aspettato il figlio con pid=%i ed ha ritornato=%i\n",pidFiglio,ritorno);
        }   //Stampo il pid dei figli e il valore che ritornano
    }

    exit(0);
}