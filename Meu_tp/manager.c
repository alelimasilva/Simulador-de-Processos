#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include "fila.h"

#define TAM_PCB 1000

//variaveis globais
int t = 0;
CPU *atual;
Fila *e_pronto = NULL;
Fila *e_bloqueado = NULL;
tabela_pcb *t_pcb = NULL;
//int mult_pcb = 1;


void enfilera(Fila **fila, CPU *id){
	Fila *celula_fila = (Fila*)malloc(sizeof(Fila));
	celula_fila->chave = id; 
	celula_fila->prox = NULL;
	if(*fila == NULL){
		*fila = celula_fila;
		return;
	}

	Fila *aux = *fila;
	while(aux->prox != NULL){
		aux = aux->prox;
	}
	aux->prox = celula_fila;
}

/*Remove o primeiro da fila*/
Fila *desenfilera(Fila **fila){
	Fila *aux = *fila;
	*fila = aux->prox;
	aux->prox = NULL;
	return aux;
}

int conta_linhas(char* arquivo){
	FILE *f;
	f = fopen(arquivo, "r");

     if (f == NULL){
          printf("ERRO\n");
          exit(1);    
	}

	int linhas = 0;
	char caractere;
	
	do{
    	caractere = getc(arquivo);
        
        if(caractere == '\n' || caractere == '\0'){
            linhas++;
        }
    }while(caractere != EOF);
		
	return linhas;
}

CPU criar(char *arquivo){
	CPU cpu;
	cpu.tam = conta_linhas(arquivo);
    cpu.array_programas = (programa*)malloc(cpu.tam * sizeof(programa));
	cpu.id = 0;
	cpu.cont_prog = 0;
	cpu.t_atual = 0;
	cpu.t_total = 0;

    //abrindo o arquivo em modo de leitura, apenas
	FILE *fp;
    fp = fopen(arquivo, "r");

    if (fp == NULL){ //erro em caso de arquivo defeituoso ou inexistente
          printf("ERRO\n");
          exit(1);
    }
	int i,j = 0;
	char *line = NULL; //recebe uma linha do arquivo
    size_t len = 0; //tamanho da linha
    size_t read;

	/*Coloca o comando no char comando da struct processo e*/
	/*o argumento na string valor */
	while ((read = getline(&line, &len, fp)) != -1){ // mudar o jeito de ler o arquivo
		cpu.array_programas[i].operacao = line[0]; //primeira letra do comando principal	
		if(line[0] != 'B' && line[0] != 'E'){
			/*Calcula o numero de caracteres no argumento*/
			for(j = 2; line[j+1] != '\0' && line[j+1] != '\n'; j++); //o for começa de 2, pois o argumento tem início na posição 2, lê até um \n ou \0 para obter o tamanho do argumento
			/*aloca espaço para o argumento*/
			cpu.array_programas[i].valor = (char*)malloc(j * sizeof(char));
			memcpy(cpu.array_programas[i].valor, &line[2], j-1); //copiando coisas de um endereço de memória para outro
			cpu.array_programas[i].valor[j-1] = '\0'; //introduzindo o \0 para fim de arquivo
		}else cpu.array_programas[i].valor = NULL; //caso seja B ou E, será NULL para demonstrar não ter utilização
		i++;
    }
	fclose(fp);
	return cpu;
}

