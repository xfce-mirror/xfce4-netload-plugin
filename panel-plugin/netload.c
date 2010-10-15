/*
 * Id: $Id$
 * -------------------------------------------------------------------------------------------------
 * 
 * This program is free software; you can redistribute it and/or modify it under the terms of the 
 * GNU General Public License as published by the Free Software Foundation; You may only use 
 * version 2 of the License, you have no option to use any other version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See 
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program; if 
 * not, write to the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * ------------------------------------------------------------------------------------------------- 
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "net.h"
#include "utils.h"
#include "global.h"

#include <gtk/gtk.h>

#include <libxfce4util/libxfce4util.h>
#include <libxfcegui4/libxfcegui4.h>
#include <libxfce4panel/xfce-panel-plugin.h>


static GtkTooltips *tooltips = NULL;

#define BORDER 8

/* Defaults */
#define DEFAULT_TEXT "Net"

#define INIT_MAX 4096
#define MINIMAL_MAX 1024
#define SHRINK_MAX 0.75

#define HISTSIZE_CALCULATE 4
#define HISTSIZE_STORE 20

static gchar* DEFAULT_COLOR[] = { "#FF4F00", "#FFE500" };

#define UPDATE_TIMEOUT 250
#define MAX_LENGTH 10

#define IN 0
#define OUT 1
#define TOT 2
#define SUM 2

#define APP_NAME N_("Xfce4-Netload-Plugin")

static char *errormessages[] = {
    N_("Unknown error."),
    N_("Linux proc device '/proc/net/dev' not found."),
    N_("Interface was not found.")
};

typedef struct
{
    gboolean use_label;
    gboolean auto_max;
    gulong   max[SUM];
    gint     update_interval;
    GdkColor color[SUM];
    gchar    *label_text;
    gchar    *network_device;
    gchar    *old_network_device;
} t_monitor_options;


typedef struct
{
    GtkWidget  *box;
    GtkWidget  *label;
    GtkWidget  *status[SUM];

    gulong     history[SUM][HISTSIZE_STORE];
    gulong     net_max[SUM];
    
    t_monitor_options options;
    
    /* for the network part */
    netdata    data;

    /* Displayed text */
    GtkBox    *opt_vbox;
    GtkWidget *opt_entry;
    GtkBox    *opt_hbox;
    GtkWidget *opt_use_label;
    
    /* Update interval */
    GtkWidget *update_spinner;
    
    /* Network device */
    GtkWidget *net_entry;
    
    /* Maximum */
    GtkWidget *max_use_label;
    GtkWidget *max_entry[SUM];
    GtkBox    *max_hbox[SUM];
    
    /* Color */
    GtkWidget *opt_button[SUM];
    GtkWidget *opt_da[SUM];
    
} t_monitor;


typedef struct
{
    XfcePanelPlugin *plugin;

    GtkWidget         *ebox;
    GtkWidget         *box;
    guint             timeout_id;
    t_monitor         *monitor;

    /* options dialog */
    GtkWidget  *opt_dialog;
} t_global_monitor;


