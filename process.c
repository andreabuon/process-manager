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

#define PATH_LEN 255

info* info_new(){
	info* process_info = malloc(sizeof(info));
	if(!process_info){
		fprintf(stderr, "%s: Errore allocazione info: %s\n", __func__, strerror(errno));
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

void info_set(info* info, pid_t pid, char* comm, char state, unsigned flags, int cpu, long mem){
	info->pid = pid;
	info->command = comm;
	info->state = state;
	info->flags = flags;
	info->cpu_usage = cpu;
	info->memory = mem;
}

void info_print(const info* process_info){
	printf("%d %s %c %d %ld\n", process_info->pid, process_info->command, process_info->state, process_info->cpu_usage, process_info->memory);
}

//Esegue il parsing dei dati del processo dal file e li salva nella struttura process_info. Ritorna 0 in caso di successo e 1 in caso di errore.
int parseProcessData(FILE* file, info* process_info){
	int ret;

	pid_t pid = 0; 
	char* comm = NULL;
	char state;
	unsigned flags;
	float cpu_usage = 0;
	long rss;	//memoria residente
	long unsigned utime, stime;	//espresso in clock ticks
	long long unsigned starttime;	//espresso in secondi
	long ticks = sysconf(_SC_CLK_TCK);	//numero clock ticks al secondo

	/* contenuto /proc/[pid]/stat - per altre info consultare "man 5 proc"
	(1) %d pid -> PID del processo
	(2) %s comm -> Nome dell'eseguibile del processo. Uso "%*[(]%m[^)]%*[)]" per togliere le parentesi tonde iniziali e finali. Alloca automaticamente la memoria necessaria per contenere la stringa e il null terminator. Il null terminator viene aggiunto automaticamente. Supporta anche nomi che contengono spazi (a differenza di "%s")
	(3) %c state
	(9) %u flags
	(14) %lu utime
	(15) %lu stime
	(22) %llu starttime  
	(24) %ld rss -> memoria residente
	(-) %* -> valori ignorati
	*/
	char* format_string = "%d %*[(]%m[^)]%*[)] %c %*d %*d %*d %*d %*d %u %*u %*u %*u %*u %lu %lu %*d %*d %*d %*d %*d %*d %llu %*u %ld";
	ret = fscanf(file, format_string, &pid, &comm, &state, &flags, &utime, &stime, &starttime, &rss);
	if(ret==EOF || ret < 8){ //FIXME
		if(ret == EOF)
			fprintf(stderr, "%s: Errore Scanf: %s\n", __func__, strerror(errno));
		else
			fprintf(stderr, "%s: Errore pattern matching: %s\n", __func__, strerror(errno));
		if(comm)
			free(comm);
		return 1;
	}

	//Conversione Memoria da Numero Pagine a MegaBytes
	/*
	la memoria residente è espressa in numero di pagine
	per	convertirla in MB ->  totale kilobytes / numero kilobytes in un MB
	totale kilobyte = numero pagine * 4096 = numero pagine << 12
	numero kilobytes in un MB = 1000 (approssimato a 2^20) 
	quindi memoria [MB] =~  memoria [pagine] >> 8
	*/
	rss = rss >> 8;

	//Calcolo Percentuale Utilizzo CPU - media uso cpu su uptime //FIXME
	long unsigned uptime;
	FILE* uptime_file = fopen("/proc/uptime", "r");
	if(uptime_file){
		ret = fscanf(uptime_file, "%lu", &uptime);
		if(ret == EOF)
			fprintf(stderr, "%s: Errore Scanf Uptime: %s\n", __func__, strerror(errno));
		else
			cpu_usage = 100 * ((utime + stime) / ticks ) / uptime;
		fclose(uptime_file);
	}

	info_set(process_info, pid, comm, state, flags, (int) cpu_usage, rss);
	return 0;
}

info* getProcessInfoByFD(const int dir_fd){
	//Apertura file "stat" nella directory dir_fd (in input). dir_fd = "/proc/[pid]"
	int stat_fd = openat(dir_fd, "stat", O_RDONLY);
	if(stat_fd == -1){
		fprintf(stderr, "%s: Errore Openat: %s\n", __func__, strerror(errno));
		return NULL;
	}
	FILE* stat_file = fdopen(stat_fd, "r");
	if(!stat_file){
		fprintf(stderr, "%s: Errore fdopen: %s\n", __func__, strerror(errno));
		close(stat_fd);
		return NULL;
	}
	//Parsing e salvataggio dei dati del processo
	info* process_info = info_new();
	if(!process_info){
		fclose(stat_file);
		close(stat_fd);
		return NULL;
	}

	int ret = parseProcessData(stat_file, process_info);
	if(ret){
		info_free(process_info);
		fclose(stat_file);
		close(stat_fd);
		return NULL;
	}

	fclose(stat_file);
	close(stat_fd);
	return process_info;
}

info* getProcessInfoByPid(pid_t pid){
	//Compone stringa path file stat del processo
	char path[PATH_LEN];
	snprintf(path, PATH_LEN, "/proc/%d/stat", pid);

	info* process_info = info_new();
	if(!process_info){
		return NULL;
	}

	//Apertura file stat
	FILE* stat_file = fopen(path, "r");
	if(!stat_file){
		fprintf(stderr, "%s: Errore apertura file /proc/%d/stat: %s\n", __func__, pid, strerror(errno));
		info_free(process_info);
		return NULL;
	}
	
	//Parsing e salvataggio dei dati del processo
	int ret = parseProcessData(stat_file, process_info);
	if(ret){
		fprintf(stderr, "%s: Errore parsing dati processo %d.\n", __func__, pid); //FIXME va messa sulla funzione chiamante
		fclose(stat_file);
		info_free(process_info);
		return NULL;
	}

	fclose(stat_file);
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
		fprintf(stderr, "%s: Errore apertura directory /proc/: %s\n", __func__, strerror(errno));
		return NULL;
	}

	//Filtra le directory relative a processi.
	struct dirent** results;
	int procs_n = scandirat(proc_fd, ".", &results, &filter, NULL);
	if (procs_n == -1){
		fprintf(stderr, "%s: Errore scansione directory /proc/: %s\n", __func__, strerror(errno));
		close(proc_fd);
		return NULL;
	}

	//Creazione array per salvare info processi
	info** processes = malloc(procs_n * sizeof(info*));
	if(!processes){
		fprintf(stderr, "%s: Errore allocazione array processi: %s\n", __func__, strerror(errno));
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
			fprintf(stderr, "%s: Errore Openat processo %d: %s\n", __func__, i, strerror(errno));
			processes[i] = NULL;
			continue;
		}
		
		//Lettura info processo
		//NOTE in caso di errore imposta a NULL
		processes[i] = getProcessInfoByFD(pid_fd);
		if(!processes[i])
			fprintf(stderr, "%s: Errore lettura info del processo %s\n", __func__, results[i]->d_name);
		
		close(pid_fd);
		//Dealloca singola entry
		free(results[i]);
	}
	//Dealloca array entries
	free(results);

	//Salva dimensione array processi
	*len = procs_n;

	close(proc_fd);
	return processes;
}

char* getStateString(char s){
	switch(s){
		case 'R':
			return "Running";
		case 'S':
			return "Sleeping";
		case 'D':
			return "Waiting";
		case 'Z':
			return "Zombie";
		case 'T':
			return "Stopped";
		case 't':
			return "Stopped";
		case 'X':
			return "Dead";
		case 'x':
			return "Dead";
		case 'K':
			return "Wavekill";
		case 'W':
			return "Waking";
		case 'P':
			return "Parking";
		//
		case 'I':
			return "Idle";
		default:
			return "Unknown";
	}
}