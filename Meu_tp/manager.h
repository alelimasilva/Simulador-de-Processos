typedef struct{
	char operacao; //movimentos a serem efetuados no o programa
	char *valor;   //entrada do usuário para operações 
}programa;

typedef struct{
	programa *array_programas; //processo
	int cont_prog; //instrução atual do programa (contador)
	//char *nome_arquivo;
	int t_total;
	int t_atual;
	int tam; //numero de instruções do programa
	int id;	//status atual
}CPU;

typedef struct{
	int pid; //identificador de processo
	int ppid; //nº identificação do processo pai
	CPU *programa_cpu;
	char status;
	int *contador; // um inteiro contador que aponta para o contador de programa do processo
	int t_inicio;
}celula_pcb; //controle do processo (cada celula da tabela, um processo em si)

typedef struct{
	celula_pcb *pcb;
	int tam;
	celula_pcb *pcb_atual;
}tabela_pcb; 

int conta_linhas(char* arquivo);
CPU criar(char *arquivo);
void insere_tabelapcb(CPU *cpu);
void executa_processo_simulado();
void teste_escalonador (int tempo);
void escalonar();
celula_pcb *busca(CPU *cpu);
void bloqueia();
void encerra();
void reporter();
void printa_processo(celula_pcb *p);
void printa_comando(programa p);