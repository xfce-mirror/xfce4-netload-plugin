/*  XFce 4 - Netload Plugin
 *    Copyright (c) 2003 Bernhard Walle <bernhard.walle@gmx.de>
 * 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>

#include <libxfce4util/i18n.h>
#include <libxfcegui4/dialogs.h>
#include <panel/plugins.h>
#include <panel/xfce.h>

#include "net.h"

/* for xml: */
#define MONITOR_ROOT "Netload"

static GtkTooltips *tooltips = NULL;

extern xmlDocPtr xmlconfig;
#define MYDATA(node) xmlNodeListGetString(xmlconfig, node->children, 1)

/* Defaults */
#define DEFAULT_TEXT "Net"
#define DEFAULT_DEVICE "eth0"

static gchar* DEFAULT_COLOR[] = { "#FF4F00", "#FFE500" };

#define UPDATE_TIMEOUT 250
#define MAX_LENGTH 10

#define IN 0
#define OUT 1
#define SUM 2
#define TOT 2

typedef struct
{
    gboolean use_label;
    GdkColor color[SUM];
    gchar    *label_text;
    gchar    *network_device;
} t_monitor_options;


typedef struct
{
    GtkWidget  *box;
    GtkWidget  *label;
    GtkWidget  *status[SUM];
    GtkWidget  *ebox;

    gulong     history[SUM][4];
    gulong     value_read[SUM];

    t_monitor_options options;

    /*options*/
    GtkBox    *opt_vbox;
    GtkWidget *opt_entry;
    GtkBox    *opt_hbox;
    GtkWidget *net_entry;
    GtkWidget *opt_use_label;
    GtkWidget *opt_button[SUM];
    GtkWidget *opt_da[SUM];
} t_monitor;


typedef struct
{
    GtkWidget         *ebox;
    GtkWidget         *box;
    guint             timeout_id;
    t_monitor         *monitor;

    /* options dialog */
    GtkWidget  *opt_dialog;
} t_global_monitor;



static gint update_monitors(t_global_monitor *global)
{
    static guint64 net_max[SUM] = { 2000, 2000 };
    gchar caption[BUFSIZ];
    guint64 net[SUM+1];
    gint i;

    get_current_netload( global->monitor->options.network_device, 
            &(net[IN]), &(net[OUT]), &(net[TOT]) );
    

    for (i = 0; i < SUM; i++)
    {
        /* correct value to be from 1 ... 100 */
        global->monitor->history[i][0] = (int)((double)net[i] / net_max[i] * 100);

        if (global->monitor->history[i][0] > 100)
        {
            global->monitor->history[i][0] = 100;
        }
        else if (global->monitor->history[i][0] < 0)
        {
            global->monitor->history[i][0] = 0;
        }

        global->monitor->value_read[i] = 
            (global->monitor->history[i][0] + global->monitor->history[i][1] +
             global->monitor->history[i][2] + global->monitor->history[i][3]) / 4;
        global->monitor->history[i][3] = global->monitor->history[i][2];
        global->monitor->history[i][2] = global->monitor->history[i][1];
        global->monitor->history[i][1] = global->monitor->history[i][0];

        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(global->monitor->status[i]),
                global->monitor->value_read[i] / 100.0);

        /* update maximum */
        if (global->monitor->value_read[i] > net_max[i])
        {
            net_max[i] = net[i];
        }
    }
    
    g_snprintf(caption, sizeof(caption), 
            _("Incoming: %lld byte/s\nOutgoing: %lld byte/s\nTotal: %lld byte/s"),
            net[IN], net[OUT], net[TOT]);
    gtk_tooltips_set_tip(tooltips, GTK_WIDGET(global->monitor->ebox), caption, NULL);

    return TRUE;
}

