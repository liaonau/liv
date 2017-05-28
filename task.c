/*
 * Copyright © 2017 Roman Leonov <rliaonau@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "task.h"
#include "util.h"
#include "inlined.h"

#define READ_FRAME_TIME_LOCKED(var, td)        \
{                                              \
    g_rw_lock_reader_lock(&((td)->f->lock));   \
    {                                          \
        var = (td)->f->time;                   \
    }                                          \
    g_rw_lock_reader_unlock(&((td)->f->lock)); \
}
#define RETURN_ERROR_AND_CLEANUP(pxb, canc)    \
{                                              \
    task_frame_pointer_free(pxb);              \
    g_cancellable_cancel(canc);                \
}

static void task_frame_pointer_free(gpointer pointer)
{
    if (pointer)
        g_object_unref(pointer);
}

static void task_frame_cb(GObject* source_object, GAsyncResult* res, gpointer user_data)
{
    GTask* task = G_TASK(res);
    if (!g_task_had_error(task))
    {
        task_frame_t* td = (task_frame_t*)g_task_get_task_data(task);
        gulong time;
        READ_FRAME_TIME_LOCKED(time, td);
        if (time == td->time)
        {
            GdkPixbuf* pxb = (GdkPixbuf*)g_task_propagate_pointer(task, NULL);
            td->func(td->f, pxb);
            gtk_window_set_icon(window, APPpxb);
        }
    }
}

static void task_frame_destroy_notify_task_data(gpointer task_data)
{
    task_frame_t* td = (task_frame_t*)task_data;
    g_free(td);
}

static void task_frame_func(GTask* task, gpointer source_object, gpointer task_data, GCancellable* cancellable)
{
    task_frame_t* td = (task_frame_t*)task_data;
    gulong time;
    READ_FRAME_TIME_LOCKED(time, td);
    if (time == td->time)
    {
        GdkPixbuf* scaledpxb = image_get_scaled_pixbuf(td->i, td->width, td->height);
        READ_FRAME_TIME_LOCKED(time, td);
        if (time == td->time)
        {
            GdkPixbuf* statedpxb = image_get_stated_pixbuf(td->i, scaledpxb, td->state);
            READ_FRAME_TIME_LOCKED(time, td);
            if (time == td->time)
                g_task_return_pointer(task, statedpxb, (GDestroyNotify)task_frame_pointer_free);
            else
                RETURN_ERROR_AND_CLEANUP(statedpxb, cancellable);
        }
        else
            RETURN_ERROR_AND_CLEANUP(scaledpxb, cancellable);
    }
    else
        g_cancellable_cancel(cancellable);
    g_object_unref(cancellable);
}

void task_frame_from_image_pixbuf(frameL* f, imageL* i, gulong time, callback_t func)
{
    task_frame_t* task_data = (task_frame_t*)g_new(task_frame_t, 1);
    task_data->f      = f;
    task_data->i      = i;
    task_data->time   = time;
    task_data->func   = func;
    task_data->width  = i->width;
    task_data->height = i->height;
    task_data->state  = i->state;

    GCancellable* cancellable = g_cancellable_new();
    GTask* task = g_task_new(NULL, cancellable, (GAsyncReadyCallback)task_frame_cb, NULL);
    g_task_set_task_data(task, task_data, (GDestroyNotify)task_frame_destroy_notify_task_data);
    g_task_set_return_on_cancel(task, TRUE);
    g_task_run_in_thread(task, task_frame_func);
    g_object_unref(task);
}


static void task_image_cb(GObject* source_object, GAsyncResult* res, gpointer user_data)
{
    GTask* task = G_TASK(res);
    imageL* i = (imageL*)g_task_get_task_data(task);
    GdkPixbuf* pxb = (GdkPixbuf*)g_task_propagate_pointer(task, NULL);
    i->pxb = pxb;
}

static void task_image_func(GTask* task, gpointer source_object, gpointer task_data, GCancellable* cancellable)
{
    imageL* i = (imageL*)task_data;
    GdkPixbuf* pxb = gdk_pixbuf_new_from_file_at_scale(i->path, i->native_width, i->native_height, FALSE, NULL);
    g_task_return_pointer(task, pxb, NULL);
}

void task_image_pixbuf_from_file(imageL* i)
{
    GTask* task = g_task_new(NULL, NULL, (GAsyncReadyCallback)task_image_cb, NULL);
    g_task_set_task_data(task, i, NULL);
    g_task_run_in_thread(task, task_image_func);
    g_object_unref(task);
}


static void task_pixbuf_cb(GObject* source_object, GAsyncResult* res, gpointer user_data)
{
    GTask* task = G_TASK(res);
    task_pixbuf_t* td = (task_pixbuf_t*)g_task_get_task_data(task);
    GdkPixbuf* pxb = (GdkPixbuf*)g_task_propagate_pointer(task, NULL);
    td->i->pxb = pxb;
}

static void task_pixbuf_destroy_notify_task_data(gpointer task_data)
{
    task_pixbuf_t* td = (task_pixbuf_t*)task_data;
    g_object_unref(td->pxb);
    g_free(td);
}

static void task_pixbuf_func(GTask* task, gpointer source_object, gpointer task_data, GCancellable* cancellable)
{
    task_pixbuf_t* td = (task_pixbuf_t*)task_data;
    GdkPixbuf* pxb = gdk_pixbuf_scale_simple(td->pxb, td->i->native_width, td->i->native_height, GDK_INTERP_BILINEAR);
    g_task_return_pointer(task, pxb, NULL);
}

void task_image_pixbuf_from_pixbuf(imageL* i, GdkPixbuf* pxb)
{
    g_object_ref(pxb);
    task_pixbuf_t* task_data = (task_pixbuf_t*)g_new(task_pixbuf_t, 1);
    task_data->i   = i;
    task_data->pxb = pxb;
    GTask* task = g_task_new(NULL, NULL, (GAsyncReadyCallback)task_pixbuf_cb, NULL);
    g_task_set_task_data(task, task_data, (GDestroyNotify)task_pixbuf_destroy_notify_task_data);
    g_task_run_in_thread(task, task_pixbuf_func);
    g_object_unref(task);
}


static void task_dump_pointer_free(gpointer pointer)
{
    if (pointer)
        g_object_unref(pointer);
}

static void task_dump_cb(GObject* source_object, GAsyncResult* res, gpointer user_data)
{
    GTask* task = G_TASK(res);
    task_dump_t* td = (task_dump_t*)g_task_get_task_data(task);
    if (g_task_had_error(task))
    {
        GError* error = (GError*)g_task_propagate_pointer(task, NULL);
        warn("cannot save «%s». %s", td->path, error->message);
        g_error_free(error);
    }
    else
        info("saved «%s».", td->path);
}

static void task_dump_destroy_notify_task_data(gpointer task_data)
{
    task_dump_t* td = (task_dump_t*)task_data;
    g_free(td->path);
    luaL_unref(td->L, LUA_REGISTRYINDEX, td->ref);
    g_free(td);
}

static void task_dump_func(GTask* task, gpointer source_object, gpointer task_data, GCancellable* cancellable)
{
    task_dump_t* td = (task_dump_t*)task_data;
    GdkPixbuf* scaledpxb = image_get_scaled_pixbuf(td->i, td->width, td->height);
    GdkPixbuf* statedpxb = image_get_stated_pixbuf(td->i, scaledpxb, td->state);
    GError* error = NULL;
    gdk_pixbuf_save(statedpxb, td->path, td->name, &error, NULL);
    if (error != NULL)
        g_task_return_error(task, error);
    else
        g_task_return_pointer(task, statedpxb, (GDestroyNotify)task_dump_pointer_free);
}

void task_dump_image(imageL* i, const gchar* name, const gchar* path, lua_State* L, gint ref)
{
    task_dump_t* task_data = (task_dump_t*)g_new(task_dump_t, 1);
    task_data->i      = i;
    task_data->name   = name;
    task_data->path   = g_strdup(path);
    task_data->width  = i->width;
    task_data->height = i->height;
    task_data->state  = i->state;
    task_data->L      = L;
    task_data->ref    = ref;
    GTask* task = g_task_new(NULL, NULL, (GAsyncReadyCallback)task_dump_cb, NULL);
    g_task_set_task_data(task, task_data, (GDestroyNotify)task_dump_destroy_notify_task_data);
    g_task_run_in_thread(task, task_dump_func);
    g_object_unref(task);
}
