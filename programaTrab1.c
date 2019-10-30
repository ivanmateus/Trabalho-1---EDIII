//Felipe Tiago de Carli - 10525686 
//Gabriel de Andrade Dezan - 10525706
//Ivan Mateus de Lima Azevedo - 10525602

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h> 
#include "./src/escreverNaTela.h"
#include "./src/cityList.h"

//-----Constants for header register
#define STATUSSIZE 1 * sizeof(char)
#define VERTSIZE 1 * sizeof(int)
#define EDGESIZE 1 * sizeof(int)
#define LASTCOMPSIZE 10 * sizeof(char)
#define HREGSIZE 19

//-----Constants for data register
#define ORIGINSIZE 2 * sizeof(char)
#define DESTSIZE 2 * sizeof(char)
#define DISTANCESIZE 1 * sizeof(int)
#define DREGSIZE 85
#define VARSIZE (DREGSIZE - ORIGINSIZE - DESTSIZE - DISTANCESIZE)

//-----Data structure for header register
typedef struct {
  char status;
  int numeroVertices;
  int numeroArestas;
  char dataUltimaCompactacao[10];
} headerReg;

//-----Data structure for data register
typedef struct {
  char estadoOrigem[2];
  char estadoDestino[2];
  int distancia;
  char *cidadeOrigem;
  char *cidadeDestino;
  char *tempoViagem;
} dataReg;

//Read the .csv file and 
//build the .bin with the retrieved data
void csv_to_bin(char *fInp, char *fOut){
	FILE *fpInp = fopen(fInp, "r");
	FILE *fpOut = fopen(fOut, "wb");
	if (!fpInp || !fpOut) {
		printf("Falha no carregamento do arquivo.\n");
		return;
	}

	//Allocate memory for the header register, set
	//its initial values:
	//status: '0' (the file has been opened for writing)
	//numeroVertices: 0 (the file doesn't have any vertices; i.e. cities)
	//numeroArestas: 0 (the file doesn't have any edges; i.e. data about two cities)
	//dataUltimaCompactacao: "00/00/0000" (the file has been first opened)
	//and write them in the beginning of the file
	headerReg *hReg = (headerReg *)malloc(HREGSIZE);
	hReg->status = '0';
	fwrite(&(hReg->status),STATUSSIZE,1,fpOut);
	hReg->numeroVertices = 0;
	fwrite(&(hReg->numeroVertices),VERTSIZE,1,fpOut);
	hReg->numeroArestas = 0;
	fwrite(&(hReg->numeroArestas),EDGESIZE,1,fpOut);
	strcpy(hReg->dataUltimaCompactacao,"00/00/0000");
	fwrite(hReg->dataUltimaCompactacao,sizeof(char),LASTCOMPSIZE,fpOut);

	//Create the list for the cities in the file, where
	//the variable 'len' stores the size of the list
	int len = 0;
	cityList *cities = (cityList *)malloc(sizeof(cityList));
	*cities = NULL;

	//Create an auxiliary node that will temporarily
	//store the data read from the .csv file
	dataReg *dReg = (dataReg *)malloc(DREGSIZE);

	//Create an array that will read the lines in
	//the .csv file that contain the data
	char buf[DREGSIZE];

	//Create a variable to count the number of registers
	//(or edges) in the file
	int row_count = 0;

	//While there are lines to be read, continue
	while(fgets(buf, DREGSIZE, fpInp)) {
	    	//If it's the first line, it means that it is
		//the line with the name of the fields, so
		//it is skipped
		if (row_count == 0) {
	     		row_count++;
	      		continue;
	    	}

		//Get the first token (the value for the field "estadoOrigem")
		char *field = strtok(buf,",");
		strcpy(dReg->estadoOrigem,field);

		//Get the next token (the value for the field "estadoDestino")
		field = strtok(NULL,",");
		strcpy(dReg->estadoDestino,field);
		
		//Get the next token (the value for the field "estadoDestino")
		field = strtok(NULL,",");
		dReg->distancia = atoi(field);

    		//Get the next token (the value for the field "distancia")
		field = strtok(NULL,",");
		dReg->cidadeOrigem = field;
		
		//Check if this city (or vertice) is already in the list
		if(check_for_city(dReg->cidadeOrigem,cities)){
			++len;
		}

		//Get the next token (the value for the field "cidadeOrigem")
		field = strtok(NULL,",");
		dReg->cidadeDestino = field;

		//Check if this city (or vertice) is already in the list
		if(check_for_city(dReg->cidadeDestino,cities)){
			++len;
		}

		//Get the next token (the value for the field "cidadeDestino")
		field = strtok(NULL,",");
		trim(field);
		dReg->tempoViagem = field;
	
		//Write the data in the .bin file also with the delimiters
		//for the variable size fields
		fwrite(dReg->estadoOrigem,ORIGINSIZE,1,fpOut);
		fwrite(dReg->estadoDestino,DESTSIZE,1,fpOut);
		fwrite(&(dReg->distancia),DISTANCESIZE,1,fpOut);
		fwrite(dReg->cidadeOrigem,sizeof(char),strlen(dReg->cidadeOrigem),fpOut);
		fwrite("|",sizeof(char),1,fpOut);
		fwrite(dReg->cidadeDestino,sizeof(char),strlen(dReg->cidadeDestino),fpOut);
		fwrite("|",sizeof(char),1,fpOut);
		fwrite(dReg->tempoViagem,sizeof(char),strlen(dReg->tempoViagem),fpOut);
   		fwrite("|",sizeof(char),1,fpOut);
		
		//Calculate the amount of space occupied inside the register
		//and complete the register with the garbage character '#'
		//until its end
	 	int partRegSize = ORIGINSIZE + DESTSIZE + DISTANCESIZE + sizeof(char) * strlen(dReg->cidadeOrigem) + sizeof(char) * strlen(dReg->cidadeDestino) + sizeof(char) * strlen(dReg->tempoViagem) + 3 * sizeof(char);
		while(partRegSize < DREGSIZE) {
			fwrite("#",sizeof(char),1,fpOut);
			partRegSize += sizeof(char);
		}
		row_count++;
	}

	//Then, when all the registers have been written,
	//return to the beginning of the file to overwrite
	//the new data of the header register, which are:
	//status: '1' (the file is consistent)
	//numeroVertices: len (the number of new cities)
	//numeroArestas: row_count - 1 (the number of registers minus the first line skipped)
	//dataUltimaCompactacao: "##########" (the file hasn't been compressed yet)
	fseek(fpOut,0,SEEK_SET);
	hReg->status = '1';
	fwrite(&(hReg->status),STATUSSIZE,1,fpOut);
	hReg->numeroVertices = len;
	fwrite(&(hReg->numeroVertices),VERTSIZE,1,fpOut);
	hReg->numeroArestas = row_count - 1;
	fwrite(&(hReg->numeroArestas),EDGESIZE,1,fpOut);
	strcpy(hReg->dataUltimaCompactacao,"##########");
	fwrite(hReg->dataUltimaCompactacao,sizeof(char),LASTCOMPSIZE,fpOut);

	fclose(fpOut);
	fclose(fpInp);
}

