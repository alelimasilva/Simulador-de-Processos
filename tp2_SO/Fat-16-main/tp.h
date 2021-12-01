#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define CLUSTER_SIZE 1024 //numero de clusters
#define STRINGS_SIZE 200	// TAMNHO LIMITE DAS STRINGS

/*Informa¸c˜oes sobre o valor das entradas na FAT de 16 bits:
0x0000 -> cluster livre
0x0001 - 0xfffc -> arquivo (ponteiro p/ proximo cluster)
0xfffd -> boot block
0xfffe -> FAT
0xffff -> fim do arquivo
Informa¸c˜oes sobre a estrutura das entradas de diret´orio:
18 bytes -> nome do arquivo
1 byte -> atributo do arquivo
7 bytes -> reservado
2 bytes -> numero do primeiro cluster ocupado
4 bytes -> tamanho do arquivo
Byte de atributo do arquivo - valor: 0 - arquivo, 1 - diret´orio*/

/* entrada de diretorio, 32 bytes cada */
typedef struct
{
	uint8_t filename[18];
	uint8_t attributes;
	uint8_t reserved[7];
	uint16_t first_block;
	uint32_t size;
} dir_entry_t;

/* 8 clusters da tabela FAT, 4096 entradas de 16 bits = 8192 bytes*/
uint16_t fat[4096];
/* diretorios (incluindo ROOT), 32 entradas de diretorio
com 32 bytes cada = 1024 bytes ou bloco de dados de 1024 bytes*/
//typedef union _data_cluster data_cluster;
typedef union
{
	dir_entry_t dir[CLUSTER_SIZE / sizeof(dir_entry_t)]; //1024 bytes / 32 bytes = 32 posições
	uint8_t data[CLUSTER_SIZE];
} data_cluster;

//assinatura das funções
//funcoes fat
int init();
int load();
void ls(char *diretorio);
void mkdir(char *diretorio);
void create(char *dir);
void unlink(char *diretorio);
void write(char *paramentros);
void append(char *parametros);
void read(char *diretorio);
//funcoes string
void separaString(char *string1, char *string2, char *string3, char *separador);
data_cluster *quebrarStringClusters(char *string, int *numClusters);
//funcoes diretorio
int procurarDIr(char *diretorio, char *dirAtual, int procura);
int getNumDiretorios(char *caminho);
//funcoes clusters
data_cluster lerCluster(int index);
void salvarCluster(int index, data_cluster cluster);

void atualizarFat();
