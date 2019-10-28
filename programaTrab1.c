#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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

//-----Data structure for a node of a list of cities
typedef struct cityNode {
	char *city;
	struct cityNode* next;
} cityNode;

//-----Data structure for a node of a list of cities
typedef struct cityNode* cityList;

/*
* Abaixo seguem funções que fazem a escrita do binário em "stdout" (tela) pra poder ser comparado no run.codes.
*
* Funciona assim: você faz tudo o que tiver que fazer na funcionalidade no arquivo em disco, assim como ensinado nas aulas da disciplina.
* Ao fim da funcionalidade, use a função "binarioNaTela" e a função já cuida de tudo para você. É só chamar a função.
*
* Note que ao usar a binarioNaTela1, o fclose no arquivo binário já deve ter sido feito anteriormente. Você passa o nome do arquivo binário ("arquivoTrabX.bin") pra ela e ela vai ler tudo e escrever na tela.
*
* Você pode colocar isso num módulo .h separado, ou incluir as funções no próprio código .c: como preferir.
* VOCÊ NÃO PRECISA ENTENDER ESSAS FUNÇÕES. É só usar elas da forma certa depois de acabar a funcionalidade.
*
* Tá tudo testado e funcionando, mas qualquer dúvida acerca dessas funções, falar com o monitor Matheus (mcarvalhor@usp.br).
*/

void binarioNaTela1(char *nomeArquivoBinario) {

	/* Use essa função para comparação no run.codes. Lembre-se de ter fechado (fclose) o arquivo anteriormente.
	*  Ela vai abrir de novo para leitura e depois fechar. */

	unsigned long i, cs;
	unsigned char *mb;
	size_t fl;
	FILE *fs;
	if(nomeArquivoBinario == NULL || !(fs = fopen(nomeArquivoBinario, "rb"))) {
		fprintf(stderr, "ERRO AO ESCREVER O BINARIO NA TELA (função binarioNaTela1): não foi possível abrir o arquivo que me passou para leitura. Ele existe e você tá passando o nome certo? Você lembrou de fechar ele com fclose depois de usar?\n");
		return;
	}
	fseek(fs, 0, SEEK_END);
	fl = ftell(fs);
	fseek(fs, 0, SEEK_SET);
	mb = (unsigned char *) malloc(fl);
	fread(mb, 1, fl, fs);

	cs = 0;
	for(i = 0; i < fl; i++) {
		cs += (unsigned long) mb[i];
	}
	printf("%lf\n", (cs / (double) 100));
	free(mb);
	fclose(fs);
}

void trim(char *str) {

	/*
	*	Essa função arruma uma string de entrada "str".
	*	Manda pra ela uma string que tem '\r' e ela retorna sem.
	*	Ela remove do início e do fim da string todo tipo de espaçamento (\r, \n, \t, espaço, ...).
	*	Por exemplo:
	*
	*	char minhaString[] = "    \t TESTE  DE STRING COM BARRA R     \t  \r\n ";
	*	trim(minhaString);
	*	printf("[%s]", minhaString); // vai imprimir "[TESTE  DE STRING COM BARRA R]"
	*
	*/

	size_t len;
	char *p;

	for(len = strlen(str); len > 0 && isspace(str[len - 1]); len--); // remove espaçamentos do fim
	str[len] = '\0';
	for(p = str; *p != '\0' && isspace(*p); p++); // remove espaçamentos do começo
	len = strlen(p);
	memmove(str, p, sizeof(char) * (len + 1));
}

