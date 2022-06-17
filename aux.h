//Struttura dati che contiene le informazioni che ci interessano sul processo
typedef struct info{
	int pid;
	char comm[16];
	char state;
} info;

//Alloca una nuova info
info* info_new();

//Stampa su console l'info in input
void info_print(info* info);

//Dealloca l'info in input
void info_free(info* info);

//Stampa su stdout le informazioni relative al processore
void getCPUinfo();

//Stampa su stdout l'elenco dei processi in esecuzione.
//Viene stampata una riga per ogni cartella nel fs /proc che come nome ha un valore numerico (pid)
void getProcessesList();

//Ritorna una nuova struttura info con le informazioni relative al processo con pid 1
info* getProcessData();

//Controlla se la stringa in input contiene solo numeri
int isNumber(char* string);

//wrapper per mandare segnali ad un processo
//verrà collegato ai pulsanti nella gui
int sendSignal(int signal);