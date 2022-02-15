#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include "fila.h"

// constante para o tamanho da tabela pcb
#define TAM_PCB 50

//variaveis globais
int t = 0;
CPU *atual;
Fila *e_pronto = NULL;
Fila *e_bloqueado = NULL;
tabela_pcb *t_pcb = NULL;

int conta_linhas(char* arquivo){ //função que conta o número total de linhas em um arquivo
	FILE *f;
	f = fopen(arquivo, "r");
    if(f == NULL){
          printf("ERRO NA CONTAGEM DE LINHAS\n");
          exit(1);    
	}	
	int linhas = 0;
	char caractere;
	do{
    	caractere = getc(f);
        
        if(caractere == '\n' || caractere == '\0'){
            linhas++;
        }
    }while(caractere != EOF);
	fclose(f);
	fflush(stdout);			
	return linhas;
}

CPU criar(char *arquivo){
	// inicializa os dados de um processo simulado
	CPU cpu;
	cpu.tam = conta_linhas(arquivo);		
    cpu.array_programas = (programa*)malloc(cpu.tam * sizeof(programa));
	cpu.id = 0;
	cpu.cont_prog = 0;
	cpu.t_atual = 0;
	cpu.t_total = 0;

    //abrindo o arquivo em modo de leitura
	FILE *fp;
    fp = fopen(arquivo, "r");

    if (fp == NULL){ //erro em caso de arquivo defeituoso ou inexistente
          printf("ERRO NO ARQUIVO\n");
          exit(1);
    }
	int i, j = 0;
	char *line; //recebe uma linha do arquivo
    size_t len = 0; //tamanho da linha
    size_t nread;
	
	while((nread = getline(&line, &len, fp)) != -1){ // lendo o arquivo linha por linha
		cpu.array_programas[i].operacao = line[0]; //primeira letra do comando principal							
		if(line[0] != 'B' && line[0] != 'E'){	 // caso o comando não seja B ou E, salva a segunda parte da linha em valor		
			cpu.array_programas[i].valor = strndup(line + 2, (strlen(line+2)-1));													
		}
		i++;
    }	
	fclose(fp);	
	free(line); 
	return cpu;
}

void insere_tabelapcb(CPU *cpu){
	if(t_pcb == NULL){ // caso seja o primeiro elemento e inserido na posição 0
		t_pcb = (tabela_pcb*)malloc(sizeof(tabela_pcb));
		t_pcb->pcb = (celula_pcb*)malloc(sizeof(celula_pcb) * TAM_PCB);
		t_pcb->pcb[0].pid = 0;
		t_pcb->pcb[0].ppid = 0;
		t_pcb->pcb[0].programa_cpu = (CPU*)malloc(sizeof(CPU) * cpu->tam ); // aloca espaço para o programa
		t_pcb->pcb[0].programa_cpu = cpu;
		t_pcb->pcb[0].contador = &(cpu->cont_prog);
		t_pcb->pcb[0].t_inicio = t; 
		t_pcb->pcb[0].status = 'E';
		t_pcb->pcb_atual = &(t_pcb->pcb[0]);
		t_pcb->tam = 1;
	}else{	// caso não seja o primeiro elemento
		t_pcb->pcb[t_pcb->tam].pid = t_pcb->pcb[t_pcb->tam - 1].pid + 1;// pid é o anterior mais 1
		t_pcb->pcb[t_pcb->tam].ppid = t_pcb->pcb_atual->pid; // pega o pid do pai
		t_pcb->pcb[t_pcb->tam].programa_cpu = (CPU*)malloc(sizeof(CPU) * cpu->tam);
		t_pcb->pcb[t_pcb->tam].programa_cpu = cpu;	
		t_pcb->pcb_atual = &(t_pcb->pcb[t_pcb->tam]);
		t_pcb->pcb[t_pcb->tam].t_inicio = t; 
		t_pcb->pcb[t_pcb->tam].status = 'E'; 
		t_pcb->tam++;
	}
}

void teste_escalonador (int tempo){ // faz o teste para ver se é necessário escalonar
	if(tempo >= 3){
		escalonar();		
	}
}

