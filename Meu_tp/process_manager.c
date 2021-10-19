#include <stdio.h>
#include <stdlib.h>
#include "process_manager.h"

#define TAM_PCB 200
#define T_PADRAO 3

int t = 0;
CPU *atual;
fila *e_pronto = NULL;
fila *e_bloqueado = NULL;
//array_list *pcb_list = NULL;
int mult_pcb = 1;

int main() {
	char instrucao;

    //declarando a CPU
    CPU *c = (CPU*)malloc(sizeof(CPU));

    //inicializando a CPU
    CPU *cpu;
	cpu.tam = line_count("init");
    cpu.array_programas = (programa*)malloc(cpu.tam * sizeof(programa));
	cpu.id = 0;
	cpu.cont_prog = 0;
	cpu.t_atual = 0;
	cpu.t_total = 0;

    //abrindo o arquivo em modo de leitura, apenas
	FILE *fp;
    fp = fopen("init", "r");

    if (fp == NULL){ //erro em caso de arquivo defeituoso ou inexistente
          printf("ERRO: ARQUIVO NÃO ENCONTRADO\n");
          exit(1);
    }

	insere_pcb(c);
	executando = c;
	do {
		scanf("%c", &string);
		switch(string){
			case 'Q':
				if(executando->contador <= executando->tamanho - 1 || estado_pronto != NULL)
					executa_processo();
				break;
			case 'U':
				if(estado_bloqueado != NULL)
					desbloqueia_processo();
				break;
			case 'P':
				/*system("clear");*/
				reporter();
				break;
			case 'T':
				/*system("clear");*/
				reporter();
				break;
		}
		
		fflush(stdout);
	} while(string != 'T');

}

CPU cria_processo(char *prog){
	 FILE *fp; //arquivo
     char *line = NULL; //recebe uma linha do arquivo
     size_t len = 0; //tamanho da linha
     size_t read;

     fp = fopen(prog, "r"); //Abre o arquivo no modo de leitura
     if (fp == NULL){
          printf("ERRO: ARQUIVO NÃO ENCONTRADO\n");
          exit(EXIT_FAILURE);
     }
    
	/*Cria a struct para guardar o programa*/
	/*
    CPU c;
	int num_linhas = line_count(prog);
	c.processo = (programa*)malloc(num_linhas * sizeof(programa));
	c.tamanho = num_linhas;
	c.chave = 0;
	c.contador = 0;
	c.tempo_atual = 0;
	c.tempo_total = 0;
	c.nome_arquivo = prog;
    */

	int i,j = 0;
	/*Coloca o comando no char comando da struct processo e*/
	/*o argumento na string valor */
	while ((read = getline(&line, &len, fp)) != -1) { 
		c.processo[i].comando = line[0]; //primeira letra do comando principal	
		if(line[0] != 'B' && line[0] != 'E'){
			/*Calcula o numero de caracteres no argumento*/
			for(j = 2; line[j+1] != '\0' && line[j+1] != '\n'; j++); //o for começa de 2, pois o argumento tem início na posição 2, lê até um \n ou \0 para obter o tamanho do argumento
			/*aloca espaço para o argumento*/
			c.processo[i].valor = (char*)malloc(j * sizeof(char));
			memcpy(c.processo[i].valor, &line[2], j-1); //copiando coisas de um endereço de memória para outro
			c.processo[i].valor[j-1] = '\0'; //introduzindo o \0 para fim de arquivo
		} else c.processo[i].valor = NULL; //caso seja B ou E, será NULL para demonstrar não ter utilização
		i++;
    }
	fclose(fp);
	return c;
}
