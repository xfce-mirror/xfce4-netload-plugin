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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <gtk/gtk.h>
#include <time.h>
#include <sys/time.h>

#include <sys/socket.h>
#include <sys/sysctl.h>
#include <net/if.h>
#include <net/if_mib.h>

#include "net.h" 


/*
 * Basic code taken from IceWM
 * (c) Marko Macek and others, GNU General Public License
 * 
 * Converted from C++ to C (using glib data types) by Bernhard Walle
 */


static guint64 prev_ibytes, cur_ibytes, offset_ibytes;
static guint64 prev_obytes, cur_obytes, offset_obytes;
static struct timeval prev_time;

void init_netload()
{
    prev_ibytes = cur_ibytes = offset_ibytes = 0;
    prev_obytes = cur_obytes = offset_obytes = 0;
    gettimeofday(&prev_time, NULL);
}


void get_current_netload(const gchar* device, gint64 *in, gint64 *out, gint64 *tot) 
{
    static gboolean first = TRUE;
    struct timeval curr_time;
    gdouble delta_t;
    gint64 ni, no;
    struct ifmibdata ifmd;
    size_t ifmd_size = sizeof(ifmd);
    gint nr_network_devs;
    size_t int_size = sizeof(nr_network_devs);
    gint name[] = {
        CTL_NET,            /* 0 */
        PF_LINK,            /* 1 */
        NETLINK_GENERIC,    /* 2 */
        IFMIB_IFDATA,       /* 3 */
        0,                  /* 4 */
        IFDATA_GENERAL      /* 5 */
    };
    gint i;

    gettimeofday(&curr_time, NULL);
    
    delta_t = (gdouble) ((curr_time.tv_sec  - prev_time.tv_sec) * 1000000L
                             + (curr_time.tv_usec - prev_time.tv_usec)) / 1000000.0;
    
    if(sysctlbyname("net.link.generic.system.ifcount", &nr_network_devs, &int_size, NULL, 0) == -1)
    {
        g_printf("%s@%d: %s\n", __FILE__, __LINE__, strerror(errno));
    }
    else 
    {
        for( i = 1; i <= nr_network_devs; i++ ) 
        {
            name[4] = i; /* row of the ifmib table */
            
            if( sysctl(name, 6, &ifmd, &ifmd_size, (void *)0, 0) == -1 ) 
            {
                printf("%s@%d: %s\n" ,__FILE__ ,__LINE__, strerror(errno));
                continue;
            }
            if ( strncmp(ifmd.ifmd_name, device, strlen(device)) == 0 ) 
            {
                cur_ibytes = ifmd.ifmd_data.ifi_ibytes;
                cur_obytes = ifmd.ifmd_data.ifi_obytes;
                break;
            }
        }
    }

    cur_ibytes += offset_ibytes;
    cur_obytes += offset_obytes;

    if (cur_ibytes < prev_ibytes)
    {
        /* har, har, overflow. Use the recent prev_ibytes value as offset this time */
        cur_ibytes = offset_ibytes = prev_ibytes;
    }

    if (cur_obytes < prev_obytes)
    {
        /* har, har, overflow. Use the recent prev_obytes value as offset this time */
        cur_obytes = offset_obytes = prev_obytes;
    }


    ni = (gint64)((cur_ibytes - prev_ibytes) / delta_t);
    no = (gint64)((cur_obytes - prev_obytes) / delta_t);

    if (in != NULL && out != NULL && tot != NULL && !first)
    {
        *in  = ni;
        *out = no;
        *tot = *in + *out;
    }
    else
    {
        *in = *out = *tot = 0;
        first = FALSE;
    }

    prev_time.tv_sec = curr_time.tv_sec;
    prev_time.tv_usec = curr_time.tv_usec;

    prev_ibytes = cur_ibytes;
    prev_obytes = cur_obytes;

}

void close_netload()
{
    /* Do nothing here */
}

