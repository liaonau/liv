#include "luaL.h"
#include "frameL.h"
#include "imageL.h"
#include "util.h"
#include "idle.h"

static int new_frameL(lua_State *L)
{
    frameL* f = (frameL*)lua_newuserdata(L, sizeof(frameL));

    f->frame = (GtkFrame*)gtk_frame_new(NULL);
    g_object_ref_sink(f->frame);
    /*gtk_frame_set_shadow_type((GtkFrame*)frame, GTK_SHADOW_ETCHED_OUT);*/
    gtk_frame_set_shadow_type((GtkFrame*)frame, GTK_SHADOW_NONE);
    
    GtkLabel* label = (GtkLabel*)gtk_frame_get_label_widget((GtkFrame*)f);
    gtk_label_set_max_width_chars(label, 1);
    gtk_label_set_ellipsize(label, PANGO_ELLIPSIZE_START);
    gtk_label_set_line_wrap(label, FALSE);

    f->image = (GtkImage*)gtk_image_new();
    gtk_container_add(GTK_CONTAINER(f->frame), (GtkWidget*)f->image);

    gtk_widget_show_all(f);

    luaL_getmetatable(L, FRAME);
    lua_setmetatable(L, -2);
    return 1;
}

static int get_name_frameL(lua_State* L)
{
    frameL* f = (frameL*)luaL_checkudata(L, 1, FRAME);
    const gchar* name = gtk_widget_get_name((GtkWidget*)f->frame);
    if (name)
        lua_pushstring(L, name);
    return 1;
}
static int set_name_frameL(lua_State* L)
{
    frameL* f = (frameL*)luaL_checkudata(L, 1, FRAME);
    const gchar* name = luaL_checkstring(L, 2);
    gtk_widget_set_name((GtkWidget*)f->frame, name);
    gtk_widget_show_all((GtkWidget*)f->frame);
    return 0;
}
static int get_label_frameL(lua_State* L)
{
    frameL* f = (frameL*)luaL_checkudata(L, 1, FRAME);
    const gchar* label = gtk_frame_get_label(f->frame);
    if (label)
        lua_pushstring(L, label);
    return 1;
}
static int set_label_frameL(lua_State* L)
{
    frameL* f = (frameL*)luaL_checkudata(L, 1, FRAME);
    const gchar* label = lua_tostring(L, 2);
    gtk_frame_set_label(f->frame, label);
    gtk_widget_show_all((GtkWidget*)f->frame);
    return 0;
}

static int load_frameL(lua_State *L)
{
    frameL* f = (frameL*)luaL_checkudata(L, 1, FRAME);
    imageL* i = (imageL*)luaL_checkudata(L, 2, IMAGE);
    gboolean show_deferred = lua_toboolean(L, 3);
    idle_load(L, f->image, i, show_deferred);
    return 0;
}
static int clear_frameL(lua_State *L)
{
    frameL* f = (frameL*)luaL_checkudata(L, 1, FRAME);
    gtk_image_clear(f->image);
    gtk_widget_show((GtkWidget*)f->frame);
    return 0;
}

static int gc_frameL(lua_State *L)
{
    frameL* f = (frameL*)luaL_checkudata(L, 1, FRAME);
    g_object_unref(f->image);
    g_object_unref(f->frame);
    gtk_widget_destroy((GtkWidget*)f->frame);
    return 0;
}
static int index_frameL(lua_State *L)
{
    frameL *s = (frameL*)luaL_checkudata(L, 1, FRAME);
    const gchar* field = luaL_checkstring(L, 2);

    CASE_FUNC(L, field, get_name,   frame);
    CASE_FUNC(L, field, set_name,   frame);
    CASE_FUNC(L, field, get_label,  frame);
    CASE_FUNC(L, field, set_label,  frame);

    CASE_FUNC(L, field, load,       frame);
    CASE_FUNC(L, field, clear,      frame);
    return 1;
}

static const struct luaL_Reg frameLlib_f [] =
{
    {"new", new_frameL},
    {NULL,  NULL      }
};
static const struct luaL_Reg frameLlib_m [] =
{
    {"__gc",    gc_frameL   },
    {"__index", index_frameL},
    {NULL,      NULL        }
};
int luaopen_frameL(lua_State *L, const gchar* name)
{
    luaL_newmetatable(L, FRAME);
    luaL_setfuncs(L, frameLlib_m, 0);
    luaL_newlib(L, frameLlib_f, name);
    return 1;
}
