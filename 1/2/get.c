#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
GtkTextBuffer *buffer;
GtkWidget *text_view;
int main(int argc, char *argv[])
{       
	GtkWidget *window;
	GtkWidget *vbox;
	gtk_init(&argc, &argv);
	/* Create a Window. */
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "get");
	/* Set a decent default size for the window. */
	gtk_window_set_default_size(GTK_WINDOW (window), 200, 200);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_NONE);
	g_signal_connect(G_OBJECT(window), "destroy", 
			G_CALLBACK(gtk_main_quit), NULL);
	vbox = gtk_vbox_new (FALSE, 2);
	gtk_container_add (GTK_CONTAINER (window), vbox);
	/* Create a multiline text widget. */
	text_view = gtk_text_view_new ();
	gtk_box_pack_start (GTK_BOX (vbox), text_view, 1, 1, 0);
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
	gtk_text_buffer_set_text(buffer, " ", -1);
	gtk_text_buffer_set_text(buffer, "hello world", -1);
	gtk_widget_show_all(window);
	gtk_main();
	return 0;
}