static t_global_monitor * monitor_new(void)
{
    t_global_monitor *global;
    GtkRcStyle *rc;
    gint i;

    global = g_new(t_global_monitor, 1);
    global->timeout_id = 0;
    global->ebox = gtk_event_box_new();
    gtk_widget_show(global->ebox);
    global->box = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(global->box);

    if (!tooltips) 
    {
        tooltips = gtk_tooltips_new();
    }

    global->monitor = g_new(t_monitor, 1);
    global->monitor->options.label_text = g_strdup(DEFAULT_TEXT);
    global->monitor->options.network_device = g_strdup(DEFAULT_DEVICE);
    global->monitor->options.use_label = TRUE;

    for (i = 0; i < SUM; i++)
    {
        gdk_color_parse(DEFAULT_COLOR[i], &global->monitor->options.color[i]);

        global->monitor->history[i][0] = 0;
        global->monitor->history[i][1] = 0;
        global->monitor->history[i][2] = 0;
        global->monitor->history[i][3] = 0;
    }

    global->monitor->ebox = gtk_event_box_new();
    gtk_widget_show(global->monitor->ebox);

    global->monitor->box = GTK_WIDGET(gtk_hbox_new(FALSE, 0));
    gtk_container_set_border_width(GTK_CONTAINER(global->monitor->box),
                                   border_width);
    gtk_widget_show(GTK_WIDGET(global->monitor->box));

    gtk_container_add(GTK_CONTAINER(global->monitor->ebox),
                      GTK_WIDGET(global->monitor->box));

    global->monitor->label =
        gtk_label_new(global->monitor->options.label_text);
    gtk_widget_show(global->monitor->label);
    gtk_box_pack_start(GTK_BOX(global->monitor->box),
                       GTK_WIDGET(global->monitor->label),
                       FALSE, FALSE, 0);

    for (i = 0; i < SUM; i++)
    {
        global->monitor->status[i] = GTK_WIDGET(gtk_progress_bar_new());
        gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(global->monitor->status[i]),
                                     GTK_PROGRESS_BOTTOM_TO_TOP);

        rc = gtk_widget_get_modifier_style(GTK_WIDGET(global->monitor->status[i]));
        if (!rc) 
        {
            rc = gtk_rc_style_new();
        }
        else
        {
            rc->color_flags[GTK_STATE_PRELIGHT] |= GTK_RC_BG;
            rc->bg[GTK_STATE_PRELIGHT] = global->monitor->options.color[i];
        }

        gtk_widget_modify_style(GTK_WIDGET(global->monitor->status[i]), rc);
        gtk_widget_show(GTK_WIDGET(global->monitor->status[i]));

        gtk_box_pack_start(GTK_BOX(global->monitor->box),
                GTK_WIDGET(global->monitor->status[i]),
                FALSE, FALSE, 0);
    }

    gtk_box_pack_start(GTK_BOX(global->box),
                       GTK_WIDGET(global->monitor->ebox),
                       FALSE, FALSE, 0);


    gtk_container_add(GTK_CONTAINER(global->ebox), GTK_WIDGET(global->box));
    
    init_netload();

    return global;
}