//Put a null terminator character in the end of the
//string in order to properly compare it with another
void remove_garbage(char *field, int whichField){
	//If the flag is equal to 0, it means that
	//it is the "estadoOrigem" field
	//Else, it is the "estadoDestino" field
	if(whichField == 0){
		field[ORIGINSIZE] = '\0';
	} else if(whichField == 1){
		field[DESTSIZE] = '\0';
	}
}

//Read a .bin file and display its data correctly (without the garbage)
void read_bin(char *fName){
	FILE *fp = fopen(fName, "rb");
	if (!fp) {
  		printf("Falha no processamento do arquivo.\n");
   		return;
  	}

	//Skip the header register
	fseek(fp,HREGSIZE,SEEK_SET);

	//Create an auxiliary register to temporarily store the data
	dataReg *reg = (dataReg *)malloc(DREGSIZE);
	
	//Create a buffer to read the data of the variable
	//part of the register (i.e. the fields "cidadeOrigem",
	//"cidadeDestino" and "tempoViagem")
	char buf[VARSIZE];

	//Variable to store the RRN of the current register
	int rrn = 0;

	//While there are registers to be read
	while(fread(reg->estadoOrigem,ORIGINSIZE,1,fp)) {
		//If the register hasn't been removed (situation
		//where the first character of the register is '*'),
		//then read print its data
		//Else, calculate the next RRN (++rrn) and
		//go to its memory position using fseek
		if(reg->estadoOrigem[0] != '*'){
			//Read and print the fixed size fields
			printf("%d ",rrn);

			printf("%c%c ",reg->estadoOrigem[0],reg->estadoOrigem[1]);
			
			fread(reg->estadoDestino,DESTSIZE,1,fp);
			printf("%c%c ",reg->estadoDestino[0],reg->estadoDestino[1]);
			
			fread(&(reg->distancia),DISTANCESIZE,1,fp);
			printf("%d ",reg->distancia);
			
			//Read the rest of the register
			fread(buf,VARSIZE,1,fp);

			//Separate the fields using the "|" delimiter
			//then print
			char *bufPtr = buf;
			char *field = strsep(&bufPtr,"|");
			reg->cidadeOrigem = field;
			printf("%s ",reg->cidadeOrigem);
			
			field = strsep(&bufPtr,"|");
			reg->cidadeDestino = field;
			printf("%s ",reg->cidadeDestino);
			
			field = strsep(&bufPtr,"|");
			reg->tempoViagem = field;
			printf("%s\n",reg->tempoViagem);
			
			//Go to the next RRN
			++rrn;
		} else {
			++rrn;
			fseek(fp,rrn*DREGSIZE-ORIGINSIZE,SEEK_CUR);
		}
	}

	//If the RRN is still zero, it means that there aren't any registers
	if(rrn == 0){
		printf("Registro inexistente.\n");
	}

	fclose(fp);
}

