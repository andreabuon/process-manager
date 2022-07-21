#define _GNU_SOURCE 
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include "process.h"
#include "util.h"

#define PATH_LEN 32

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

/*****/

long long unsigned getStatTime(){
	/*	/proc/stat
			  kernel/system statistics.  Varies with architecture.
			  Common entries include:

			  cpu 10132153 290696 3084719 46828483 16683 0 25195 0
			  175628 0
			  cpu0 1393280 32966 572056 13343292 6130 0 17875 0 23933 0
					 The amount of time, measured in units of USER_HZ
					 (1/100ths of a second on most architectures, use
					 sysconf(_SC_CLK_TCK) to obtain the right value),
					 that the system ("cpu" line) or the specific CPU
					 ("cpuN" line) spent in various states:

					 user   (1) Time spent in user mode.

					 nice   (2) Time spent in user mode with low
							priority (nice).

					 system (3) Time spent in system mode.

					 idle   (4) Time spent in the idle task.  This value
							should be USER_HZ times the second entry in
							the /proc/uptime pseudo-file.

					 iowait (since Linux 2.5.41)
							(5) Time waiting for I/O to complete.  This
							value is not reliable, for the following
							reasons:

							1. The CPU will not wait for I/O to
							   complete; iowait is the time that a task
							   is waiting for I/O to complete.  When a
							   CPU goes into idle state for outstanding
							   task I/O, another task will be scheduled
							   on this CPU.

							2. On a multi-core CPU, the task waiting for
							   I/O to complete is not running on any
							   CPU, so the iowait of each CPU is
							   difficult to calculate.

							3. The value in this field may decrease in
							   certain conditions.

					 irq (since Linux 2.6.0)
							(6) Time servicing interrupts.

					 softirq (since Linux 2.6.0)
							(7) Time servicing softirqs.

					 steal (since Linux 2.6.11)
							(8) Stolen time, which is the time spent in
							other operating systems when running in a
							virtualized environment

					 guest (since Linux 2.6.24)
							(9) Time spent running a virtual CPU for
							guest operating systems under the control of
							the Linux kernel.

					 guest_nice (since Linux 2.6.33)
							(10) Time spent running a niced guest
							(virtual CPU for guest operating systems
							under the control of the Linux kernel).
	*/
	long unsigned user, nice, system, idle, iowait, irq, softirq; //steal, guest, guest_nice

	FILE* stat_file = fopen("/proc/stat", "r");
	if(!stat_file){
		fprintf(stderr, "%s: Errore Apertura File /proc/stat: %s\n", __func__, strerror(errno));
		return -1;
	}

	int ret = fscanf(stat_file, "%lu %lu %lu %lu %lu %lu %lu", &user, &nice, &system, &idle, &iowait, &irq, &softirq);
	if(ret == EOF){
		fprintf(stderr, "%s: Errore Scanf Stat: %s\n", __func__, strerror(errno));
		fclose(stat_file);
		return -1;
	}
	fclose(stat_file);
	return user + nice + system + idle + iowait +  irq + softirq; //TODO
}