void insere_tabelapcb(CPU *cpu){
	/*Se a tabela pcb estiver vazia, iserir na posição 0*/
	if(t_pcb == NULL){
		t_pcb = (tabela_pcb*)malloc(sizeof(tabela_pcb));
		t_pcb->pcb = (celula_pcb*)malloc(sizeof(celula_pcb) * TAM_PCB);
		t_pcb->pcb[0].pid = 0;
		t_pcb->pcb[0].ppid = 0;
		t_pcb->pcb[0].programa_cpu = (CPU*)malloc(sizeof(CPU) * cpu->tam );
		t_pcb->pcb[0].programa_cpu = cpu;
		t_pcb->pcb[0].contador = &(cpu->cont_prog);
		t_pcb->pcb[0].t_inicio = t; 
		t_pcb->pcb[0].status = 'E';
		t_pcb->pcb_atual = &(t_pcb->pcb[0]);
		t_pcb->tam = 1;
		return;
	}	
	/*Insere na primeira posição vazia*/
	t_pcb->pcb[t_pcb->tam].pid = t_pcb->pcb[t_pcb->tam - 1].pid + 1;
	t_pcb->pcb[t_pcb->tam].ppid = t_pcb->pcb_atual->pid;
	t_pcb->pcb[t_pcb->tam].programa_cpu = (CPU*)malloc(sizeof(CPU) * cpu->tam);
	t_pcb->pcb[t_pcb->tam].programa_cpu = cpu;
	t_pcb->pcb_atual = &(t_pcb->pcb[t_pcb->tam]);
	t_pcb->pcb[t_pcb->tam].t_inicio = t; 
	t_pcb->pcb[t_pcb->tam].status = 'E'; 
	t_pcb->tam++;
}