//Search for registers that satisfy a given search criterion
//which is a field and its value
void search_by_field(char *fName,char *searchField,char *value){
	FILE *fp = fopen(fName, "rb");
	if (!fp) {
		printf("Falha no processamento do arquivo.\n");
    		return;
  	}

	//Skip the header register
	fseek(fp,HREGSIZE,SEEK_SET);

	//Create an auxiliary register to temporarily store the data	
	dataReg *reg = (dataReg *)malloc(DREGSIZE);

	//Create a buffer to temporarily store the variable size data
	char buf[VARSIZE];

	//Variable to store the RRN of the current register
	int rrn = 0;

	//Flag to indicate that a register satisfy the given condition,
	//so it must be printed
	int print = 0;

	//Variable to count the number of printed registers
	int printedNum = 0;

	//While there are registers to be read
	while(fread(reg->estadoOrigem,ORIGINSIZE,1,fp)) {
		//If the register hasn't been removed, continue
		//Else, calculate the next RRN (++rrn) and
		//go to the corresponding register using fseek
		if(reg->estadoOrigem[0] != '*'){
			//Put the '\0' at the end of the string to compare it
			//with the given value

			//***Obs.: this is a temporary measure. When the register
			//is created, since it is a struct, the fields are in
			//contiguous positions of memory, which means that right after
			//the two bytes of "estadoOrigem", there are the two bytes
			//of "estadoDestino". So, when we put the '\0' at the end of
			//the string, actually we are putting it in the first
			//byte of "estadoDestino". Then the comparison is made and
			//when the value for "estadoDestino" is read, the '\0' is overwritten
			//by the first byte of "estadoDestino". So this is done only
			//to properly compare the strings.***
			remove_garbage(reg->estadoOrigem,0);

			//If the condition is matched, set the flag 'print'
			if(!strcmp(searchField,"estadoOrigem") && !strcmp(reg->estadoOrigem,value)){
				print = 1;
			}
		
			fread(reg->estadoDestino,DESTSIZE,1,fp);
			
			//The same logic described above can be applied here. Except that
			//instead of "estadoOrigem", we have "estadoDestino" and instead
			//of "estadoDestino", we have "distancia".
			remove_garbage(reg->estadoDestino,1);

			//If the condition is matched, set the flag 'print'
			if(!strcmp(searchField,"estadoDestino") && !strcmp(reg->estadoDestino,value)){
				print = 1;
			}
		
			fread(&(reg->distancia),DISTANCESIZE,1,fp);

			//If the condition is matched, set the flag 'print'
			if(!strcmp(searchField,"distancia") && reg->distancia == atoi(value)){
				print = 1;
			}
		
			fread(buf,VARSIZE,1,fp);

			char *bufPtr = buf;
			char *field = strsep(&bufPtr,"|");
			reg->cidadeOrigem = field;

			//If the condition is matched, set the flag 'print'
			if(!strcmp(searchField,"cidadeOrigem") && !strcmp(reg->cidadeOrigem,value)){
				print = 1;
			}
		
			field = strsep(&bufPtr,"|");
			reg->cidadeDestino = field;
			
			//If the condition is matched, set the flag 'print'
			if(!strcmp(searchField,"cidadeDestino") && !strcmp(reg->cidadeDestino,value)){
				print = 1;
			}
			
			field = strsep(&bufPtr,"|");
			reg->tempoViagem = field;
		
			//If the condition is matched, set the flag 'print'
			if(!strcmp(searchField,"tempoViagem") && !strcmp(reg->tempoViagem,value)){
				print = 1;
			}
			
			//If the flag is set, print the register
			if(print){
				printf("%d ",rrn);
				printf("%c%c ",reg->estadoOrigem[0],reg->estadoOrigem[1]);
				printf("%c%c ",reg->estadoDestino[0],reg->estadoDestino[1]);
				printf("%d ",reg->distancia);
				printf("%s ",reg->cidadeOrigem);
				printf("%s ",reg->cidadeDestino);
				printf("%s\n",reg->tempoViagem);
			
				//Clear the flag, so that it can be used again
				print = 0;
				
				//Increase the number of printed registers
				++printedNum;
			}
			++rrn;
		} else {
			++rrn;
			fseek(fp,rrn*DREGSIZE-ORIGINSIZE,SEEK_CUR);
		}
	}

	//If no register was printed, there aren't any matches to the conditions
	if(printedNum == 0){
		printf("Registro inexistente.\n");
	}
  
	fclose(fp);
}

