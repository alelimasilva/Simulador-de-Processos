#include "fila.h"
#include <stdlib.h>
#include <stdio.h>

void adiciona(Fila **fila, CPU *id){
	// aloca espaço para o novo elemento e atualiza seus valores
	Fila *celula_fila = (Fila*)malloc(sizeof(Fila));
	celula_fila->chave = id; 
	celula_fila->prox = NULL;
	if(*fila == NULL){ // se estiver vazia, será o primeiro elemento
		*fila = celula_fila;
		return;
	}
	Fila *aux = *fila;
	while(aux->prox != NULL){ // loop para encontrar ultima posição
		aux = aux->prox;
	}
	aux->prox = celula_fila;
}

Fila *remover(Fila **fila){
	Fila *aux = *fila;
	*fila = aux->prox; // primeira posição se torna a segunda
	aux->prox = NULL;	
	return aux;
}