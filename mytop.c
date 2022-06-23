#include <gtk/gtk.h>

#include "process.h"
#include <signal.h>

GObject *treeview; //sistemare

void aggiungiProcessi(GtkListStore *liststore){
	GtkTreeIter iter;
	list* processList = getProcessesList();

	listItem* entry = processList->first;
	while(entry){		
		gtk_list_store_append(liststore, &iter);
		gtk_list_store_set(liststore, &iter, 0, entry->proc->command, 1, entry->proc->pid, 2, entry->proc->state, 3, 100, -1);
		entry = entry->next;
	}
}

int getSelectedProcessPID(){
	GtkTreeSelection* treeSelection = gtk_tree_view_get_selection((GtkTreeView*) treeview);
	GtkTreeModel* model;
	GtkTreeIter iter;
	pid_t pid;

	gtk_tree_selection_get_selected(treeSelection, &model, &iter);
	gtk_tree_model_get(model, &iter, 1, &pid, -1);
	//printf("Click! PID: %d", pid);
	//fflush(stdout);
	return pid;
}

void killProcess(){
	int pid = getSelectedProcessPID();
	kill(pid, SIGKILL);
}

void terminateProcess(){
	int pid = getSelectedProcessPID();
	kill(pid, SIGTERM);
}

void suspendProcess(){
	int pid = getSelectedProcessPID();
	kill(pid, SIGSTOP);
}

void resumeProcess(){
	int pid = getSelectedProcessPID();
	kill(pid, SIGCONT);
}

static void activate(GtkApplication *app, gpointer user_data)
{
	/* Construct a GtkBuilder instance and load our UI description */
	GtkBuilder *builder = gtk_builder_new();
	gtk_builder_add_from_file(builder, "mytop.ui", NULL);

	/* Connect signal handlers to the constructed widgets. */
	GObject *window = gtk_builder_get_object(builder, "window");
	gtk_window_set_application(GTK_WINDOW(window), app);
	
	treeview = gtk_builder_get_object(builder, "treeview");
	GtkListStore *liststore = gtk_list_store_new(4, G_TYPE_STRING, G_TYPE_INT, G_TYPE_CHAR, G_TYPE_INT); //sistemare, non visualizza il carattere ma int corrispondente
	
	aggiungiProcessi(liststore);
	gtk_tree_view_set_model((GtkTreeView*) treeview, (GtkTreeModel*) liststore);

	GObject *btn_kill = gtk_builder_get_object(builder, "btn_kill");
	g_signal_connect(btn_kill, "clicked", G_CALLBACK(killProcess), NULL);

	GObject *btn_terminate = gtk_builder_get_object(builder, "btn_terminate");
	g_signal_connect(btn_terminate, "clicked", G_CALLBACK(terminateProcess), NULL);

	GObject *btn_suspend = gtk_builder_get_object(builder, "btn_suspend");
	g_signal_connect(btn_suspend, "clicked", G_CALLBACK(suspendProcess), NULL);

	GObject *btn_resume = gtk_builder_get_object(builder, "btn_resume");
	g_signal_connect(btn_resume, "clicked", G_CALLBACK(resumeProcess), NULL);

	gtk_widget_show(GTK_WIDGET(window));

	/* We do not need the builder any more */
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
