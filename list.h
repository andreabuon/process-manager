//forward declarations
typedef struct info info;

typedef struct listItem listItem;
//

typedef struct listItem {
	info* proc;
	listItem* next;
} listItem;

typedef struct list {
	listItem* first;
	listItem* last;
	unsigned int size;
} list;

listItem* listItem_new(info* proc);
void listItem_free(listItem* elem);

list* list_new();
void list_init(list* list);
void list_append(list* list, info* proc);
void list_free(list* list);