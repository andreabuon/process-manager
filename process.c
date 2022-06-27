#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <dirent.h>
#include <assert.h>
#include <sys/types.h>

#include "process.h"
#include "util.h"

info* info_new(){
	info* process_info = malloc(sizeof(info));
	return process_info;
}

void info_print(const info* process_info){
	printf("%d %s %s %lu\n", process_info->pid, process_info->command, process_info->state, process_info->memory);
}

void info_free(info* process_info){
	free(process_info->command);
	free(process_info);
}

info* getProcessInfo(const char *pid){
	char* stat_path = cuncatenateStrings("/proc/", pid, "/stat");

	FILE *file = fopen(stat_path, "r");
	assert(file && "Errore apertura file");

	info* process_info = info_new();
	//man 5 proc
	/*
	fscanf legge in ordine:
	%d PID del processo
	(%m[^)]) Nome dell'eseguibile del processo, togliendo le parentesi tonde iniziali e finali. Alloca automaticamente la memoria necessaria per contenere la stringa e il null terminator. Il null terminator viene aggiunto automaticamente. Supporta anche nomi che contengono spazi (al contrario di %s)
	%1s Stato del processo. Lo stato è descritto da 1 carattere. Ho usato %s invece di %c in modo da aggiungere automaticamente il null terminator dopo il carattere.
	%* valori ignorati
	%lu Memoria del processo
	*/
	int ret = fscanf(file, "%d (%m[^)]) %1s %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %*u %*u %*d %*d %*d %*d %*d %*d %*u %lu", &(process_info->pid), &(process_info->command), (process_info->state), &(process_info->memory)); //sistemare
	assert(ret>0 && "Errore Scanf");

	free(stat_path);
	fclose(file);
	return process_info;
}

list* getProcessesList(){
	DIR *dir = opendir("/proc/");
	assert(dir && "Errore apertura directory");

	struct dirent *entry;
	entry = readdir(dir);
	assert(entry && "Errore lettura directory");

	list* lista = list_new();

	while(entry){
		// Valuta solo le entry delle directory che corrispondono a processi 
		// ovvero quelle che hanno come nome un numero [il pid del processo]
		if(entry->d_type == DT_DIR && isNumber(entry->d_name)){
			info* process_info = getProcessInfo(entry->d_name);
			list_append(lista, process_info);
		}
		entry = readdir(dir);
	}
	closedir(dir);
	return lista;
}
