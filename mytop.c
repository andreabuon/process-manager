#include <gtk/gtk.h>

#include "process.h"
#include "list.h"
#include "handlers.h"

#define DEBUG 1

#define APP_NAME "andrea.top"

GtkTreeView *treeview; //sistemare
GtkListStore *liststore; //sistemare

void caricaProcessi(GtkListStore *liststore){
	GtkTreeIter iter;
	List* processList = getProcessesList();

	ListItem* entry = processList->first;
	while(entry){		
		gtk_list_store_append(liststore, &iter);
		info* process = entry->data;
		gtk_list_store_set(liststore, &iter, 0, process->command, 1, process->pid, 2, process->state, 3, process->memory, -1); //sistemare
		entry = entry->next;
	}
	List_free(processList);
}

void aggiornaLista(){
	gtk_tree_view_set_model(treeview, NULL);
	gtk_list_store_clear(liststore);
	caricaProcessi(liststore);
	gtk_tree_view_set_model(treeview, (GtkTreeModel*) liststore);
}

pid_t getSelectedProcessPID(){
	GtkTreeSelection* treeSelection = gtk_tree_view_get_selection(treeview);
	GtkTreeIter iter;
	pid_t pid;
	gtk_tree_selection_get_selected(treeSelection, NULL, &iter);
	gtk_tree_model_get((GtkTreeModel*) liststore, &iter, 1, &pid, -1);
	
	#if DEBUG
		printf("Selected process %d.\n", pid);
	#endif

	return pid;
}

static void activate(GtkApplication *app, gpointer user_data)
{
	GtkBuilder *builder = gtk_builder_new();
	gtk_builder_add_from_file(builder, "mytop.ui", NULL);

	GObject *window = gtk_builder_get_object(builder, "window");
	gtk_window_set_application(GTK_WINDOW(window), app);
	
	treeview = (GtkTreeView*) gtk_builder_get_object(builder, "treeview");
	liststore = gtk_list_store_new(4, G_TYPE_STRING, G_TYPE_INT, G_TYPE_STRING, G_TYPE_UINT);
	
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

int main(int argc, char *argv[])
{
	GtkApplication *app = gtk_application_new(APP_NAME, G_APPLICATION_FLAGS_NONE);
	g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
	
	int status = g_application_run(G_APPLICATION(app), argc, argv);

	g_object_unref(app);

	return status;
}
