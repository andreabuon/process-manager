#include "aux.h"

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <dirent.h>
#include <assert.h>
#include <sys/types.h>
#include <ctype.h>
#include <signal.h>
#include <string.h>

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
		//stampa solo le entry di directory che corrispondono a processi (ovvero che hanno come nome un numero [il pid])
		if(entry->d_type == DT_DIR && isNumber(entry->d_name)){
			printf(" "); //commentando crasha il programma?!?
			info* info = getProcessInfo(entry->d_name);
			info_print(info);
			info_free(info);
		}
		entry = readdir(dir);
	}
	closedir(dir);
}

info* getProcessInfo(char *pid){
	//Le informazioni sul processo vengono estratte dal file: "/proc/[pid]/stat"
	char path[50]; //50 numero a caso, dovrebbe bastare; sistemare
	char dir[] = "/proc/"; 
	char filename[] = "/stat";
	memset(path, 0, 50);
	strcat(path, dir); //sistemare n
	strcat(path, pid);
	strcat(path, filename);

	FILE *file = fopen(path, "r");
	assert(file && "Errore apertura file");

	char* line = NULL;
	size_t n;
	getline(&line, &n, file);

	info* info = info_new();
	sscanf(line, "%d %s %c", &(info->pid), &(info->comm), &(info->state)); //sistemare

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
	while(*string){
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
	printf("%d %s %c\n", info->pid, info->comm, info->state);
}

void info_free(info* info){
	free(info);
}