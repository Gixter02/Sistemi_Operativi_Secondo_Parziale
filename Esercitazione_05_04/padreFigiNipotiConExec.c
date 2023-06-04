#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>
#define PERM 0644

int main(int argc, char ** argv){
    int pid;        			/* per fork */
    int pidFiglio, status, ritorno;         /* per wait padre */
    if (argc < 4){
        printf("Troppi pochi parametri\n");
        exit(1);
    }
    for(int i = 0; i < argc - 1; i++){
        if ((pid = fork()) < 0)
	{	/* fork fallita */
		printf("Errore in fork\n");
		exit(2);
	}
    if(pid==0){
        int f;
        char* str = argv[i+1];
        str = strcat(str,".sort");
        if((f=creat(str, PERM))< 0){
            printf("ERRORE nella creazione del file\n");
            exit(-1);
        }
        if ((pid = fork()) < 0){
            printf("ERRORE nella creazione del nipote\n");
            exit(-1);
        }
        if(pid==0){
            close(0);
            if((open(argv[i+1],O_RDONLY))< 0){
                printf("ERRORE nell'apertura del file %s\n", argv[i+1]);
                exit(-1);
            }
            close(1);
            if((open(str,O_WRONLY))< 0){
                printf("ERRORE nell'apertura del file %s\n", argv[i+1]);
                exit(-1);
            }

        }
    }
    }
}