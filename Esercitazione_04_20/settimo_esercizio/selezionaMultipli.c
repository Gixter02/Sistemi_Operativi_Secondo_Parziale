#include <stdio.h> /* Standard Input-Output */
/* include necessario per usare printf e altre; inoltre
definisce la costante BUFSIZ */
#include <fcntl.h> /* File CoNTroL */
/* include necessario per usare la primitiva open e le
relative costanti come O_RDONLY */
#include <stdlib.h> /* Standard Library */
/* include necessario per usare la primitiva exit */
#include <unistd.h> /* UNIX Standard */
/* include necessario per usare la primitiva read, write
e close */
#include <string.h>

int main(int argc, char** argv){
    if (argc != 3){
        printf("ERRORE:Il programma deve essere invocato con esattamente 3 parametri, invece che %d\n", argc);
        exit(1);
    }
    int fd;
    long int pos; 		/* per la lseek */
    long int lunghezza; 	/* per la lunghezza del file */
    int i; 			/* indice del multiplo */
    char ch;

    if((fd=open(argv[1],O_RDONLY))< 0){
        printf("ERRORE nell'apretura del file %s, oppure non esiste", argv[1]);
        exit(2);
    }
    int n = atoi(argv[2]);
    if(n<=0){
        printf("Errore: Il secondo parametro non e' un numero strettamente maggiore di 0\n");
        	exit(3);
    }
    lunghezza=lseek(fd, 0L, SEEK_END); /* NOTA BENE: dopo l'invocazione di questa primitiva il file pointer si trova alla fine del file; non importa pero' riportarlo all'inizio dato che comunque all'interno del ciclo ci si spostera' sempre sul carattere 'giusto' */

	printf("Stiamo per selezionare i caratteri multipli di %d\n", n);
	i = 1; /* valore iniziale del multiplo */
	pos=0L; /* inizializzazione per entrare il ciclo */
    while (pos < lunghezza){
        /* calcoliamo la posizione del carattere che deve essere letto */
        	pos=(long int)(i * n);
        	if (pos < lunghezza) 	/* dobbiamo controllare di essere sempre dentro al file */
		{
			/* chiamiamo la lseek passando come offset pos-1 dall'inizio del file: dobbiamo considerare -1 altrimenti leggeremmo il carattere sbagliato */
        		lseek(fd, pos-1, SEEK_SET);
        		read(fd, &ch, 1);
			printf("Il carattere multiplo %d-esimo all'interno del file %s e' %c\n", i, argv[1], ch); 
  			i++; 	/* incrementiamo il conteggio */
		}	
		else printf("Il file non ha una dimensione multipla di %d\n", n);
	}
    exit(0);
}