/* ---------------------------------------------------------------------------------------------- */
static gboolean update_monitors(t_global_monitor *global)
{
    char buffer[SUM+1][BUFSIZ];
    gchar caption[BUFSIZ];
    gulong net[SUM+1];
    gulong display[SUM+1], max;
    guint64 histcalculate;
    double temp;
    gint i, j;

    if (!get_interface_up(&(global->monitor->data)))
    {
        g_snprintf(caption, sizeof(caption), 
                _("<< %s >> (Interface down)"),
                    get_name(&(global->monitor->data)));
        gtk_tooltips_set_tip(tooltips, GTK_WIDGET(global->ebox), caption, NULL);

        return TRUE;
    }

    get_current_netload( &(global->monitor->data), &(net[IN]), &(net[OUT]), &(net[TOT]) );
    

    for (i = 0; i < SUM; i++)
    {
        /* correct value to be from 1 ... 100 */
        global->monitor->history[i][0] = net[i];

        if (global->monitor->history[i][0] < 0)
        {
            global->monitor->history[i][0] = 0;
        }

        histcalculate = 0;
        for( j = 0; j < HISTSIZE_CALCULATE; j++ )
        {
            histcalculate += global->monitor->history[i][j];
        }
        display[i] = histcalculate / HISTSIZE_CALCULATE;
        
        /* shift for next run */
        for( j = HISTSIZE_STORE - 1; j > 0; j-- )
        {
            global->monitor->history[i][j] = global->monitor->history[i][j-1];
        }
        
        /* update maximum */
        if( global->monitor->options.auto_max )
        {
            max = max_array( global->monitor->history[i], HISTSIZE_STORE );
            if( display[i] > global->monitor->net_max[i] )
            {
                global->monitor->net_max[i] = display[i];
            }
            else if( max < global->monitor->net_max[i] * SHRINK_MAX 
                    && global->monitor->net_max[i] * SHRINK_MAX >= MINIMAL_MAX )
            {
                global->monitor->net_max[i] *= SHRINK_MAX;
            }
        }

#ifdef DEBUG        
        switch (i) 
        {
            case IN:
                PRINT_DBG("input: Max = %lu", global->monitor->net_max[i]);
                break;
                
            case OUT:
                PRINT_DBG("output: Max = %lu", global->monitor->net_max[i]);
                break;
                
            case TOT:
                PRINT_DBG("total: Max = %lu", global->monitor->net_max[i]);
                break;
        }
#endif /* DEBUG */
        
        temp = (double)display[i] / global->monitor->net_max[i];
        if (temp > 1)
        {
            temp = 1.0;
        }
        else if (temp < 0)
        {
            temp = 0.0;
        }
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(global->monitor->status[i]), temp);

        format_with_thousandssep( buffer[i], BUFSIZ, display[i] / 1024.0, 2 );
    }
    
    format_with_thousandssep( buffer[TOT], BUFSIZ, (display[IN]+display[OUT])  / 1024.0, 2 );
    
    {
        char* ip = get_ip_address(&(global->monitor->data));
        g_snprintf(caption, sizeof(caption), 
                _("<< %s >> (%s)\nAverage of last %d measures:\n"
                    "Incoming: %s kByte/s\nOutgoing: %s kByte/s\nTotal: %s kByte/s"),
                    get_name(&(global->monitor->data)), ip ? ip : _("no IP address"),
                    HISTSIZE_CALCULATE, buffer[IN], buffer[OUT], buffer[TOT]);
        gtk_tooltips_set_tip(tooltips, GTK_WIDGET(global->ebox), caption, NULL);
    }

    return TRUE;
}


/* ---------------------------------------------------------------------------------------------- */
static void run_update (t_global_monitor *global)
{
    if (global->timeout_id > 0)
    {
        g_source_remove(global->timeout_id);
        global->timeout_id = 0;
    }

    if (global->monitor->options.update_interval > 0)
    {
        global->timeout_id =  g_timeout_add( global->monitor->options.update_interval, 
            (GtkFunction)update_monitors, global);
    }
}


/* ---------------------------------------------------------------------------------------------- */
static void monitor_set_orientation (XfcePanelPlugin *plugin, GtkOrientation orientation, 
                                     t_global_monitor *global)
{
    gint i;

    if (global->timeout_id)
    {
        g_source_remove(global->timeout_id);
    }

    gtk_widget_hide(GTK_WIDGET(global->ebox));
    if (global->box)
    {
        gtk_container_remove(GTK_CONTAINER(global->ebox), GTK_WIDGET(global->box));
    }
    if (orientation == GTK_ORIENTATION_HORIZONTAL)
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

    if (orientation == GTK_ORIENTATION_HORIZONTAL)
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

    gtk_container_set_border_width(GTK_CONTAINER(global->monitor->box), 2);
    gtk_widget_show(GTK_WIDGET(global->monitor->box));

    for (i = 0; i < SUM; i++)
    {
        gtk_widget_modify_bg(GTK_WIDGET(global->monitor->status[i]),
                             GTK_STATE_PRELIGHT,
                             &global->monitor->options.color[i]);
        gtk_widget_modify_bg(GTK_WIDGET(global->monitor->status[i]),
                             GTK_STATE_SELECTED,
                             &global->monitor->options.color[i]);
        gtk_widget_modify_base(GTK_WIDGET(global->monitor->status[i]),
                               GTK_STATE_SELECTED,
                               &global->monitor->options.color[i]);

        gtk_widget_show(GTK_WIDGET(global->monitor->status[i]));

        gtk_box_pack_start(GTK_BOX(global->monitor->box),
                GTK_WIDGET(global->monitor->status[i]), FALSE, FALSE, 0);
    }
    gtk_box_pack_start(GTK_BOX(global->box),
                       GTK_WIDGET(global->monitor->box), FALSE, FALSE, 0);


    gtk_container_add(GTK_CONTAINER(global->ebox), GTK_WIDGET(global->box));
    gtk_widget_show(GTK_WIDGET(global->ebox));

    run_update( global );
}


