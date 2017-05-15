#include "graph.h"

static void clear_cb(GtkWidget* widget, gpointer data)
{
    GtkContainer* box = GTK_CONTAINER(data);
    gtk_container_remove(box, widget);
}

void show_widget(GtkWidget* widget)
{
    gtk_container_foreach(GTK_CONTAINER(content), clear_cb, content);
    gtk_box_pack_start(content, (GtkWidget*)widget, TRUE, TRUE, 0);
}

