#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <dirent.h>
#include <assert.h>
#include <sys/types.h>
#include <string.h>

#include "process.h"
#include "util.h"

#define PATH_LEN 256

info* info_new(){
	info* process_info = malloc(sizeof(info));
	return process_info;
}

void info_print(const info* process_info){
	printf("%d %s %s %lu\n", process_info->pid, process_info->command, process_info->state, process_info->memory);
}

void info_free(info* process_info){
	free(process_info);
}

// Dato un PID assembla e restituisce la stringa: "/proc/[pid]/stat"
char* buildPathByPID(const char* pid){
	char *path = calloc(PATH_LEN, sizeof(char));
	strcat(path, "/proc/");
	strcat(path, pid);
	strcat(path, "/stat");
	return path;
}

info* getProcessInfo(const char *pid){
	char* path = buildPathByPID(pid);

	FILE *file = fopen(path, "r");
	assert(file && "Errore apertura file");

	char* line = NULL;
	size_t n;
	int read_chars = getline(&line, &n, file);
	assert(read_chars>0 && "Errore lettura file stat del processo");

	info* process_info = info_new();
	// man 5 proc
	//%s e %[] aggiungono in automatico il null terminator
	int ret = sscanf(line, "%d (%30[^)]) %s %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %*u %*u %*d %*d %*d %*d %*d %*d %*u %lu", &(process_info->pid), (process_info->command), (process_info->state), &(process_info->memory)); //sistemare
	//sistemare
	//info_print(process_info);
	assert(ret>0 && "Errore Scanf");

	free(line);
	free(path);
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