//Search a register by its RRN
void search_by_rrn(char *fName,int rrn) {
	FILE *fp = fopen(fName, "rb");

	if (!fp) {
		printf("Falha no processamento do arquivo.\n");
		return;
	}
	
	//Create an auxiliary register to temporarily store the data
	dataReg *reg = (dataReg *)malloc(DREGSIZE);

	//Create a buffer to store the variable size part of the register
	char buf[VARSIZE];

	//Skip the header register and go to the register
	fseek(fp,HREGSIZE+rrn*DREGSIZE,SEEK_SET);
	
	//If the register exists and hasn't been removed, print it
	//Else, print a warning message
	if(fread(reg->estadoOrigem,ORIGINSIZE,1,fp) && reg->estadoOrigem[0] != '*') {
		printf("%d ",rrn);

		printf("%c%c ",reg->estadoOrigem[0],reg->estadoOrigem[1]);
		
		fread(reg->estadoDestino,DESTSIZE,1,fp);
		printf("%c%c ",reg->estadoDestino[0],reg->estadoDestino[1]);
		
		fread(&(reg->distancia),DISTANCESIZE,1,fp);
		printf("%d ",reg->distancia);
		
		fread(buf,VARSIZE,1,fp);

		char *bufPtr = buf;
		char *field = strsep(&bufPtr,"|");
		reg->cidadeOrigem = field;
		printf("%s ",reg->cidadeOrigem);
		
		field = strsep(&bufPtr,"|");
		reg->cidadeDestino = field;
		printf("%s ",reg->cidadeDestino);
		
		field = strsep(&bufPtr,"|");
		reg->tempoViagem = field;
		printf("%s\n",reg->tempoViagem);

		++rrn;
	} else{
		printf("Registro inexistente.\n");
	}

	fclose(fp);
}

