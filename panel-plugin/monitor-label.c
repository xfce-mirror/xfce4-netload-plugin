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



struct _XnlpMonitorLabel
{
        GtkLabel                parent;
        /*<private>*/
        GtkCssProvider          *css_provider;
        gboolean                short_units;
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
        label->short_units = FALSE;
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

        if (label->short_units)
                return;

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
        gchar *color_str = gdk_rgba_to_string(color);
        css = g_strdup_printf("label { color: %s; }", color_str);
        g_free(color_str);
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


void
xnlp_monitor_label_set_short_units (XnlpMonitorLabel *label, gboolean enabled)
{
        PangoAttrList *attrs = NULL;

        if (label->short_units == enabled)
                return;
        label->short_units = enabled;
        if (enabled)
        {
                attrs = pango_attr_list_new ();
                pango_attr_list_insert (attrs, pango_attr_family_new ("monospace"));
        }
        gtk_label_set_attributes (GTK_LABEL (label), attrs);
        if (attrs != NULL)
                pango_attr_list_unref (attrs);
        gtk_label_set_width_chars (GTK_LABEL (label), enabled ? 4 : -1);
        gtk_label_set_max_width_chars (GTK_LABEL (label), enabled ? 4 : -1);
        gtk_label_set_xalign (GTK_LABEL (label), enabled ? 1.0f : 0.5f);
        xnlp_monitor_label_reinit_size_request (label);
        gtk_label_set_text (GTK_LABEL (label), enabled ? "   -" : "-");
}


void
xnlp_monitor_label_set_font (XnlpMonitorLabel *label, const gchar *font)
{
        PangoAttrList *attrs = NULL;

        if (font != NULL && *font != '\0')
        {
                PangoFontDescription *desc = pango_font_description_from_string (font);
                attrs = pango_attr_list_new ();
                pango_attr_list_insert (attrs, pango_attr_font_desc_new (desc));
                pango_font_description_free (desc);
        }
        else if (label->short_units)
        {
                attrs = pango_attr_list_new ();
                pango_attr_list_insert (attrs, pango_attr_family_new ("monospace"));
        }
        gtk_label_set_attributes (GTK_LABEL (label), attrs);
        if (attrs != NULL)
                pango_attr_list_unref (attrs);
        xnlp_monitor_label_reinit_size_request (label);
        if (!label->short_units)
                cb_label_changed (G_OBJECT (label), NULL, NULL);
}