/* ---------------------------------------------------------------------------------------------- */
static void monitor_free(XfcePanelPlugin *plugin, t_global_monitor *global)
{
    if (global->timeout_id)
    {
        g_source_remove(global->timeout_id);
    }

    if (global->monitor->options.label_text)
    {
        g_free(global->monitor->options.label_text);
    }
    g_free(global);
    
    close_netload( &(global->monitor->data) );
}


/* ---------------------------------------------------------------------------------------------- */
static t_global_monitor * monitor_new(XfcePanelPlugin *plugin)
{
    t_global_monitor *global;
    gint i;

    global = g_new(t_global_monitor, 1);
    global->timeout_id = 0;
    global->ebox = gtk_event_box_new();
    gtk_widget_show(global->ebox);
    global->box = NULL;

    global->plugin = plugin;
    xfce_panel_plugin_add_action_widget (plugin, global->ebox);

    if (!tooltips) 
    {
        tooltips = gtk_tooltips_new();
    }

    global->monitor = g_new(t_monitor, 1);
    global->monitor->options.label_text = g_strdup(DEFAULT_TEXT);
    global->monitor->options.network_device = g_strdup("");
    global->monitor->options.old_network_device = g_strdup("");
    global->monitor->options.use_label = TRUE;
    global->monitor->options.auto_max = TRUE;
    global->monitor->options.update_interval = UPDATE_TIMEOUT;
    
    for (i = 0; i < SUM; i++)
    {
        gdk_color_parse(DEFAULT_COLOR[i], &global->monitor->options.color[i]);

        global->monitor->history[i][0] = 0;
        global->monitor->history[i][1] = 0;
        global->monitor->history[i][2] = 0;
        global->monitor->history[i][3] = 0;
        global->monitor->net_max[i]    = INIT_MAX;
        
        global->monitor->options.max[i] = INIT_MAX;
    }

    monitor_set_orientation (plugin, xfce_panel_plugin_get_orientation (plugin), global);
    
    return global;
}


/* ---------------------------------------------------------------------------------------------- */
static void setup_monitor(t_global_monitor *global, gboolean supress_warnings)
{
    gint i;

    gtk_widget_hide(GTK_WIDGET(global->monitor->box));
    gtk_widget_hide(global->monitor->label);
    gtk_label_set_text(GTK_LABEL(global->monitor->label),
            global->monitor->options.label_text);

    for (i = 0; i < SUM; i++)
    {
        gtk_widget_modify_bg(GTK_WIDGET(global->monitor->status[i]),
                             GTK_STATE_PRELIGHT,
                             &global->monitor->options.color[i]);
        gtk_widget_modify_bg(GTK_WIDGET(global->monitor->status[i]),
                             GTK_STATE_SELECTED,
                             &global->monitor->options.color[i]);
        gtk_widget_modify_base(GTK_WIDGET(global->monitor->status[i]),
                               GTK_STATE_SELECTED,
                               &global->monitor->options.color[i]);
        
        /* Maximum */
        if( global->monitor->options.auto_max )
        {
            global->monitor->net_max[i] = INIT_MAX;
        }
        else
        {
            global->monitor->net_max[i] = global->monitor->options.max[i];
        }
    }

    gtk_widget_show(GTK_WIDGET(global->monitor->box));
    if (global->monitor->options.use_label)
    {
        gtk_widget_show(global->monitor->label);
    }
    
    if (!init_netload( &(global->monitor->data), global->monitor->options.network_device)
            && !supress_warnings)
    {
        xfce_err (_("%s: Error in initalizing:\n%s"),
            _(APP_NAME),
            _(errormessages[
                global->monitor->data.errorcode == 0 
              ? INTERFACE_NOT_FOUND
              : global->monitor->data.errorcode ]));
    }
    
    if (global->monitor->options.old_network_device)
    {
        g_free(global->monitor->options.old_network_device);
    }
    global->monitor->options.old_network_device = g_strdup(global->monitor->options.network_device);
    
    run_update( global );

}


