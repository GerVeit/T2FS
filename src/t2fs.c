#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/apidisk.h"
#include "../include/t2fs.h"

#define ERROR -1
#define SUCCESS 0
#define SECTOR_SIZE 256
#define SIZE_OF_INT 4
#define Size_of_Element 4
#define NAME_SIZE 55
#define RECORD_SIZE 64
#define DIRMAX 16
#define TYPEVAL_INVALIDO    0x00
#define TYPEVAL_REGULAR     0x01
#define TYPEVAL_DIRETORIO   0x02


int firstexec = 0;
int sizeOfPath = 0;
unsigned int FATarray_global[8192];	//Array representing FAT elements
char parsedPath[DIRMAX][NAME_SIZE];	//Matrix to represnt parsed filename

struct t2fs_superbloco *data_superbloco;//SuperBlock's struct 
struct t2fs_record list_of_records[DIRMAX]; //Max number of directories in a cluster


/*Initialization function to read superblock and FAT */

int init(){

	if (read_superbloco() == SUCCESS && read_FAT() == SUCCESS)
		return SUCCESS;
	else
		return ERROR;
	
}

//Reads superblock's info
int read_superbloco(){

	data_superbloco = (struct t2fs_superbloco*)malloc(sizeof(struct t2fs_superbloco));

	unsigned char bufferRead[SECTOR_SIZE];
	unsigned int superblock_sector = 0x00000000;

	if (read_sector(superblock_sector, bufferRead) != 0){
		printf("Error with reading sector\n");
		return ERROR;
	}
	
	printf("\n\nDissected inforamation:\n\n");

	char id[SIZE_OF_INT+1];

	printf("id: \t\t\t");
	for(int loop_control=0; loop_control<SIZE_OF_INT; loop_control++){
		id[loop_control] = (char)bufferRead[loop_control];
		printf("%c", id[loop_control]);
	}
	strcpy(data_superbloco->id, id);
		
	/*Calculates by little endian format*/	
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

}

int read_FAT(){

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

	//Populating FAT's global array
	for(cont=0; cont<=NumberOfElements; cont++){
		FATarray_global[cont] = FATarray[cont];
	}

	return SUCCESS;
	
}

//Breaks filename in multiple strings separeted by '/'
void breakIn(char *name){
	
	const char s[2] = "/";
	char *token;

	token = strtok(name, s);

	while (token != NULL){

		strcpy(parsedPath[sizeOfPath], token);
		puts(parsedPath[sizeOfPath]);

		sizeOfPath++;
		token = strtok(NULL, s);
	}

	printf("size = %d", sizeOfPath);
	printf("\n\tNULL reached!!\n");

	sizeOfPath = 0;
}

/*
	Writes a record in a directory cluster
	Calculates first Logic Sector by its cluster
*/

int writeRecord(char *filename, unsigned int cluster){
	
	unsigned int firstSector;
	unsigned char buffer[SECTOR_SIZE];

	int ac=0;	//acumulator	
	int cont=0;

	firstSector = data_superbloco->DataSectorStart + ((cluster - 1)*4);

	if(read_sector(firstSector, buffer) != 0) 
		return ERROR;	

	//verify if there is 64 consecutive bytes to write a record	
	else{	
		while(buffer[cont] == 0){
			ac++;
		}	
		//if(ac >= RECORD_SIZE)
			//write2 		
	}
}

/*
	Does all the file's path since root directory by comparing 
	the directories's names and finding their first logic sector 
	by their cluster till reach the directory father

	Example of execution: filename is /sisop/trab2/t2fs.c

	1) Find root (cluster 2) and read it to a buffer (remind that one cluster correspond to 4 logic sectors)
	2) This buffer contains the records of all files (regular OR directory) 
	3) Convert the bytes read into record's struct type
	4) Compare all records names (string type) with "sisop"
	5) When found, gets record's cluster (Ex. 1265)
	6) Now we have to read this cluster and repeat the process till we find "trab2"
	7) When in directory father, include a record with "t2fs.c" infos by allocation
*/

/* Not tested yet.
void handleAbs(char *filename){
	
	struct t2fs_record *newRecord;
	char parents[sizeOfPath][NAME_SIZE];

	list_of_records = (struct t2fs_record*)malloc(sizeof(struct t2fs_record)*16);
	newRecord = (struct t2fs_record*)malloc(sizeof(struct t2fs_record));

	unsigned char buffer[4*SECTOR_SIZE];
	unsigned int initCluster;

	int cont=0, i=0;	//iterators

	while(parsedPath[i] != NULL || i==0){			//i = 0 indicates that it's root directory
		
		if(i == 0){
			buffer = readCluster(data_superbloco->RootDirCluster);
			list_of_records = convertToRecord(buffer);	i--;
		}
		else{
			buffer = readCluster(list_of_records[initCluster]->firstCluster);
			list_of_records = convertToRecord(buffer);
		}

		while(strcmp(list_of_records[cont]->name, parsedPath[i]) != 0)
			cont++;
		
		initCuster = list_of_records[cont]->firstCluster;	i++;
	}

	if (writeRecord(filename, initCluster) != 0){
		printf("Error in write record\n");break;
	}
}
*/

/*
void handleRel(){	//Pending.
}
*/

//Gets relative or absolute filename
void getType(char *name){

	breakIn(name);

	if(name[0] == '/'){
		printf("is absolute\n\n");
		//handleAbs(name); //Pending
	}

	else if(name != NULL && sizeOfPath !=0){
		printf("is relative\n\n");
		//handleRel(name); //Pending.
	}
	else
		printf("\n\n->->Given name is NULL\n\n");
}

