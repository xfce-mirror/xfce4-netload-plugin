/*
 * Copyright 2003,2005 Bernhard Walle <bernhard@bwalle.de>
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

netdata data;

_Noreturn void sig_end_handler (int sig);

/* ---------------------------------------------------------------------------------------------- */
_Noreturn void sig_end_handler (int sig)
{
    close_netload(&data);
    exit(0);
}


/* ---------------------------------------------------------------------------------------------- */
int main(int argc, char* argv[])
{
    unsigned long in, out, tot;
    char* device;
    char bufIn[20], bufOut[20], bufTot[20], bufBS[20];
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
        format_byte_humanreadable(bufIn, 20, (double)in, 0, FALSE);
        format_byte_humanreadable(bufOut, 20, (double)out, 0, FALSE);
        format_byte_humanreadable(bufTot, 20, (double)tot, 2, TRUE);
        format_byte_humanreadable(bufBS, 20, 12235345.0, 2, TRUE);
        printf("Current netload:\nIN : %s\nOUT: %s\nTOT: %s\nBS: %s\nIP: %s\n",
            bufIn, bufOut, bufTot, bufBS, get_ip_address(&data));
        sleep(1);
    }
    
    return 0;
}