//Remove the registers that satisfy a given condition
void remove_register(char *fName,char *searchField,char *value){
	FILE *fp = fopen(fName, "rb+");
	if (!fp) {
		printf("Falha no processamento do arquivo.\n");
    		return;
  	}

	//Skip the header register
	fseek(fp,HREGSIZE,SEEK_SET);
	
	//Create a register to temporarily store the retrieved data
	dataReg *reg = (dataReg *)malloc(DREGSIZE);

	//Create a buffer to temporarily store the variable 
	//size part of the register
	char buf[VARSIZE];

	//Variable to store the RRN of the current register
	int rrn = 0;
	
	//Flag to determine if the current register has to be removed or not
	int remove = 0;

	//While there are registers to be read, continue
	while(fread(reg->estadoOrigem,ORIGINSIZE,1,fp)) {
		//Same logic applied to the 'search_by_field' function 
		remove_garbage(reg->estadoOrigem,0);

		//If the condition is matched, set the 'remove' flag
		if(!strcmp(searchField,"estadoOrigem") && !strcmp(reg->estadoOrigem,value)){
			remove = 1;
		}
		
		//Same logic applied to the 'search_by_field' function
		fread(reg->estadoDestino,DESTSIZE,1,fp);

		//If the condition is matched, set the 'remove' flag
		remove_garbage(reg->estadoDestino,1);
		if(!strcmp(searchField,"estadoDestino") && !strcmp(reg->estadoDestino,value)){
			remove = 1;
		}
		
		fread(&(reg->distancia),DISTANCESIZE,1,fp);
		
		//If the condition is matched, set the 'remove' flag
		if(!strcmp(searchField,"distancia") && reg->distancia == atoi(value)){
			remove = 1;
		}
		
		fread(buf,VARSIZE,1,fp);

		char *bufPtr = buf;
		char *field = strsep(&bufPtr,"|");
		reg->cidadeOrigem = field;
		
		//If the condition is matched, set the 'remove' flag
		if(!strcmp(searchField,"cidadeOrigem") && !strcmp(reg->cidadeOrigem,value)){
			remove = 1;
		}
		
		field = strsep(&bufPtr,"|");
		reg->cidadeDestino = field;
		
		//If the condition is matched, set the 'remove' flag
		if(!strcmp(searchField,"cidadeDestino") && !strcmp(reg->cidadeDestino,value)){
			remove = 1;
		}
		
		field = strsep(&bufPtr,"|");
		reg->tempoViagem = field;
		
		//If the condition is matched, set the 'remove' flag
		if(!strcmp(searchField,"tempoViagem") && !strcmp(reg->tempoViagem,value)){
			remove = 1;
		}
		
		//If the flag is set, remove the register using the
		//static method specified in the project document (i.e.
		//put a '*' in the first byte of the register)
		if(remove){
			//Go to the beginning of the register
			fseek(fp,HREGSIZE+rrn*DREGSIZE,SEEK_SET);

			//Write the '*' character
			fwrite("*",sizeof(char),1,fp);
		
			//Go to the next register
			fseek(fp,HREGSIZE+(rrn+1)*DREGSIZE,SEEK_SET);

			//Clear the flag to use it again
			remove = 0;
		}
		++rrn;
	}
  
	fclose(fp);
}

//Insert a new register in the .bin file
void insert_register(char *fName, char estadoOrigem[], char estadoDestino[], int distancia, char *cidadeOrigem, char *cidadeDestino, char *tempoViagem){
	FILE *fp = fopen(fName, "rb+");
 	if (!fp) {
    		printf("Falha no carregamento do arquivo.\n");
    		return;
  	}

	int rrn = 0;
	int len = 0;
	cityList *cities = (cityList *)malloc(sizeof(cityList));
	*cities = NULL;

 	dataReg *dReg = (dataReg *)malloc(DREGSIZE);
	dataReg *aux = (dataReg*)malloc(DREGSIZE);
	headerReg *hReg = (headerReg *)malloc(HREGSIZE);
	char buf[VARSIZE];
	int row_count = 0;

	hReg->status = '0';
	fwrite(&(hReg->status),STATUSSIZE,1,fp);
	fread(&(hReg->numeroVertices),VERTSIZE,1,fp);
	fread(&(hReg->numeroArestas),EDGESIZE,1,fp);
	fread(hReg->dataUltimaCompactacao,sizeof(char),LASTCOMPSIZE,fp);

    	strcpy(dReg->estadoOrigem,estadoOrigem);
	strcpy(dReg->estadoDestino,estadoDestino);
	dReg->distancia = distancia;
	dReg->cidadeOrigem = cidadeOrigem;
    	dReg->cidadeDestino = cidadeDestino;
    	dReg->tempoViagem = tempoViagem;

	while(fread(aux->estadoOrigem,ORIGINSIZE,1,fp)) {
		if(aux->estadoOrigem[0] != '*'){
			fread(aux->estadoDestino,DESTSIZE,1,fp);
			fread(&(aux->distancia),DISTANCESIZE,1,fp);
			
			fread(buf,VARSIZE,1,fp);

			char *bufPtr = buf;
			char *field = strsep(&bufPtr,"|");
			aux->cidadeOrigem = field;
			if(check_for_city(aux->cidadeOrigem,cities)){
				++len;
			}
			
			field = strsep(&bufPtr,"|");
			aux->cidadeDestino = field;
			if(check_for_city(aux->cidadeDestino,cities)){
				++len;
			}

			field = strsep(&bufPtr,"|");
			aux->tempoViagem = field;

			++row_count;
			++rrn;
		} else {
			++rrn;
			fseek(fp,HREGSIZE+rrn*DREGSIZE,SEEK_SET);
		}
	}
	
	if(check_for_city(dReg->cidadeOrigem,cities)){
		++len;
	}
	if(check_for_city(dReg->cidadeDestino,cities)){
		++len;
	}


    	fwrite(dReg->estadoOrigem,ORIGINSIZE,1,fp);
	fwrite(dReg->estadoDestino,DESTSIZE,1,fp);
	fwrite(&(dReg->distancia),DISTANCESIZE,1,fp);
	fwrite(dReg->cidadeOrigem,sizeof(char),strlen(dReg->cidadeOrigem),fp);
    	fwrite("|",sizeof(char),1,fp);
    	fwrite(dReg->cidadeDestino,sizeof(char),strlen(dReg->cidadeDestino),fp);
    	fwrite("|",sizeof(char),1,fp);
    	fwrite(dReg->tempoViagem,sizeof(char),strlen(dReg->tempoViagem),fp);
   	fwrite("|",sizeof(char),1,fp);
		
	int partRegSize = ORIGINSIZE + DESTSIZE + DISTANCESIZE + sizeof(char) * strlen(dReg->cidadeOrigem) + sizeof(char) * strlen(dReg->cidadeDestino) + sizeof(char) * strlen(dReg->tempoViagem) + 3 * sizeof(char);
	while(partRegSize < DREGSIZE) {
		fwrite("#",sizeof(char),1,fp);
		partRegSize += sizeof(char);
	}

	fseek(fp,0,SEEK_SET);
	hReg->status = '1';
	fwrite(&(hReg->status),STATUSSIZE,1,fp);
	hReg->numeroVertices = len;
	fwrite(&(hReg->numeroVertices),VERTSIZE,1,fp);
	hReg->numeroArestas = row_count + 1;
	fwrite(&(hReg->numeroArestas),EDGESIZE,1,fp);
	fwrite(hReg->dataUltimaCompactacao,sizeof(char),LASTCOMPSIZE,fp);

	fclose(fp);
}

