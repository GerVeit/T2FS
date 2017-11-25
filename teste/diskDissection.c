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
		
	printf("id: \t\t\t%s\n", id); //ta meio zoada mas eh nois

	unsigned int version = bufferRead[5]*256+bufferRead[4];
	printf("Version: \t\t0x%.4x\t(0x7e1 = 2017; 2 = 2 Semestre)\n", version);

	unsigned int SuperBlockSize = (bufferRead[6]*256+bufferRead[7])/256;
	printf ("SuperBlockSize: \t%d Setores Logicos\n", SuperBlockSize);

	unsigned int DiskSize = (bufferRead[8]+bufferRead[9]*256+bufferRead[10]*65536+bufferRead[11]*16777216);
	printf ("DiskSize: \t\t%d bytes\n", DiskSize);

	unsigned int NofSectors = (bufferRead[12]+bufferRead[13]*256+bufferRead[14]*65536+bufferRead[15]*16777216);
	printf ("NofSectors: \t\t%d Setores Logicos\n", NofSectors);

	unsigned int SectorsPerCluster = (bufferRead[16]+bufferRead[17]*256+bufferRead[18]*65536+bufferRead[19]*16777216);
	printf ("SectorsPerCluster: \t%d Setores Logicos\n", SectorsPerCluster);
	
	unsigned int pFATSectorStart = (bufferRead[20]+bufferRead[21]*256+bufferRead[22]*65536+bufferRead[23]*16777216);
	printf ("pFATSectorStart: \t%d\n", pFATSectorStart);

	unsigned int RootDirCluster = (bufferRead[24]+bufferRead[25]*256+bufferRead[26]*65536+bufferRead[27]*16777216);
	printf ("RootDirCluster: \t%d\n", RootDirCluster);

	unsigned int DataSectorStart = (bufferRead[28]+bufferRead[29]*256+bufferRead[30]*65536+bufferRead[31]*16777216);
	printf ("DataSectorStart: \t%d\n", DataSectorStart);

	//Reading and storing FAT as array in Memory
	
	printf("\n->->->Readind FAT Data<-<-<-\n");

	unsigned int Size_of_Element = 4;  //according to specification (in bytes) 
	unsigned int FATsize = DataSectorStart - pFATSectorStart;
	unsigned int NumberOfElements = (FATsize*SECTOR_SIZE)/Size_of_Element;

	printf("\n->Number of FAT's logic sectors: %d\n", FATsize);
	printf("\n->Size of an element in array: %d\n", Size_of_Element);
	printf("\n->Number of FAT's array elements: %d\n", NumberOfElements);

	unsigned char FATbuffer[SECTOR_SIZE];		
	unsigned int  FATarray[NumberOfElements];

	unsigned int  currentElement = 0, currentSectorPosition = 0, i = 0; //i => mask to currentSectorPosition
	
	for(unsigned int currentSector = pFATSectorStart; currentSector <= FATsize; currentSector++){
	
		if(read_sector(currentSector, FATbuffer) != 0){
			printf("Erro ao ler setor da fat!\n");
			return ERROR;
		}

		//storing in array 
		while (currentSectorPosition < SECTOR_SIZE){
			
			i = currentSectorPosition;

			FATarray[currentElement] = (FATbuffer[i]+FATbuffer[i++]*256+FATbuffer[i++]*65536+FATbuffer[i++]*16777216);
			currentElement++;currentSectorPosition = i; currentSectorPosition++;

		}currentSectorPosition = 0;
	}
	
	//printing FAT	
	printf("\n->Size of FAT array = %d elements of 4 bytes\n\n", (sizeof(FATarray)/Size_of_Element)); 
								//must be equal to 8192, same number of clusters

	for(unsigned int FATcounter = 0; FATcounter<(sizeof(FATarray)/Size_of_Element); FATcounter++)
		printf("\t%u[%d]", FATarray[FATcounter], FATcounter);

	printf("\n\n->->->End of FAT<-<-<-\n");


	return SUCCESS;
}
