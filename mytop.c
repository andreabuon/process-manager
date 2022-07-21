#include <gtk/gtk.h>
#include <sys/types.h>
#include <signal.h>
#include "mytop.h"
#include "process.h"
#include "handlers.h"

#define APP_NAME "andrea.top"
#define UI_FILE "mytop.ui"

enum columns_names{
	COLUMN_COMMAND = 0,
	COLUMN_PID,
	COLUMN_STATE,
	COLUMN_FLAGS,
	COLUMN_CPU,
	COLUMN_MEMORY,
	COLS_NUM
};
 
//NOTE
static GtkWindow *window = NULL;
static GtkTreeView *treeview = NULL;

//Ottiene la lista dei processi in esecuzione e inserisce i loro dati nella GtkListStore in input
void loadProcessesData(GtkListStore *liststore){
	//Ottieni lista processi
	int size;
	info** processList = getProcessesList(&size);
	if(!processList){
		fprintf(stderr, "%s:  Errore caricamento lista processi.\n", __func__);
		showErrorDialog("Errore caricamento lista processi.");
		return;
	}

	//Scorri l'array e inserisci una riga per ciascun processo
	for(int i=0; i<size; i++){
		info* process = processList[i];
		if(!process) continue;

		GtkTreeIter iter;
		gtk_list_store_append(liststore, &iter);
		gtk_list_store_set(liststore, &iter, COLUMN_COMMAND, process->command, COLUMN_PID, process->pid, COLUMN_STATE, getStateString(process->state), COLUMN_FLAGS, process->flags, COLUMN_CPU, process->cpu_usage, COLUMN_MEMORY, process->memory, -1);
		
		//Dopo averne inserito i dati del modello dealloca l'elemento dell'array
		info_free(process);
	}
	//Infine elimina array
	free(processList);
}

//Carica/aggiorna i dati nella TreeView mantenendo l'ordinamento selezionato.
void updateTreeView(){
	if(!treeview) return;
	
	//Ordinamento di default
	gint column_num = COLUMN_CPU;
	GtkSortType sort_type = GTK_SORT_DESCENDING;

	GtkTreeModel *model =  gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
	GtkListStore* liststore = GTK_LIST_STORE(model);
	if(liststore){
		//Salva l'ordinamento selezionato
		gtk_tree_sortable_get_sort_column_id(GTK_TREE_SORTABLE(liststore), &column_num, &sort_type);
		//Rimuovi dati dal modello precedente
		gtk_tree_view_set_model(treeview, NULL);
		gtk_list_store_clear(liststore);
	}else{
		//Crea nuovo modello dati
		GType tipi_colonne[COLS_NUM] = {
									G_TYPE_STRING, //command
									G_TYPE_INT, //pid
									G_TYPE_STRING, //state
									G_TYPE_LONG, //flags
									G_TYPE_INT, //cpu
									G_TYPE_LONG //memory
									};
		liststore = gtk_list_store_newv(COLS_NUM, tipi_colonne);
	}

	//Carica i dati dei processi
	loadProcessesData(liststore);

	//Ordina i dati
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(liststore), column_num, sort_type);

	//Imposta il nuovo modello ordinato nella TreeView
	gtk_tree_view_set_model(treeview, GTK_TREE_MODEL(liststore));
}

//Costruisce la finestra e i vari elementi. Legge la descrizione dell'interfaccia grafica da file.
static void buildWindow(GtkApplication *app, gpointer user_data){
	GtkBuilder *builder = gtk_builder_new_from_file(UI_FILE);

	window = GTK_WINDOW(gtk_builder_get_object(builder, "window"));
	gtk_window_set_application(GTK_WINDOW(window), app);
	
	treeview = GTK_TREE_VIEW(gtk_builder_get_object(builder, "treeview"));
	gtk_tree_view_set_model(treeview, NULL);
	
	//Assegna funzioni ai pulsanti
	GObject *btn_kill = gtk_builder_get_object(builder, "btn_kill");
	g_signal_connect(btn_kill, "clicked", G_CALLBACK(killProcess), NULL);
	GObject *btn_terminate = gtk_builder_get_object(builder, "btn_terminate");
	g_signal_connect(btn_terminate, "clicked", G_CALLBACK(terminateProcess), NULL);
	GObject *btn_suspend = gtk_builder_get_object(builder, "btn_suspend");
	g_signal_connect(btn_suspend, "clicked", G_CALLBACK(suspendProcess), NULL);
	GObject *btn_resume = gtk_builder_get_object(builder, "btn_resume");
	g_signal_connect(btn_resume, "clicked", G_CALLBACK(resumeProcess), NULL);
	GObject *btn_refresh = gtk_builder_get_object(builder, "btn_refresh");
	g_signal_connect(btn_refresh, "clicked", G_CALLBACK(updateTreeView), NULL);

	gtk_widget_show(GTK_WIDGET(window));
	
	//Carica dati nella finestra
	updateTreeView();

	g_object_unref(builder);
}

int main(int argc, char *argv[]){
	GtkApplication *app = gtk_application_new(APP_NAME, G_APPLICATION_FLAGS_NONE);
	g_signal_connect(app, "activate", G_CALLBACK(buildWindow), NULL);
	
	int status = g_application_run(G_APPLICATION(app), argc, argv);

	g_object_unref(app);
	return status;
}


/***/


void showErrorDialog(char* error){
	GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Errore");
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), error);
	g_signal_connect(dialog, "response", G_CALLBACK(gtk_window_destroy), NULL);
	gtk_widget_show(dialog);
}

void sendSignal(int signal_n){
	if(!treeview) return;
	//Prendi la riga selezionata nella treeview
	GtkTreeSelection* treeSelection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
	GtkTreeModel* model;
	GtkTreeIter iter;
	gboolean res = gtk_tree_selection_get_selected(treeSelection, &model, &iter);
	if(!model){
		fprintf(stderr, "%s: Nessuna modello presente.\n", __func__);
		return;
	}
	if(!res){
		fprintf(stderr, "%s: Nessuna riga selezionata.\n", __func__);
		return;
	}

	//Leggi il valore del PID nella riga
	pid_t pid;
	gtk_tree_model_get(model, &iter, COLUMN_PID, &pid, -1);
	
	#ifdef DEBUG
		printf("Selezionato processo %d.\n", pid);
	#endif

	//Invia segnale al processo
	int ret = kill(pid, signal_n);
	if(ret){
		char* errore = strerror(errno);
		fprintf(stderr, "%s: Errore invio segnale: %s\n", __func__, errore);
		showErrorDialog(errore);
		return;
	}
	
	#ifdef DEBUG
		printf("Inviato segnale %d al processo %d.\n", signal_n, pid);
	#endif

	g_usleep(10000); //NOTE

	//Salva i dati aggiornati del processo.
	//Se il processo non è più presente nel sistema elimina la sua riga dalla lista.
	info* process = getProcessInfo(pid);
	if(!process){
		fprintf(stderr, "%s: Errore lettura info del processo %d.\n", __func__, pid);
		gtk_list_store_remove(GTK_LIST_STORE(model), &iter); //NOTE
		return;
	}
	
	//Inserisci i dati aggiornati nella riga del processo selezionato
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, COLUMN_COMMAND, process->command, COLUMN_PID, process->pid, COLUMN_STATE, getStateString(process->state), COLUMN_FLAGS, process->flags, COLUMN_CPU, process->cpu_usage, COLUMN_MEMORY, process->memory, -1);
	//Elimina i dati del processo
	info_free(process);
}
