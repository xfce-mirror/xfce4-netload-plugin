/*
 * Copyright (c) 2003 Bernhard Walle <bernhard.walle@gmx.de>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <gtk/gtk.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <net/if.h>
#include <net/if_mib.h>
#include <time.h>
#include <sys/time.h>

#include <libxfce4util/i18n.h>
#include <libxfcegui4/dialogs.h>
#include <panel/plugins.h>
#include <panel/xfce.h>

#include "net.h" 


/*
 * Basic code taken from IceWM
 * (c) Marko Macek and others, GNU General Public License
 * 
 * Converted from C++ to C (using glib data types) by Bernhard Walle
 */

void get_current_netload (const gchar* device, gint64 *in, gint64 *out, gint64 *tot) 
{
    static guint64 prev_ibytes = 0, cur_ibytes = 0, offset_ibytes = 0;
    static guint64 prev_obytes = 0, cur_obytes = 0, offset_obytes = 0;
    static struct timeval prev_time;
    static gboolean first = TRUE;
    struct timeval curr_time;
    gdouble delta_t;
    gint64 ni, no;
    struct ifmibdata ifmd;
    size_t ifmd_size=sizeof ifmd;
    gint nr_network_devs;
    size_t int_size=sizeof nr_network_devs;
    gint name[6];
    gint i;

    if (in != NULL && out != NULL && tot != NULL)
    {
        *in = *out = *tot = 0;
    }

    if (first)
    {
        gettimeofday(&prev_time, NULL);
    }
    gettimeofday(&curr_time, NULL);
    
    delta_t = (gdouble) ((curr_time.tv_sec  - prev_time.tv_sec) * 1000000L
                             + (curr_time.tv_usec - prev_time.tv_usec)) / 1000000.0;
    
    // FreeBSD code by Ronald Klop <ronald@cs.vu.nl>
    name[0] = CTL_NET;
    name[1] = PF_LINK;
    name[2] = NETLINK_GENERIC;
    name[3] = IFMIB_IFDATA;
    name[5] = IFDATA_GENERAL;

    if(sysctlbyname("net.link.generic.system.ifcount",&nr_network_devs,
                                       &int_size,(void*)0,0) == -1) {
               g_printf("%s@%d: %s\n",__FILE__,__LINE__,strerror(errno));
       } else {
           for(i=1;i<=nr_network_devs;i++) {
                   name[4] = i; /* row of the ifmib table */

           if(sysctl(name, 6, &ifmd, &ifmd_size, (void *)0, 0) == -1) {
                           printf(_("%s@%d: %s\n"),__FILE__,__LINE__,strerror(errno));
                           continue;
                   }
           if (strncmp(ifmd.ifmd_name, device, strlen(device)) == 0) {
                           cur_ibytes = ifmd.ifmd_data.ifi_ibytes;
                           cur_obytes = ifmd.ifmd_data.ifi_obytes;
                           break;
                   }
           }
       }

    cur_ibytes += offset_ibytes;
    cur_obytes += offset_obytes;

    if (cur_ibytes < prev_ibytes)
        // har, har, overflow. Use the recent prev_ibytes value as offset this time
        cur_ibytes = offset_ibytes = prev_ibytes;

    if (cur_obytes < prev_obytes)
        // har, har, overflow. Use the recent prev_obytes value as offset this time
        cur_obytes = offset_obytes = prev_obytes;



    ni = (gint64)((cur_ibytes - prev_ibytes) / delta_t);
    no = (gint64)((cur_obytes - prev_obytes) / delta_t);

    if (in != NULL && out != NULL && tot != NULL)
    {
        *in  = ni;
        *out = no;
        *tot = *in + *out;
    }

    prev_time.tv_sec = curr_time.tv_sec;
    prev_time.tv_usec = curr_time.tv_usec;

    prev_ibytes = cur_ibytes;
    prev_obytes = cur_obytes;

    if (first) 
    {
        *in = *out = *tot = 0;
        first = FALSE;
    }
}

