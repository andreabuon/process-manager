#include "aux.c"

#include <gtk/gtk.h>

static void activate(GtkApplication *app, gpointer user_data)
{
	/* Construct a GtkBuilder instance and load our UI description */
	GtkBuilder *builder = gtk_builder_new();
	gtk_builder_add_from_file(builder, "mytop0.ui", NULL);

	/* Connect signal handlers to the constructed widgets. */
	GObject *window = gtk_builder_get_object(builder, "window");
	gtk_window_set_application(GTK_WINDOW(window), app);
	
	//GObject *btn_kill = gtk_builder_get_object(builder, "btn_kill");
	//g_signal_connect(btn_kill, "clicked", G_CALLBACK(getCPUinfo), NULL);

	GObject *treeview = gtk_builder_get_object(builder, "treeview");

	GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(treeview), 0, "Processo", renderer, "text", 0, NULL);

	//GtkCellRenderer *renderer1 = gtk_cell_renderer_text_new();
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(treeview), 1, "PID", renderer, "text", 1, NULL);

	//GtkCellRenderer *renderer2 = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(treeview), 2, "Stato", renderer, "text", 2, NULL);

	GtkListStore *liststore = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_CHAR);
	GtkTreeIter iter;
	gtk_list_store_append(liststore, &iter);
	gtk_list_store_set(liststore, &iter, 0, "Processo0", 1, "0", 2, 'R', -1);
	gtk_list_store_append(liststore, &iter);
	gtk_list_store_set(liststore, &iter, 0, "Processo1", 1, "1", 2, 'S', -1);
	gtk_tree_view_set_model((GtkTreeView*) treeview, (GtkTreeModel*) liststore);

	gtk_widget_show(GTK_WIDGET(window));

	/* We do not need the builder any more */
	g_object_unref(builder);
}

int main(int argc, char *argv[])
{
	// getCPUinfo();
	// getProcessesList();

	GtkApplication *app = gtk_application_new("andrea.top", G_APPLICATION_FLAGS_NONE);
	g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

	int status = g_application_run(G_APPLICATION(app), argc, argv);

	g_object_unref(app);

	return status;
}