static void monitor_set_orientation (Control * ctrl, int orientation)
{
    t_global_monitor *global = ctrl->data;
    GtkRcStyle *rc;
    gint i;

    if (global->timeout_id)
    {
        g_source_remove(global->timeout_id);
    }

    gtk_widget_hide(GTK_WIDGET(global->ebox));
    gtk_container_remove(GTK_CONTAINER(global->ebox), GTK_WIDGET(global->box));
    if (orientation == HORIZONTAL)
    {
        global->box = gtk_hbox_new(FALSE, 0);
    }
    else
    {
        global->box = gtk_vbox_new(FALSE, 0);
    }
    gtk_widget_show(global->box);

    global->monitor->label = gtk_label_new(global->monitor->options.label_text);
    gtk_widget_show(global->monitor->label);

    for (i = 0; i < SUM; i++)
    {
        global->monitor->status[i] = GTK_WIDGET(gtk_progress_bar_new());
    }

    if (orientation == HORIZONTAL)
    {
        global->monitor->box = GTK_WIDGET(gtk_hbox_new(FALSE, 0));
        for (i = 0; i < SUM; i++)
        {
            gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(global->monitor->status[i]),
                    GTK_PROGRESS_BOTTOM_TO_TOP);
        }
    }
    else
    {
        global->monitor->box = GTK_WIDGET(gtk_vbox_new(FALSE, 0));
        for (i = 0; i < SUM; i++)
        {
            gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(global->monitor->status[i]), 
                    GTK_PROGRESS_LEFT_TO_RIGHT);
        }
    }

    gtk_box_pack_start(GTK_BOX(global->monitor->box),
                       GTK_WIDGET(global->monitor->label),
                       FALSE, FALSE, 0);

    gtk_container_set_border_width(GTK_CONTAINER(global->monitor->box),
                                   border_width);
    gtk_widget_show(GTK_WIDGET(global->monitor->box));

    global->monitor->ebox = gtk_event_box_new();
    gtk_widget_show(global->monitor->ebox);
    gtk_container_add(GTK_CONTAINER(global->monitor->ebox),
                      GTK_WIDGET(global->monitor->box));

    for (i = 0; i < SUM; i++)
    {
        rc = gtk_widget_get_modifier_style(GTK_WIDGET(global->monitor->status[i]));
        if (!rc) {
            rc = gtk_rc_style_new();
        }
        if (rc) {
            rc->color_flags[GTK_STATE_PRELIGHT] |= GTK_RC_BG;
            rc->bg[GTK_STATE_PRELIGHT] =
                global->monitor->options.color[i];
        }

        gtk_widget_modify_style(GTK_WIDGET(global->monitor->status[i]), rc);
        gtk_widget_show(GTK_WIDGET(global->monitor->status[i]));

        gtk_box_pack_start(GTK_BOX(global->monitor->box),
                GTK_WIDGET(global->monitor->status[i]), FALSE, FALSE, 0);
    }
    gtk_box_pack_start(GTK_BOX(global->box),
                       GTK_WIDGET(global->monitor->ebox), FALSE, FALSE, 0);


    gtk_container_add(GTK_CONTAINER(global->ebox), GTK_WIDGET(global->box));
    gtk_widget_show(GTK_WIDGET(global->ebox));

    global->timeout_id = g_timeout_add(UPDATE_TIMEOUT, 
            (GtkFunction)update_monitors, global);
}


static gboolean monitor_control_new(Control *ctrl)
{
    t_global_monitor *global;

    global = monitor_new();

    gtk_container_add(GTK_CONTAINER(ctrl->base), GTK_WIDGET(global->ebox));

    if (!global->timeout_id) 
    {
        global->timeout_id = g_timeout_add(UPDATE_TIMEOUT,
                                           (GtkFunction)update_monitors, global);
    }

    ctrl->data = (gpointer)global;
    ctrl->with_popup = FALSE;

    gtk_widget_set_size_request(ctrl->base, -1, -1);

    return TRUE;
}


static void
monitor_free(Control *ctrl)
{
    t_global_monitor *global;

    g_return_if_fail(ctrl != NULL);
    g_return_if_fail(ctrl->data != NULL);

    global = (t_global_monitor *)ctrl->data;

    if (global->timeout_id)
    {
        g_source_remove(global->timeout_id);
    }

    if (global->monitor->options.label_text)
    {
        g_free(global->monitor->options.label_text);
    }
    g_free(global);
    
    close_netload();
}

static void setup_monitor(t_global_monitor *global)
{
    GtkRcStyle *rc;
    gint i;

    gtk_widget_hide(GTK_WIDGET(global->monitor->ebox));
    gtk_widget_hide(global->monitor->label);
    gtk_label_set_text(GTK_LABEL(global->monitor->label),
            global->monitor->options.label_text);

    for (i = 0; i < SUM; i++)
    {
        gtk_widget_hide(GTK_WIDGET(global->monitor->status[i]));
        rc = gtk_widget_get_modifier_style(GTK_WIDGET(global->monitor->status[i]));
        if (!rc) {
            rc = gtk_rc_style_new();
        }
        else
        {
            rc->color_flags[GTK_STATE_PRELIGHT] |= GTK_RC_BG;
            rc->bg[GTK_STATE_PRELIGHT] = global->monitor->options.color[i];
        }

        gtk_widget_modify_style(GTK_WIDGET(global->monitor->status[i]), rc);
        gtk_widget_show(GTK_WIDGET(global->monitor->status[i]));
    }

    gtk_widget_show(GTK_WIDGET(global->monitor->ebox));
    if (global->monitor->options.use_label)
    {
        gtk_widget_show(global->monitor->label);
    }

}

