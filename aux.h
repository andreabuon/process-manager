//Struttura dati che contiene le informazioni che interessano del processo
typedef struct info{
	int pid;
	char comm[17];
	char state;
} info;

//Alloca una nuova struttura info e ne restituisce il puntatore
info* info_new();
//Dealloca la struttura info puntata dal puntatore in input
void info_free(info* process_info);
//Stampa su console i dati del processo salvati nella struttura info in input
void info_print(const info* process_info);

//Stampa su console le informazioni relative al processore
void getCPUinfo();

//Stampa su console l'elenco dei processi in esecuzione.
void getProcessesList();

//Crea una nuova struttura info con le informazioni relative al processo (il cui pid è passato in input) e ne ritorna il puntatore.
info* getProcessInfo(const char* pid);

//Controlla se la stringa passata come argomento è composta esclusivamente da numeri
int isNumber(const char* string);

//wrapper per mandare segnali ad un processo
//verrà collegato ai pulsanti nella gui
int sendSignal(int signal);