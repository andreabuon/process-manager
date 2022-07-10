#include <stdlib.h>
#include <stdio.h>
#include "list.h"

//forward declaration
extern void info_free(); //TODO togliere

ListItem* ListItem_new(info* val){
	ListItem* elem = malloc(sizeof(ListItem));
	if(!elem){
		perror("Errore allocazione ListItem");
		return NULL;
	}
	elem->next = NULL;
	elem->data = val;
	return elem;
}

void ListItem_free(ListItem* elem){
	info_free(elem->data); //TODO togliere
	free(elem);
}

List* List_new(){
	List* lista = malloc(sizeof(List));
	if(!lista){
		perror("Errore allocazione Lista");
		return NULL;
	}
	List_init(lista);
	return lista;
}

void List_init(List* lista){
	lista->first = NULL;
	lista->last = NULL;
	lista->size = 0;
}

ListItem* List_append(List* lista, info* val){
	ListItem* elem = ListItem_new(val);
	if(!elem) return NULL;
	if(!lista->first){
		lista->first = elem;
	}
	if(lista->last){
		lista->last->next = elem;
	}
	lista->last = elem;
	lista->size++;
	return elem;
}

void List_free(List* lista){
	ListItem* elem = lista->first;
	while(elem){
		ListItem* e = elem;
		elem = elem->next;
		ListItem_free(e);
	}
	free(lista);
}