void escalonar(){
	if(e_pronto != NULL){ 
		atual->t_atual = 0;
		adiciona(&e_pronto, atual); //insere na fila de processos prontos
		Fila *aux = (Fila*)malloc(sizeof(Fila)); // aloca uma fila auxiliar
		aux = remover(&e_pronto); //remove o primeiro processo na fila de prontos
		CPU *cpu = aux->chave; 
		if(t_pcb->pcb_atual->status != 'P'){
			t_pcb->pcb_atual->status = 'P';
		}
		atual = cpu; // atualiza o programa atual
		celula_pcb *aux1 = busca(atual);
		if(aux1 != NULL){
			t_pcb->pcb_atual = aux1;
		}
		if(t_pcb->pcb_atual->status != 'E'){
			t_pcb->pcb_atual->status = 'E';
		}
	}
}

celula_pcb *busca(CPU *cpu){ //busca pelo processo
	for(int i = 0; i < t_pcb->tam; i++)
		if(t_pcb->pcb[i].programa_cpu == cpu)
			return &(t_pcb->pcb[i]); // se encontrar, retorna seu endereço
	return NULL; // NULL caso não encontrado
}

void bloqueia(){	
	atual->t_atual = 0;
	atual->cont_prog++;
	
	t_pcb->pcb_atual->status = 'B'; // atualiza o status para bloqueado
	adiciona(&e_bloqueado, atual); //insere na fila de processos prontos
	Fila *aux = (Fila*)malloc(sizeof(Fila)); // aloca espaço para uma fila auxiliar
	aux = remover(&e_pronto);//remove o primeiro processo na fila de prontos
	CPU *cpu = aux->chave; 
	if(t_pcb->pcb_atual->status != 'P'){
		t_pcb->pcb_atual->status = 'P';
	}
	
	atual = cpu; // atualiza o programa atual
	celula_pcb *aux1 = busca(atual);
	if(aux1 != NULL){
		t_pcb->pcb_atual = aux1;
	}
	
	if(t_pcb->pcb_atual->status != 'E'){
		t_pcb->pcb_atual->status = 'E';
	}
}

void encerra(){
	if(atual != NULL){
		bool volta_posicao = false; // suporte para caso encontre o elemento
		int j = 0;
		for(int i = 0; i < t_pcb->tam; i++){ // procura pelo processo atual para encerra-lo
			if(t_pcb->pcb[i].programa_cpu == atual){
				free(t_pcb->pcb[i].programa_cpu->array_programas); // libera a memoria de onde ele estava
				free(t_pcb->pcb[i].programa_cpu);
				volta_posicao = true;
				j = i;
			}
		}
		if (volta_posicao == true){ // se encontrado, volta uma posição no vetor de todos os seguintes
			for (; j < t_pcb->tam; j++){
				t_pcb->pcb[j] = t_pcb->pcb[j + 1];
			}
			t_pcb->tam--;
		}			
	}
	if(e_pronto != NULL){
		Fila *aux = (Fila*)malloc(sizeof(Fila)); // aloca espaço para uma fila auxiliar
		aux = remover(&e_pronto);//remove o primeiro processo na fila de prontos
		CPU *cpu = aux->chave; 
		if(t_pcb->pcb_atual->status != 'P'){
			t_pcb->pcb_atual->status = 'P';
		}
		atual = cpu; // atualiza o programa atual
		celula_pcb *aux1 = busca(atual); // busca pelo atual
		if(aux1 != NULL){ // se encontrado atualiza a tabela
			t_pcb->pcb_atual = aux1;
		}

		if(t_pcb->pcb_atual->status != 'E'){
			t_pcb->pcb_atual->status = 'E';
		}
	}
}

