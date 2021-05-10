/*
 * Copyright 2003,2005,2006 Bernhard Walle <bernhard@bwalle.de>
 * Copyright 2010 Florian Rivoal <frivoal@gmail.com>
 * -------------------------------------------------------------------------------------------------
 * 
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the Free
 *  Software Foundation; either version 2 of the License, or (at your option)
 *  any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 675 Mass
 * Ave, Cambridge, MA 02139, USA.
 *
 * ------------------------------------------------------------------------------------------------- 
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

#include <libxfce4util/libxfce4util.h>


/* From Wormulon */
#include "net.h"
#include "os.h"
#include "wormulon.h"
#include "slurm.h"	/* slurm structs */

#include <sys/types.h>
#include <errno.h>

#if defined(__HPUX__)
# include "wormulon/hpux.h"
# include "wormulon/hpux.c"
#elif defined(__APPLE__)
# include "src/macos.h"
# include "src/macos.c"
#elif (defined(__FreeBSD__) || defined(__DragonFly__) || defined(__FreeBSD_kernel__))
# include "wormulon/freebsd.h"
# include "wormulon/freebsd.c"
#elif defined(__linux__)
# include "wormulon/linux.h"
# include "wormulon/linux.c"
#elif (defined(__OpenBSD__) || defined(__MicroBSD__))
# include "wormulon/openbsd.h"
# include "wormulon/openbsd.c"
#elif defined(__NetBSD__)
# include "wormulon/netbsd.h"
# include "wormulon/netbsd.c"
#elif defined(__Solaris__)
# include "wormulon/solaris.h"
# include "wormulon/solaris.c"
#else
/* should not get here */
# error "OS not supported"
#endif


/* ---------------------------------------------------------------------------------------------- */
int init_netload(netdata* data, const char* device)
{
    memset( data, 0, sizeof(netdata) );
    
    if (device == NULL || strlen(device) == 0)
    {
        return TRUE;
    }
    
    g_strlcpy(data->ifdata.if_name, device, sizeof(data->ifdata.if_name));
    
    init_osspecific( data );
    
    data->ip_address[0] = 0;
    data->ip_update_count = 0;
    data->up = FALSE;
    data->up_update_count = 0;
    
    if (checkinterface(data) != TRUE)
	{
        data->correct_interface = FALSE;
		return FALSE;
	}
    
    /* init in a sane state */
    get_stat(data);
    data->backup_in  = data->stats.rx_bytes;
    data->backup_out = data->stats.tx_bytes;
    
    data->correct_interface = TRUE;
    
    DBG("The netload plugin was initialized for '%s'.", device);
    
    return TRUE;
}


/* ---------------------------------------------------------------------------------------------- */
void get_current_netload(netdata* data, unsigned long *in, unsigned long *out, unsigned long *tot) 
{
    struct timeval curr_time;
    double delta_t;
    
    if (! data->correct_interface)
    {
        if (in != NULL && out != NULL && tot != NULL)
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

    if (in != NULL && out != NULL && tot != NULL)
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


/* ---------------------------------------------------------------------------------------------- */
char* get_name(netdata* data)
{
    return data->ifdata.if_name;
}


/* ---------------------------------------------------------------------------------------------- */
int get_interface_up(netdata* data)
{
    int sockfd;
    struct ifreq ifr;
        
    /* if the update count is non-zero */ 
    if (data->up_update_count > 0)
    {
        data->up_update_count--;
        return data->up;
    }

    /* get the value from the operating system */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        DBG("Error in socket: %s", strerror(errno));
        return FALSE;
    }
    
    g_snprintf(ifr.ifr_name, IF_NAMESIZE, "%s", data->ifdata.if_name);
    if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) != 0)
    {
        DBG("Error in ioctl(sockfd): %s", strerror(errno));
        close(sockfd);
        return FALSE;
    }
    close(sockfd);

    data->up = ((ifr.ifr_flags & IFF_UP) == IFF_UP) ? TRUE : FALSE;
    data->up_update_count = UP_UPDATE_INTERVAL;

    return data->up;
}


/* ---------------------------------------------------------------------------------------------- */
char* get_ip_address(netdata* data)
{
    int sockfd;
    struct ifreq ifr;
    union {
        struct sockaddr *addr;
        struct sockaddr_in *p_sa;
    } sa;

    /* use cached value if possible and if the update count is non-zero */ 
    if (data->ip_address && data->ip_update_count > 0)
    {
        data->ip_update_count--;
        return data->ip_address;
    }
    
    /* get the value from the operating system */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        DBG("Error in socket: %s", strerror(errno));
        return NULL;
    }
    
    g_snprintf(ifr.ifr_name, IF_NAMESIZE, "%s", data->ifdata.if_name);
    if (ioctl(sockfd, SIOCGIFADDR, &ifr) != 0)
    {
	    if (errno != EADDRNOTAVAIL)
        {
            DBG("Error in ioctl(sockfd): %s", strerror(errno));
        }
        close(sockfd);
        return NULL;
    }
    close(sockfd);

    sa.addr = &ifr.ifr_addr;

    if (!inet_ntop(AF_INET, &(sa.p_sa->sin_addr), data->ip_address, IP_ADDRESS_LENGTH))
    {
        DBG("Error in inet_ntop: %s", strerror(errno));
        return NULL;
    }

    /* now updated */
    data->ip_update_count = IP_UPDATE_INTERVAL;

    return data->ip_address;
}


/* ---------------------------------------------------------------------------------------------- */
void close_netload(netdata* data)
{
    /* We need not code here */
}