void update_register(char *searchField, char *newValue, char *fName, int rrn){
	FILE *fp = fopen(fName, "rb+");

	if (!fp) {
		printf("Falha no processamento do arquivo.\n");
		return;
	}
	
	dataReg *reg = (dataReg *)malloc(DREGSIZE);
	char buf[VARSIZE];

	fseek(fp,HREGSIZE+rrn*DREGSIZE,SEEK_SET);
	
	if(!strcmp(searchField,"estadoOrigem")){
		strcpy(reg->estadoOrigem,newValue);
		fwrite(reg->estadoOrigem,ORIGINSIZE,1,fp);
  	fclose(fp);
		return;
	}
	if(!strcmp(searchField,"estadoDestino")){
		fseek(fp,ORIGINSIZE,SEEK_CUR);
		strcpy(reg->estadoDestino,newValue);
		fwrite(reg->estadoDestino,DESTSIZE,1,fp);
		fclose(fp);
		return;
	}	
	if(!strcmp(searchField,"distancia")){
		fseek(fp,ORIGINSIZE+DESTSIZE,SEEK_CUR);
		reg->distancia = atoi(newValue);
		fwrite(&(reg->distancia),DISTANCESIZE,1,fp);
		fclose(fp);
		return;
	}
	
	fseek(fp,ORIGINSIZE+DESTSIZE+DISTANCESIZE,SEEK_CUR);
	fread(buf,VARSIZE,1,fp);

	char *bufPtr = buf;
	char *field = strsep(&bufPtr,"|");
	reg->cidadeOrigem = field;
	
	field = strsep(&bufPtr,"|");
	reg->cidadeDestino = field;
		
	field = strsep(&bufPtr,"|");
	reg->tempoViagem = field;
	
	int oldPartRegSize = ORIGINSIZE + DESTSIZE + DISTANCESIZE + sizeof(char)*strlen(reg->cidadeOrigem) + sizeof(char)*strlen(reg->cidadeDestino) + sizeof(char)*strlen(reg->tempoViagem) + 3 * sizeof(char);

	fseek(fp,HREGSIZE+rrn*DREGSIZE+ORIGINSIZE+DESTSIZE+DISTANCESIZE,SEEK_SET);

	if(!strcmp(searchField,"cidadeOrigem")){
		reg->cidadeOrigem = newValue;
		fwrite(reg->cidadeOrigem,sizeof(char),strlen(reg->cidadeOrigem),fp);
		fwrite("|",sizeof(char),1,fp);
		fwrite(reg->cidadeDestino,sizeof(char),strlen(reg->cidadeDestino),fp);
		fwrite("|",sizeof(char),1,fp);
		fwrite(reg->tempoViagem,sizeof(char),strlen(reg->tempoViagem),fp);
		fwrite("|",sizeof(char),1,fp);
	}
	if(!strcmp(searchField,"cidadeDestino")){
		fseek(fp,(sizeof(char)*strlen(reg->cidadeOrigem))+sizeof(char),SEEK_CUR);
		reg->cidadeDestino = newValue;
		fwrite(reg->cidadeDestino,sizeof(char),strlen(reg->cidadeDestino),fp);
		fwrite("|",sizeof(char),1,fp);
		fwrite(reg->tempoViagem,sizeof(char),strlen(reg->tempoViagem),fp);
		fwrite("|",sizeof(char),1,fp);
	}
	if(!strcmp(searchField,"tempoViagem")){
		fseek(fp,(sizeof(char)*strlen(reg->cidadeOrigem))+sizeof(char)+(sizeof(char)*strlen(reg->cidadeDestino))+sizeof(char),SEEK_CUR);
		reg->tempoViagem = newValue;
		fwrite(reg->tempoViagem,sizeof(char),strlen(reg->tempoViagem),fp);
		fwrite("|",sizeof(char),1,fp);
	}
		
	int newPartRegSize = ORIGINSIZE + DESTSIZE + DISTANCESIZE + sizeof(char)*strlen(reg->cidadeOrigem) + sizeof(char)*strlen(reg->cidadeDestino) + sizeof(char)*strlen(reg->tempoViagem) + 3 * sizeof(char);
	while(newPartRegSize < oldPartRegSize && newPartRegSize < DREGSIZE) {
		fwrite("#",sizeof(char),1,fp);
		newPartRegSize += sizeof(char);
	}

	fclose(fp);
}

