#include "conf.h"
#include "opts.h"
#include "util.h"

void parseopts(gint argc, gchar **argv)
{
    GOptionContext      *context;
    const GOptionEntry  entries[] =
    {
        /*{ "onecolumn",      'o', 0,                     G_OPTION_ARG_NONE,   &conf.onecolumn,  "output in one column",                 NULL },*/
        /*{ "checkbox",       'x', 0,                     G_OPTION_ARG_NONE,   &conf.checkbox,   "show checkbox",                        NULL },*/
        /*{ "numbers",        'n', 0,                     G_OPTION_ARG_NONE,   &conf.numbers,    "show numbers of lines",                NULL },*/
        { "numbers",        'n', 0,                     G_OPTION_ARG_NONE,   &conf.numbers,    "show numbers of files",                NULL },
        /*{ "underline",      'u', 0,                     G_OPTION_ARG_NONE,   &conf.underline,  "underline checked lines",              NULL },*/
        /*{ "color",          'c', 0,                     G_OPTION_ARG_NONE,   &conf.color,      "highlight checked lines with color",   NULL },*/
        /*{ "radiobox",       'r', 0,                     G_OPTION_ARG_NONE,   &conf.radiobox,   "only one line can be checked",         NULL },*/
        /*{ "initial",        'i', 0,                     G_OPTION_ARG_NONE,   &conf.initial,    "check all lines initially",            NULL },*/
        /*{ "whitelines",     'w', 0,                     G_OPTION_ARG_NONE,   &conf.whitelines, "do not skip white lines",              NULL },*/
        /*{ "fullattr",       'l', 0,                     G_OPTION_ARG_NONE,   &conf.fullattr,   "draw attributes till the end of line", NULL },*/
        /*{ "not-onecolumn",  'O', G_OPTION_FLAG_REVERSE, G_OPTION_ARG_NONE,   &conf.onecolumn,  "not --onecolumn",                      NULL },*/
        /*{ "not-checkbox",   'X', G_OPTION_FLAG_REVERSE, G_OPTION_ARG_NONE,   &conf.checkbox,   "not --checkbox",                       NULL },*/
        /*{ "not-numbers",    'N', G_OPTION_FLAG_REVERSE, G_OPTION_ARG_NONE,   &conf.numbers,    "not --numbers",                        NULL },*/
        /*{ "not-underline",  'U', G_OPTION_FLAG_REVERSE, G_OPTION_ARG_NONE,   &conf.underline,  "not --underline",                      NULL },*/
        /*{ "not-color",      'C', G_OPTION_FLAG_REVERSE, G_OPTION_ARG_NONE,   &conf.color,      "not --color",                          NULL },*/
        /*{ "not-radiobox",   'R', G_OPTION_FLAG_REVERSE, G_OPTION_ARG_NONE,   &conf.radiobox,   "not --radiobox",                       NULL },*/
        /*{ "not-initial",    'I', G_OPTION_FLAG_REVERSE, G_OPTION_ARG_NONE,   &conf.initial,    "not --initial",                        NULL },*/
        /*{ "not-whitelines", 'W', G_OPTION_FLAG_REVERSE, G_OPTION_ARG_NONE,   &conf.whitelines, "not --whitelines",                     NULL },*/
        /*{ "not-fullattr",   'L', G_OPTION_FLAG_REVERSE, G_OPTION_ARG_NONE,   &conf.fullattr,   "not --fullattr",                       NULL },*/
        /*{ "foreground",     'f', 0,                     G_OPTION_ARG_STRING, &conf.foreground, "foreground color to use to highlight", NULL },*/
        { "background",     'b', 0,                     G_OPTION_ARG_STRING, &conf.background, "background color to use", NULL },
        { NULL,              0,  0,                     0,                   NULL,             NULL,                                   NULL },
    };

    context = g_option_context_new("[FILES]");
    g_option_context_add_main_entries(context, entries, NULL);
    GError* err = NULL;
    if (!g_option_context_parse(context, &argc, &argv, &err))
        fatal("%s\n", err->message);
    g_option_context_free(context);

    conf.execpath   = g_strdup(argv[0]);
    conf.infiles    = argv + 1;
}
