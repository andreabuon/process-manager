#include <gtk/gtk.h>

#include "process.h"
#include "handlers.c"

#define DEBUG 1

GObject *treeview; //sistemare
GtkListStore *liststore; //sistemare

void caricaProcessi(GtkListStore *liststore){
	GtkTreeIter iter;
	list* processList = getProcessesList();

	listItem* entry = processList->first;
	while(entry){		
		gtk_list_store_append(liststore, &iter);
		gtk_list_store_set(liststore, &iter, 0, entry->proc->command, 1, entry->proc->pid, 2, entry->proc->state, 3, entry->proc->memory, -1); //sistemare
		entry = entry->next;
	}
	list_free(processList);
}

void aggiornaProcessi(){
	gtk_tree_view_set_model((GtkTreeView*) treeview, NULL);
	gtk_list_store_clear(liststore);
	caricaProcessi(liststore);
	gtk_tree_view_set_model((GtkTreeView*) treeview, (GtkTreeModel*) liststore);
}

pid_t getSelectedProcessPID(){
	GtkTreeSelection* treeSelection = gtk_tree_view_get_selection((GtkTreeView*) treeview);
	//GtkTreeModel* model;
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
	
	treeview = gtk_builder_get_object(builder, "treeview");
	liststore = gtk_list_store_new(4, G_TYPE_STRING, G_TYPE_INT, G_TYPE_CHAR, G_TYPE_UINT); //sistemare, non visualizza il carattere ma int corrispondente
	
	GObject *btn_kill = gtk_builder_get_object(builder, "btn_kill");
	g_signal_connect(btn_kill, "clicked", G_CALLBACK(killProcess), NULL);
	GObject *btn_terminate = gtk_builder_get_object(builder, "btn_terminate");
	g_signal_connect(btn_terminate, "clicked", G_CALLBACK(terminateProcess), NULL);
	GObject *btn_suspend = gtk_builder_get_object(builder, "btn_suspend");
	g_signal_connect(btn_suspend, "clicked", G_CALLBACK(suspendProcess), NULL);
	GObject *btn_resume = gtk_builder_get_object(builder, "btn_resume");
	g_signal_connect(btn_resume, "clicked", G_CALLBACK(resumeProcess), NULL);
	GObject *btn_refresh = gtk_builder_get_object(builder, "btn_refresh");
	g_signal_connect(btn_refresh, "clicked", G_CALLBACK(aggiornaProcessi), NULL);

	aggiornaProcessi();

	gtk_widget_show(GTK_WIDGET(window));

	g_object_unref(builder);
}

int main(int argc, char *argv[])
{
	GtkApplication *app = gtk_application_new("andrea.top", G_APPLICATION_FLAGS_NONE);
	g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
	
	int status = g_application_run(G_APPLICATION(app), argc, argv);

	g_object_unref(app);

	return status;
}
