#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/types.h>
#include <errno.h>
#include "process.h"
#include "util.h"

info* info_new(){
	info* process_info = malloc(sizeof(info));
	if(!process_info){
		perror("Errore allocazione Info");
		return NULL;
	}
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
	if(!stat_path){
		fprintf(stderr, "Errore Creazione Stringa Path.\n");	
		return NULL;
	}

	FILE *file = fopen(stat_path, "r");
	if(!file){
		perror("Errore apertura file");
		free(stat_path);
		return NULL;
	}

	info* process_info = info_new();
	if(!process_info){
		free(stat_path);
		fclose(file);
		return NULL;
	}
	/*
	fscanf legge in ordine:
	%d PID del processo
	(%m[^)]) Nome dell'eseguibile del processo, togliendo la parentesi tonda iniziale e finale. Alloca automaticamente la memoria necessaria per contenere la stringa e il null terminator. Il null terminator viene aggiunto automaticamente. Supporta anche nomi che contengono spazi (al contrario di %s)
	%1s Stato del processo. Lo stato è descritto da 1 carattere. Ho usato %s invece di %c in modo da aggiungere automaticamente il null terminator dopo il carattere.
	%* valori ignorati
	%lu Memoria virtuale del processo
	per altro leggere 'man 5 proc'
	*/
	int ret = fscanf(file,
					"%d (%m[^)]) %1s %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %*u %*u %*d %*d %*d %*d %*d %*d %*u %lu",
					&(process_info->pid),
					&(process_info->command),
					(process_info->state),
					&(process_info->memory)); //sistemare
	if(ret==EOF){
		perror("Errore fscanf");
		info_free(process_info);
		free(stat_path);
		fclose(file);
		return NULL;
	}

	free(stat_path);
	fclose(file);
	return process_info;
}

List* getProcessesList(){
	DIR *dir = opendir("/proc/");
	if(!dir){
		perror("Errore apertura directory");
		return NULL;
	}

	List* lista = List_new();
	if(!lista){
		closedir(dir);
		return NULL;
	}

	for(;;){
		errno = 0;
		struct dirent *entry = readdir(dir);
		// Se si raggiunge la fine della directory (entry==NULL) o si verifica un errore (entry==NULL && errno) esci dal ciclo
		if(!entry){ 
			if(errno)
				perror("Errore lettura directory");
			break; 
		}
		//Valuta solo le entry delle directory che corrispondono a processi ovvero quelle che hanno come nome un numero [il pid del processo]
		if(entry->d_type == DT_DIR && isNumber(entry->d_name)){
			info* process_info = getProcessInfo(entry->d_name);
			if(!process_info){
				fprintf(stderr, "Errore Lettura Info Processo %s.\n", entry->d_name);	
				continue;
			}
			List_append(lista, process_info);
		}
	}
	closedir(dir);
	return lista;
}