static void monitor_read_config(Control *ctrl, xmlNodePtr node)
{
    xmlChar *value;
    t_global_monitor *global;

    global = (t_global_monitor *)ctrl->data;
    
    if (node == NULL || node->children == NULL)
    {
        return;
    }
    
    for (node = node->children; node; node = node->next)
    {
        if (xmlStrEqual(node->name, (const xmlChar *)MONITOR_ROOT))
        {
            if ((value = xmlGetProp(node, (const xmlChar *)"Use_Label")))
            {
                global->monitor->options.use_label = atoi(value);
                g_free(value);
            }
            if ((value = xmlGetProp(node, (const xmlChar *)"Color_In")))
            {
                gdk_color_parse(value,
                                &global->monitor->options.color[IN]);
                g_free(value);
            }
            if ((value = xmlGetProp(node, (const xmlChar *)"Color_Out")))
            {
                gdk_color_parse(value,
                                &global->monitor->options.color[OUT]);
                g_free(value);
            }
            if ((value = xmlGetProp(node, (const xmlChar *) "Text")))
            {
                if (global->monitor->options.label_text)
                    g_free(global->monitor->options.label_text);
                global->monitor->options.label_text =
                    g_strdup((gchar *)value);
                g_free(value);
            }
            if ((value = xmlGetProp(node, (const xmlChar *) "Network_Device")))
            {
                if (global->monitor->options.network_device)
                    g_free(global->monitor->options.network_device);
                global->monitor->options.network_device =
                    g_strdup((gchar *)value);
                g_free(value);
            }
            break;
        }
    }
    setup_monitor(global);
}




static void monitor_write_config(Control *ctrl, xmlNodePtr parent)
{
    xmlNodePtr root;
    char value[10];
    t_global_monitor *global;

    global = (t_global_monitor *)ctrl->data;

    root = xmlNewTextChild(parent, NULL, MONITOR_ROOT, NULL);

    g_snprintf(value, 2, "%d", global->monitor->options.use_label);
    xmlSetProp(root, "Use_Label", value);

    g_snprintf(value, 8, "#%02X%02X%02X",
               (guint)global->monitor->options.color[IN].red >> 8,
               (guint)global->monitor->options.color[IN].green >> 8,
               (guint)global->monitor->options.color[IN].blue >> 8);

    xmlSetProp(root, "Color_In", value);

    g_snprintf(value, 8, "#%02X%02X%02X",
               (guint)global->monitor->options.color[OUT].red >> 8,
               (guint)global->monitor->options.color[OUT].green >> 8,
               (guint)global->monitor->options.color[OUT].blue >> 8);

    xmlSetProp(root, "Color_Out", value);

    if (global->monitor->options.label_text) 
    {
        xmlSetProp(root, "Text",
                   global->monitor->options.label_text);
    }
    else 
    {
        xmlSetProp(root, "Text", DEFAULT_TEXT);
    }

    if (global->monitor->options.network_device)
    {
        xmlSetProp(root, "Network_Device",
                   global->monitor->options.network_device);
    }
    else
    {
        xmlSetProp(root, "Network_Device", DEFAULT_DEVICE);
    }

    root = xmlNewTextChild(parent, NULL, MONITOR_ROOT, NULL);


}

static void monitor_attach_callback(Control *ctrl, const gchar *signal, GCallback cb,
    gpointer data)
{
    t_global_monitor *global;

    global = (t_global_monitor *)ctrl->data;
    g_signal_connect(global->ebox, signal, cb, data);
}



