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

#ifndef MONITOR_LABEL_H
#define MONITOR_LABEL_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib-object.h>
#include <gtk/gtk.h>

#define XNLP_TYPE_MONITOR_LABEL              (xnlp_monitor_label_get_type ())
#define XNLP_MONITOR_LABEL(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), XNLP_TYPE_MONITOR_LABEL, XnlpMonitorLabel))
#define XNLP_MONITOR_LABEL_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), XNLP_TYPE_MONITOR_LABEL, XnlpMonitorLabelClass))
#define XNLP_IS_MONITOR_LABEL(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XNLP_TYPE_MONITOR_LABEL))
#define XNLP_IS_MONITOR_LABEL_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), XNLP_TYPE_MONITOR_LABEL))
#define XNLP_MONITOR_LABEL_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), XNLP_TYPE_MONITOR_LABEL, XnlpMonitorLabelClass))

typedef struct _XnlpMonitorLabel XnlpMonitorLabel;

GType           xnlp_monitor_label_get_type                  (void);
GtkWidget *     xnlp_monitor_label_new                       (const gchar *str);
void            xnlp_monitor_label_reinit_size_request       (XnlpMonitorLabel *label);

#endif /* !MONITOR_LABEL_H */

