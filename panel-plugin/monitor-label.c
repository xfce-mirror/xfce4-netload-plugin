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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>

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
        gint                    count_width;
        gint                    count_height;
        gint                    width;
        gint                    height;
};

G_DEFINE_TYPE (XnlpMonitorLabel, xnlp_monitor_label, GTK_TYPE_LABEL)

static void     xnlp_monitor_label_constructed          (GObject *object);

static void     cb_label_changed                        (GObject *object,
                                                         GParamSpec *pspec,
                                                         gpointer user_data);



static void
xnlp_monitor_label_class_init (XnlpMonitorLabelClass *klass)
{
        GObjectClass *class = G_OBJECT_CLASS (klass);
        xnlp_monitor_label_parent_class = g_type_class_peek_parent (klass);
}

static void
xnlp_monitor_label_init (XnlpMonitorLabel *label)
{
        label->count_width = 0;
        label->count_height = 0;
        label->width = 0;
        label->height = 0;

        g_signal_connect (label, "notify::label", G_CALLBACK (cb_label_changed), NULL);
}



static void
cb_label_changed (GObject *object, GParamSpec *pspec, gpointer user_data)
{
        XnlpMonitorLabel *label = XNLP_MONITOR_LABEL (object);
        GtkWidget *widget = GTK_WIDGET (object);
        GtkRequisition req;

        gtk_widget_set_size_request (widget, -1, -1);
        gtk_widget_size_request (widget, &req);

        if (req.width >= label->width)
        {
                label->width = req.width;
                label->count_width = 0;
        }
        else if (label->count_width > 10)
        {
                label->width = req.width;
                label->count_width = 0;
        }
        else
        {
                req.width = label->width;
                label->count_width++;
        }

        if (req.height >= label->height)
        {
                label->height = req.height;
                label->count_height = 0;
        }
        else if (label->count_height > 10)
        {
                label->height = req.height;
                label->count_height = 0;
        }
        else
        {
                req.height = label->height;
                label->count_height++;
        }

        gtk_widget_set_size_request (label, req.width, req.height);
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
xnlp_monitor_label_reinit_size_request (XnlpMonitorLabel *label)
{
        label->count_width = 0;
        label->count_height = 0;
        label->width = 0;
        label->height = 0;

        gtk_widget_set_size_request (label, -1, -1);
}

