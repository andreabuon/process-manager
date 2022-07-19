#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "handlers.h"
#include "mytop.h"

void sendSignal(int signal_n){
	pid_t pid = getSelectedPID();
	if(pid<0){
		return;
	}
	
	int ret = kill(pid, signal_n);
	if(ret){
		char* errore = strerror(errno);
		fprintf(stderr, "%s: Errore invio segnale: %s\n", __func__, errore);
		showErrorDialog(errore);
	}
	
	#ifdef DEBUG
		printf("Inviato segnale %d al processo %d.\n", signal_n, pid);
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