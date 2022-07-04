#include <signal.h>
#include <sys/types.h>

#include "handlers.h"

//forward declarations
extern pid_t getSelectedProcessPID();
extern void aggiornaLista();

void killProcess(){
	pid_t pid = getSelectedProcessPID();
	kill(pid, SIGKILL);
	aggiornaLista();
}

void terminateProcess(){
	pid_t pid = getSelectedProcessPID();
	kill(pid, SIGTERM);
	aggiornaLista();
}

void suspendProcess(){
	pid_t pid = getSelectedProcessPID();
	kill(pid, SIGSTOP);
	aggiornaLista();
}

void resumeProcess(){
	pid_t pid = getSelectedProcessPID();
	kill(pid, SIGCONT);
	aggiornaLista();
}
