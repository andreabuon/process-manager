#ifndef LIST_H
#define LIST_H

//forward declaration
typedef struct info info;

typedef struct ListItem {
	struct ListItem* next;
	info* data;
} ListItem;

typedef struct List {
	ListItem* first;
	ListItem* last;
	unsigned int size;
} List;

//Crea e restituisce un nuovo ListItem con l'informazione passata come argomento. Ritorna NULL in caso di errore.
ListItem* ListItem_new(info* val);

//Dealloca un ListItem.
void ListItem_free(ListItem* elem);

//Crea e restituisce una nuova Lista. Ritorna NULL in caso di errore.
List* List_new();

//Imposta a NULL e a 0 i campi della lista in input.
void List_init(List* list);

//Aggiunge alla fine della Lista list un nuovo ListItem che contiene i dati val. Restituisce puntatore a ListItem o NULL in caso di errore.
ListItem* List_append(List* list, info* val);

//Dealloca la Lista list e tutti i suoi ListItem.
void List_free(List* list);

#endif