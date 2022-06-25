#include "list.h"

//Struttura dati che contiene le informazioni che interessano del processo
typedef struct info{
	pid_t pid;
	char* command;
	char state[2];
	long unsigned int memory;
} info;

//Alloca una nuova struttura info e ne restituisce il puntatore
info* info_new();

//Dealloca la struttura info puntata dal puntatore in input
void info_free(info* process_info);

//Stampa su console i dati del processo salvati nella struttura info in input
void info_print(const info* process_info);

//Crea lista processi in esecuzione.
list* getProcessesList();

//Crea una nuova struttura info con le informazioni relative al processo (il cui pid è passato in input) e ne ritorna il puntatore.
info* getProcessInfo(const char* pid);