#include <gtk/gtk.h>
#include "process.h"
#include "handlers.h"

#define APP_NAME "andrea.top"
#define UI_FILE "mytop.ui"

enum columns_names{
	COLUMN_COMMAND,
	COLUMN_PID,
	COLUMN_STATE,
	COLUMN_FLAGS,
	COLUMN_CPU,
	COLUMN_MEMORY,
	COLS_NUM
};

GtkTreeView *treeview = NULL; //FIXME

//Ottiene la lista dei processi in esecuzione e li inserisce nella GtkListStore in input
void loadProcesses(GtkListStore *liststore){
	int size;
	info** processList = getProcessesList(&size);
	if(!processList){
		fprintf(stderr, "loadProcesses: Errore caricamento lista processi.\n");
		return;
	}

	for(int i=0; i<size; i++){
		info* process = processList[i];
		if(!process) continue;

		GtkTreeIter iter;
		gtk_list_store_append(liststore, &iter);
		gtk_list_store_set(liststore, &iter, COLUMN_COMMAND, process->command, COLUMN_PID, process->pid, COLUMN_STATE, process->state, COLUMN_FLAGS, process->flags, COLUMN_CPU, process->cpu_usage, COLUMN_MEMORY, process->memory, -1);
		
		info_free(process);
	}
	free(processList);
}

//Aggiorna i dati nella TreeView 
void updateTreeView(){
	if(!treeview) return;
	GtkListStore* old = (GtkListStore*) gtk_tree_view_get_model(treeview);
	gtk_tree_view_set_model(treeview, NULL); //rimuove il modello corrente
	if(old){
		gtk_list_store_clear(old); //elimina tutte le righe dal modello corrente
		g_object_unref(old);
	}
	
	GtkListStore* liststore = gtk_list_store_new(COLS_NUM, G_TYPE_STRING, G_TYPE_INT, G_TYPE_STRING, G_TYPE_LONG, G_TYPE_INT, G_TYPE_UINT);
	loadProcesses(liststore);
	gtk_tree_view_set_model(treeview, (GtkTreeModel*) liststore); //imposta il modello aggiornato
}

//Ritorna il pid del processo selezionato nella TreeView. Ritorna -1 se nessuna riga è stata selezionata.
pid_t getSelectedProcessPID(){
	GtkTreeSelection* treeSelection = gtk_tree_view_get_selection(treeview);
	GtkTreeModel* model;
	GtkTreeIter iter;
	gboolean res = gtk_tree_selection_get_selected(treeSelection, &model, &iter);
	if(!res){
		fprintf(stderr, "getSelectedProcessPID: Nessuna riga selezionata.\n");
		return -1;
	}

	pid_t pid;
	gtk_tree_model_get(model, &iter, COLUMN_PID, &pid, -1);
	
	#ifdef DEBUG
		printf("Selezionato processo %d.\n", pid);
	#endif
	return pid;
}

//Costruisce la finestra e i vari elementi
static void activate(GtkApplication *app, gpointer user_data){
	GtkBuilder *builder = gtk_builder_new_from_file(UI_FILE);

	GObject *window = gtk_builder_get_object(builder, "window");
	gtk_window_set_application(GTK_WINDOW(window), app);
	
	treeview = (GtkTreeView*) gtk_builder_get_object(builder, "treeview");
	
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

	updateTreeView();

	gtk_widget_show(GTK_WIDGET(window));

	g_object_unref(builder);
}

int main(int argc, char *argv[]){
	GtkApplication *app = gtk_application_new(APP_NAME, G_APPLICATION_FLAGS_NONE);
	g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
	
	int status = g_application_run(G_APPLICATION(app), argc, argv);

	g_object_unref(app);

	return status;
}
