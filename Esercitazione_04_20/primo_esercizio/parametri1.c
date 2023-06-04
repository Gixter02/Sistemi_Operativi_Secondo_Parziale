#include <stdio.h>
#include <stdlib.h>



int main(int argc, char** argv){

if (argc < 2){
printf("Inserire almeno un parametro\n");
exit(1);
}

printf("Il nome dell'eseguibile e' %s\n", argv[0]);

int N = argc - 1;
for (int i = 0; i < N ; i++){
	printf("Il parametro numero %d e' %s\n", i, argv[i+1]);
}

exit(0);
}
