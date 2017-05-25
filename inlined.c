#include "inlined.h" 
#include "resource.h" 

gboolean init_inlined_objects(void)
{
    BROKENpxb = NULL;
    PNGformat = NULL;
    #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    #pragma GCC diagnostic push
    BROKENpxb = gdk_pixbuf_new_from_inline(-1, inline_broken, FALSE, NULL);
    #pragma GCC diagnostic pop
    if (!GDK_IS_PIXBUF(BROKENpxb))
        return FALSE;
    GSList* formats = gdk_pixbuf_get_formats();
    GSList* f = formats;
    while(f)
    {
        GdkPixbufFormat* format = (GdkPixbufFormat*)f->data;
        if (g_strcmp0(gdk_pixbuf_format_get_name(format), PNG_FORMAT_NAME) == 0)
        {
            PNGformat = format;
            break;
        }
        f = f->next;
    }
    g_slist_free(formats);
    if (!PNGformat)
        return FALSE;
    return TRUE;
}
