#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/types.h>
#include <string.h>
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

info* getProcessInfo(const char *path){
	FILE *file = fopen(path, "r");
	if(!file){
		perror("Errore apertura file");
		return NULL;
	}

	info* process_info = info_new();
	if(!process_info){
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
					"%d (%m[^)]) %1s %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %*u %*u %*d %*d %*d %*d %*d %*d %*u %*u %ld",
					&(process_info->pid),
					&(process_info->command),
					(process_info->state),
					&(process_info->memory)); //sistemare
	if(ret==EOF || ret<4){ //sistemare, skippa processo (sd-pam)
		if(ret == EOF)
			perror("Errore Fscanf");
		info_free(process_info);
		fclose(file);
		return NULL;
	}

	fclose(file);
	return process_info;
}

List* getProcessesList(){
	const char* proc_path = "/proc/";
	const int proc_len = strlen(proc_path);
	const char* stat_path = "/stat";
	const int stat_len = strlen(stat_path);

	DIR *dir = opendir(proc_path);
	if(!dir){
		perror("Errore apertura directory");
		return NULL;
	}

	List* lista = List_new();
	if(!lista){
		closedir(dir);
		return NULL;
	}

	//Ciclo su tutte le entry della directory
	for(;;){
		errno = 0;
		struct dirent *entry = readdir(dir);
		if(!entry){ 
			if(errno)
				perror("Errore lettura directory");
			break; 
		}
		
		//Valuta solo le entry delle directory che corrispondono a processi ovvero quelle che hanno come nome un numero [il pid del processo]
		if(entry->d_type == DT_DIR){
			int len = isNumeric(entry->d_name);
			if(!len) continue;

			int len_tot = proc_len + len + stat_len + 1; //+1 per il null terminator!

			char *path = malloc((len_tot)*sizeof(char)); 
			if(!path){
				perror("Errore allocazione stringa path");
				break;
			}
			
			int ret = snprintf(path, len_tot, "%s%s%s", proc_path, entry->d_name, stat_path);
			if(ret < 0){
				perror("Errore creazione stringa path processo");
				free(path);
				continue;
			}
			
			info* process_info = getProcessInfo(path);
			if(!process_info){
				fprintf(stderr, "Errore Lettura Info Processo %s.\n", entry->d_name);	
				free(path);
				continue;
			}
			
			List_append(lista, process_info);
			free(path);
		}
	}
	
	closedir(dir);
	return lista;
}