void scan_quote_string(char *str) {

	/*
	*	Use essa função para ler um campo string delimitado entre aspas (").
	*	Chame ela na hora que for ler tal campo. Por exemplo:
	*
	*	A entrada está da seguinte forma:
	*		nomeDoCampo "MARIA DA SILVA"
	*
	*	Para ler isso para as strings já alocadas str1 e str2 do seu programa, você faz:
	*		scanf("%s", str1); // Vai salvar nomeDoCampo em str1
	*		scan_quote_string(str2); // Vai salvar MARIA DA SILVA em str2 (sem as aspas)
	*
	*/

	char R;

	while((R = getchar()) != EOF && isspace(R)); // ignorar espaços, \r, \n...

	if(R == 'N' || R == 'n') { // campo NULO
		getchar(); getchar(); getchar(); // ignorar o "ULO" de NULO.
		strcpy(str, ""); // copia string vazia
	} else if(R == '\"') {
		if(scanf("%[^\"]", str) != 1) { // ler até o fechamento das aspas
			strcpy(str, "");
		}
		getchar(); // ignorar aspas fechando
	} else if(R != EOF){ // vc tá tentando ler uma string que não tá entre aspas! Fazer leitura normal %s então...
		str[0] = R;
		scanf("%s", &str[1]);
	} else { // EOF
		strcpy(str, "");
	}
}

int check_for_city(char *city, cityList *cities){
	cityNode *aux = *cities;

	if(aux == NULL){
		cityNode *new = (cityNode *)malloc(sizeof(cityNode));	//Allocate memory for the node
		if(new == NULL){
			return 0;
		}
		new->city = (char *)malloc(strlen(city) * sizeof(char));
		new->city = city;
		new->next = NULL;
		*cities = new;
		return 1;
	}

	while(aux != NULL){
		if(!strcmp(city, aux->city)){
			return 0;
		}
		if(aux->next == NULL){
			cityNode *new = (cityNode *)malloc(sizeof(cityNode));	//Allocate memory for the node
			if(new == NULL){
				return 0;
			}
			new->city = (char *)malloc(strlen(city) * sizeof(char));
			new->city = city;
			new->next = NULL;
			aux->next = new;
			return 1;
		}
		aux = aux->next;
	}
	return 0;
}

void csv_to_bin(char *fInp, char *fOut){
  FILE *fpInp = fopen(fInp, "r");
  FILE *fpOut = fopen(fOut, "wb");

  if (!fpInp || !fpOut) {
      printf("Falha no carregamento do arquivo.\n");
      return;
  }

	int len = 0;
	cityList *cities = (cityList *)malloc(sizeof(cityList));
	*cities = NULL;
  dataReg *dReg = (dataReg *)malloc(DREGSIZE);
	headerReg *hReg = (headerReg *)malloc(HREGSIZE);
	hReg->status = '0';

  char buf[DREGSIZE];
  int row_count = 0;
  while(fgets(buf, DREGSIZE, fpInp)) {
    if (row_count == 0) {
      row_count++;
      continue;
    }

    char *field = strtok(buf,",");
    strcpy(dReg->estadoOrigem,field);

    field = strtok(NULL,",");
    strcpy(dReg->estadoDestino,field);
    
    field = strtok(NULL,",");
    dReg->distancia = atoi(field);
    
    field = strtok(NULL,",");
    dReg->cidadeOrigem = (char *)malloc(sizeof(char) * strlen(field));
		strcpy(dReg->cidadeOrigem,field);
		if(check_for_city(dReg->cidadeOrigem,cities)){
			++len;
		}

    field = strtok(NULL,",");
    dReg->cidadeDestino = (char *)malloc(sizeof(char) * strlen(field));
		strcpy(dReg->cidadeDestino,field);
		if(check_for_city(dReg->cidadeDestino,cities)){
			++len;
		}

    field = strtok(NULL,",");
    trim(field);
    dReg->tempoViagem = (char *)malloc(sizeof(char) * strlen(field));
		strcpy(dReg->tempoViagem,field);

    fwrite(dReg->estadoOrigem,ORIGINSIZE,1,fpOut);
    fwrite(dReg->estadoDestino,DESTSIZE,1,fpOut);
    fwrite(&(dReg->distancia),DISTANCESIZE,1,fpOut);
    fwrite(dReg->cidadeOrigem,sizeof(char),strlen(dReg->cidadeOrigem),fpOut);
    fwrite("|",sizeof(char),1,fpOut);
    fwrite(dReg->cidadeDestino,sizeof(char),strlen(dReg->cidadeDestino),fpOut);
    fwrite("|",sizeof(char),1,fpOut);
    fwrite(dReg->tempoViagem,sizeof(char),strlen(dReg->tempoViagem),fpOut);
   	fwrite("|",sizeof(char),1,fpOut);
		
	 	int partRegSize = ORIGINSIZE + DESTSIZE + DISTANCESIZE + sizeof(char) * strlen(dReg->cidadeOrigem) + sizeof(char) * strlen(dReg->cidadeDestino) + sizeof(char) * strlen(dReg->tempoViagem) + 3 * sizeof(char);
		while(partRegSize < DREGSIZE) {
			fwrite("#",sizeof(char),1,fpOut);
			partRegSize += sizeof(char);
		}
		row_count++;
  }

	hReg->status = '1';
	hReg->numeroVertices = len;
	hReg->numeroArestas = row_count - 1;
	strcpy(hReg->dataUltimaCompactacao,"00/00/0000");

	fseek(fpOut,0,SEEK_SET);
  fwrite(&(hReg->status),STATUSSIZE,1,fpOut);
  fwrite(&(hReg->numeroVertices),VERTSIZE,1,fpOut);
  fwrite(&(hReg->numeroArestas),EDGESIZE,1,fpOut);
  fwrite(&(hReg->dataUltimaCompactacao),LASTCOMPSIZE,1,fpOut);

  fclose(fpOut);
  fclose(fpInp);
}

