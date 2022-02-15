typedef struct{
	char operacao; //movimentos a serem efetuados no o programa
	char *valor;   //entrada do usuário para operações 
}programa;

typedef struct{
	programa *array_programas; //processos
	int cont_prog; //instrução atual do programa
	int t_total; // tempo total
	int t_atual; // tempo atual
	int tam; //numero de instruções do programa
	int id;	//status atual
}CPU;

typedef struct{ // celula da tabela pcb
	int pid; //identificador de processo
	int ppid; //nº identificação do processo pai
	CPU *programa_cpu; //programa atual da cpu
	char status;
	int *contador; // um inteiro contador que aponta para o contador de programa do processo
	int t_inicio;
}celula_pcb;

typedef struct{ // tabela pcb
	celula_pcb *pcb;
	int tam; // tamanho da tabela
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
