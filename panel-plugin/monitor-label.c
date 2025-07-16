/*
 * Copyright (c) 2012 Mike Massonnet <mmassonnet@xfce.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * See the file COPYING for the full license text.
 */

#include <gtk/gtk.h>
#include <libxfce4util/libxfce4util.h>

#include "monitor-label.h"



typedef struct _XnlpMonitorLabelClass XnlpMonitorLabelClass;
struct _XnlpMonitorLabelClass
{
        GtkLabelClass           parent_class;
};
struct _XnlpMonitorLabel
{
        GtkLabel                parent;
        /*<private>*/
        GtkCssProvider          *css_provider;
        gint                    width;
        gint                    height;
};

G_DEFINE_TYPE (XnlpMonitorLabel, xnlp_monitor_label, GTK_TYPE_LABEL)

static void     cb_label_changed                        (GObject *object,
                                                         GParamSpec *pspec,
                                                         gpointer user_data);



static void
xnlp_monitor_label_class_init (XnlpMonitorLabelClass *klass)
{
        xnlp_monitor_label_parent_class = g_type_class_peek_parent (klass);
}

static void
xnlp_monitor_label_init (XnlpMonitorLabel *label)
{
        label->width = 0;
        label->height = 0;
        label->css_provider = gtk_css_provider_new ();
        gtk_style_context_add_provider (
            GTK_STYLE_CONTEXT (gtk_widget_get_style_context (GTK_WIDGET (label))),
            GTK_STYLE_PROVIDER (label->css_provider),
            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

        g_signal_connect (label, "notify::label", G_CALLBACK (cb_label_changed), NULL);
}



static void
cb_label_changed (GObject *object, GParamSpec *pspec, gpointer user_data)
{
        XnlpMonitorLabel *label = XNLP_MONITOR_LABEL (object);
        GtkWidget *widget = GTK_WIDGET (object);
        GtkRequisition minimum_size;
        GtkRequisition natural_size;

        gtk_widget_set_size_request (widget, -1, -1);
        gtk_widget_get_preferred_size (widget, &minimum_size, &natural_size);

        if (minimum_size.width >= label->width)
        {
                label->width = minimum_size.width;
        }
        else
        {
                minimum_size.width = label->width;
        }

        if (minimum_size.height >= label->height)
        {
                label->height = minimum_size.height;
        }
        else
        {
                minimum_size.height = label->height;
        }

        gtk_widget_set_size_request (widget, minimum_size.width, minimum_size.height);
}



GtkWidget *
xnlp_monitor_label_new (const gchar *str)
{
        GtkLabel *label;

        label = g_object_new (XNLP_TYPE_MONITOR_LABEL, NULL);

        if (str && *str)
                gtk_label_set_text (label, str);

        return GTK_WIDGET (label);
}

void
xnlp_monitor_label_set_color (XnlpMonitorLabel *label, GdkRGBA* color)
{
    gchar * css;
    if (color != NULL)
    {
        css = g_strdup_printf("label { color: %s; }",
                              gdk_rgba_to_string(color));
    }
    else
    {
        css = g_strdup_printf("label { color: inherit; }");
    }
    gtk_css_provider_load_from_data (label->css_provider, css, strlen(css), NULL);
    DBG("setting label css: %s", gtk_css_provider_to_string (label->css_provider));
    g_free(css);

}

void
xnlp_monitor_label_reinit_size_request (XnlpMonitorLabel *label)
{
        label->width = 0;
        label->height = 0;

        gtk_widget_set_size_request (GTK_WIDGET (label), -1, -1);
}
