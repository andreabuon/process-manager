#include <gtk/gtk.h>

#include "process.h"

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

static void activate(GtkApplication *app, gpointer user_data)
{
	/* Construct a GtkBuilder instance and load our UI description */
	GtkBuilder *builder = gtk_builder_new();
	gtk_builder_add_from_file(builder, "mytop.ui", NULL);

	/* Connect signal handlers to the constructed widgets. */
	GObject *window = gtk_builder_get_object(builder, "window");
	gtk_window_set_application(GTK_WINDOW(window), app);
	
	//GObject *btn_kill = gtk_builder_get_object(builder, "btn_kill");
	//g_signal_connect(btn_kill, "clicked", G_CALLBACK(), NULL); //sistemare

	GObject *treeview = gtk_builder_get_object(builder, "treeview");
	GtkListStore *liststore = gtk_list_store_new(4, G_TYPE_STRING, G_TYPE_INT, G_TYPE_CHAR, G_TYPE_INT); //sistemare, non visualizza il carattere ma int corrispondente
	
	aggiungiProcessi(liststore);
	gtk_tree_view_set_model((GtkTreeView*) treeview, (GtkTreeModel*) liststore);

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