int write_FAT(){
	unsigned int currentElement = 0, currentSectorPosition = 0, currentSector = 0, i = 0;
	unsigned char FATbuffer[SECTOR_SIZE];

	for(currentSector = 1; currentSector <=128; currentSector++){
		currentSectorPosition = 0;
		while(currentSectorPosition < SECTOR_SIZE){
			i = currentSectorPosition;
			//memcpy(FATbuffer[currentElement], FATarray_global[i], 4);
			FATbuffer[currentElement] = FATarray_global[i];//(FATarray_global[i++]/256)-(FATarray_global[i++]/65536))
			currentElement++;
			currentSectorPosition++;
		}
		if(write_sector(currentSector, FATbuffer) != 0){
			printf("Error in write FAT!\n");
			return ERROR;
		}
	}
	return SUCCESS;
}

//Finds a free entry ins FAT's array
int getFreeEntry(){
	int i;
	for(i=0; i<8192; i++){
		if(FATarray_global[i]==0){
			printf("Finded blank sapce\n");
			return i;
		}
	}
	return ERROR;
}

//Completes with /0 unusable name's bytes 
void completeFileName(char *name){
	int i=0, cont;   		//controllers to fill 55 chars
	char superName[NAME_SIZE];

	while(name[i] != '\0'){
		superName[i] = name[i];i++;
	}
	for (cont = i; cont<NAME_SIZE; cont++)
		superName[cont] = '\0';

	strcpy(name, superName);
}

FILE2 create2(char *filename){

	if(firstexec == 0){
		if(init() == ERROR){
			return ERROR;
		}
		firstexec = 1;
	}

	struct t2fs_record record;

 	int free_entry = getFreeEntry();
	FATarray_global[free_entry] = 0xffffffff;

	//for(int cont=0; cont<8192; cont++){
	//	printf("*%.4x[%.4x]*\t", FATarray_global[cont], cont);
	//}

	/*Missing filename parse.
	
	completeFileName(filename);
	getType(filename);	
	record.TypeVal = TYPEVAL_REGULAR;
	strcpy(record.name, filename);
	record.bytesFileSize = 0;
	record.firstCluster = free_entry;
	*/

	write_FAT();
}


int write2(FILE2 handle, char *buffer, int size){
    unsigned int CLUSTER_SIZE = 1024;
    unsigned int i, q, pontBuffer=0, numClusters;
    unsigned int numSetores, setorCluster;
    unsigned char bufferaux[CLUSTER_SIZE+1];
    
    setorCluster= 131+(handle-1)*4;

    if(size>CLUSTER_SIZE){
        printf("\nNAO CABE EM UM SO CLUSTER\n");
        
        numClusters=size/CLUSTER_SIZE;      //CONTA NUMERO DE CLUSTERS NECESSARIOS

        if(size%CLUSTER_SIZE!=0)
            numClusters++;
        
        //buscar cluster livre
                            
        numSetores=size/SECTOR_SIZE;        //CONTA NUMERO TOTAL DE SETORES

        if(size%SECTOR_SIZE != 0)
            numSetores++;



        for(i=0; i<numClusters; i++){       //PERCORRE CLUSTER A CLUSTER

            pontBuffer=pontBuffer*i;
            
            for(q=0; q<CLUSTER_SIZE;q++){   //COPIA 1 CLUSTER
                bufferaux[q]=buffer[pontBuffer];
                pontBuffer++;
            }
            bufferaux[q]=NULL;

        //************ESCREVE NO DISCO***********
            if(i<numClusters-1)
                escreveCluster(4, bufferaux, setorCluster+(4*i));

            else{
                numSetores=numSetores%4;
                if(numSetores==0)
                    numSetores=4;

                escreveCluster(numSetores, bufferaux, setorCluster+(4*i));
            }
        }
    }

    else{
        printf("\nCABE EM UM CLUTER APENAS\n");
        numSetores=size/SECTOR_SIZE;            //CONTA NUMERO DE SETORES
        
            if(size%SECTOR_SIZE != 0)
                numSetores++;

        escreveCluster(numSetores, buffer, setorCluster);

    }
    
}

int escreveCluster(int numSetores, char *buffer, unsigned int setorCluster){
        int CLUSTER_SIZE=1024;
        unsigned int pontBuffer=0;
        int q=0, i,tl;
        unsigned char bufferaux[SECTOR_SIZE], leitura[SECTOR_SIZE];
                

        for(q=0;q<numSetores;q++){          //VARRE SETOR A SETOR
            
            pontBuffer=q*(SECTOR_SIZE-1);

            for(i=0; i<SECTOR_SIZE-1||buffer[pontBuffer]==NULL; i++){       //COPIA 1 SETOR PARA ESCREVER
                bufferaux[i]=buffer[pontBuffer];
                pontBuffer++;           
            }

            bufferaux[i]=NULL;

            write_sector(setorCluster+q, bufferaux);

            
            strcpy(bufferaux,"");           
        }               
}

int identify2 (char *name, int size){
	  strcpy(name, "Geronimo Veit\t260004\nJulia Rittmann\t262512\nVilmar Fonseca\t262519\n");

	  if (sizeof(name)<0 || sizeof(name)>size)
	    return -1;
	  else
	    return 1;
}
