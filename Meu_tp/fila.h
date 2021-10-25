#include"manager.h"

typedef struct fila{ //escalonador 
	CPU *chave;
	struct Fila *prox;
}Fila;

void enfilera(Fila **f, CPU *id);
Fila *desenfilera(Fila **f);