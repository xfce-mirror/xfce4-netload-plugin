/*  XFce 4 - Netload Plugin
 *    Copyright (c) 2003 Bernhard Walle <bernhard.walle@gmx.de>
 *  
 *  Id: $Id: net.c,v 1.4 2003/08/31 12:54:36 bwalle Exp $
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



/*
 * This is just a wrapper between the netload-plugin and the wormulon source.
 * Wormulon is a small command-line util which displays the netload. You can find it
 * at http://raisdorf.net/wormulon. Most sourcecode is taken from wormulon.
 *
 * Thanks to Hendrik Scholz. Only his work made it possible to support a large
 * number of operating systems quickly without a library! Without him only 
 * Linux and FreeBSD (with foreign code from IceWM) would be supported.
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/* From Wormulon */
#include "net.h"
#include "os.h"
#include "wormulon.h"
#include "slurm.h"	/* slurm structs */

#ifdef __HPUX__
#include "wormulon/hpux.h"
#include "wormulon/hpux.c"
#elif __FreeBSD__
#include "wormulon/freebsd.h"
#include "wormulon/freebsd.c"
#elif __linux__
#include "wormulon/linux.h"
#include "wormulon/linux.c"
#elif __OpenBSD__ || __MicroBSD__
#include "wormulon/openbsd.h"
#include "wormulon/openbsd.c"
#elif __NetBSD__
#include "wormulon/netbsd.h"
#include "wormulon/netbsd.c"
#elif __Solaris__
#include "wormulon/solaris.h"
#include "wormulon/solaris.c"
#else
/* should not get here */
#error "OS not supported"
#endif




void init_netload(netdata* data, const char* device)
{
    memset( data, 0, sizeof(netdata) );
    strncpy( data->ifdata.if_name, device, 9 );
    data->ifdata.if_name[9] = '\0';
    
    init_osspecific( data );
    
    if (checkinterface(data) != TRUE)
	{
        data->correct_interface = FALSE;
		return;
	}
    
    /* init in a sane state */
    get_stat(data);
    data->backup_in  = data->stats.rx_bytes;
    data->backup_out = data->stats.tx_bytes;
    
    data->correct_interface = TRUE;
    
#ifdef DEBUG
    fprintf( stderr, "The netload plugin was initialized for '%s'.\n", device );
#endif /* DEBUG */
}


/**
 * Gets the current netload.
 * @param   in          Will be filled with the "in"-load.
 * @param   out         Will be filled with the "out"-load.
 * @param   tot         Will be filled with the "total"-load.
 */
void get_current_netload(netdata* data, unsigned long *in, unsigned long *out, unsigned long *tot) 
{
    struct timeval curr_time;
    double delta_t;
    
    if( ! data->correct_interface )
    {
        if( in != NULL && out != NULL && tot != NULL )
        {
            *in = *out = *tot = 0;
        }
    }
    
    gettimeofday(&curr_time, NULL);
    
    delta_t = (double) ((curr_time.tv_sec  - data->prev_time.tv_sec) * 1000000L
                             + (curr_time.tv_usec - data->prev_time.tv_usec)) / 1000000.0;
    
    /* update */
    get_stat(data);
    if (data->backup_in > data->stats.rx_bytes)
    {
        data->cur_in = (int)( data->stats.rx_bytes / delta_t + 0.5);
    }
    else
    {
        data->cur_in = (int)( (data->stats.rx_bytes - data->backup_in) / delta_t + 0.5);
    }
	
    if (data->backup_out > data->stats.tx_bytes)
    {
        data->cur_out = (int)( data->stats.tx_bytes / delta_t + 0.5);
    }
    else
    {
        data->cur_out = (int)( (data->stats.tx_bytes - data->backup_out) / delta_t + 0.5);
    }

    if( in != NULL && out != NULL && tot != NULL )
    {
        *in = data->cur_in;
        *out = data->cur_out;
        *tot = *in + *out;
    }
    
    /* save 'new old' values */
    data->backup_in = data->stats.rx_bytes;
    data->backup_out = data->stats.tx_bytes;
    
    /* do the same with time */
    data->prev_time.tv_sec = curr_time.tv_sec;
    data->prev_time.tv_usec = curr_time.tv_usec;
}

void close_netload(netdata* data)
{
    /* We need not code here */
}