void executa_processo_simulado(){
	atual->t_total++;
	t++; // atualiza tempo do sistema
	
	switch(atual->array_programas[atual->cont_prog].operacao){// pega a proxima operaçao a ser realizada	
		case 'S':{
			int aux = atoi(atual->array_programas[atual->cont_prog].valor); // recebe valor em inteiro
			atual->id = aux;			
			atual->cont_prog++;
			teste_escalonador(++atual->t_atual);
			break;
		}
		case 'A':{
			int aux = atoi(atual->array_programas[atual->cont_prog].valor);	// recebe valor em inteiro
			atual->id += aux;	
			atual->cont_prog++;
			teste_escalonador(++atual->t_atual);
			break;
		}
		case 'D':{
			int aux = atoi(atual->array_programas[atual->cont_prog].valor);	// recebe valor em inteiro
			atual->id -= aux;
			atual->cont_prog++;
			teste_escalonador(++atual->t_atual);
			break;
		}
		case 'B':
			bloqueia();
			break;
		case 'E':
			encerra();
			atual->cont_prog++;
			teste_escalonador(++atual->t_atual);
			break;
		case 'F':{
			CPU *aux = (CPU*)malloc(sizeof(CPU));
			aux->array_programas = (programa*)malloc(sizeof(programa) * atual->tam);
			atual->t_atual = 0; // caso o tempo estiver errado, o erro ta aqui
			*aux = *atual;
			insere_tabelapcb(aux);
			int auxiliar = atoi(atual->array_programas[atual->cont_prog].valor) + 1; // recebe valor em inteiro
			atual->cont_prog += auxiliar;
			adiciona(&e_pronto, atual); //insere na fila de processos prontos
			if(t_pcb->pcb_atual->status != 'P'){
				t_pcb->pcb_atual->status = 'P';
			}
			atual = aux; // atualiza o programa atual
			celula_pcb *aux1 = busca(atual);
			if(aux1 != NULL){
				t_pcb->pcb_atual = aux1;
			}
			if(t_pcb->pcb_atual->status != 'E'){
				t_pcb->pcb_atual->status = 'E';
			}
			atual->cont_prog++;
			teste_escalonador(++atual->t_atual);						
			break;
		}
		case 'R':{
			CPU *aux =(CPU*) malloc(sizeof(CPU));
			*aux = criar(atual->array_programas[atual->cont_prog].valor);
			atual = aux;			
			atual->t_total = 0;
			t_pcb->pcb_atual->programa_cpu = atual;			
			break;
		}
	}
}

void reporter(){
	Fila *aux;
	celula_pcb *c_pcb_atual = busca(atual);
	printf("*****************************************************************************\n");
	printf("                       \tTEMPO DE SISTEMA: %d\n",t);
	printf("*****************************************************************************\n");
	printf("EXECUTANDO:\n");
	if(atual->cont_prog < atual->tam)
		printf("\tPID: %d - PPID: %d - Valor: %d - Tempo inicio: %d - CPU: %d\n", c_pcb_atual->pid,c_pcb_atual->ppid,c_pcb_atual->programa_cpu->id,c_pcb_atual->t_inicio, c_pcb_atual->programa_cpu->t_total); 
	aux = e_bloqueado;
	if(aux != NULL){
		printf("-----------------------------------------------------------------------------\n");
		printf("BLOQUEADOS:\n");
	}
	while(aux != NULL){
		c_pcb_atual = busca(aux->chave);
		aux = aux->prox;
		printf("\tPID: %d - PPID: %d - Valor: %d - Tempo inicio: %d - CPU: %d\n", c_pcb_atual->pid,c_pcb_atual->ppid,c_pcb_atual->programa_cpu->id,c_pcb_atual->t_inicio, c_pcb_atual->programa_cpu->t_total);
	}
	aux = e_pronto;
	if(aux != NULL){
		printf("-----------------------------------------------------------------------------\n");
		printf("PRONTOS:\n");
	}
	while(aux != NULL){
		c_pcb_atual = busca(aux->chave);
		aux = aux->prox;
		printf("\tPID: %d - PPID: %d - Valor: %d - Tempo inicio: %d - CPU: %d\n", c_pcb_atual->pid,c_pcb_atual->ppid,c_pcb_atual->programa_cpu->id,c_pcb_atual->t_inicio, c_pcb_atual->programa_cpu->t_total);
	}
	printf("*****************************************************************************\n\n\n");
}

int main(){
	char instrucao;
    //inicializando a CPU
    CPU *cpu = (CPU*)malloc(sizeof(CPU));
	*cpu = criar("init");
	insere_tabelapcb(cpu);
	atual = cpu;
	do{
		scanf("%c", &instrucao);
		switch(instrucao){
			case 'Q':
					executa_processo_simulado();
				break;
			case 'U':
				if(e_bloqueado != NULL){ 
					Fila *aux = (Fila*)malloc(sizeof(Fila));
					aux = remover(&e_bloqueado);
					CPU *aux1 = aux->chave; //remove o primeiro processo na fila de prontos
					adiciona(&e_pronto, aux1); //insere na fila de processos prontos										
				}			
				break;
			case 'P':
				reporter();
				break;
			case 'T':
				reporter();
				break;
		}
		fflush(stdout);
	}while(instrucao != 'T');
	
	return 0;
}
