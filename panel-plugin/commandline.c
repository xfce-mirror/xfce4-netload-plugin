/*  XFce 4 - Netload Plugin
 *    Copyright (c) 2003 Bernhard Walle <bernhard.walle@gmx.de>
 *  
 *  Id: $Id: commandline.c,v 1.1 2003/08/31 12:45:25 bwalle Exp $
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
 * This is just a command-line wrapper for the operating-system specific code in wormulon/.
 * I wrote it because with this I'm able to test on systems with no GUI. Since I'm only running
 * Linux but develop for other Operating systems this is important!
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <unistd.h>
#include <stdio.h>
#include <signal.h>

#include "net.h"
#include "utils.h"
/* This is not good style, but it works for testing and simplifies compilation */ 
#include "net.c"
#include "utils.c"

netdata data;

void sig_end_handler (int sig)
{
    close_netload(&data);
    exit(0);
}

int main (int argc, char* argv[])
{
    unsigned long in, out, tot;
    char* device;
    char bufIn[20], bufOut[20], bufTot[20];
    struct sigaction sig_struct;
    
    /* Signal fuer's Beenden */
    sig_struct.sa_handler = sig_end_handler;
    sigemptyset(&sig_struct.sa_mask);
    sig_struct.sa_flags = 0;
    
    if (sigaction(SIGINT, &sig_struct, NULL) != 0)
    {
        perror("Fehler");
    }
    
    if (argc != 2)
    {
        fprintf(stderr, "No device given. Exiting ...\n");
        return 1;
    }
    device = argv[1];
    init_netload(&data, device);
    
    for (;;)
    {
        get_current_netload(&data, &in, &out, &tot);
        format_with_thousandssep(bufIn, 20, (double)in, 2);
        format_with_thousandssep(bufOut, 20, (double)in, 2);
        format_with_thousandssep(bufTot, 20, (double)in, 2);
        printf("Current netload:\nIN : %s\nOUT: %s\nTOT: %s\n", bufIn, bufOut, bufTot);
        sleep(1);
    }
    
    return 0;
}