/* ---------------------------------------------------------------------------------------------- */
static void monitor_read_config(XfcePanelPlugin *plugin, t_global_monitor *global)
{
    const char *value;
    char *file;
    XfceRc *rc;
    
    if (!(file = xfce_panel_plugin_lookup_rc_file (plugin)))
        return;
    
    rc = xfce_rc_simple_open (file, TRUE);
    g_free (file);

    if (!rc)
        return;
    
    global->monitor->options.use_label = xfce_rc_read_bool_entry (rc, "Use_Label", TRUE);

    if ((value = xfce_rc_read_entry (rc, "Color_In", NULL)) != NULL)
    {
        gdk_color_parse(value,
                        &global->monitor->options.color[IN]);
    }
    if ((value = xfce_rc_read_entry (rc, "Color_Out", NULL)) != NULL)
    {
        gdk_color_parse(value,
                        &global->monitor->options.color[OUT]);
    }
    if ((value = xfce_rc_read_entry (rc, "Text", NULL)) && *value)
    {
        if (global->monitor->options.label_text)
            g_free(global->monitor->options.label_text);
        global->monitor->options.label_text = g_strdup(value);
    }
    if ((value = xfce_rc_read_entry (rc, "Network_Device", NULL)) && *value)
    {
        if (global->monitor->options.network_device)
            g_free(global->monitor->options.network_device);
        global->monitor->options.network_device = g_strdup(value);
    }    
    if ((value = xfce_rc_read_entry (rc, "Max_In", NULL)) != NULL)
    {
        global->monitor->options.max[IN] = strtol (value, NULL, 0); 
    }
    if ((value = xfce_rc_read_entry (rc, "Max_Out", NULL)) != NULL)
    {
        global->monitor->options.max[OUT] = strtol (value, NULL, 0);
    }
    
    global->monitor->options.auto_max = xfce_rc_read_bool_entry (rc, "Auto_Max", TRUE);
    
    global->monitor->options.update_interval = 
        xfce_rc_read_int_entry (rc, "Update_Interval", UPDATE_TIMEOUT);

    PRINT_DBG("monitor_read_config");
    setup_monitor(global, TRUE);

    xfce_rc_close (rc);
}


/* ---------------------------------------------------------------------------------------------- */
static void monitor_write_config(XfcePanelPlugin *plugin, t_global_monitor *global)
{
    XfceRc *rc;
    char *file;
    char value[20];

    if (!(file = xfce_panel_plugin_save_location (plugin, TRUE)))
        return;
    
    rc = xfce_rc_simple_open (file, FALSE);
    g_free (file);

    if (!rc)
        return;
    
    xfce_rc_write_bool_entry (rc, "Use_Label", global->monitor->options.use_label);

    g_snprintf(value, 8, "#%02X%02X%02X",
               (guint)global->monitor->options.color[IN].red >> 8,
               (guint)global->monitor->options.color[IN].green >> 8,
               (guint)global->monitor->options.color[IN].blue >> 8);
    xfce_rc_write_entry (rc, "Color_In", value);

    g_snprintf(value, 8, "#%02X%02X%02X",
               (guint)global->monitor->options.color[OUT].red >> 8,
               (guint)global->monitor->options.color[OUT].green >> 8,
               (guint)global->monitor->options.color[OUT].blue >> 8);
    xfce_rc_write_entry (rc, "Color_Out", value);

    xfce_rc_write_entry (rc, "Text", global->monitor->options.label_text ?
                                     global->monitor->options.label_text : "");

    xfce_rc_write_entry (rc, "Network_Device", global->monitor->options.network_device ? 
                                               global->monitor->options.network_device : "");
    
    g_snprintf(value, 20, "%lu", global->monitor->options.max[IN]);
    xfce_rc_write_entry (rc, "Max_In", value);
    
    g_snprintf(value, 20, "%lu", global->monitor->options.max[OUT]);
    xfce_rc_write_entry (rc, "Max_Out", value);
    
    xfce_rc_write_bool_entry (rc, "Auto_Max", global->monitor->options.auto_max);
    
    xfce_rc_write_int_entry (rc, "Update_Interval", global->monitor->options.update_interval);

    xfce_rc_close (rc);
}


