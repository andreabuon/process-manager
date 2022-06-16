#include <fcntl.h>
#include <dirent.h>
#include <assert.h>
#include <stdio.h>
#include <sys/types.h>
#include <ctype.h>

#include <signal.h>

#define CHUNK 1024 //numero di byte letti per volta

//Controlla se la stringa in input contiene solo numeri
int isNumber(char* string){
	while(*string != '\0'){
		if(!isdigit(*string))
			return 0;
		string++;
	}
	return 1;
}

//Stampa su stdout le informazioni relative al processore
void getCPUinfo() {
	FILE *file = fopen("/proc/cpuinfo", "r");
	assert(file && "Errore apertura file");

	char c[CHUNK];
	while (fgets(c, CHUNK, file)){
		printf("%s", c);
	}

	fclose(file);
}

//Stampa su stdout l'elenco dei processi in esecuzione.
//Viene stampata una riga per ogni cartella nel fs /proc che come nome ha un valore numerico (pid)
void getProcessesList(){
	DIR *dir = opendir("/proc/");
	assert(dir && "Errore apertura directory");

	struct dirent *entry;
	entry = readdir(dir);
	assert(entry && "Errore lettura directory");

	while(entry){
		switch (entry->d_type){
			//stampa solo le entry directory che corrispondono ai processi (ovvero che come nome hanno il pid numerico)
			case DT_DIR:
				char *name = entry->d_name;
				if(isNumber(name))
					printf("%s\n", entry->d_name);
				break;
			default:
				break;
		}
		entry = readdir(dir);
	}

	closedir(dir);
}

//Stampa su stdout le informazioni relativa al processo con pid 1 (nel mio caso systemd)
void getProcessData(){
	char path[] = "/proc/1/status";
	FILE *file = fopen(path, "r");
	assert(file && "Errore apertura file");

	char c[CHUNK];
	while (fgets(c, CHUNK, file)){
		printf("%s", c);
	}

	fclose(file);
}

//wrapper per mandare segnali ad un processo
//verrà collegato ai pulsanti nella gui
int sendSignal(int signal){
	//pid_t pid = getSelectedProcess(); ottieni il pid del processo selezionato nella gui (da implementare)
	pid_t pid = 0; // sistemare
	kill(pid, signal);
}

int main(int argc, char *argv[]) {
	//getCPUinfo();
	//getProcessesList();
	//getProcessData();
}

