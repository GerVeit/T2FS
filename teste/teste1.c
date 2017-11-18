#include <stdio.h>
#include <stdlib.h>
#include "../include/apidisk.h"
#include "../include/t2fs.h"

#define TAM_MAX 255

int main(){

  char *name;
  int size = TAM_MAX;
  
  if (identify2(name, size) == 1)
    return 0;

  else{
    printf("Erro no identify\n");
    return 0;
  }

}
