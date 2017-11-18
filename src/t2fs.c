#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/apidisk.h"
#include "../include/t2fs.h"

int identify2 (char *name, int size){
	

	  strcpy(name, "Geronimo Veit\t260004\nJulia Rittmann\t262512\nVilmar Fonseca\t262519\n");

	  if (sizeof(name)<0 || sizeof(name)>size)
	    return -1;
	  else
	    return 1;
}
