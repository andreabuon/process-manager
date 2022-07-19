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
	int size;
	info** processList = getProcessesList(&size);
	if(!processList){
		fprintf(stderr, "%s:  Errore caricamento lista processi.\n", __func__);
		return;
	}

	for(int i=0; i<size; i++){
		info* process = processList[i];
		if(!process) continue;

		GtkTreeIter iter;
		gtk_list_store_append(liststore, &iter);
		gtk_list_store_set(liststore, &iter, COLUMN_COMMAND, process->command, COLUMN_PID, process->pid, COLUMN_STATE, getStateString(process->state), COLUMN_FLAGS, process->flags, COLUMN_CPU, process->cpu_usage, COLUMN_MEMORY, process->memory, -1);
		
		info_free(process);
	}
	free(processList);
}

//Aggiorna i dati nella TreeView 
void updateTreeView(){
	if(!treeview) return;

	GtkListStore* prev_liststore =  GTK_LIST_STORE(gtk_tree_view_get_model(treeview));
	gtk_tree_view_set_model(treeview, NULL); //rimuove il modello corrente
	if(prev_liststore){
		gtk_list_store_clear(prev_liststore); //elimina tutte le righe dal modello precedente
		g_object_unref(prev_liststore);
	}
	
	GtkListStore* liststore = gtk_list_store_new(COLS_NUM, G_TYPE_STRING, G_TYPE_INT, G_TYPE_STRING, G_TYPE_LONG, G_TYPE_INT, G_TYPE_LONG);
	loadProcessesData(liststore);
	gtk_tree_view_set_model(treeview, GTK_TREE_MODEL(liststore)); //imposta il nuovo modello aggiornato
	//gtk_tree_view_expand_all(treeview);
}

//Aggiorna i dati del processo selezionato nella TreeView.
void updateRow(){
	GtkTreeSelection* treeSelection = gtk_tree_view_get_selection(treeview);
	GtkTreeModel* model;
	GtkTreeIter iter;
	gboolean res = gtk_tree_selection_get_selected(treeSelection, &model, &iter);
	if(!res){
		fprintf(stderr, "%s: Nessuna riga selezionata.\n", __func__);
	}

	pid_t pid;
	gtk_tree_model_get(model, &iter, COLUMN_PID, &pid, -1);

	info* process = getProcessInfoByPid(pid);
	if(!process){
		fprintf(stderr, "%s: Errore lettura info del processo %d.\n", __func__, pid);
		gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
		return;
	}
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, COLUMN_COMMAND, process->command, COLUMN_PID, process->pid, COLUMN_STATE, getStateString(process->state), COLUMN_FLAGS, process->flags, COLUMN_CPU, process->cpu_usage, COLUMN_MEMORY, process->memory, -1);
	info_free(process);
}

//Ritorna il pid del processo selezionato nella TreeView. Ritorna -1 se nessuna riga è stata selezionata.
pid_t getSelectedPID(){
	GtkTreeSelection* treeSelection = gtk_tree_view_get_selection(treeview);
	GtkTreeModel* model;
	GtkTreeIter iter;
	gboolean res = gtk_tree_selection_get_selected(treeSelection, &model, &iter);
	if(!res){
		fprintf(stderr, "%s: Nessuna riga selezionata.\n", __func__);
		return -1;
	}

	pid_t pid;
	gtk_tree_model_get(model, &iter, COLUMN_PID, &pid, -1);
	
	#ifdef DEBUG
		printf("Selezionato processo %d.\n", pid);
	#endif
	return pid;
}

void showErrorDialog(char* error){
	GtkWidget *dialog = gtk_message_dialog_new(window, GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Errore");
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), error);
	g_signal_connect(dialog, "response", G_CALLBACK(gtk_window_destroy), NULL);
	gtk_widget_show(dialog);
}

//Costruisce la finestra e i vari elementi
static void buildWindow(GtkApplication *app, gpointer user_data){
	GtkBuilder *builder = gtk_builder_new_from_file(UI_FILE);

	window = GTK_WINDOW(gtk_builder_get_object(builder, "window"));
	gtk_window_set_application(window, app);
	
	treeview = GTK_TREE_VIEW(gtk_builder_get_object(builder, "treeview"));
	
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
