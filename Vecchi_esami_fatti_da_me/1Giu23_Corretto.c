#include <stdio.h>		// Includo la libreria per la funzione printf e BUFSIZ
#include <stdlib.h>		// Includo la libreria per la funzione exit
#include <unistd.h>		// Includo la libreria per la funzione close, fork, famiglia exec, read, write, lseek, famiglia get, pipe,
#include <fcntl.h>		// Includo la libreria per la funzione open, creat e le relative macro
#include <sys/wait.h>	// Includo la libreria per la funzione wait
#include <errno.h>
#include <string.h>

//definisco il tipo pipe_t
typedef int pipe_t[2];

typedef struct {
    int Pid_PIDNipote;
    char ultima_linea_PIDNipote[250];
    int lunghezza;
} struct_t ;

int main(int argc, char** argv) {
    int H; /* numero di file */
    int h; /* numero dei processi figli */
    char store [250]; /* variabile per leggere le linee del file associato */
    struct_t st; /* struttura dati sia per figli che per padri */
    pipe_t* piped; /* pipe per comunicazione padre-figlio */
    //pipe_t p;  /* pipe per comunicazione figlio-PIDNipote */
    int pid;	// memorizza il valore di ritorno della funzione fork
    int PIDNipote;	// memorizza il valore di ritorno della funzione fork
    int status;	// La variabile usata per memorizzare quanto ritornato dalla primitiva wait
    int ritorno;	// La variabile usata per memorizzare il valore di ritorno del processo figlio
            

    if (argc < 2 + 1) /* controllo sul numero di parametri: devono essere in numero maggiore o uguale a 2*/
    {
        printf("Errore: numero di argomenti sbagliato dato che argc = %d\n", argc);
        exit(1);
    }
    H = argc -1;
    //apro H pipes
    piped = (pipe_t*)malloc(sizeof(pipe_t) * (H));
    if (piped == NULL) {
        printf("Errore nell'allocazione della memoria\n");
        exit(2);
    }
    
    for(int i = 0; i < H; i++){
        if(pipe(piped[i])<0){
            printf("Errore nella creazione della pipe numero:%d\n",i);
            exit(3);
        }
    }
    for(h = 0; h < H; h++){
        
        if ((pid = fork()) < 0)	/* Il processo padre crea un figlio */
        {	/* In questo caso la fork e' fallita */
            printf("Errore durante la fork\n");
            exit(4);
        }
        
        if (pid == 0)
        {	/* processo figlio */
            printf("Creo il figlio numero %d\n", h);
            //chiudo tutte le pipe in lettura e scrittura tranne la i-esima pipe, aperta in lettura
            for(int k = 0; k < H; k++){
                close(piped[k][0]);
                if (k != h) {
                    close(piped[k][1]);
                }
            }
            //apro il file argv[h+1]
            
             pipe_t p;
            
            
            if (pipe(p) < 0 )
            {	printf("Errore creazione pipe\n");
                exit(7);
            }

            if ((PIDNipote = fork()) < 0)	/* Il processo figlio crea un nipote */
            {	/* In questo caso la fork e' fallita */
                printf("Errore durante la fork\n");
                exit(6);
            }
           

            if (PIDNipote == 0)
            {	/* processo nipote */
                printf("Creo il nipote %d\n", h);
                close(p[0]);
                close(0);
                printf("fatta la close 0\n");
                if((open(argv[h+1], O_RDONLY)) < 0){		/* ERRORE se non si riesce ad aprire il file */
                    printf("Errore in apertura file %s\n", argv[h+1]);
                    exit(5);
                }
                printf("aperto il file\n");
                // chiudo lo stdout
                
                
                close(1);
                printf("fatta la close 1\n");
                // duplico la pipe dal lato di scrittura
                dup(p[1]);
                printf("fatta la dup 1\n");
                // chiudo le pipe che non mi servono
                
                printf("fatta la clsoe p 0\n");
                //close(p[1]);
                printf("faccio la rev numero %d\n", h);
                execlp("rev","rev", (char *)0);
                exit(-1);
            }
            /* processo figlio */
            close(p[1]);
            st.Pid_PIDNipote = PIDNipote; 

            int j=0;
            printf("DEBUG:debug print\n");
            // store = calloc(250, sizeof(char));
            while (read(p[0], &store[j], 1))	/* ciclo di lettura fino a che riesco a leggere un carattere da file */
            {

                /*if(store[j] == '\0'){   //non sono stringhe, quindi non serve
                    st.Pid_PIDNipote = PIDNipote;
                    strcpy(st.ultima_linea_PIDNipote, store );
                    st.lunghezza = j+1;
                    write(piped[h][1], &st, sizeof(struct_t));
                }*/

                if(store[j]=='\n'){
                                       
                    st.lunghezza=j+1;
                   
                    j=0;
                }   else    j++;
            }
                memcpy(st.ultima_linea_PIDNipote, store, 250);
                write(piped[h][1], &st, sizeof(struct_t));
            exit(st.lunghezza-1);
        }
        
    }
    /* processo padre */
    //chiudo tutte le pipe in lettura e scrittura tranne la i-esima pipe, aperta in lettura
    for(int k = 0; k < H; k++){
        close(piped[k][1]);
    }
    
    
    /*while ((nread = read(piped[h][0], &st, sizeof(struct_t))) > 0 )	 ciclo di lettura fino a che nread e' maggiore di 0 
    {
        printf("L'ultima linea ricevuta dal nipote %d del file %s e' lunga %d e contiene :%s", st.Pid_PIDNipote, argv[h], st.lunghezza, st.ultima_linea_PIDNipote);
    }*/

    //Devi fare un for per la lettura dei figli, non il while

    for (h=0; h<H; h++) {
        read(piped[h][0], &st, sizeof(struct_t));
        st.ultima_linea_PIDNipote[st.lunghezza]='\0';
        printf("L'ultima linea ricevuta dal nipote %d del file %s e' lunga %d e contiene :%s\n", st.Pid_PIDNipote, argv[h], st.lunghezza, st.ultima_linea_PIDNipote);

    }

    for(int i = 0; i < H; i++){
        if ((pid = wait(&status)) < 0) {
            printf("Non e' stato creato nessun processo figlio\n");
            exit(9);
        }

        if ((status & 0xFF) != 0)
        {
            printf("Il processo figlio con PID %d e' stato terminato in modo anomalo\n", pid);
        } else {
            ritorno = (status >> 8) & 0xFF;

            printf("Il filgio con PID = %d ha ritornato il valore %d\n", pid, ritorno);
        }
    }

    exit(0);
}