/* ---------------------------------------------------------------------------------------------- */
static gboolean monitor_set_size(XfcePanelPlugin *plugin, int size, t_global_monitor *global)
{
    gint i;

    for (i = 0; i < SUM; i++)
    {
        if (xfce_panel_plugin_get_orientation (plugin) == GTK_ORIENTATION_HORIZONTAL)
        {
            gtk_widget_set_size_request(GTK_WIDGET(global->monitor->status[i]),
                    BORDER, size - BORDER);
        }
        else
        {
            gtk_widget_set_size_request(GTK_WIDGET(global->monitor->status[i]),
                    size - BORDER, BORDER);
        }
    }
    PRINT_DBG("monitor_set_size");
    setup_monitor(global, TRUE);

    return TRUE;
}


/* ---------------------------------------------------------------------------------------------- */
static void monitor_apply_options(t_global_monitor *global)
{
    gint i;
    
    if (global->monitor->options.label_text)
    {
        g_free(global->monitor->options.label_text);
    }

    global->monitor->options.label_text =
        g_strdup(gtk_entry_get_text(GTK_ENTRY(global->monitor->opt_entry)));
    setup_monitor(global, FALSE);


    if (global->monitor->options.network_device)
    {
        g_free(global->monitor->options.network_device);
    }
    global->monitor->options.network_device =
        g_strdup(gtk_entry_get_text(GTK_ENTRY(global->monitor->net_entry)));
    setup_monitor(global, FALSE);
    
    for( i = 0; i < SUM; i++ )
    {
        global->monitor->options.max[i] = 
            strtol(gtk_entry_get_text(GTK_ENTRY(global->monitor->max_entry[i])), NULL, 0) * 1024;
    }
    
    global->monitor->options.update_interval = 
        (gint)(gtk_spin_button_get_value( 
            GTK_SPIN_BUTTON(global->monitor->update_spinner) ) * 1000 + 0.5);
    
    setup_monitor(global, FALSE);
    PRINT_DBG("monitor_apply_options_cb");
}


/* ---------------------------------------------------------------------------------------------- */
static void label_changed(GtkWidget *button, t_global_monitor *global)
{
    if (global->monitor->options.label_text)
    {
        g_free(global->monitor->options.label_text);
    }

    global->monitor->options.label_text =
        g_strdup(gtk_entry_get_text(GTK_ENTRY(global->monitor->opt_entry)));

    setup_monitor(global, FALSE);
    PRINT_DBG("label_changed");
}


/* ---------------------------------------------------------------------------------------------- */
static void max_label_changed(GtkWidget *button, t_global_monitor *global)
{
    gint i;
    for( i = 0; i < SUM; i++ )
    {
        global->monitor->options.max[i] = 
            strtol(gtk_entry_get_text(GTK_ENTRY(global->monitor->max_entry[i])), NULL, 0) * 1024;
    }

    setup_monitor(global, FALSE);
    PRINT_DBG("max_label_changed");
}


/* ---------------------------------------------------------------------------------------------- */
static void network_changed(GtkWidget *button, t_global_monitor *global)
{
    if (global->monitor->options.network_device)
    {
        g_free(global->monitor->options.network_device);
    }

    global->monitor->options.network_device =
        g_strdup(gtk_entry_get_text(GTK_ENTRY(global->monitor->net_entry)));

    setup_monitor(global, FALSE);
    PRINT_DBG("network_changed");
}


/* ---------------------------------------------------------------------------------------------- */
static void label_toggled(GtkWidget *check_button, t_global_monitor *global)
{
    global->monitor->options.use_label = !global->monitor->options.use_label;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(global->monitor->opt_use_label),
                                 global->monitor->options.use_label);
    gtk_widget_set_sensitive(GTK_WIDGET(global->monitor->opt_entry),
                             global->monitor->options.use_label);

    setup_monitor(global, FALSE);
    PRINT_DBG("label_toggled");
}


/* ---------------------------------------------------------------------------------------------- */
static void max_label_toggled(GtkWidget *check_button, t_global_monitor *global)
{
    gint i;
    
    global->monitor->options.auto_max = !global->monitor->options.auto_max;
    
    for( i = 0; i < SUM; i++ )
    {
        gtk_widget_set_sensitive(GTK_WIDGET(global->monitor->max_hbox[i]),
                                     !(global->monitor->options.auto_max));
        
        /* reset maximum if necessary */
        if( global->monitor->options.auto_max )
        {
            global->monitor->net_max[i] = INIT_MAX;
        }
    }
    setup_monitor(global, FALSE);
    PRINT_DBG("max_label_toggled");

}


