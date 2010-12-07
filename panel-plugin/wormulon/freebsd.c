/*****************************************************************************
 *
 * init_osspecific()
 *
 * Init function
 *
 ****************************************************************************/

void init_osspecific(netdata* data)
{
    data->watchif = -1;
    
#ifdef DEBUG
    fprintf( stderr, "The netload plugin was initialized for FreeBSD.\n" );
#endif
}


/*****************************************************************************
 *
 * checkinterface()
 *
 * check if a given interface exists and is up.
 * return TRUE if it does and FALSE if not
 *
 ****************************************************************************/

int checkinterface(netdata* data)
{
    int validinterface = FALSE;

    int i, num_iface;
    size_t len;
    int name[6];
    struct ifmibdata ifmd;

#ifdef DEBUG
    fprintf( stderr, "Checking the interface '%s' now ...\n", data->ifdata.if_name );
#endif
    
    
    len = sizeof(num_iface);
    sysctlbyname("net.link.generic.system.ifcount", &num_iface, &len, NULL, 0);
    for (i=1; i <= num_iface; i++)
    {   
        name[0] = CTL_NET;
        name[1] = PF_LINK;
        name[2] = NETLINK_GENERIC;
        name[3] = IFMIB_IFDATA;
        name[4] = i;
        name[5] = IFDATA_GENERAL;

        len = sizeof(ifmd);
        sysctl(name, 6, &ifmd, &len, NULL, 0);
        if (strcmp(ifmd.ifmd_name, (char *)data->ifdata.if_name) == 0)
        {   
            /*
             * now we have an interface and just have to see if it's up
             * in case we just want to debug media types we disable
             * IFF_UP flags
             */
#ifndef MEDIADEBUG
            if (ifmd.ifmd_flags & IFF_UP)
#endif
                validinterface = TRUE;
            break; /* in any case we can stop searching here */
        }
    }
    return validinterface;
}

/******************************************************************************
 *
 * get_stat()
 *
 * use sysctl() to read the statistics and fill statistics struct
 *
 ****************************************************************************/

int get_stat(netdata* data)
{
    /*
     * use sysctl() to get the right interface number if !dev_opened
     * then read the data directly from the ifmd_data struct
     */

    int i, num_iface;
    size_t len;
    int name[6];
    struct ifmibdata ifmd;
    unsigned long rx_o, tx_o;

    if (!data->dev_opened)
    {   
        len = sizeof(num_iface);
        sysctlbyname("net.link.generic.system.ifcount", &num_iface, &len,
                        NULL, 0);
        for (i=1; i <= num_iface; i++)
        {   
            name[0] = CTL_NET;
            name[1] = PF_LINK;
            name[2] = NETLINK_GENERIC;
            name[3] = IFMIB_IFDATA;
            name[4] = i;
            name[5] = IFDATA_GENERAL;

            len = sizeof(ifmd);
            sysctl(name, 6, &ifmd, &len, NULL, 0);
            if (strcmp(ifmd.ifmd_name, (char *)data->ifdata.if_name) == 0)
            {   
                /* got the right interface */
#ifdef DEBUG
                fprintf( stderr, "Got the right interface.\n");
#endif
                data->watchif = i;
                data->dev_opened++;
            }
            else
            {
#ifdef DEBUG
                fprintf( stderr, "Got NOT the right interface.\n");
#endif
            }
        }
    }
    /* in any case read the struct and record statistics */
    name[0] = CTL_NET;
    name[1] = PF_LINK;
    name[2] = NETLINK_GENERIC;
    name[3] = IFMIB_IFDATA;
    name[4] = data->watchif;
    name[5] = IFDATA_GENERAL;

    len = sizeof(ifmd);
    sysctl(name, 6, &ifmd, &len, NULL, 0);

    rx_o = data->stats.rx_bytes; tx_o = data->stats.tx_bytes;

    data->stats.tx_packets = ifmd.ifmd_data.ifi_opackets;
    data->stats.rx_packets = ifmd.ifmd_data.ifi_ipackets;
    data->stats.rx_bytes = ifmd.ifmd_data.ifi_ibytes;
    data->stats.tx_bytes = ifmd.ifmd_data.ifi_obytes;
    data->stats.rx_errors = ifmd.ifmd_data.ifi_ierrors;
    data->stats.tx_errors = ifmd.ifmd_data.ifi_oerrors;

    if (rx_o > data->stats.rx_bytes)
        data->stats.rx_over++;
    if (tx_o > data->stats.tx_bytes)
        data->stats.tx_over++;

    return (0);
}
