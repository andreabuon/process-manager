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

#define PF_KTHREAD 0x00200000 //Kernel threads Flag from /linux/sched.h

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

void info_set(info* info, pid_t pid, char* comm, char* state, unsigned flags, int cpu, long mem){
	info->pid = pid;
	info->command = comm;
	strncpy(info->state, state, STATE_LEN);
	info->flags = flags;
	info->cpu_usage = cpu;
	info->memory = mem;
}

void info_print(const info* process_info){
	printf("%d %s %s %d %ld\n", process_info->pid, process_info->command, process_info->state, process_info->cpu_usage, process_info->memory);
}

info* getProcessInfo(const int dir_fd){
	int stat_fd = openat(dir_fd, "stat", O_RDONLY);
	if(stat_fd == -1){
		perror("getProcessInfo: Errore openat stat_fd");
		return NULL;
	}

	FILE *file = fdopen(stat_fd, "r");
	if(!file){
		perror("getProcessInfo: Errore fdopen file");
		close(stat_fd);
		return NULL;
	}

/*	fscanf legge in ordine:
	(1) %d pid -> PID del processo
	(2) (%m[^)]) comm -> Nome dell'eseguibile del processo, togliendo la parentesi tonda iniziale e finale. Alloca automaticamente la memoria necessaria per contenere la stringa e il null terminator. Il null terminator viene aggiunto automaticamente. Supporta anche nomi che contengono spazi (al contrario di %s)
	(3) %1s state -> Stato del processo. Lo stato è descritto da 1 carattere. Ho usato %s invece di %c in modo da aggiungere automaticamente il null terminator dopo il carattere.
	(9) %u flags
	(-) %* -> valori ignorati
	(24) %ld -> Memoria residente del processo
	per altre info vedere "man 5 proc"
*/
	pid_t pid = 0; 
	char* comm;
	char state[STATE_LEN];
	unsigned flags;
	int cpu_usage = 123454321; //TODO placeholder
	long mem;

	int ret = fscanf(file, "%d (%m[^)]) %1s %*d %*d %*d %*d %*d %u %*u %*u %*u %*u %*u %*u %*d %*d %*d %*d %*d %*d %*u %*u %ld", &pid, &comm, state, &flags, &mem);
	if(ret==EOF || ret < 5){
		if(ret == EOF){
			perror("getProcessInfo: Errore Fscanf");
		}
		printf("getProcessInfo: Errore Lettura info del Processo");
		if(pid)
			printf(" pid: %d.", pid);
		printf("\n");
		free(comm);
		fclose(file);
		close(stat_fd);
		return NULL;
	}

	//ignora kernel threads //TODO //FIXME //HACK
	if(flags & PF_KTHREAD){
		free(comm);
		fclose(file);
		close(stat_fd);
		return NULL;
	}

	info* process_info = info_new();
	if(!process_info){
		free(comm);
		fclose(file);
		close(stat_fd);
		return NULL;
	}

	/*
	mem è espresso in numero di pagine
	per	convertire mem in MB -> numero kilobytes totali / num kilobytes in un MB
	numero kilobyte = num pagine * 4096 (ovvero num pagine << 12)
	num kilobytes in un MB = 1000 (approssimato a 2^20) 
	quindi num megabyte =  mem >> 8
	*/
	info_set(process_info, pid, comm, state, flags, cpu_usage, mem>>8);

	fclose(file);
	close(stat_fd);
	return process_info;
}

int filter(const struct dirent* dir){
	//Seleziona solo le directory che hanno come nome un PID.
	return dir->d_type == DT_DIR && isNumeric(dir->d_name);
}

info** getProcessesList(int* len){
	int proc_fd;
	proc_fd = open("/proc/", O_RDONLY | O_DIRECTORY);
	if(proc_fd == -1){
		perror("GetProcessesList: Errore apertura /proc/");
		return NULL;
	}

	struct dirent** results;
	int procs_n = scandirat(proc_fd, ".", &results, &filter, NULL);
	if (procs_n == -1){
        perror("GetProcessesList: Errore Scandir");
		close(proc_fd);
		return NULL;
	}

	#ifdef DEBUG
		printf("Trovati %d processi.\n", procs_n);
	#endif

	info** processes = malloc(procs_n * sizeof(info*));
	if(!processes){
		perror("GetProcessesList: Errore allocazione array processi");
		
		for(int i = 0 ; i<procs_n; i++){
			free(results[i]);
		}
		free(results);
		close(proc_fd);
		return NULL;
	}

	for(int i = 0; i<procs_n; i++){
		int pid_fd = openat(proc_fd, results[i]->d_name, O_RDONLY | O_DIRECTORY);
		if(pid_fd == -1){
			fprintf(stderr, "GetProcessesList: Errore Openat per il processo %d: %s\n", i, strerror(errno));
			processes[i] = NULL;
			continue;
		}
		
		info* proc = getProcessInfo(pid_fd); 
		//NOTE in caso di errore proc vale NULL
		processes[i] = proc;
		
		close(pid_fd);
		free(results[i]);
	}
	
	close(proc_fd);
	free(results);
	*len = procs_n;
	return processes;
}