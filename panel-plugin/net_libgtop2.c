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
#include <gtk/gtk.h>
#include <time.h>
#include <sys/time.h>

#include <glibtop.h>
#include <glibtop/close.h>
#include <glibtop/netload.h>

#include "net.h" 

static glibtop_netload netload;
static struct timeval prev_time;
static guint64 prev_ibytes, cur_ibytes;
static guint64 prev_obytes, cur_obytes;
static guint64 prev_tbytes, cur_tbytes;

void init_netload()
{
    glibtop_init();
    prev_ibytes = cur_ibytes = 0;
    prev_obytes = cur_obytes = 0;
    prev_tbytes = cur_tbytes = 0;
    gettimeofday(&prev_time, NULL);
}

void get_current_netload(const gchar* device, gint64 *in, gint64 *out, gint64 *tot) 
{
    static gboolean first = TRUE;
    struct timeval curr_time;
    gdouble delta_t;
    
    if (in != NULL && out != NULL && tot != NULL)
    {
        *in = *out = *tot = 0;
    }

    gettimeofday(&curr_time, NULL);
    delta_t = (gdouble) ((curr_time.tv_sec  - prev_time.tv_sec) * 1000000L
                             + (curr_time.tv_usec - prev_time.tv_usec)) / 1000000.0;
	
	glibtop_get_netload (&netload, device);

    cur_ibytes = (gint64) netload.bytes_in;
    cur_obytes = (gint64) netload.bytes_out;
    cur_tbytes = (gint64) netload.bytes_total;
    
    if (in != NULL && out != NULL && tot != NULL && !first)
    {
        *in  = (gint64)((cur_ibytes - prev_ibytes) / delta_t);
        *out = (gint64)((cur_obytes - prev_obytes) / delta_t);
        *tot = (gint64)((cur_tbytes - prev_tbytes) / delta_t);
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
    prev_tbytes = cur_tbytes;
    
}

void close_netload()
{
    glibtop_close();
}

