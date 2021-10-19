typedef struct{
	char operacao; //movimentos a serem efetuados no o programa
	char *valor;   //entrada do usuário para operações 
}programa;

typedef struct{
	programa *array_programas;
	int cont_prog;
	//char *nome_arquivo;
	int t_total;
	int t_atual;
	int tam;
	int id;	
}CPU;

typedef struct{
	int pid; //identificador de processo
	int ppid; //nº identificação do processo pai
	CPU *contador_programas;
	char status;
	int *contador; // um inteiro contador que aponta para o contador de programa do processo
	int t_inicio;
}tabela_pcb; //controle do processo (cada celula da tabela, um processo em si)

/*typedef struct{ // mudar
	tabela_pcb *pcb;
	int tam;
	tabela_pcb *pcb_atual;
}array_list; //lista de arranjos de todos os programas que estão nos 3 possíveis estados
*/

typedef struct fila{ //escalonador 
	CPU *chave;
	struct fila *prox;
}fila;