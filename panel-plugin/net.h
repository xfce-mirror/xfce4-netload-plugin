/*  XFce 4 - Netload Plugin
 *    Copyright (c) 2003 Bernhard Walle <bernhard.walle@gmx.de>
 *  
 *  Id: $Id: net.h,v 1.4 2003/08/25 21:08:58 bwalle Exp $
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


#ifndef NET_H
#define NET_H

/* ----------------------- Some defines here ------------------------------- */

#if defined (__sun__)
#define __Solaris__ 1
#endif

#include "os.h"
#include "slurm.h"


/*
 * We need this because we cannot use static variables. Using of static variables allows
 * us not to use several instances of the plugin.
 * I know that this change makes it a bit incompatible with wormulon, but that's the
 * price to pay ...
 */

typedef struct
{
    double          backup_in;
    double          backup_out;
    double          cur_in;
    double          cur_out;
    struct timeval  prev_time;
    int             correct_interface;          /* treated as boolean */
    IfData          ifdata;
    DataStats       stats;
#ifdef __HPUX__
    int             wait_pcks_counter;
    nmapi_logstat*  if_ptr;
#elif __FreeBSD__
    int             watchif;
    int             dev_opened;
#elif __NetBSD__
    int             mib_name1[6];
    int             mib_name2[6];
    char*           buf1;
    char*           buf2;
    int             alloc1;
    int             alloc2;
#elif __OpenBSD__ || __MicroBSD__
    int             mib_name1[6];
    int             mib_name2[6];
    char*           buf1;
    char*           buf2;
    int             alloc1;
    int             alloc2;
#elif __linux__
    FILE*           proc_net_dev;
#elif __Solaris__
#else
#error "OS not supported"
#endif
    
} netdata;



/**
 * Initializes the netload plugin. Used to set up inital values. This function must
 * be called after each change of the network interface.
 * @param   device      The network device, e.g. <code>ippp0</code> for ISDN on Linux.
 */
void init_netload(netdata* data, const char* device);

/**
 * Gets the current netload. You must call init_netload() once before you use this function!
 * @param in        Input load in byte/s.
 * @param out       Output load in byte/s.
 * @param tot       Total load in byte/s.
 */
void get_current_netload(netdata* data, unsigned long *in, unsigned long *out, unsigned long *tot);

/**
 * Should be called to do cleanup work.
 */
void close_netload(netdata* data);

#endif /* NET_H */
