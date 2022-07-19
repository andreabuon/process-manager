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

//Dealloca la struttura info puntata dal puntatore in input
void info_free(info* process_info);

//Imposta i campi della struttura info
void info_set(info* info, pid_t pid, char* comm, char state, unsigned flags, int cpu, long mem);

//Stampa su console i dati del processo salvati nella struttura info in input
void info_print(const info* process_info);

//Crea una nuova struttura info contenente le informazioni relative ad un processo e ne ritorna il puntatore.
//Le informazioni vengono estratte dal file "stat" contenuto nella directory dal file descriptor in input.
//In caso di errore la funzione ritorna NULL.
info* getProcessInfoByFD(const int dir_fd);

//Funzione che prende come input il PID di un processo e ritorna una struttura info con tutte le sue info. In caso di errore ritorna NULL;
//Le informazioni vengono estratte dal file "stat" contenuto nella directory /proc/[pid]/stat.
//In caso di errore la funzione ritorna NULL.
info* getProcessInfoByPid(pid_t pid);

//Crea array di info sui processi in esecuzione.
//Le informazioni sui processi sono estratte dalla cartella /proc/
//La dimensione dell'array viene salvata nella variabile size in input. 
//In caso di errore la funzione ritorna NULL.
//L'array può contenere elementi nulli.
info** getProcessesList(int* size);

//Dato un carattere che rappresenta lo stato di un processo ne restituisce una breve descrizione. Esempio: 'S' -> "Sleeping"
char* getStateString(char s);

#endif //PROCESS_H
