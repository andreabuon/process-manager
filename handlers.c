#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include "handlers.h"

//forward declarations
extern pid_t getSelectedProcessPID();
extern void updateTreeView();

void sendSignal(int signal_n){
	pid_t pid = getSelectedProcessPID();
	if(pid<0){
		return;
	}
	int ret = kill(pid, signal_n);
	if(ret){
		perror("Errore invio segnale");
		return;
	}
	#ifdef DEBUG
		printf("Sent signal %d to pid %d.\n", signal_n, pid);
	#endif
	updateTreeView();
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