//Esegue il parsing dei dati del processo dal file e li salva nella struttura process_info. Ritorna 0 in caso di successo e -1 in caso di errore.
int parseProcessData(char* path, info* process_info){
	int ret;
	//
	pid_t pid = 0; 
	char* comm = NULL;
	char state;
	unsigned flags;
	long rss;	//memoria residente

	long unsigned utime1, stime1; 	//espresso in clock ticks
	long unsigned utime2, stime2;	//espresso in clock ticks
	
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
	
	/*------------------------------------LETTURA1--------------------------------*/
	//Apertura file stat
	FILE* file1 = fopen(path, "r");
	if(!file1){
		fprintf(stderr, "%s: Errore apertura file /proc/%d/stat: %s\n", __func__, pid, strerror(errno));
		return -1;
	}
	
	char* format_string = "%d %*[(]%m[^)]%*[)] %c %*d %*d %*d %*d %*d %u %*u %*u %*u %*u %lu %lu %*d %*d %*d %*d %*d %*d %*u %*u %ld";
	ret = fscanf(file1, format_string, &pid, &comm, &state, &flags, &utime1, &stime1, &rss);
	if(ret==EOF){
		fprintf(stderr, "%s: Errore Scanf1: %s\n", __func__, strerror(errno));
		if(comm)
			free(comm);
		return -1;
	}
	fclose(file1);
	
	long long unsigned cpu_time1 = getStatTime();
	if(cpu_time1 == -1){
		fprintf(stderr, "%s: Errore cpu_time1.\n", __func__);
		return -1;
	}

///////

	usleep(5000); //Aspetta un po'

///////

	/*------------------------------------LETTURA2--------------------------------*/
	FILE* file2 = fopen(path, "r");
	if(!file2){
		fprintf(stderr, "%s: Errore apertura file /proc/%d/stat: %s\n", __func__, pid, strerror(errno));
		return -1;
	}

	char* format_string2 = "%*d %*[(]%*[^)]%*[)] %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu %*d %*d %*d %*d %*d %*d %*u %*u %*d";
	ret = fscanf(file2, format_string2, &utime2, &stime2);
	if(ret == EOF){
		fprintf(stderr, "%s: Errore Scanf2: %s\n", __func__, strerror(errno));
		fclose(file2);
		return -1;
	}
	fclose(file2);

	long long unsigned cpu_time2 = getStatTime();
	if(cpu_time1 == -1){
		fprintf(stderr, "%s: Errore cpu_time2.\n", __func__);
		return -1;
	}

	//--------------------------CALCOLO PERCENTUALI---------------------------// 

	long long unsigned time1 = utime1 + stime1;
	long long unsigned time2 = utime2 + stime2;
	long long unsigned interval = cpu_time2 - cpu_time1;

	#ifdef DEBUG
		printf("Processo %d - differenza: %llu, intervallo: %llu .\n", pid, time2-time1, interval);
	#endif

	float cpu_usage;
	if(interval){
		cpu_usage =  100 * ((time2 - time1) / interval);
	}else{
		fprintf(stderr, "%s: Errore - intervallo = 0\n", __func__);
		cpu_usage = -987.1234; //FIXME //PLACEHOLDER
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

	info_set(process_info, pid, comm, state, flags, (int) cpu_usage, rss);
	return 0;
}

info* getProcessInfo(pid_t pid){
	//Compone stringa path file stat del processo
	char path[PATH_LEN];
	int ret = snprintf(path, PATH_LEN, "/proc/%d/stat", pid);
	if(ret < 0 || ret >= PATH_LEN){
		fprintf(stderr, "%s: Errore composizione stringa path.\n", __func__);
		if(ret >= PATH_LEN){
			fprintf(stderr, "Errore %s: Stringa path troppo lunga.\n", __func__);
		}
		return NULL;
	}
	
	//Parsing e salvataggio dei dati del processo
	info* process_info = info_new();
	if(!process_info){
		return NULL;
	}

	ret = parseProcessData(path, process_info);
	if(ret){
		fprintf(stderr, "%s: Errore parsing dati processo %d.\n", __func__, pid);
		info_free(process_info);
		return NULL;
	}

	return process_info;
}

int filter(const struct dirent* dir){
	//Seleziona solo le entry relative a processi, ovvero directory che hanno come nome un numero (il PID del processo).
	return dir->d_type == DT_DIR && isNumeric(dir->d_name);
}

info** getProcessesList(int* len){
	//Scansiona la directory /proc/ e filtra le directory relative a processi.
	struct dirent** directories;
	int procs_n = scandir("/proc/", &directories, &filter, NULL);
	if (procs_n == -1){
		fprintf(stderr, "%s: Errore scansione directory /proc/: %s\n", __func__, strerror(errno));
		return NULL;
	}

	//Creazione array per salvare info processi
	info** processes = malloc(procs_n * sizeof(info*));
	if(!processes){
		fprintf(stderr, "%s: Errore allocazione array processi: %s\n", __func__, strerror(errno));
		//Dealloca array dirent
		for(int i = 0 ; i<procs_n; i++)
			free(directories[i]);
		free(directories);
		return NULL;
	}

	//Ricava e salva le informazioni di ogni processo.
	for(int i = 0; i<procs_n; i++){
		errno = 0; 
		long pid_long = strtol(directories[i]->d_name, NULL, 10); //NOTE
		if(errno || pid_long > INT_MAX || pid_long < INT_MIN){
			fprintf(stderr, "%s: Errore scansione PID del processo %s: %s\n", __func__,  directories[i]->d_name, strerror(errno));
			processes[i] = NULL;
			free(directories[i]);
			continue;
		}
		pid_t pid = (pid_t) pid_long; //NOTE

		//Lettura info processo (in caso di errore imposta a NULL)
		processes[i] = getProcessInfo(pid);
		if(!processes[i]){
			fprintf(stderr, "%s: Errore lettura info del processo %s\n", __func__, directories[i]->d_name);
			free(directories[i]);
			continue;
		}
		
		//Dealloca singola entry
		free(directories[i]);
	}
	
	//Dealloca array entries
	free(directories);

	//Salva dimensione array
	*len = procs_n;

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