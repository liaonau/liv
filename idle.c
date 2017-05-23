#include "idle.h"
#include "imageL.h"
#include "resource.h"
#include "util.h"

static GdkPixbuf* DEFERREDpxb = NULL;

static inline void idle_init_deferred_pxb(void)
{
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#pragma GCC diagnostic push
    if (!DEFERREDpxb)
        DEFERREDpxb = gdk_pixbuf_new_from_inline(-1, inline_deferred, FALSE, NULL);
#pragma GCC diagnostic pop
}
static inline void idle_load_set_assosiated_image(lua_State* L, GtkImage* image, gint* ref)
{
    lua_pushlightuserdata(L, (void*)image);
    if (ref)
        lua_pushnumber(L, *ref);
    else
        lua_pushnil(L);
    lua_settable(L, LUA_REGISTRYINDEX);
}
static gboolean idle_load_gsource_func(gpointer data)
{
    idle_load_t* idle = (idle_load_t*)data;
    lua_State* L = idle->L;
    lua_pushlightuserdata(L, (void*)idle->image);
    lua_gettable(L, LUA_REGISTRYINDEX);
    if (!lua_isnil(L, -1))
    {
        if (lua_tonumber(L, -1) == idle->ref) // или нет смысла загружать
        {
            idle->pxb = image_get_pixbuf(idle->i);
            idle_load_set_assosiated_image(L, idle->image, NULL);
        }
        /*else*/
            /*info("экономия");*/
    }
    lua_pop(L, 1);
    return FALSE;
}
static void idle_load_destroy_notify(gpointer data)
{
    idle_load_t* idle = (idle_load_t*)data;
    lua_State* L = idle->L;
    if (idle->pxb)
    {
        gtk_image_set_from_pixbuf(idle->image, idle->pxb);
        g_object_unref(idle->pxb);
    }
    luaL_unref(L, LUA_REGISTRYINDEX, idle->ref);
    g_free(idle);
}

void idle_load(lua_State* L, GtkImage* image, imageL* i, gboolean show_deferred)
{
    idle_load_t* idle = g_new(idle_load_t, 1);
    idle->L     = L;
    idle->image = image;
    idle->i     = i;
    idle->pxb   = NULL;
    lua_pushlightuserdata(L, (void*)idle);
    idle->ref   = luaL_ref(L, LUA_REGISTRYINDEX);
    idle_load_set_assosiated_image(L, image, &idle->ref);
    if (show_deferred)
    {
        idle_init_deferred_pxb();
        gtk_image_set_from_pixbuf(image, DEFERREDpxb);
    }
    g_idle_add_full(G_PRIORITY_DEFAULT_IDLE, (GSourceFunc)&idle_load_gsource_func, (gpointer)idle, (GDestroyNotify)&idle_load_destroy_notify);
}

static gboolean idle_pixbuf_gsource_func(gpointer data)
{
    idle_pixbuf_t* idle = (idle_pixbuf_t*)data;
    idle->pxb = idle->load_cb(idle->load_source);
    return FALSE;
}
static void idle_pixbuf_destroy_notify(gpointer data)
{
    idle_pixbuf_t* idle = (idle_pixbuf_t*)data;
    if (G_LIKELY(GDK_IS_PIXBUF(idle->pxb)))
        idle->i->pxb = idle->pxb;
    else
        if (idle->fail_cb)
            idle->fail_cb(idle->i);
    g_free(idle);
}

void idle_pixbuf(imageL* i, idle_pixbuf_load_cb load_cb, imageL* load_source, idle_pixbuf_fail_cb fail_cb)
{
    idle_pixbuf_t* idle = g_new(idle_pixbuf_t, 1);
    idle->i           = i;
    idle->load_cb     = load_cb;
    idle->load_source = load_source;
    idle->fail_cb     = fail_cb;
    idle->pxb         = NULL;
    g_idle_add_full(G_PRIORITY_DEFAULT_IDLE, (GSourceFunc)&idle_pixbuf_gsource_func, (gpointer)idle, (GDestroyNotify)&idle_pixbuf_destroy_notify);
}
