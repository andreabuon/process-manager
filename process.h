#pragma once
#include "list.h"
#include <sys/types.h>

//Struttura dati che contiene le informazioni che interessano del processo
typedef struct info{
	pid_t pid;
	char* command;
	char state[2];
	int memory; //resident memory in MB		//TODO controllare
} info;

//Alloca una nuova struttura info e ne restituisce il puntatore. In caso di errore ritorna NULL.
info* info_new();

//Dealloca la struttura info puntata dal puntatore in input
void info_free(info* process_info);

//Stampa su console i dati del processo salvati nella struttura info in input
void info_print(const info* process_info);

//Crea una nuova struttura info con le informazioni relative al processo (il cui pid è passato in input) e ne ritorna il puntatore. Ritorna NULL in caso di errore.
info* getProcessInfo(const char* path);

//Crea e ritorna lista di processi in esecuzione. In caso di errore ritorna NULL.
List* getProcessesList();
