#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/apidisk.h"
#include "../include/t2fs.h"

#define TAM_MAX 255

int main(int argc, char *argv[]){

	  char name[TAM_MAX];
	  int size = TAM_MAX;


	  if ( identify2(name, size) == 1){
		puts(name);	
		return 0;
	  }
	    

	  else{
	    printf("Erro no identify\n");
	    return 0;
	  }
}