/* ---------------------------------------------------------------------------------------------- */
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


/* ---------------------------------------------------------------------------------------------- */
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
        setup_monitor(global, FALSE);
    }
    PRINT_DBG("change_color");
    gtk_widget_destroy(dialog);
}


/* ---------------------------------------------------------------------------------------------- */
static void change_color_in(GtkWidget *button, t_global_monitor *global)
{
    change_color(button, global, IN);
}


/* ---------------------------------------------------------------------------------------------- */
static void change_color_out(GtkWidget *button, t_global_monitor *global)
{
    change_color(button, global, OUT);
}


/* ---------------------------------------------------------------------------------------------- */
static void
monitor_dialog_response (GtkWidget *dlg, int response, t_global_monitor *global)
{

    monitor_apply_options (global);
    gtk_widget_destroy (dlg);
    xfce_panel_plugin_unblock_menu (global->plugin);
    monitor_write_config (global->plugin, global);
}

static void monitor_create_options(XfcePanelPlugin *plugin, t_global_monitor *global)
{
    GtkWidget        *dlg, *header;
    GtkBox           *vbox, *global_vbox, *net_hbox;
    GtkWidget        *device_label, *unit_label[SUM], *max_label[SUM];
    GtkWidget        *sep1, *sep2;
    GtkBox           *update_hbox;
    GtkWidget        *update_label, *update_unit_label;
    GtkWidget        *color_label[SUM];
    GtkWidget        *align;
    GtkBox           *color_hbox[SUM];
    GtkSizeGroup     *sg;
    gint             i;
    gchar            buffer[BUFSIZ];
    gchar            *color_text[] = { 
                        N_("Bar color (incoming):"), 
                        N_("Bar color (outgoing):") 
                     };
    gchar            *maximum_text_label[] = {
                        N_("Maximum (incoming):"),
                        N_("Maximum (outgoing):")
                     };
    
    xfce_panel_plugin_block_menu (plugin);
    
    dlg = xfce_titled_dialog_new_with_buttons (_("Network Monitor"), NULL,
                                               GTK_DIALOG_NO_SEPARATOR,
                                               GTK_STOCK_CLOSE, GTK_RESPONSE_OK,
                                               NULL);
    
    gtk_window_set_icon_name (GTK_WINDOW (dlg), "xfce4-settings");
    g_signal_connect (dlg, "response", G_CALLBACK (monitor_dialog_response),
                      global);

    global->opt_dialog = dlg;
    
    global_vbox = GTK_BOX (gtk_vbox_new(FALSE, BORDER));
    gtk_container_set_border_width (GTK_CONTAINER (global_vbox), BORDER - 2);
    gtk_widget_show(GTK_WIDGET (global_vbox));
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dlg)->vbox), GTK_WIDGET (global_vbox), TRUE, TRUE, 0);
    
    sg = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
    
    vbox = GTK_BOX(gtk_vbox_new(FALSE, 5));
    gtk_widget_show(GTK_WIDGET(vbox));

    global->monitor->opt_vbox = GTK_BOX(gtk_vbox_new(FALSE, 5));
    gtk_widget_show(GTK_WIDGET(global->monitor->opt_vbox));

    /* Displayed text */
    global->monitor->opt_hbox = GTK_BOX(gtk_hbox_new(FALSE, 5));
    gtk_widget_show(GTK_WIDGET(global->monitor->opt_hbox));
    
    global->monitor->opt_use_label =
        gtk_check_button_new_with_mnemonic(_("Text to display:"));
    gtk_widget_show(global->monitor->opt_use_label);
    gtk_box_pack_start(GTK_BOX(global->monitor->opt_hbox),
                       GTK_WIDGET(global->monitor->opt_use_label),
                       FALSE, FALSE, 0);
    gtk_size_group_add_widget(sg, global->monitor->opt_use_label);

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
    net_hbox = GTK_BOX(gtk_hbox_new(FALSE, 5));
    gtk_box_pack_start(GTK_BOX(global->monitor->opt_vbox),
                        GTK_WIDGET(net_hbox), FALSE, FALSE, 0);

    device_label = gtk_label_new(_("Network device:"));
    gtk_misc_set_alignment(GTK_MISC(device_label), 0, 0.5);
    gtk_widget_show(GTK_WIDGET(device_label));
    gtk_box_pack_start(GTK_BOX(net_hbox), GTK_WIDGET(device_label),
                       FALSE, FALSE, 0);

    global->monitor->net_entry = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(global->monitor->net_entry),
                             MAX_LENGTH);
    gtk_entry_set_text(GTK_ENTRY(global->monitor->net_entry),
                       global->monitor->options.network_device);
    gtk_widget_show(global->monitor->opt_entry);

    gtk_box_pack_start(GTK_BOX(net_hbox), GTK_WIDGET(global->monitor->net_entry),
                       FALSE, FALSE, 0);
    
    gtk_size_group_add_widget(sg, device_label);

    gtk_widget_show_all(GTK_WIDGET(net_hbox));
    
    
    /* Update timevalue */
    update_hbox = GTK_BOX(gtk_hbox_new(FALSE, 5));
    gtk_box_pack_start(GTK_BOX(global->monitor->opt_vbox),
                        GTK_WIDGET(update_hbox), FALSE, FALSE, 0);
    
    update_label = gtk_label_new(_("Update interval:"));
    gtk_misc_set_alignment(GTK_MISC(update_label), 0, 0.5);
    gtk_box_pack_start(GTK_BOX(update_hbox), GTK_WIDGET(update_label), FALSE, FALSE, 0);
    
    global->monitor->update_spinner = gtk_spin_button_new_with_range (0.1, 10.0, 0.05);
    gtk_spin_button_set_digits( GTK_SPIN_BUTTON(global->monitor->update_spinner), 2 );
    gtk_spin_button_set_value( GTK_SPIN_BUTTON(global->monitor->update_spinner), 
            global->monitor->options.update_interval / 1000.0 );
    gtk_box_pack_start(GTK_BOX(update_hbox), GTK_WIDGET(global->monitor->update_spinner), 
        FALSE, FALSE, 0);
        
    update_unit_label = gtk_label_new(_("s"));
    gtk_box_pack_start(GTK_BOX(update_hbox), GTK_WIDGET(update_unit_label), 
        FALSE, FALSE, 0);
    
    gtk_widget_show_all(GTK_WIDGET(update_hbox));
    gtk_size_group_add_widget(sg, update_label);
    
    
    sep1 = gtk_hseparator_new();
    gtk_box_pack_start(GTK_BOX(global->monitor->opt_vbox), GTK_WIDGET(sep1), FALSE, FALSE, 0);
    gtk_widget_show(sep1);
    
    global->monitor->max_use_label = 
                gtk_check_button_new_with_mnemonic(_("Automatic maximum"));
    gtk_widget_show(global->monitor->max_use_label);
    gtk_box_pack_start(GTK_BOX(global->monitor->opt_vbox),
                GTK_WIDGET(global->monitor->max_use_label), FALSE, FALSE, 0);
    
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(global->monitor->max_use_label),
                                 global->monitor->options.auto_max);
    
    /* Input maximum */
    for( i = 0; i < SUM; i++)
    {
        global->monitor->max_hbox[i] = GTK_BOX(gtk_hbox_new(FALSE, 5));
        gtk_box_pack_start(GTK_BOX(global->monitor->opt_vbox), 
                    GTK_WIDGET(global->monitor->max_hbox[i]), FALSE, FALSE, 0);
        
        max_label[i] = gtk_label_new(_(maximum_text_label[i]));
        gtk_misc_set_alignment(GTK_MISC(max_label[i]), 0, 0.5);
        gtk_widget_show(GTK_WIDGET(max_label[i]));
        gtk_box_pack_start(GTK_BOX(global->monitor->max_hbox[i]), GTK_WIDGET(max_label[i]), FALSE, FALSE, 0);
        
        global->monitor->max_entry[i] = gtk_entry_new();
        gtk_entry_set_max_length(GTK_ENTRY(global->monitor->max_entry[i]), MAX_LENGTH);
        
        g_snprintf( buffer, BUFSIZ, "%.2f", global->monitor->options.max[i] / 1024.0 );
        gtk_entry_set_text(GTK_ENTRY(global->monitor->max_entry[i]), buffer);
        
        gtk_entry_set_width_chars(GTK_ENTRY(global->monitor->max_entry[i]), 7);
        gtk_widget_show(global->monitor->max_entry[i]);
        
        gtk_box_pack_start(GTK_BOX(global->monitor->max_hbox[i]), GTK_WIDGET(global->monitor->max_entry[i]),
                       FALSE, FALSE, 0);
        
        unit_label[i] = gtk_label_new(_("kByte/s"));
        gtk_box_pack_start(GTK_BOX(global->monitor->max_hbox[i]), GTK_WIDGET(unit_label[i]), FALSE, FALSE, 0);
        
        gtk_size_group_add_widget(sg, max_label[i]);
        
        gtk_widget_show_all(GTK_WIDGET(global->monitor->max_hbox[i]));
        
        gtk_widget_set_sensitive(GTK_WIDGET(global->monitor->max_hbox[i]),
                                 !(global->monitor->options.auto_max) );
                                 
        g_signal_connect(GTK_WIDGET(global->monitor->max_entry[i]), "activate",
            G_CALLBACK(max_label_changed), global);

    } 
    
    sep2 = gtk_hseparator_new();
    gtk_box_pack_start(GTK_BOX(global->monitor->opt_vbox), GTK_WIDGET(sep2), FALSE, FALSE, 0);
    gtk_widget_show(sep2);
    
    /* Color 1 */
    for (i = 0; i < SUM; i++)
    {
        color_hbox[i] = GTK_BOX(gtk_hbox_new(FALSE, 5));
        gtk_widget_show(GTK_WIDGET(color_hbox[i]));
        gtk_box_pack_start(GTK_BOX(global->monitor->opt_vbox),
                GTK_WIDGET(color_hbox[i]), FALSE, FALSE, 0);

        color_label[i] = gtk_label_new(_(color_text[i]));
        gtk_misc_set_alignment(GTK_MISC(color_label[i]), 0, 0.5);
        gtk_widget_show(GTK_WIDGET(color_label[i]));
        gtk_box_pack_start(GTK_BOX(color_hbox[i]), GTK_WIDGET(color_label[i]),
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
        gtk_box_pack_start(GTK_BOX(color_hbox[i]),
                GTK_WIDGET(global->monitor->opt_button[i]),
                FALSE, FALSE, 0);

        gtk_size_group_add_widget(sg, color_label[i]);

    }
    
    gtk_box_pack_start(GTK_BOX(vbox),
                GTK_WIDGET(global->monitor->opt_vbox),
                FALSE, FALSE, 0);
    
    align = gtk_alignment_new(0, 0, 0, 0);
    gtk_widget_set_size_request(align, 5, 5);
    gtk_widget_show(GTK_WIDGET(align));
    gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(align), FALSE, FALSE, 0);
    
    gtk_box_pack_start( GTK_BOX(global_vbox), GTK_WIDGET(vbox), FALSE, FALSE, 0);
    
    g_signal_connect(GTK_WIDGET(global->monitor->max_use_label), "toggled",
            G_CALLBACK(max_label_toggled), global);
    g_signal_connect(GTK_WIDGET(global->monitor->opt_da[0]), "expose_event",
            G_CALLBACK(expose_event_cb), NULL);
    g_signal_connect(GTK_WIDGET(global->monitor->opt_da[1]), "expose_event",
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

    gtk_widget_show (dlg);
}


/* ---------------------------------------------------------------------------------------------- */
static void netload_construct (XfcePanelPlugin *plugin)
{
    t_global_monitor *global;

    xfce_textdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");

    global = monitor_new(plugin);

    monitor_read_config (plugin, global);
    
    g_signal_connect (plugin, "free-data", G_CALLBACK (monitor_free), global);
    
    g_signal_connect (plugin, "save", G_CALLBACK (monitor_write_config), global);
    
    xfce_panel_plugin_menu_show_configure (plugin);
    g_signal_connect (plugin, "configure-plugin", G_CALLBACK (monitor_create_options), global);
    
    g_signal_connect (plugin, "size-changed", G_CALLBACK (monitor_set_size), global);
    
    g_signal_connect (plugin, "orientation-changed", G_CALLBACK (monitor_set_orientation), global);
    
    gtk_container_add(GTK_CONTAINER(plugin), global->ebox);
	
    run_update( global );
}

XFCE_PANEL_PLUGIN_REGISTER_EXTERNAL (netload_construct);