static void monitor_set_size(Control *ctrl, int size)
{
    /* do the resize */
    t_global_monitor *global;
    gint i;

    global = (t_global_monitor *)ctrl->data;

    for (i = 0; i < SUM; i++)
    {
        if (settings.orientation == HORIZONTAL)
        {
            gtk_widget_set_size_request(GTK_WIDGET(global->monitor->status[i]),
                    6 + 2 * size, icon_size[size]);
        }
        else
        {
            gtk_widget_set_size_request(GTK_WIDGET(global->monitor->status[i]),
                    icon_size[size], 6 + 2 * size);
        }
        gtk_widget_queue_resize(GTK_WIDGET(global->monitor->status[i]));
    }
    setup_monitor(global);
}

static void monitor_apply_options_cb(GtkWidget *button, t_global_monitor *global)
{
    if (global->monitor->options.label_text)
    {
        g_free(global->monitor->options.label_text);
    }

    global->monitor->options.label_text =
        g_strdup(gtk_entry_get_text(GTK_ENTRY(global->monitor->opt_entry)));
    setup_monitor(global);


    if (global->monitor->options.network_device)
    {
        g_free(global->monitor->options.network_device);
    }
    global->monitor->options.network_device =
        g_strdup(gtk_entry_get_text(GTK_ENTRY(global->monitor->net_entry)));
    setup_monitor(global);
}


static void label_changed(GtkWidget *button, t_global_monitor *global)
{ 
    if (global->monitor->options.label_text)
    {
        g_free(global->monitor->options.label_text);
    }

    global->monitor->options.label_text =
        g_strdup(gtk_entry_get_text(GTK_ENTRY(global->monitor->opt_entry)));

    setup_monitor(global);
}

static void network_changed(GtkWidget *button, t_global_monitor *global)
{
    if (global->monitor->options.network_device)
    {
        g_free(global->monitor->options.network_device);
    }

    global->monitor->options.network_device =
        g_strdup(gtk_entry_get_text(GTK_ENTRY(global->monitor->net_entry)));

    setup_monitor(global);
}


static void
label_toggled(GtkWidget *check_button, t_global_monitor *global)
{
    global->monitor->options.use_label =
        !global->monitor->options.use_label;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(global->monitor->opt_use_label),
                                 global->monitor->options.use_label);
    gtk_widget_set_sensitive(GTK_WIDGET(global->monitor->opt_entry),
                             global->monitor->options.use_label);

    setup_monitor(global);
}

static gboolean expose_event_cb(GtkWidget *widget, GdkEventExpose *event)
{
    if (widget->window)
    {
        GtkStyle *style;

        style = gtk_widget_get_style(widget);

        gdk_draw_rectangle(widget->window,
                           style->bg_gc[GTK_STATE_NORMAL],
                           TRUE,
                           event->area.x, event->area.y,
                           event->area.width, event->area.height);
    }

    return TRUE;
}


static void change_color(GtkWidget *button, t_global_monitor *global, gint type)
{
    GtkWidget *dialog;
    GtkColorSelection *colorsel;
    gint response;

    dialog = gtk_color_selection_dialog_new(_("Select color"));
    gtk_window_set_transient_for(GTK_WINDOW(dialog),
                                 GTK_WINDOW(global->opt_dialog));
    colorsel =
        GTK_COLOR_SELECTION(GTK_COLOR_SELECTION_DIALOG(dialog)->colorsel);
    gtk_color_selection_set_previous_color(colorsel,
                                           &global->monitor->options.color[type]);
    gtk_color_selection_set_current_color(colorsel,
                                          &global->monitor->options.color[type]);
    gtk_color_selection_set_has_palette(colorsel, TRUE);

    response = gtk_dialog_run(GTK_DIALOG(dialog));
    if (response == GTK_RESPONSE_OK)
    {
        gtk_color_selection_get_current_color(colorsel,
                                              &global->monitor->options.color[type]);
        gtk_widget_modify_bg(global->monitor->opt_da[type],
                             GTK_STATE_NORMAL,
                             &global->monitor->options.color[type]);
        setup_monitor(global);
    }

    gtk_widget_destroy(dialog);
}


static void change_color_in(GtkWidget *button, t_global_monitor *global)
{
    change_color(button, global, IN);
}

