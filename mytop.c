#include <gtk/gtk.h>
#include "process.h"
#include "list.h"
#include "handlers.h"
//#include <unistd.h>

#define APP_NAME "andrea.top"
#define UI_FILE "mytop.ui"

enum columns_names{
	COLUMN_COMMAND,
	COLUMN_PID,
	COLUMN_STATE,
	COLUMN_MEMORY
};

GtkTreeView *treeview = NULL; //FIXME
GtkListStore *liststore = NULL; //FIXME

//Ottiene la lista dei processi in esecuzione e li carica nella GtkListStore in input
void caricaProcessi(GtkListStore *liststore){
	List* processList = getProcessesList();
	if(!processList){
		fprintf(stderr, "Errore caricamento lista processi.\n");
		return;
	}

	ListItem* entry = processList->first;
	while(entry){
		info* process = entry->data;
		
		GtkTreeIter iter;
		gtk_list_store_append(liststore, &iter);
		gtk_list_store_set(liststore, &iter, COLUMN_COMMAND, process->command, COLUMN_PID, process->pid, COLUMN_STATE, process->state, COLUMN_MEMORY, process->memory, -1); //FIXME
		
		entry = entry->next;
	}
	
	List_free(processList);
}

//Aggiorna TreeView 
void aggiornaLista(){
	if(!treeview) return;
	if(!liststore) return;
	gtk_tree_view_set_model(treeview, NULL); //rimuove il modello corrente
	gtk_list_store_clear(liststore); //elimina tutte le righe dal modello corrente
	caricaProcessi(liststore);
	gtk_tree_view_set_model(treeview, (GtkTreeModel*) liststore); //imposta il modello aggiornato
}

//Ritorna il pid_t del processo selezionato nella ListView. Ritorna -1 se nessuna riga è stata selezionata.
pid_t getSelectedProcessPID(){
	GtkTreeSelection* treeSelection = gtk_tree_view_get_selection(treeview);
	GtkTreeIter iter;
	gboolean res = gtk_tree_selection_get_selected(treeSelection, NULL, &iter);
	if(!res){
		fprintf(stderr, "Nessuna riga selezionata.\n");
		return -1;
	}

	pid_t pid;
	gtk_tree_model_get((GtkTreeModel*) liststore, &iter, 1, &pid, -1);
	
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
	
	liststore = gtk_list_store_new(4, G_TYPE_STRING, G_TYPE_INT, G_TYPE_STRING, G_TYPE_LONG);
	
	GObject *btn_kill = gtk_builder_get_object(builder, "btn_kill");
	g_signal_connect(btn_kill, "clicked", G_CALLBACK(killProcess), NULL);
	GObject *btn_terminate = gtk_builder_get_object(builder, "btn_terminate");
	g_signal_connect(btn_terminate, "clicked", G_CALLBACK(terminateProcess), NULL);
	GObject *btn_suspend = gtk_builder_get_object(builder, "btn_suspend");
	g_signal_connect(btn_suspend, "clicked", G_CALLBACK(suspendProcess), NULL);
	GObject *btn_resume = gtk_builder_get_object(builder, "btn_resume");
	g_signal_connect(btn_resume, "clicked", G_CALLBACK(resumeProcess), NULL);
	GObject *btn_refresh = gtk_builder_get_object(builder, "btn_refresh");
	g_signal_connect(btn_refresh, "clicked", G_CALLBACK(aggiornaLista), NULL);

	aggiornaLista();

	gtk_widget_show(GTK_WIDGET(window));

	g_object_unref(builder);
}

int main(int argc, char *argv[]){
	GtkApplication *app = gtk_application_new(APP_NAME, G_APPLICATION_FLAGS_NONE);
	g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
	
	int status = g_application_run(G_APPLICATION(app), argc, argv);

	g_object_unref(liststore);
	g_object_unref(app);

	return status;
}