void executa_processo_simulado(){
	atual->t_total++;
	t++;

	switch(atual->array_programas[atual->cont_prog].operacao){
		case 'S':
			atual->id = atoi(atual->array_programas[atual->cont_prog].valor);			
			atual->cont_prog++;
			teste_escalonador(++atual->t_atual);
			break;
		case 'A':
			atual->id += atoi(atual->array_programas[atual->cont_prog].valor);
			atual->cont_prog++;
			teste_escalonador(++atual->t_atual);
			break;
		case 'D':
			atual->id -= atoi(atual->array_programas[atual->cont_prog].valor);
			atual->cont_prog++;
			teste_escalonador(++atual->t_atual);
			break;
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
			aux->array_programas = (CPU*)malloc(sizeof(programa) * atual->tam);
			atual->t_atual = 0; // caso o tempo estiver errado, o erro ta aqui
			*aux = *atual;
			//aux->t_atual = 0;
			insere_tabelapcb(aux);
			//atual->t_atual = 0;
			atual->cont_prog += atoi(atual->array_programas[atual->cont_prog].valor) + 1; 
			enfilera(&e_pronto, atual); //insere na fila de processos prontos
			// troca de imagem
			if(t_pcb->pcb_atual->status != 'P'){
				t_pcb->pcb_atual->status = 'P';
			}

			atual = aux;
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

void teste_escalonador (int tempo){
	if(tempo >= 3){
		escalonar();
	}
}

void escalonar(){
	if(e_pronto != NULL){
		atual->t_atual = 0;
		enfilera(&e_pronto, atual); //insere na fila de processos prontos
		Fila *aux = desenfilera(&e_pronto);
		CPU *cpu = aux->chave; //remove o primeiro processo na fila de prontos
		// troca de imagem
		if(t_pcb->pcb_atual->status != 'P'){
			t_pcb->pcb_atual->status = 'P';
		}
		atual = cpu;
		// aqui
		celula_pcb *aux1 = busca(atual);
		if(aux1 != NULL){
			t_pcb->pcb_atual = aux1;
		}
		// aqui

		if(t_pcb->pcb_atual->status != 'E'){
			t_pcb->pcb_atual->status = 'E';
		}
	}
}

celula_pcb *busca(CPU *cpu){
	for(int i = 0; i < t_pcb->tam; i++)
		if(t_pcb->pcb[i].programa_cpu == cpu)
			return &(t_pcb->pcb[i]);
	return NULL;
}

void bloqueia(){
	atual->cont_prog++;
	atual->t_atual = 0;
	if(t_pcb->pcb_atual->status != 'B'){
		t_pcb->pcb_atual->status = 'B';
	}
	enfilera(&e_bloqueado, atual); //insere na fila de processos prontos
	Fila *aux = desenfilera(&e_pronto);
	CPU *cpu = aux->chave; //remove o primeiro processo na fila de prontos
	// troca de imagem
	if(t_pcb->pcb_atual->status != 'P'){
		t_pcb->pcb_atual->status = 'P';
	}
	atual = cpu;
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
		bool volta_posicao = false;
		int j = 0;
		for(int i = 0; i < t_pcb->tam; i++){
			if(t_pcb->pcb[i].programa_cpu == atual){
				free(t_pcb->pcb[i].programa_cpu->array_programas);
				free(t_pcb->pcb[i].programa_cpu);
				volta_posicao = true;
				j = i;
			}
			//if(volta_posicao == true)
			//	t_pcb->pcb[i] = t_pcb->pcb[i + 1];
		}
		if (volta_posicao == true){
			for (; j < t_pcb->tam; j++){
				t_pcb->pcb[j] = t_pcb->pcb[j + 1];
			}
			t_pcb->tam--;
		}			
	}
	if(e_pronto != NULL){
		Fila *aux = desenfilera(&e_pronto);
		CPU *cpu = aux->chave; //remove o primeiro processo na fila de prontos
		// troca de imagem
		if(t_pcb->pcb_atual->status != 'P'){
			t_pcb->pcb_atual->status = 'P';
		}
		atual = cpu;
		celula_pcb *aux1 = busca(atual);
		if(aux1 != NULL){
			t_pcb->pcb_atual = aux1;
		}

		if(t_pcb->pcb_atual->status != 'E'){
			t_pcb->pcb_atual->status = 'E';
		}
	}else{
		return;
	}
}



void reporter(){
	celula_pcb *a = busca(atual);
	printf("*****************************************************************************\n"
		  "Estado do sistema:\n"
		  "*****************************************************************************\n");
	printf("TEMPO ATUAL: %d\n",t);
	printf("PROCESSO EXECUTANDO:\n");
	printf("pid	ppid	valor	tempo inicio	CPU usada ate agora	nome processo\n");
	if(atual->cont_prog < atual->tam)
		printf("%d\t%d	%d\t%d\t\t%d\t\t\t\n", a->pid,a->ppid,a->programa_cpu->id,
			  a->t_inicio, a->programa_cpu->t_total);
	printf("PROCESSO BLOQUEADOS:\n");
	Fila *temp = e_bloqueado;
	while(temp != NULL){
		a = busca(temp->chave);
		temp = temp->prox;
		printf("%d\t%d	%d\t%d\t\t%d\t\t\t\n", a->pid,a->ppid,a->programa_cpu->id,
			  a->t_inicio, a->programa_cpu->t_total);
	}
	printf("PROCESSO PRONTOS:\n");
	temp = e_pronto;
	while(temp != NULL){
		a = busca(temp->chave);
		temp = temp->prox;
		printf("%d\t%d	%d\t%d\t\t%d\t\t\t\n", a->pid,a->ppid,a->programa_cpu->id,
			  a->t_inicio, a->programa_cpu->t_total);
	}
	printf("*****************************************************************************\n\n");
}

int main(){
	char instrucao = '\0';
    //inicializando a CPU
    CPU *cpu = (CPU*)malloc(sizeof(CPU));
	*cpu = criar("init");
	// cria processo do carneiro termina aqui
	insere_tabelapcb(cpu);
	atual = cpu;
	while(instrucao != 'T'){
		scanf("%c", &instrucao);
		switch(instrucao){
			case 'Q':
				if(e_pronto != NULL)
					executa_processo_simulado();
				break;
			case 'U':
				if(e_bloqueado != NULL){
					//desbloqueia_processo();
					//CPU *aux = desenfilera(&e_bloqueado)->chave; //remove o primeiro elemento na fila de bloqueados 
					Fila *aux = desenfilera(&e_pronto);
					CPU *aux1 = aux->chave; //remove o primeiro processo na fila de prontos
					enfilera(&e_pronto, aux1); //insere na fila de processos prontos
				}
				break;
			case 'P':
				reporter();
				break;
			case 'T':
				reporter();
				break;
		}
		setbuf(stdout,NULL);
		//fflush(stdout);
	}
}

