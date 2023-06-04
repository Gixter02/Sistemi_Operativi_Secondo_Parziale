#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

int main(int argc, char ** argv){
    int pid, pidFiglio, status, ritorno;
    if (argc < 3){
        printf("Numero dei parametri errato %d: ci vogliono più di 1 parametro\n", argc);
    		exit(1);
    }
    exit(0);
}