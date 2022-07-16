#define _GNU_SOURCE 

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include "process.h"
#include "util.h"

//Flag Kernel Thread (da "/linux/sched.h")
#define PF_KTHREAD 0x00200000 

info* info_new(){
	info* process_info = malloc(sizeof(info));
	if(!process_info){
		perror("info_new: Errore allocazione info");
		return NULL;
	}
	process_info->command = NULL;
	return process_info;
}

void info_free(info* process_info){
	if(process_info->command)
		free(process_info->command);
	free(process_info);
}

void info_set(info* info, pid_t pid, char* comm, unsigned flags, int cpu, long mem){
	info->pid = pid;
	info->command = comm;
	info->flags = flags;
	info->cpu_usage = cpu;
	info->memory = mem;
}

void info_print(const info* process_info){
	printf("%d %s %d %ld\n", process_info->pid, process_info->command, process_info->cpu_usage, process_info->memory);
}

//Esegue il parsing dei dati del processo dal file e li salva nella struttura process_info. Ritorna 0 in caso di successo e 1 in caso di errore.
int parseData(FILE* file, info* process_info){
	int ret;

	pid_t pid = 0; 
	char* comm;
	unsigned flags;
	float cpu_usage;
	long mem;

	//variabili necessarie per calcolo uso della cpu
	long unsigned utime, stime; //espresso in clock ticks
	long long unsigned starttime; //espresso in secondi
	long ticks = sysconf(_SC_CLK_TCK); //numero clock ticks al secondo

	/* Parametri SCANF - per altre info consultare "man 5 proc"
	(1) %d pid -> PID del processo
	(2) (%m[^)]) comm -> Nome dell'eseguibile del processo, togliendo la parentesi tonda iniziale e finale. Alloca automaticamente la memoria necessaria per contenere la stringa e il null terminator. Il null terminator viene aggiunto automaticamente. Supporta anche nomi che contengono spazi (a differenza di "%s") //FIXME processo (sd-pam) provoca errore
	(9) %u flags
	(14) %lu utime
	(15) %lu stime
	(22) %llu starttime  
	(24) %ld rss -> memoria residente
	(-) %* -> valori ignorati
	*/
	char* format_string = "%d (%m[^)]) %*c %*d %*d %*d %*d %*d %u %*u %*u %*u %*u %lu %lu %*d %*d %*d %*d %*d %*d %llu %*u %ld";
	ret = fscanf(file, format_string, &pid, &comm, &flags, &utime, &stime, &starttime, &mem);
	if(ret==EOF || ret < 7){
		if(ret == EOF)
			perror("parseData: Errore fscanf");
		else
			fprintf(stderr, "parseData: Errore matching\n");
		fprintf(stderr, "parseData: Errore lettura info del processo");
		if(pid)
			fprintf(stderr, " [%d]", pid);
		fprintf(stderr, "\n");
		return 1;
	}

	/*
	mem è espressa in numero di pagine
	per	convertire mem in MB ->  totale kilobytes / numero kilobytes in un MB
	totale kilobyte = num pagine * 4096 = num pagine << 12
	numero kilobytes in un MB = 1000 (approssimato a 2^20) 
	quindi mem [MB] =~  mem [pag] >> 8
	*/
	mem = mem >> 8;

	//Ignora Kernel Threads //TODO //FIXME //HACK
	if(flags & PF_KTHREAD){
		free(comm);
		return 1;
	}

	//CALCOLO USO CPU
	long unsigned uptime;
	FILE* uptime_file = fopen("/proc/uptime", "r");
	if(uptime_file){
		ret = fscanf(uptime_file, "%lu", &uptime);
		if(ret == EOF)
			fprintf(stderr, "parseData: Errore Scanf Uptime");
		else
			cpu_usage = 100 * ((utime + stime) / ticks ) / uptime;
		fclose(uptime_file);
	}
	//fine CPU

	info_set(process_info, pid, comm, flags, (int) cpu_usage, mem); //FIXME
	return 0;
}

info* getProcessInfo(const int dir_fd){
	//Apertura file "stat" nella directory dir_fd (in input). dir_fd = /proc/[pid]
	int stat_fd = openat(dir_fd, "stat", O_RDONLY);
	if(stat_fd == -1){
		perror("getProcessInfo: Errore openat");
		return NULL;
	}

	FILE* stat_file = fdopen(stat_fd, "r");
	if(!stat_file){
		perror("getProcessInfo: Errore fdopen");
		close(stat_fd);
		return NULL;
	}

	info* process_info = info_new();
	if(!process_info){
		fclose(stat_file);
		close(stat_fd);
		return NULL;
	}

	int ret = parseData(stat_file, process_info);
	if(ret){
		fclose(stat_file);
		close(stat_fd);
		return NULL;
	}

	fclose(stat_file);
	close(stat_fd);
	return process_info;
}

int filter(const struct dirent* dir){
	//Seleziona solo le entry relative a processi, ovvero directory che hanno come nome un numero (il PID del processo).
	return dir->d_type == DT_DIR && isNumeric(dir->d_name);
}

info** getProcessesList(int* len){
	//Apertura directory /proc/
	int proc_fd = open("/proc/", O_RDONLY | O_DIRECTORY);
	if(proc_fd == -1){
		perror("GetProcessesList: Errore apertura directory /proc/");
		return NULL;
	}

	//Filtra le directory relative a processi.
	struct dirent** results;
	int procs_n = scandirat(proc_fd, ".", &results, &filter, NULL);
	if (procs_n == -1){
        perror("GetProcessesList: Errore lettura directory /proc/");
		close(proc_fd);
		return NULL;
	}

	//Creazione array per salvare info processi
	info** processes = malloc(procs_n * sizeof(info*));
	if(!processes){
		perror("GetProcessesList: Errore allocazione array processi");
		//Dealloca array dirent
		for(int i = 0 ; i<procs_n; i++)
			free(results[i]);
		free(results);
		close(proc_fd);
		return NULL;
	}

	//Ricava e salva le informazioni di ogni processo.
	for(int i = 0; i<procs_n; i++){
		int pid_fd = openat(proc_fd, results[i]->d_name, O_RDONLY | O_DIRECTORY);
		if(pid_fd == -1){
			fprintf(stderr, "GetProcessesList: Errore Openat processo %d: %s\n", i, strerror(errno));
			processes[i] = NULL;
			continue;
		}
		
		//Lettura info processo
		//NOTE in caso di errore imposta a NULL
		processes[i] = getProcessInfo(pid_fd); 
		
		close(pid_fd);
		//Dealloca singola entry
		free(results[i]);
	}
	free(results);

	//Salva dimensione array processi
	*len = procs_n;
	close(proc_fd);
	return processes;
}