void compress_bin(char *fInp, char *fOut){
	FILE *fpInp = fopen(fInp, "r");
	FILE *fpOut = fopen(fOut, "wb");
	if (!fpInp || !fpOut) {
	printf("Falha no processamento do arquivo.\n");
	return;
	}

	headerReg *hReg = (headerReg *)malloc(HREGSIZE);
	hReg->status = '0';
	fwrite(&(hReg->status),STATUSSIZE,1,fpOut);
	hReg->numeroVertices = 0;
	fwrite(&(hReg->numeroVertices),VERTSIZE,1,fpOut);
	hReg->numeroArestas = 0;
	fwrite(&(hReg->numeroArestas),EDGESIZE,1,fpOut);
	strcpy(hReg->dataUltimaCompactacao,"00/00/0000");
	fwrite(hReg->dataUltimaCompactacao,sizeof(char),LASTCOMPSIZE,fpOut);

	int len = 0;
	cityList *cities = (cityList *)malloc(sizeof(cityList));
	*cities = NULL;

	fseek(fpInp,HREGSIZE,SEEK_SET);
	
	dataReg *reg = (dataReg *)malloc(DREGSIZE);
	char buf[VARSIZE];
	int row_count = 0;

	int rrn = 0;
	while(fread(reg->estadoOrigem,ORIGINSIZE,1,fpInp)) {
		if(reg->estadoOrigem[0] != '*'){
			fread(reg->estadoDestino,DESTSIZE,1,fpInp);
			fread(&(reg->distancia),DISTANCESIZE,1,fpInp);
			
			fread(buf,VARSIZE,1,fpInp);

			char *bufPtr = buf;
			char *field = strsep(&bufPtr,"|");
			reg->cidadeOrigem = field;
			if(check_for_city(reg->cidadeOrigem,cities)){
				++len;
			}
			
			field = strsep(&bufPtr,"|");
			reg->cidadeDestino = field;
			if(check_for_city(reg->cidadeOrigem,cities)){
				++len;
			}

			field = strsep(&bufPtr,"|");
			reg->tempoViagem = field;

			fwrite(reg->estadoOrigem,ORIGINSIZE,1,fpOut);
			fwrite(reg->estadoDestino,DESTSIZE,1,fpOut);
			fwrite(&(reg->distancia),DISTANCESIZE,1,fpOut);
			fwrite(reg->cidadeOrigem,sizeof(char),strlen(reg->cidadeOrigem),fpOut);
			fwrite("|",sizeof(char),1,fpOut);
			fwrite(reg->cidadeDestino,sizeof(char),strlen(reg->cidadeDestino),fpOut);
			fwrite("|",sizeof(char),1,fpOut);
			fwrite(reg->tempoViagem,sizeof(char),strlen(reg->tempoViagem),fpOut);
			fwrite("|",sizeof(char),1,fpOut);

			int partRegSize = ORIGINSIZE + DESTSIZE + DISTANCESIZE + sizeof(char) * strlen(reg->cidadeOrigem) + sizeof(char) * strlen(reg->cidadeDestino) + sizeof(char) * strlen(reg->tempoViagem) + 3 * sizeof(char);
			while(partRegSize < DREGSIZE) {
				fwrite("#",sizeof(char),1,fpOut);
				partRegSize += sizeof(char);
			}

			++row_count;
			++rrn;
		} else {
			++rrn;
			fseek(fpInp,HREGSIZE+rrn*DREGSIZE,SEEK_SET);
		}
	}

	fseek(fpOut,0,SEEK_SET);
	hReg->status = '1';
	fwrite(&(hReg->status),STATUSSIZE,1,fpOut);
	hReg->numeroVertices = len;
	fwrite(&(hReg->numeroVertices),VERTSIZE,1,fpOut);
	hReg->numeroArestas = row_count;
	fwrite(&(hReg->numeroArestas),EDGESIZE,1,fpOut);

	time_t rawtime;
	struct tm * timeinfo;
	char date[11];

	time (&rawtime);
	timeinfo = localtime (&rawtime);
	strftime (date,11,"%d/%m/%Y",timeinfo);

	strcpy(hReg->dataUltimaCompactacao,date);
	fwrite(hReg->dataUltimaCompactacao,sizeof(char),LASTCOMPSIZE,fpOut);

	fclose(fpOut);
	fclose(fpInp);
}

