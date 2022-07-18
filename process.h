#pragma once
#include <sys/types.h>

//Struttura dati che contiene le informazioni relative ad un processo
typedef struct info{
	pid_t pid;
	char* command;
	char state;
	unsigned flags;
	int cpu_usage;
	long memory; //resident memory [MB]
} info;

//Alloca una nuova struttura info e ne restituisce il puntatore.
//In caso di errore ritorna NULL.
info* info_new();

//Dealloca la struttura info puntata dal puntatore in input
void info_free(info* process_info);

//Imposta i campi della struttura info
void info_set(info* info, pid_t pid, char* comm, char state, unsigned flags, int cpu, long mem);

//Stampa su console i dati del processo salvati nella struttura info in input
void info_print(const info* process_info);

//Crea una nuova struttura info contenete le informazioni relative ad un processo e ne ritorna il puntatore.
//Le informazioni vengono estratte dal file "stat" contenuto nella directory dal file descriptor in input.
//In caso di errore la funzione ritorna NULL.
info* getProcessInfo(const int dir_fd);

//Crea array di info sui processi in esecuzione.
//La dimensione dell'array viene salvata nella variabile size in input. 
//In caso di errore la funzione ritorna NULL.
//L'array può contenere elementi nulli.
info** getProcessesList(int* size);

//Restituisce descrizione dello stato di un processo dato un carattere. esempio: S -> Sleeping
char* getStateString(char s);//FIXME //HACK //TODO
