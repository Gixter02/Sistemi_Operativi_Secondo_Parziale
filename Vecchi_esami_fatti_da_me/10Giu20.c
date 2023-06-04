#include <stdio.h>		// Includo la libreria per la funzione printf e BUFSIZ
#include <stdlib.h>		// Includo la libreria per la funzione exit
#include <unistd.h>		// Includo la libreria per la funzione close, fork, famiglia exec, read, write, lseek, famiglia get, pipe,
#include <fcntl.h>		// Includo la libreria per la funzione open, creat e le relative macro
#include <sys/wait.h>	// Includo la libreria per la funzione wait
#include <ctype.h>

//definisco il tipo pipe_t
typedef int pipe_t[2];

int main(int argc, char** argv) {
    if (argc < 2 + 1) /* controllo sul numero di parametri: devono essere in numero maggiore uguale a 2*/
    {
        printf("Errore: numero di argomenti sbagliato dato che argc = %d\n", argc);
        exit(1);
    }
    int Q = argc -1; /* numero di file e figli */
    char linea[250];
    //apro Q pipes
    pipe_t* piped = (pipe_t*)malloc(sizeof(pipe_t) * (Q));
    if (piped == NULL) {
        printf("Errore nell'allocazione della memoria\n");
        exit(2);
    }
    
    for(int i = 0; i < Q; i++){
        if(pipe(piped[i])<0){
            printf("Errore nella creazione della pipe numero:%d\n",i);
            exit(3);
        }
    }
    int pid;	// memorizza il valore di ritorno della funzione fork
    //creo il file "camilla"
    char* filename = "camilla";
    int fd = 0; // variabile che conterra il file descriptor del file "camilla" */
    if((fd = creat(filename , 0644)) < 0){		/* ERRORE se non si riesce a creare il file */
        printf("Errore in creazione del file %s dato che fd = %d\n", filename, fd);
        exit(4);
    }
    for (int q = 0; q < Q; q++){

        
        if ((pid = fork()) < 0)	/* Il processo padre crea un figlio */
        {	/* In questo caso la fork e' fallita */
            printf("Errore durante la fork\n");
            exit(5);
        }
        
        if (pid == 0)
        {	/* processo figlio */

            for(int k = 0; k < Q; k++){
                close(piped[k][0]);
                if (k != q) {
                    close(piped[k][1]);
                }
            }

            //apro il file argv[q+1]
            int fd = 0; // variabile che conterra' il file descriptor del file argv[q+1]
            if((fd = open(argv[q+1], O_RDONLY)) < 0){		/* ERRORE se non si riesce ad aprire il file */
                printf("Errore in apertura file %s dato che fd = %d\n", argv[q+1], fd);
                exit(6);
            }

            char buffer;	// carattere usato per leggere il contenuto del file
            int numero_linee_inviate = 0;
            int contatore = 0;
            
            while (read(fd, &buffer, sizeof(char)) > 0)	/* ciclo di lettura fino a che riesco a leggere un carattere da file */
            {
                linea[contatore] = buffer;
                contatore++;
                if(buffer == '\n'){
                    if(contatore <= 10){
                        if(isdigit(linea[0])){
                            linea[contatore-1] = '\0';
                            int nw = write(piped[q][1],linea, contatore*sizeof(char));
                            if (nw != contatore){
                                printf("Errore, nella write\n"); 
                                exit(7);
                            }
                            numero_linee_inviate++;
                            
                        }
                    }
                    contatore = 0;
                }
            }
            exit(numero_linee_inviate);
        }
     
    }
    /* processo padre */
    //chiudo tutte le pipe in lettura e scrittura tranne la i-esima pipe, aperta in lettura
    for(int i = 0; i < Q; i++){
 
            close(piped[i][1]);
            
        }

    for(int i = 0; i < Q; i++){
        char buffer;	// carattere usato per leggere il contenuto del file
        int contatore = 0;
        while (read(piped[i][0], &buffer, sizeof(char)) > 0)	/* ciclo di lettura fino a che riesco a leggere un carattere da file */
        {
            contatore++;
            linea[contatore-1]=buffer;
            if(buffer == '\0'){
                printf("Figlio di indice %d ha letto dal file %s la linea: %s\n", i, argv[i+1], linea);
                contatore = 0;
            }
        }

    }
 
    
    for(int i = 0; i < Q; i++){
        int status;	// La variabile usata per memorizzare quanto ritornato dalla primitiva wait
        int ritorno;	// La variabile usata per memorizzare il valore di ritorno del processo figlio
        if ((pid = wait(&status)) < 0) {
            printf("Non e' stato creato nessun processo figlio\n");
            exit(7);
        }
        
        if ((status & 0xFF) != 0)
        {
            printf("Il processo figlio è stato terminato in modo anomalo\n");
        } else {
            ritorno = (status >> 8) & 0xFF;
            printf("Il figlio con pid=%d ha ritornato %d (se 255 significa che il figlio e' terminato con un errore)\n", pid, ritorno);
        }
    }

    exit(0);
}