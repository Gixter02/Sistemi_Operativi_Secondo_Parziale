#include <stdio.h> /* Standard Input-Output */
/* include necessario per usare printf e altre; inoltre
definisce la costante BUFSIZ */
#include <fcntl.h> /* File CoNTroL */
/* include necessario per usare la primitiva open e le
relative costanti come O_RDONLY */
#include <stdlib.h> /* Standard Library */
/* include necessario per usare la primitiva exit */
#include <unistd.h> /* UNIx Standard */
/* include necessario per usare la primitiva read, write
e close */
#include <string.h>

int main(int argc, char** argv){
    if(argc != 3){
        printf("Il programma deve essere invocato con 2 parametri!\n");
        exit(1);
    }
    int fd = open(argv[1], O_RDONLY);
    if(fd < 0){
        printf("Errore nell'apretura del file!\n");
        exit(2);
    }
    if(strlen(argv[2]) != 1){
        printf("Il secondo parametro non e\' lungo un solo carattere\n");
        close(fd);
        exit(3);
    }
    long int occorrenze = 0;
    int nread;
    char buf[1];
    while((nread = read(fd,buf,1))>0){
        if(buf[0]==argv[2][0]){
            occorrenze++;
        }
    }
    printf("Le occorenze del carattere %s sono %ld\n", argv[2], occorrenze);
    close(fd);
    exit(0);
}