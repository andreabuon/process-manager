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

info* getProcessInfo(const int dir_fd){
	//Apertura file "stat" nella directory dir_fd (in input). dir_fd = /proc/[pid]
	int stat_fd = openat(dir_fd, "stat", O_RDONLY);
	if(stat_fd == -1){
		perror("getProcessInfo: Errore openat stat_fd");
		return NULL;
	}

	FILE* file = fdopen(stat_fd, "r");
	if(!file){
		perror("getProcessInfo: Errore fdopen file");
		close(stat_fd);
		return NULL;
	}

	pid_t pid = 0; 
	char* comm;
	unsigned flags;
	float cpu_usage;
	long mem; //leggi dopo

	//variabili necessarie per calcore l'uso della cpu
	long unsigned utime, stime; //espresso in clock ticks
	long long unsigned starttime; //espresso in secondi
	long ticks = sysconf(_SC_CLK_TCK); //numero clock ticks al secondo

	/* Parametri SCANF - per altre info consultare "man 5 proc"
	(1) %d pid -> PID del processo
	(2) (%m[^)]) comm -> Nome dell'eseguibile del processo, togliendo la parentesi tonda iniziale e finale. Alloca automaticamente la memoria necessaria per contenere la stringa e il null terminator. Il null terminator viene aggiunto automaticamente. Supporta anche nomi che contengono spazi (al contrario di %s) //FIXME errore su processo (sd-pam)
	(9) %u flags
	(14) %lu utime
	(15) %lu stime
	(22) %llu starttime  
	(-) %* -> valori ignorati
	(24) %ld -> memoria residente
	*/
	int ret;
	ret = fscanf(file, "%d (%m[^)]) %*c %*d %*d %*d %*d %*d %u %*u %*u %*u %*u %lu %lu %*d %*d %*d %*d %*d %*d %llu %*u %ld", &pid, &comm, &flags, &utime, &stime, &starttime, &mem);
	if(ret==EOF || ret < 7){
		if(ret == EOF)
			perror("getProcessInfo: Errore fscanf");
		else
			fprintf(stderr, "getProcessInfo: Errore matching\n");
		fprintf(stderr, "getProcessInfo: Errore lettura info del processo");
		if(pid)
			fprintf(stderr, " [%d]", pid);
		fprintf(stderr, "\n");
		free(comm);
		fclose(file);
		close(stat_fd);
		return NULL;
	}

	fclose(file);
	close(stat_fd);

	//Ignora Kernel Threads //TODO //FIXME //HACK
	if(flags & PF_KTHREAD){
		free(comm);
		return NULL;
	}

	//CALCOLO USO CPU
	long unsigned uptime;
	FILE* uptime_file = fopen("/proc/uptime", "r");
	if(uptime_file){
		ret = fscanf(uptime_file, "%lu", &uptime);
		if(ret == EOF)
			fprintf(stderr, "getProcessInfo: Errore Scanf Uptime");
		else
			cpu_usage = 100 * ((utime + stime) / ticks ) / uptime;
		fclose(uptime_file);
	}
	//fine CPU

	info* process_info = info_new();
	if(!process_info){
		free(comm);
		return NULL;
	}

	/*
	mem è espressa in numero di pagine
	per	convertire mem in MB ->  totale kilobytes / numero kilobytes in un MB
	totale kilobyte = num pagine * 4096 = num pagine << 12
	numero kilobytes in un MB = 1000 (approssimato a 2^20) 
	quindi mem [MB] =~  mem [pag] >> 8
	*/
	mem = mem >> 8;

	info_set(process_info, pid, comm, flags, (int) cpu_usage, mem); //FIXME
	return process_info;
}

int filter(const struct dirent* dir){
	//Seleziona solo le entry relative a processi, ovvero directory che hanno come nome un numero (il PID del processo).
	return dir->d_type == DT_DIR && isNumeric(dir->d_name);
}

info** getProcessesList(int* len){
	//Apertura directory /proc/
	int proc_fd;
	proc_fd = open("/proc/", O_RDONLY | O_DIRECTORY);
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
		info* proc = getProcessInfo(pid_fd); 
		//NOTE in caso di errore proc vale NULL
		processes[i] = proc;
		
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