void remove_garbage(char *field, int whichField){
	if(whichField == 0){
		field[ORIGINSIZE] = '\0';
	} else if(whichField == 1){
		field[DESTSIZE] = '\0';
	}
}

void read_bin(char *fName){
	FILE *fp = fopen(fName, "rb");

  if (!fp) {
      printf("Falha no processamento do arquivo.\n");
      return;
  }

	dataReg *reg = (dataReg *)malloc(DREGSIZE);
	char buf[VARSIZE];

	int rrn = 0;
	while(fread(reg->estadoOrigem,ORIGINSIZE,1,fp)) {
		printf("%d ",rrn);

		remove_garbage(reg->estadoOrigem,0);
		printf("%s ",reg->estadoOrigem);
		
		fread(reg->estadoDestino,DESTSIZE,1,fp);
		remove_garbage(reg->estadoDestino,1);
		printf("%s ",reg->estadoDestino);
		
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
	}

	if(rrn == 0){
		printf("Registro inexistente.\n");
	}

	fclose(fp);
}

void search_by_rrn(char *fName,int rrn) {
	FILE *fp = fopen(fName, "rb");

  if (!fp) {
      printf("Falha no processamento do arquivo.\n");
      return;
  }
	
	dataReg *reg = (dataReg *)malloc(DREGSIZE);
	char buf[VARSIZE];

	fseek(fp,rrn*DREGSIZE,SEEK_SET);
	
	if(fread(reg->estadoOrigem,ORIGINSIZE,1,fp)) {
		printf("%d ",rrn);

		remove_garbage(reg->estadoOrigem,0);
		printf("%s ",reg->estadoOrigem);
		
		fread(reg->estadoDestino,DESTSIZE,1,fp);
		remove_garbage(reg->estadoDestino,1);
		printf("%s ",reg->estadoDestino);
		
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

int main(){
  csv_to_bin("./casos-de-teste-e-binarios/caso02.csv","caso02.bin");
  binarioNaTela1("caso02.bin");
	printf("\n");
  //read_bin("caso02.bin");
	//printf("\n");
	//search_by_rrn("caso02.bin",28);
  return 0;
}
