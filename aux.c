#include "aux.h"

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <dirent.h>
#include <assert.h>
#include <sys/types.h>
#include <ctype.h>
#include <signal.h>

#define CHUNK 1024 //numero di byte letti per volta

void getCPUinfo() {
	FILE *file = fopen("/proc/cpuinfo", "r");
	assert(file && "Errore apertura file");

	char c[CHUNK];
	while (fgets(c, CHUNK, file)){
		printf("%s", c);
	}

	fclose(file);
}

void getProcessesList(){
	DIR *dir = opendir("/proc/");
	assert(dir && "Errore apertura directory");

	struct dirent *entry;
	entry = readdir(dir);
	assert(entry && "Errore lettura directory");

	while(entry){
		switch (entry->d_type){
			//stampa solo le entry directory che corrispondono ai processi (ovvero che come nome hanno il pid numerico)
			case DT_DIR:
				char *name = entry->d_name;
				if(isNumber(name))
					printf("%s\n", entry->d_name);
				break;
			default:
				break;
		}
		entry = readdir(dir);
	}

	closedir(dir);
}

info* getProcessData(){
	char path[] = "/proc/1/stat";
	FILE *file = fopen(path, "r");
	assert(file && "Errore apertura file");

	char* line = NULL;
	size_t n;
	getline(&line, &n, file);

	info* info = info_new();
	sscanf(line, "%d (%s %c", &(info->pid), &(info->comm), &(info->state));

	free(line);
	fclose(file);
	return info;
}

int sendSignal(int signal){
	//pid_t pid = getSelectedProcess(); ottieni il pid del processo selezionato nella gui (da implementare)
	pid_t pid = 0; // sistemare
	kill(pid, signal);
}

int isNumber(char* string){
	while(*string != '\0'){
		if(!isdigit(*string))
			return 0;
		string++;
	}
	return 1;
}

info* info_new(){
	info* info = malloc(sizeof(info));
	return info;
}

void info_print(info* info){
	printf("Pid: %d - Nome: %s - Stato: %c\n", info->pid, info->comm, info->state);
}

void info_free(info* info){
	free(info);
}