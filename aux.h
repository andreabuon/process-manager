//Struttura dati che contiene le informazioni che ci interessano sul processo
typedef struct info{
	int pid;
	char comm[17]; //sistemare, dovrebbe essere 16+1
	char state;
} info;
//Alloca una nuova struttura info
info* info_new();
//Stampa su console i dati del processo salvati nella struttura info in input
void info_print(info* info);
//Dealloca struttura info in input
void info_free(info* info);

//Stampa su console le informazioni relative al processore
void getCPUinfo();

//Stampa su console l'elenco dei processi in esecuzione.
//Viene stampata una riga per ogni cartella nel fs /proc/ che come nome ha un valore numerico (pid)
void getProcessesList();

//Crea una nuova struttura info con le informazioni relative al processo il cui pid è passato in input
info* getProcessInfo(char* pid);

//Controlla se la stringa passata come argomento contiene solo numeri
int isNumber(char* string);

//wrapper per mandare segnali ad un processo
//verrà collegato ai pulsanti nella gui
int sendSignal(int signal);