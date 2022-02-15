#include"manager.h"

typedef struct fila{
	CPU *chave; // elemento da celula
	struct fila *prox; // aponta para a proxima celula
}Fila;

void adiciona(Fila **f, CPU *id);
Fila *remover(Fila **f);