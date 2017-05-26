#include "luaL.h"
#include "frameL.h"
#include "imageL.h"
#include "util.h"
#include "inlined.h"
/*#include "idle.h"*/

static int new_frameL(lua_State *L)
{
    frameL* f = (frameL*)lua_newuserdata(L, sizeof(frameL));

    f->frame = (GtkFrame*)gtk_frame_new(NULL);
    g_object_ref_sink(f->frame);

    gtk_frame_set_shadow_type((GtkFrame*)f->frame, GTK_SHADOW_NONE);

    f->image = (GtkImage*)gtk_image_new();
    gtk_container_add(GTK_CONTAINER(f->frame), (GtkWidget*)f->image);

    f->ref = LUA_REFNIL;

    gtk_widget_show_all((GtkWidget*)f->frame);

    luaL_getmetatable(L, FRAME);
    lua_setmetatable(L, -2);
    return 1;
}

static void luaH_frame_update(lua_State* L, frameL* f)
{
    lua_rawgeti(L, LUA_REGISTRYINDEX, f->ref);
    if (lua_isuserdata(L, -1))
    {
        imageL* i = (imageL*)luaL_checkudata(L, -1, IMAGE);
        GdkPixbuf* pxb = image_get_pixbuf(i);
        if (!GDK_IS_PIXBUF(pxb))
        {
            pxb = BROKENpxb;
            g_object_ref(pxb);
        }
        lua_pop(L, 1);
        gtk_image_set_from_pixbuf(f->image, pxb);
        g_object_unref(pxb);
    }
    gtk_widget_show_all((GtkWidget*)f->frame);
    /*gboolean show_deferred = lua_toboolean(L, 3);*/
    /*idle_load(L, f->image, i, show_deferred);*/
}

static int image_get_frameL(lua_State* L)
{
    frameL* f = (frameL*)luaL_checkudata(L, 1, FRAME);
    lua_rawgeti(L, LUA_REGISTRYINDEX, f->ref);
    return 1;
}
static int image_set_frameL(lua_State* L)
{
    frameL* f = (frameL*)luaL_checkudata(L, 1, FRAME);
    luaL_unref(L, LUA_REGISTRYINDEX, f->ref);
    if (lua_isuserdata(L, 2))
    {
        luaL_checkudata(L, 2, IMAGE);
        lua_pushvalue(L, 2);
        f->ref = luaL_ref(L, LUA_REGISTRYINDEX);
    }
    else
        f->ref = LUA_REFNIL;
    luaH_frame_update(L, f);
    return 0;
}

static int name_get_frameL(lua_State* L)
{
    frameL* f = (frameL*)luaL_checkudata(L, 1, FRAME);
    const gchar* name = gtk_widget_get_name((GtkWidget*)f->frame);
    if (name)
        lua_pushstring(L, name);
    return 1;
}
static int name_set_frameL(lua_State* L)
{
    frameL* f = (frameL*)luaL_checkudata(L, 1, FRAME);
    const gchar* name = luaL_checkstring(L, 2);
    gtk_widget_set_name((GtkWidget*)f->frame, name);
    gtk_widget_show_all((GtkWidget*)f->frame);
    return 0;
}
static int label_get_frameL(lua_State* L)
{
    frameL* f = (frameL*)luaL_checkudata(L, 1, FRAME);
    const gchar* label = gtk_frame_get_label(f->frame);
    if (label)
        lua_pushstring(L, label);
    return 1;
}
static int label_set_frameL(lua_State* L)
{
    frameL* f = (frameL*)luaL_checkudata(L, 1, FRAME);
    const gchar* text = lua_tostring(L, 2);
    if (text)
    {
        gtk_frame_set_label(f->frame, "");
        GtkLabel* label = (GtkLabel*)gtk_frame_get_label_widget(f->frame);
        gtk_label_set_markup(label, text);
        /*GtkWidgetPath* p = gtk_widget_get_path((GtkWidget*)label);*/
        /*gchar* ps = gtk_widget_path_to_string(p);*/
        /*info("%s", ps);*/
        /*g_free(ps);*/
    }
    else
        gtk_frame_set_label(f->frame, text);
    gtk_widget_show_all((GtkWidget*)f->frame);
    return 0;
}

