/*  XFce 4 - Netload Plugin
 *    Copyright (c) 2003 Bernhard Walle <bernhard.walle@gmx.de>
 *  
 *  Id: $Id: net.c,v 1.1 2003/08/24 20:02:29 bwalle Exp $
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



#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

static double backup_in, backup_out;
static double cur_in, cur_out;
static char dim_in[4], dim_out[4];
static struct timeval prev_time;
int correct_interface;


void init_netload(const char* device)
{
    strncpy( ifdata.if_name, device, 9 );
    ifdata.if_name[9] = '\0';
    
    if (checkinterface() != TRUE)
	{
        correct_interface = FALSE;
		return;
	}
    
    /* init in a sane state */
    get_stat();
    backup_in  = stats.rx_bytes;
    backup_out = stats.tx_bytes;
    memset(dim_in, 0, sizeof(dim_in));
    memset(dim_out, 0, sizeof(dim_out));
    
    correct_interface = TRUE;
}


/**
 * Gets the current netload.
 * @param   in          Will be filled with the "in"-load.
 * @param   out         Will be filled with the "out"-load.
 * @param   tot         Will be filled with the "total"-load.
 */
void get_current_netload(unsigned long *in, unsigned long *out, unsigned long *tot) 
{
    struct timeval curr_time;
    double delta_t;
    
    if( !correct_interface )
    {
        if( in != NULL && out != NULL && tot != NULL )
        {
            *in = *out = *tot = 0;
        }
    }
    
    gettimeofday(&curr_time, NULL);
    
    delta_t = (double) ((curr_time.tv_sec  - prev_time.tv_sec) * 1000000L
                             + (curr_time.tv_usec - prev_time.tv_usec)) / 1000000.0;
    
    /* update */
    get_stat();
    if (backup_in > stats.rx_bytes)
    {
        cur_in = (int) stats.rx_bytes / delta_t;
    }
    else
    {
        cur_in = (int) (stats.rx_bytes - backup_in) / delta_t;
    }
	
    if (backup_out > stats.tx_bytes)
    {
        cur_out = (int) stats.tx_bytes / delta_t;
    }
    else
    {
        cur_out = (int) (stats.tx_bytes - backup_out) / delta_t;
    }

    if( in != NULL && out != NULL && tot != NULL )
    {
        *in = cur_in;
        *out = cur_out;
        *tot = *in + *out;
    }
    
    /* save 'new old' values */
    backup_in = stats.rx_bytes;
    backup_out = stats.tx_bytes;
    
    /* do the same with time */
    prev_time.tv_sec = curr_time.tv_sec;
    prev_time.tv_usec = curr_time.tv_usec;
}

void close_netload()
{
    /* We need not code here */
}

