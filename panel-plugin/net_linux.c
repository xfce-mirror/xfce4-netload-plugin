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
#include <gtk/gtk.h>
#include <time.h>
#include <sys/time.h>

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

    gchar buf[BUFSIZ];
    gchar *p;
    gint64 ipackets, opackets, ierrs, oerrs, idrop, odrop, ififo, ofifo,
           iframe, ocolls, ocarrier, icomp, ocomp, imcast;
           
    FILE *fp = NULL;
    
    gettimeofday(&curr_time, NULL);
    
    delta_t = (gdouble) ((curr_time.tv_sec  - prev_time.tv_sec) * 1000000L
                             + (curr_time.tv_usec - prev_time.tv_usec)) / 1000000.0;
    
    
    if ( (fp = fopen("/proc/net/dev", "r")) == NULL)
    {
        *in = *out = *tot = 0;
        return;
    }
    
    while ( fgets(buf, BUFSIZ, fp) != NULL ) 
    {
        p = buf;
        while (*p == ' ') 
        {
            p++;
        }
        
        if (strncmp(p, device, strlen(device)) == 0 && p[strlen(device)] == ':')
        {
            p = strchr(p, ':') + 1;
            
            if (sscanf(
                p, 
                "%llu %lld %lld %lld %lld %lld %lld %lld" " %llu %lld %lld %lld %lld %lld %lld %lld",
                &cur_ibytes, &ipackets, &ierrs, &idrop, &ififo, &iframe, &icomp, &imcast,
                &cur_obytes, &opackets, &oerrs, &odrop, &ofifo, &ocolls, &ocarrier, &ocomp) != 16 )
            {
                ipackets = opackets = 0;
                sscanf(p, "%lld %lld %lld %lld %lld" " %lld %lld %lld %lld %lld %lld",
                       &ipackets, &ierrs, &idrop, &ififo, &iframe,
                       &opackets, &oerrs, &odrop, &ofifo, &ocolls, &ocarrier);
                /* for linux<2.0 fake packets as bytes (we only need relative values anyway) */
                cur_ibytes = ipackets;
                cur_obytes = opackets;
            }
            
            break;
        }
    }
    fclose(fp);
    
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

