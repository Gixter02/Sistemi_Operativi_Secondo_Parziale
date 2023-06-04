#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char **argv)
{       int nread;                      /* valore ritorno read */
	char buffer[BUFSIZ];            /* usato per i caratteri */
	int fd = 0;			/* usato per la open; N.B. se non viene passato alcun parametro allora rimarra' uguale a 0! */
					/* quindi identifichera' lo STANDARD INPUT */

	if (argc < 2) /* controllo sul numero di parametri: devono essere in numero minore o uguali a 1! CONTROLLO LASCO! */
        {       printf("Errore: numero di argomenti sbagliato dato che argc = %d\n", argc);
                exit(1); 
	}
	int n = argc - 1; /* Numero di file che devo aprire */
	if (argc >= 2)
	/* abbiamo un parametro che deve essere considerato il nome di un file */
	for(int i = 0; i < n; i++){
        	if ((fd = open(argv[i+1], O_RDONLY)) < 0)	        /* ERRORE se non si riesce ad aprire in LETTURA il file */
		{       printf("Errore in apertura file %s dato che fd = %d\n", argv[i+1], fd);
                 	exit(2); 
		}

	/* se non abbiamo un parametro, allora fd rimane uguale a 0 */
		/* lettura dal file o dallo standard input fino a che ci sono caratteri */
		while ((nread = read(fd, buffer, BUFSIZ)) > 0 )
			/* scrittura sullo standard output dei caratteri letti */
        	        write(1, buffer, nread);
	}
	exit(0);
}

