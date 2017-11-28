#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/apidisk.h"
#include "../include/t2fs.h"

#define ERROR -1
#define SUCCESS 0
#define SIZE_OF_INT 4

/*Funcao de inicializacao e leitura dos dados, populando a struct do superbloco*/
void init(){

	struct t2fs_superbloco *data_superbloco;
	
	printf("Esta merda de trabalho (init)\n");

	/**********Lendo o Superbloco**********/

	data_superbloco = (struct t2fs_superbloco*)malloc(sizeof(struct t2fs_superbloco));
	
	unsigned char bufferRead[SECTOR_SIZE];

	unsigned int superblock_sector = 0x00000000;
	
	if (read_sector(superblock_sector, bufferRead) != 0){
		printf("Erro ao ler setor\n");
		return ERROR;
	}

	for(int buff_cont = 0; buff_cont<sizeof(bufferRead); buff_cont++)
		printf("buffer[%d] = %u\n",buff_cont, (unsigned char)bufferRead[buff_cont]); 	
	
	printf("\n\nDissected inforamation:\n\n");

	char id[SIZE_OF_INT+1];

	printf("id: \t\t\t");
	for(int loop_control=0; loop_control<SIZE_OF_INT; loop_control++){
		id[loop_control] = (char)bufferRead[loop_control];
		printf("%c", id[loop_control]);
	}

	data_superbloco->version = bufferRead[5]*256+bufferRead[4];
	printf("\nVersion: \t\t0x%.4x\t(0x7e1 = 2017; 2 = 2 Semestre)\n", data_superbloco->version);

	data_superbloco->SuperBlockSize = (bufferRead[6]*256+bufferRead[7])/256;
	printf ("SuperBlockSize: \t%d Setores Logicos\n", data_superbloco->SuperBlockSize);

	data_superbloco->DiskSize = (bufferRead[8]+bufferRead[9]*256+bufferRead[10]*65536+bufferRead[11]*16777216);
	printf ("DiskSize: \t\t%d bytes\n", data_superbloco->DiskSize);

	data_superbloco->NofSectors = (bufferRead[12]+bufferRead[13]*256+bufferRead[14]*65536+bufferRead[15]*16777216);
	printf ("NofSectors: \t\t%d Setores Logicos\n", data_superbloco->NofSectors);

	data_superbloco->SectorsPerCluster = (bufferRead[16]+bufferRead[17]*256+bufferRead[18]*65536+bufferRead[19]*16777216);
	printf ("SectorsPerCluster: \t%d Setores Logicos\n", data_superbloco->SectorsPerCluster);
	
	data_superbloco->pFATSectorStart = (bufferRead[20]+bufferRead[21]*256+bufferRead[22]*65536+bufferRead[23]*16777216);
	printf ("pFATSectorStart: \t%d\n", data_superbloco->pFATSectorStart);

	data_superbloco->RootDirCluster = (bufferRead[24]+bufferRead[25]*256+bufferRead[26]*65536+bufferRead[27]*16777216);
	printf ("RootDirCluster: \t%d\n", data_superbloco->RootDirCluster);

	data_superbloco->DataSectorStart = (bufferRead[28]+bufferRead[29]*256+bufferRead[30]*65536+bufferRead[31]*16777216);
	printf ("DataSectorStart: \t%d\n", data_superbloco->DataSectorStart);
	
	/********Fim da leitura do Superbloco*/
	
	/********Lendo a FAT*********/

	printf("\n->->->Readind FAT Data<-<-<-\n");
	
	unsigned int Size_of_Element = 4;  					//according to specification (in bytes) correspond to 1 cluster
	unsigned int FATsize = data_superbloco->DataSectorStart - data_superbloco->pFATSectorStart; 		//number of logic sectors
	unsigned int NumberOfElements = (FATsize*SECTOR_SIZE)/Size_of_Element; 	//notice that 1 cluster has 4 logic sectors
	
	printf("\n->Number of FAT's logic sectors: %d\n", FATsize);
	printf("\n->Size of an element in array: %d\n", Size_of_Element);
	printf("\n->Number of FAT's array elements: %d\n", NumberOfElements);

	unsigned char FATbuffer[SECTOR_SIZE];		
	unsigned int  FATarray[NumberOfElements];
	unsigned int  currentElement = 0, currentSectorPosition = 0, i = 0; //i => mask to currentSectorPosition
	
	printf("\n\n");
	for(unsigned int currentSector = data_superbloco->pFATSectorStart; currentSector <= FATsize; currentSector++){
	
		if(read_sector(currentSector, FATbuffer) != 0){
			printf("Erro ao ler setor da fat!\n");
			return ERROR;
		}

		//storing and printing in array 
		while (currentSectorPosition < SECTOR_SIZE){
			
			i = currentSectorPosition;

			FATarray[currentElement] = (FATbuffer[i]+FATbuffer[i++]*256+FATbuffer[i++]*65536+FATbuffer[i++]*16777216);

			printf("%.4x[%.4x]\t", FATarray[currentElement], currentElement);	//printing in hexa decimal
			currentElement++;currentSectorPosition = i; currentSectorPosition++;

		}currentSectorPosition = 0;
	}
	
	//Remind that array's elements are unsigned int and that its size is 4 bytes 
	printf("\n\n->Array size = %d bytes or %d elements\n", sizeof(FATarray), (sizeof(FATarray)/Size_of_Element));	
	
	/*********Fim da leitura da FAT*********/
}


/*
FILE create2(char *filename){
	if(primeira_vez){
			if(init() != 0)
		
	}
	struct t2fs_record record = {0};
	record.TypeVal = 0x01;
	memcpy(record.name, name_parsed, strlen(nome_parsed)*sizeof(char));
	record.blocksFileSize = 0;
	record.bytesFileSize = 0;
}*/






int identify2 (char *name, int size){
	

	  strcpy(name, "Geronimo Veit\t260004\nJulia Rittmann\t262512\nVilmar Fonseca\t262519\n");

	  if (sizeof(name)<0 || sizeof(name)>size)
	    return -1;
	  else
	    return 1;
}
