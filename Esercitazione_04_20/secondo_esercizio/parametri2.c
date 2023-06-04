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
if (argc!=4){
printf("Il programma deve essere invocato con esattamente 3 parametri!\n");
exit(1);
}

int fd = open(argv[1], O_RDONLY);
if (fd < 0){
printf("Errore nell'apertura del file!\n");
exit(2);
}



int N = atoi(argv[2]);
if(N<=0){
printf("Il numero deve essere positivo\n");
exit(3);
}

size_t len = strlen(argv[3]);
if(len!=1){
printf("Il terzo parametro deve essere un solo carattere!\n");
exit(4);
}

printf("Il nome dell'eseguibile e' %s\n", argv[0]);
printf("Il primo parametro e' un file che si chiama %s\n", argv[1]);
printf("Il secondo parametro e' %d", N);
printf("Il terzo parametro e' il carattere %s\n", argv[3]);


close(fd);
exit(0);
}
