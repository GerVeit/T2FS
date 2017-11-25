#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/apidisk.h"
#include "../include/t2fs.h"

#define SIZE_OF_INT 4

#define ERROR -1
#define SUCCESS 0

int main(int argc, char *argv[]){

	unsigned char bufferRead[SECTOR_SIZE];		//256 unsigned char type
	//unsigned char bufferWrite[SECTOR_SIZE];

	unsigned int superblock_sector = 0x00000000;
	
	unsigned char t2fs[4] = "T2FS";	
	
	//test to know endianess of my machine
	unsigned int number_test = 1;
		
	char *char_cast = (char*)&number_test;
	if (*char_cast)
		printf("My machine is Little endian\n");

	else
		printf("My machine is Big endian\n");
	

	if (read_sector(superblock_sector, bufferRead) != 0){
		printf("Erro ao ler setor\n");
		return ERROR;
	}


	printf("size of buffer is %d bytes in little endian format\n", sizeof(bufferRead));
	printf("size of unsigned char is %d byte(s)\n", sizeof(unsigned char));
	printf("size of unsigned int is %d byte(s)\n", sizeof(unsigned int));

	printf("\n\t\t\tImportant!\n\tThe infos shown below are DECIMAL-based.\n\tNote that ASCII characters will be represented by its binary sequence converted to DECIMAL\n\n");

	/*
		Printig every single byte of the first logic sector
		Warning!! File .dat MUST contain only binary data, double check it 
		when compiling
	*/

	for(int buff_cont = 0; buff_cont<sizeof(bufferRead); buff_cont++)
		printf("buffer[%d] = %u\n",buff_cont, (unsigned char)bufferRead[buff_cont]); 	
	
	
	//	Concatenating bytes (**Note that the type int's size is 4 bytes as printed above

	printf("\n\nDissected inforamation:\n\n");

	char id[SIZE_OF_INT+1];
	
	for(int loop_control=0; loop_control<SIZE_OF_INT; loop_control++)
		id[loop_control] = (char)bufferRead[loop_control];
		
	printf("id = \t\t\t%s\n", id);

	
	return SUCCESS;
}
