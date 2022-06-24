#include <stdlib.h>
#include "list.h"

listItem* listItem_new(info* proc){
	listItem* elem = malloc(sizeof(listItem));
	elem->proc = proc;
	elem->next = 0;
	return elem;
}

void listItem_free(listItem* elem){
	free(elem->proc);
	free(elem);
}

list* list_new(){
	list* lista = malloc(sizeof(list));
	list_init(lista);
	return lista;
}

void list_init(list* lista){
	lista->first = 0;
	lista->last = 0;
	lista->size = 0;
}

void list_append(list* lista, info* proc){
	listItem* elem = listItem_new(proc);
	if(!lista->first){
		lista->first = elem;
	}
	if(lista->last){
		lista->last->next = elem;
	}
	lista->last = elem;
	lista->size++;
}

void list_free(list* lista){
	listItem* elem = lista->first;
	while(elem){
		listItem* e = elem;
		elem = elem->next;
		listItem_free(e);
	}
	free(lista);
}