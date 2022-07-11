#pragma once
#include <sys/types.h>

#define STATE_LEN 2

//Struttura dati che contiene le informazioni che interessano del processo
typedef struct info{
	pid_t pid;
	char* command;
	char state[STATE_LEN];
	long memory; //resident memory [MB]
} info;

//Alloca una nuova struttura info e ne restituisce il puntatore. In caso di errore ritorna NULL.
info* info_new();

//Dealloca la struttura info puntata dal puntatore in input
void info_free(info* process_info);

//Imposta i campi della struttura info
void info_set(info* info, pid_t pid, char* comm, char* state, int mem);

//Stampa su console i dati del processo salvati nella struttura info in input
void info_print(const info* process_info);

//Crea una nuova struttura info con le informazioni relative ad un processo e ne ritorna il puntatore. Ritorna NULL in caso di errore.
//Le informazioni vengono estratte dal file stat contenuto nella directory passata come argomento.
info* getProcessInfo(const int dir_fd);

//Crea e ritorna array di info sui processi in esecuzione. Setta la variabile size in input con la dimensione dell'array. In caso di errore ritorna NULL.
info** getProcessesList(int* size);
