#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "handlers.h"

extern pid_t getSelectedPID();
extern void updateRow();
extern void updateTreeView();

void sendSignal(int signal_n){
	pid_t pid = getSelectedPID();
	if(pid<0){
		return;
	}
	int ret = kill(pid, signal_n);
	if(ret){
		fprintf(stderr, "%s: Errore invio segnale: %s\n", __func__, strerror(errno));
	}
	#ifdef DEBUG
		printf("Sent signal %d to pid %d.\n", signal_n, pid);
	#endif
	
	updateRow();
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