#include <signal.h>
#include "handlers.h"
#include "mytop.h"

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