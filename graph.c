#include "graph.h"

void show_widget(GtkWidget* widget)
{
    gtk_container_foreach(GTK_CONTAINER(content), (GtkCallback)gtk_container_remove, content);
    gtk_box_pack_start(content, (GtkWidget*)widget, TRUE, TRUE, 0);
    gtk_widget_show_all((GtkWidget*)content);
}

