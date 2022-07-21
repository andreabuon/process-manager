#pragma once
#ifndef PROCESS_H
#define PROCESS_H

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

//Dealloca la struttura info puntata dal puntatore in input e i suoi campi.
void info_free(info* process_info);

//Imposta i campi della struttura info
void info_set(info* info, pid_t pid, char* comm, char state, unsigned flags, int cpu, long mem);

//Stampa su console i dati del processo salvati nella struttura info in input
void info_print(const info* process_info);

/*****/

info* getProcessInfo(pid_t pid);

info** getProcessesList(int* size);

char* getStateString(char s);

#endif //PROCESS_H