static int size_request_frameL(lua_State* L)
{
    frameL* f = (frameL*)luaL_checkudata(L, 1, FRAME);
    gint w = luaL_checkint(L, 2);
    gint h = luaL_checkint(L, 3);
    if (w < 1 || h < 1)
    {
        w = -1;
        h = -1;
    }
    /*luaH_frame_update(L, f);*/
    gtk_widget_set_size_request((GtkWidget*)f->image, w, h);
    gtk_widget_show_all((GtkWidget*)f->frame);
    return 0;
}
static int preferred_size_frameL(lua_State* L)
{
    frameL* f = (frameL*)luaL_checkudata(L, 1, FRAME);
    GtkRequisition nat;
    gtk_widget_get_preferred_size((GtkWidget*)f->frame, NULL, &nat);
    lua_pushnumber(L, nat.width);
    lua_pushnumber(L, nat.height);
    return 2;
}
static int class_add_frameL(lua_State* L)
{
    frameL* f = (frameL*)luaL_checkudata(L, 1, FRAME);
    const gchar* class = luaL_checkstring(L, 2);
    GtkStyleContext* context = gtk_widget_get_style_context((GtkWidget*)f->frame);
    gtk_style_context_add_class(context, class);
    return 0;
}
static int class_remove_frameL(lua_State* L)
{
    frameL* f = (frameL*)luaL_checkudata(L, 1, FRAME);
    const gchar* class = luaL_checkstring(L, 2);
    GtkStyleContext* context = gtk_widget_get_style_context((GtkWidget*)f->frame);
    gtk_style_context_remove_class(context, class);
    return 0;
}

static int gc_frameL(lua_State *L)
{
    frameL* f = (frameL*)luaL_checkudata(L, 1, FRAME);
    luaL_unref(L, LUA_REGISTRYINDEX, f->ref);
    g_object_unref(f->frame);
    return 0;
}
static int tostring_frameL(lua_State *L)
{
    luaL_checkudata(L, 1, FRAME);
    lua_pushstring(L, FRAME);
    return 1;
}
static int index_frameL(lua_State *L)
{
    luaL_checkudata(L, 1, FRAME);
    const gchar* field = luaL_checkstring(L, 2);

    INDEX_FIELD(image,  frame);

    INDEX_FIELD(name,   frame);
    INDEX_FIELD(label,  frame);

    CASE_FUNC(size_request,   frame);
    CASE_FUNC(preferred_size, frame);
    CASE_FUNC(class_add,      frame);
    CASE_FUNC(class_remove,   frame);

    return 1;
}
static int newindex_frameL(lua_State *L)
{
    luaL_checkudata(L, 1, FRAME);
    const gchar* field = luaL_checkstring(L, 2);

    NEWINDEX_FIELD(image,  frame);

    NEWINDEX_FIELD(name,   frame);
    NEWINDEX_FIELD(label,  frame);

    return 0;
}

static const struct luaL_Reg frameLlib_f [] =
{
    {"new", new_frameL},
    {NULL,  NULL      }
};
static const struct luaL_Reg frameLlib_m [] =
{
    {"__gc",       gc_frameL      },
    {"__index",    index_frameL   },
    {"__newindex", newindex_frameL},
    {"__tostring", tostring_frameL},
    {NULL,         NULL           }
};
int luaopen_frameL(lua_State *L)
{
    luaL_newmetatable(L, FRAME);
    luaL_setfuncs(L, frameLlib_m, 0);
    luaL_newlib(L, frameLlib_f, FRAME);
    return 1;
}
