#include"manager.h"

typedef struct fila{ //escalonador 
	CPU *chave;
	struct fila *prox;
}Fila;

void enfilera(Fila **f, CPU *id);
Fila *desenfilera(Fila **f);
void printa_fila(Fila *f);
