#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include "handlers.h"

extern pid_t getSelectedPID();
extern void updateTreeView();

void sendSignal(int signal_n){
	pid_t pid = getSelectedPID();
	if(pid<0){
		return;
	}
	int ret = kill(pid, signal_n);
	if(ret){
		perror("sendSignal: Errore invio segnale");
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