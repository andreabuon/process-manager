#include <gtk/gtk.h>
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
 
//FIXME
GtkWindow *window = NULL;
GtkTreeView *treeview = NULL;

//Ottiene la lista dei processi in esecuzione e inserisce i loro dati nella GtkListStore in input
void loadProcessesData(GtkListStore *liststore){
	//Ottieni lista processi
	int size;
	info** processList = getProcessesList(&size);
	if(!processList){
		fprintf(stderr, "%s:  Errore caricamento lista processi.\n", __func__);
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

	GtkTreeModel* prev_sorted_model = gtk_tree_view_get_model(treeview);
	if(prev_sorted_model){
		//Se presente recupera il modello precedente
		GtkListStore* prev_liststore =  GTK_LIST_STORE(gtk_tree_model_sort_get_model(GTK_TREE_MODEL_SORT(prev_sorted_model)));
		//Salva l'ordinamento selezionato
		gtk_tree_sortable_get_sort_column_id(GTK_TREE_SORTABLE(prev_sorted_model), &column_num, &sort_type);
		//Rimuovi ed elimina il modello precedente
		gtk_tree_view_set_model(treeview, NULL);
		gtk_list_store_clear(prev_liststore);
		g_object_unref(prev_liststore);
	}
	//Creazione nuovo modello dati
	GtkListStore* liststore = gtk_list_store_new(COLS_NUM, G_TYPE_STRING, G_TYPE_INT, G_TYPE_STRING, G_TYPE_LONG, G_TYPE_INT, G_TYPE_LONG);
	//Carica i dati nel modello
	loadProcessesData(liststore);
	//Ordina i dati
	GtkTreeModel* sorted_model = gtk_tree_model_sort_new_with_model(GTK_TREE_MODEL(liststore));
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(sorted_model), column_num, sort_type);
	//Imposta il nuovo modello ordinato nella TreeView
	gtk_tree_view_set_model(treeview, GTK_TREE_MODEL(sorted_model));
}

//Aggiorna i dati del processo selezionato nella TreeView.
void updateRow(){
	//Salva riferimento alla riga selezionata
	GtkTreeSelection* treeSelection = gtk_tree_view_get_selection(treeview);
	GtkTreeModel* model;
	GtkTreeIter iter;
	gboolean res = gtk_tree_selection_get_selected(treeSelection, &model, &iter);
	if(!res){
		fprintf(stderr, "%s: Nessuna riga selezionata.\n", __func__);
	}

	//Leggi dati del processo corrispondente alla riga selezionata
	pid_t pid;
	gtk_tree_model_get(model, &iter, COLUMN_PID, &pid, -1);
	info* process = getProcessInfoByPid(pid);
	if(!process){
		fprintf(stderr, "%s: Errore lettura info del processo %d.\n", __func__, pid);
		gtk_list_store_remove(GTK_LIST_STORE(model), &iter); //TODO
		return;
	}
	//Aggiorna i dati della riga selezionata
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, COLUMN_COMMAND, process->command, COLUMN_PID, process->pid, COLUMN_STATE, getStateString(process->state), COLUMN_FLAGS, process->flags, COLUMN_CPU, process->cpu_usage, COLUMN_MEMORY, process->memory, -1);
	//Elimina i dati del processo
	info_free(process);
}

//Ritorna il pid del processo selezionato nella TreeView. Ritorna -1 se nessuna riga è stata selezionata.
pid_t getSelectedPID(){
	//Salva riferimento alla riga selezionata
	GtkTreeSelection* treeSelection = gtk_tree_view_get_selection(treeview);
	GtkTreeModel* model;
	GtkTreeIter iter;
	gboolean res = gtk_tree_selection_get_selected(treeSelection, &model, &iter);
	if(!res){
		fprintf(stderr, "%s: Nessuna riga selezionata.\n", __func__);
		return -1;
	}
	//Leggi il valore contenuto nella riga
	pid_t pid;
	gtk_tree_model_get(model, &iter, COLUMN_PID, &pid, -1);
	
	#ifdef DEBUG
		printf("Selezionato processo %d.\n", pid);
	#endif
	return pid;
}

//Visualizza una finestra di dialogo con una descrizione dell'errore.
void showErrorDialog(char* error){
	GtkWidget *dialog = gtk_message_dialog_new(window, GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Errore");
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), error);
	g_signal_connect(dialog, "response", G_CALLBACK(gtk_window_destroy), NULL);
	gtk_widget_show(dialog);
}

//Costruisce la finestra e i vari elementi. Legge la descrizione dell'interfaccia grafica da file.
static void buildWindow(GtkApplication *app, gpointer user_data){
	GtkBuilder *builder = gtk_builder_new_from_file(UI_FILE);

	window = GTK_WINDOW(gtk_builder_get_object(builder, "window"));
	gtk_window_set_application(GTK_WINDOW(window), app);
	
	treeview = GTK_TREE_VIEW(gtk_builder_get_object(builder, "treeview"));
	
	//Assegna funzioni ai pulsanti
	GtkButton *btn_kill = GTK_BUTTON(gtk_builder_get_object(builder, "btn_kill"));
	g_signal_connect(btn_kill, "clicked", G_CALLBACK(killProcess), NULL);
	GtkButton *btn_terminate = GTK_BUTTON(gtk_builder_get_object(builder, "btn_terminate"));
	g_signal_connect(btn_terminate, "clicked", G_CALLBACK(terminateProcess), NULL);
	GtkButton *btn_suspend = GTK_BUTTON(gtk_builder_get_object(builder, "btn_suspend"));
	g_signal_connect(btn_suspend, "clicked", G_CALLBACK(suspendProcess), NULL);
	GtkButton *btn_resume = GTK_BUTTON(gtk_builder_get_object(builder, "btn_resume"));
	g_signal_connect(btn_resume, "clicked", G_CALLBACK(resumeProcess), NULL);
	GtkButton *btn_refresh = GTK_BUTTON(gtk_builder_get_object(builder, "btn_refresh"));
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
