#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/apidisk.h"
#include "../include/t2fs.h"

#define ERROR -1
#define SUCCESS 0
#define SIZE_OF_INT 4
#define Size_of_Element 4
#define TYPEVAL_INVALIDO    0x00
#define TYPEVAL_REGULAR     0x01
#define TYPEVAL_DIRETORIO   0x02

int firstexec = 0;
unsigned int FATarray_global[8192];//Array com as informacoes lidas da FAT
struct t2fs_superbloco *data_superbloco;//Struct para informacoes lidas do superbloco

/*Funcao de inicializacao e leitura dos dados, populando a struct do superbloco e o array da FAT*/
int init(){
	
	printf("Esta merda de trabalho (init)\n");
	read_superbloco();
	read_FAT();
	return SUCCESS;
}

int read_FAT(){
	
	/********Lendo a FAT*********/

	printf("\n->->->Readind FAT Data<-<-<-\n");

	unsigned int FATsize = data_superbloco->DataSectorStart - data_superbloco->pFATSectorStart; //number of logic sectors
	unsigned int NumberOfElements = (FATsize*SECTOR_SIZE)/Size_of_Element; 	//notice that 1 cluster has 4 logic sectors	
	unsigned int currentElement = 0, currentSectorPosition = 0, i = 0; //i => mask to currentSectorPosition
	unsigned int FATarray[NumberOfElements];
	unsigned char FATbuffer[SECTOR_SIZE];
	int cont;
	
	printf("\n->Number of FAT's logic sectors: %d\n", FATsize);
	printf("\n->Size of an element in array: %d\n", Size_of_Element);
	printf("\n->Number of FAT's array elements: %d\n", NumberOfElements);
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
	
	//Populando u array global com as informacoes da FAT
	for(cont=0; cont<=NumberOfElements; cont++){
		FATarray_global[cont] = FATarray[cont];
	}

	return SUCCESS;
	/*********Fim da leitura da FAT*********/
}

int read_superbloco(){

	/**********Lendo o Superbloco**********/
	
	data_superbloco = (struct t2fs_superbloco*)malloc(sizeof(struct t2fs_superbloco));
	unsigned char bufferRead[SECTOR_SIZE];
	unsigned int superblock_sector = 0x00000000;
	
	if (read_sector(superblock_sector, bufferRead) != 0){
		printf("Erro ao ler setor\n");
		return ERROR;
	}

	//for(int buff_cont = 0; buff_cont<sizeof(bufferRead); buff_cont++)
	//	printf("buffer[%d] = %u\n",buff_cont, (unsigned char)bufferRead[buff_cont]); 	
	
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

	return SUCCESS;

	/********Fim da leitura do Superbloco*/
}

int getFreeEntry(){
	int i;
	for(i=0; i<8192; i++){
		if(FATarray_global[i]==0){
			printf("Achou espaco livre \n");
			return i;
		}
	}
	return ERROR;
}

int ConvertToRelative(char *pathname, char *filename){

	return SUCCESS;
}

FILE2 create2(char *filename){
	if(firstexec == 0){
		if(init() == ERROR){
			return ERROR;
		}
		firstexec = 1;
	}

	struct t2fs_record record;
	struct t2fs_record record_pai;
	//char* pathname = (char*) malloc(sizeof(strlen(filename)));

	//ConvertToRelative();

 	int free_entry = getFreeEntry();
	FATarray_global[free_entry] = 0xffffffff;

	//record.TypeVal = TYPEVAL_REGULAR;
	//memcpy(record.name, pathname, strlen(pathname)*sizeof(char));
	//record.bytesFileSize = 0;
	//record.firstCluster = free_entry;
}


int identify2 (char *name, int size){
	  strcpy(name, "Geronimo Veit\t260004\nJulia Rittmann\t262512\nVilmar Fonseca\t262519\n");

	  if (sizeof(name)<0 || sizeof(name)>size)
	    return -1;
	  else
	    return 1;
}
