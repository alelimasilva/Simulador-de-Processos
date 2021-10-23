#include"manager.h"

typedef struct fila{ //escalonador 
	CPU *chave;
	struct fila *prox;
}fila;