int main(){

	char arqInp[50];
	char arqOut[50];
	char field[15];
	char value[VARSIZE];
	int rrn = -1;
	int n = -1;
	char estadoOrigem[3];
	char estadoDestino[3];
	int distancia = -1;
	char *cidadeOrigem = (char *)malloc(sizeof(char *));
	char *cidadeDestino = (char *)malloc(sizeof(char *));
	char *tempoViagem = (char *)malloc(sizeof(char *));
	int option = -1;
	scanf("%d ",&option);

	switch (option)	{
		case 1:
			scanf("%s %s",arqInp,arqOut);
			csv_to_bin(arqInp,arqOut);
			binarioNaTela1(arqOut);
			break;
		
		case 2:
			scanf("%s",arqOut);
			read_bin(arqOut);
			break;

		case 3:
			scanf("%s %s ",arqOut,field);
			if(!strcmp(field,"distancia")){
				scanf("%s",value);
				search_by_field(arqOut,field,value);
			} else {
				scan_quote_string(value);
				search_by_field(arqOut,field,value);
			}
			break;

		case 4:
			scanf("%s %d",arqOut,&rrn);
			search_by_rrn(arqOut,rrn);
			break;

		case 5:
			scanf("%s %d",arqOut,&n);
			for(int i = 0; i < n; ++i){
				scanf("%s ",field);
				if(!strcmp(field,"distancia")){
					scanf("%s",value);
					remove_register(arqOut,field,value);
				} else {
					scan_quote_string(value);
					remove_register(arqOut,field,value);
				}
			}
			read_bin(arqOut);
			break;

		case 6:
			scanf("%s %d",arqOut,&n);
			for(int i = 0; i < n; ++i){
				scanf("%s %s %d ",estadoOrigem,estadoDestino,&distancia);
				scan_quote_string(cidadeOrigem);
				scan_quote_string(cidadeDestino);
				scan_quote_string(tempoViagem);
				insert_register(arqOut,estadoOrigem,estadoDestino,distancia,cidadeOrigem,cidadeDestino,tempoViagem);
			}
			read_bin(arqOut);

			break;

		case 7:
			scanf("%s %d",arqOut,&n);
			for(int i = 0; i < n; ++i){
				scanf("%d %s ",&rrn,field);
				if(!strcmp(field,"distancia")){
					scanf("%s",value);
					update_register(field,value,arqOut,rrn);
				} else {
					scan_quote_string(value);
					update_register(field,value,arqOut,rrn);
				}
			}
			read_bin(arqOut);
			break;

		case 8:
			scanf("%s %s",arqInp,arqOut);
			compress_bin(arqInp,arqOut);
			binarioNaTela1(arqOut);
			break;

		default:
			printf("Opção não encontrada.\n");
			break;
	}

	//update_register("cidadeOrigem","LIMEIRA","caso02.bin",3);
  return 0;
}