static void change_color_out(GtkWidget *button, t_global_monitor *global)
{
    change_color(button, global, OUT);
}


static void
monitor_create_options(Control *control, GtkContainer *container, GtkWidget *done)
{
    t_global_monitor *global;
    GtkBox           *vbox, *global_vbox;
    GtkBox           *hbox[SUM+1];
    GtkWidget        *device_label;
    GtkWidget        *color_label[SUM];
    GtkWidget        *align;
    GtkSizeGroup     *sg;
    gint             i;
    gchar            *color_text[] = { 
                        N_("Bar color (incoming):"), 
                        N_("Bar color (outgoing):") 
                     };

    global = (t_global_monitor *)control->data;
    global->opt_dialog = gtk_widget_get_toplevel(done);

    global_vbox = GTK_BOX(gtk_vbox_new(FALSE, 5));
    gtk_container_add(GTK_CONTAINER(container), GTK_WIDGET(global_vbox));

    gtk_widget_show_all(GTK_WIDGET(global_vbox));

        vbox = GTK_BOX(gtk_vbox_new(FALSE, 5));
        gtk_widget_show(GTK_WIDGET(vbox));

        global->monitor->opt_vbox = GTK_BOX(gtk_vbox_new(FALSE, 5));
        gtk_widget_show(GTK_WIDGET(global->monitor->opt_vbox));

        global->monitor->opt_hbox = GTK_BOX(gtk_hbox_new(FALSE, 5));
        gtk_widget_show(GTK_WIDGET(global->monitor->opt_hbox));

        global->monitor->opt_use_label =
            gtk_check_button_new_with_mnemonic(_("Text to display:"));
        gtk_widget_show(global->monitor->opt_use_label);
        gtk_box_pack_start(GTK_BOX(global->monitor->opt_hbox),
                           GTK_WIDGET(global->monitor->opt_use_label),
                           FALSE, FALSE, 0);

        global->monitor->opt_entry = gtk_entry_new();
        gtk_entry_set_max_length(GTK_ENTRY(global->monitor->opt_entry),
                                 MAX_LENGTH);
        gtk_entry_set_text(GTK_ENTRY(global->monitor->opt_entry),
                           global->monitor->options.label_text);
        gtk_widget_show(global->monitor->opt_entry);
        gtk_box_pack_start(GTK_BOX(global->monitor->opt_hbox),
                           GTK_WIDGET(global->monitor->opt_entry),
                           FALSE, FALSE, 0);

        gtk_box_pack_start(GTK_BOX(global->monitor->opt_vbox),
                           GTK_WIDGET(global->monitor->opt_hbox),
                           FALSE, FALSE, 0);

        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(global->monitor->opt_use_label),
                                     global->monitor->options.use_label);
        gtk_widget_set_sensitive(GTK_WIDGET(global->monitor->opt_entry),
                                 global->monitor->options.use_label);

        
        /* Network device */
        hbox[0] = GTK_BOX(gtk_hbox_new(FALSE, 5));
        gtk_box_pack_start(GTK_BOX(global->monitor->opt_vbox),
                            GTK_WIDGET(hbox[0]), FALSE, FALSE, 0);

        device_label = gtk_label_new(_("Network device:"));
        gtk_misc_set_alignment(GTK_MISC(device_label), 0, 0.5);
        gtk_widget_show(GTK_WIDGET(device_label));
        gtk_box_pack_start(GTK_BOX(hbox[0]), GTK_WIDGET(device_label),
                           FALSE, FALSE, 0);

        global->monitor->net_entry = gtk_entry_new();
        gtk_entry_set_max_length(GTK_ENTRY(global->monitor->net_entry),
                                 MAX_LENGTH);
        gtk_entry_set_text(GTK_ENTRY(global->monitor->net_entry),
                           global->monitor->options.network_device);
        gtk_widget_show(global->monitor->opt_entry);

        gtk_box_pack_start(GTK_BOX(hbox[0]), GTK_WIDGET(global->monitor->net_entry),
                           FALSE, FALSE, 0);
        
        sg = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
        gtk_size_group_add_widget(sg, global->monitor->opt_use_label);
        gtk_size_group_add_widget(sg, device_label);

        gtk_widget_show_all(GTK_WIDGET(hbox[0]));

        /* Color 1 */
        for (i = 0; i < SUM; i++)
        {
            hbox[i+1] = GTK_BOX(gtk_hbox_new(FALSE, 5));
            gtk_widget_show(GTK_WIDGET(hbox[i+1]));
            gtk_box_pack_start(GTK_BOX(global->monitor->opt_vbox),
                    GTK_WIDGET(hbox[i+1]), FALSE, FALSE, 0);

            color_label[i] = gtk_label_new(_(color_text[i]));
            gtk_misc_set_alignment(GTK_MISC(color_label[i]), 0, 0.5);
            gtk_widget_show(GTK_WIDGET(color_label[i]));
            gtk_box_pack_start(GTK_BOX(hbox[i+1]), GTK_WIDGET(color_label[i]),
                    FALSE, FALSE, 0);

            global->monitor->opt_button[i] = gtk_button_new();
            global->monitor->opt_da[i] = gtk_drawing_area_new();

            gtk_widget_modify_bg(global->monitor->opt_da[i], GTK_STATE_NORMAL,
                    &global->monitor->options.color[i]);
            gtk_widget_set_size_request(global->monitor->opt_da[i], 64, 12);
            gtk_container_add(GTK_CONTAINER(global->monitor->opt_button[i]),
                    global->monitor->opt_da[i]);
            gtk_widget_show(GTK_WIDGET(global->monitor->opt_button[i]));
            gtk_widget_show(GTK_WIDGET(global->monitor->opt_da[i]));
            gtk_box_pack_start(GTK_BOX(hbox[i+1]),
                    GTK_WIDGET(global->monitor->opt_button[i]),
                    FALSE, FALSE, 0);

            gtk_size_group_add_widget(sg, color_label[i]);

            gtk_box_pack_start(GTK_BOX(vbox),
                    GTK_WIDGET(global->monitor->opt_vbox),
                    FALSE, FALSE, 0);
        }


        align = gtk_alignment_new(0, 0, 0, 0);
        gtk_widget_set_size_request(align, 5, 5);
        gtk_widget_show(GTK_WIDGET(align));
        gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(align), FALSE, FALSE, 0);

        gtk_box_pack_start( GTK_BOX(global_vbox), GTK_WIDGET(vbox), FALSE, FALSE, 0);

        g_signal_connect(GTK_WIDGET(global->monitor->opt_da), "expose_event",
                G_CALLBACK(expose_event_cb), NULL);
        g_signal_connect(GTK_WIDGET(global->monitor->opt_button[IN]), "clicked",
                G_CALLBACK(change_color_in), global);
        g_signal_connect(GTK_WIDGET(global->monitor->opt_button[OUT]), "clicked",
                G_CALLBACK(change_color_out), global);
        g_signal_connect(GTK_WIDGET(global->monitor->opt_use_label), "toggled",
                G_CALLBACK(label_toggled), global);
        g_signal_connect(GTK_WIDGET(global->monitor->opt_entry), "activate",
                G_CALLBACK(label_changed), global);
        g_signal_connect(GTK_WIDGET(global->monitor->net_entry), "activate",
                G_CALLBACK(network_changed), global);

        g_signal_connect(GTK_WIDGET(done), "clicked",
                G_CALLBACK(monitor_apply_options_cb), global);

}

G_MODULE_EXPORT void xfce_control_class_init(ControlClass *cc)
{
    xfce_textdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");
    
    cc->name            = "netload";
    cc->caption         = _("Netload");

    cc->create_control  = (CreateControlFunc)monitor_control_new;

    cc->free            = monitor_free;
    cc->read_config     = monitor_read_config;
    cc->write_config    = monitor_write_config;
    cc->attach_callback = monitor_attach_callback;

    cc->create_options  = monitor_create_options;

    cc->set_size        = monitor_set_size;

    cc->set_orientation = monitor_set_orientation;
}


