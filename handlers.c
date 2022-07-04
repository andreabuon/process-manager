#include <signal.h>
#include <sys/types.h>
#include <stdio.h>

#include "handlers.h"

#define DEBUG 1

//forward declarations
extern pid_t getSelectedProcessPID();
extern void aggiornaLista();

void sendSignal(int signal_n){
	pid_t pid = getSelectedProcessPID();
	int ret;
	ret = kill(pid, signal_n);
	if(ret){
		perror("Errore invio segnale");
	}else{
		#ifdef DEBUG
			printf("Sent signal %d to pid %d.\n", signal_n, pid);
		#endif
	}
	aggiornaLista();
}

void killProcess(){
	sendSignal(SIGKILL);
}

void terminateProcess(){
	sendSignal(SIGTERM);
}

void suspendProcess(){
	sendSignal(SIGSTOP);
}

void resumeProcess(){
	sendSignal(SIGCONT);
}