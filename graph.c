#include "graph.h"

void container_clear_cb(GtkWidget* widget, gpointer self)
{
    gtk_container_remove(GTK_CONTAINER(self), widget);
}

