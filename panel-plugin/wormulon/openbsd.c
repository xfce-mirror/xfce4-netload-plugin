/* $Id: openbsd.c,v 1.2 2003/08/25 21:08:58 bwalle Exp $ */


/*****************************************************************************
 *
 * init_osspecific()
 *
 * Init function
 *
 ****************************************************************************/

void init_osspecific(netdata* data)
{
    mib_name1[0] = CTL_NET;
    mib_name1[1] = PF_ROUTE;
    mib_name1[2] = 0;
    mib_name1[3] = 0;
    mib_name1[4] = NET_RT_IFLIST;
    mib_name1[5] = 0;
    
    mib_name2[0] = CTL_NET;
    mib_name2[1] = PF_ROUTE;
    mib_name2[2] = 0;
    mib_name2[3] = 0;
    mib_name2[4] = NET_RT_IFLIST;
    mib_name2[5] = 0;
}

/*****************************************************************************
 *
 * checkinterface()
 *
 * check if a given interface exists, return TRUE if it does and FALSE if not
 *
 ****************************************************************************/
int checkinterface(netdata* data)
{
    int validinterface = FALSE;
    char *lim, *next;
    struct if_msghdr *ifm, *nextifm;
    struct sockaddr_dl *sdl;
    size_t needed;
    char s[32];

    if (sysctl(data->mib_name1, 6, NULL, &needed, NULL, 0) < 0)
        return FALSE;
    if (data->alloc1 < (signed long) needed)
    {
        if (data->buf1 != NULL)
            free (data->buf1);
        data->buf1 = malloc(needed);
        if (data->buf1 == NULL)
            return FALSE;
        data->alloc1 = needed;
    }

    if (sysctl(data->mib_name1, 6, data->buf1, &needed, NULL, 0) < 0)
        return FALSE;

    lim = data->buf1 + needed;
    next = data->buf1;
    while ((next < lim) && (validinterface == 0))
    {
        ifm = (struct if_msghdr *)next;
        if (ifm->ifm_type != RTM_IFINFO)
            return FALSE;
        next += ifm->ifm_msglen;

        while (next < lim)
        {
            nextifm = (struct if_msghdr *)next;
            if (nextifm->ifm_type != RTM_NEWADDR)
                break;
            next += nextifm->ifm_msglen;
        }

        if (ifm->ifm_flags & IFF_UP)
        {
            sdl = (struct sockaddr_dl *)(ifm + 1);
            strncpy(s, sdl->sdl_data, sdl->sdl_nlen);
            s[sdl->sdl_nlen] = '\0';
            /* search for the right network interface */
            if (sdl->sdl_family != AF_LINK)
                continue;
            if (strcmp(s, data->ifdata.if_name) != 0)
                continue;
            else
            {
                validinterface = TRUE;
                break; /* stop searching */
            }
        }
    }
    return validinterface;
}

/*****************************************************************************
 *
 * get_stat()
 *
 * this code is based on gkrellm code (thanks guys!)
 *
 ****************************************************************************/

int get_stat(netdata* data)
{
    char *lim, *next;
    struct if_msghdr *ifm, *nextifm;
    struct sockaddr_dl *sdl;
    char s[32];
    size_t needed;
    unsigned long rx_o, tx_o;

    if (sysctl(data->mib_name2, 6, NULL, &needed, NULL, 0) < 0)
        return 1;
    if (data->alloc2 < (signed long) needed)
    {
        if (data->buf2 != NULL)
            free (data->buf2);
        data->buf2 = malloc(needed);
        if (data->buf2 == NULL)
            return 1;
        data->alloc2 = needed;
        }

    if (sysctl(data->mib_name2, 6, data->buf2, &needed, NULL, 0) < 0)
        return 1;
    lim = data->buf2 + needed;
    next = data->buf2;
    while (next < lim)
    {
        ifm = (struct if_msghdr *)next;
        if (ifm->ifm_type != RTM_IFINFO)
            return 1;
        next += ifm->ifm_msglen;

        while (next < lim)
        {
            nextifm = (struct if_msghdr *)next;
            if (nextifm->ifm_type != RTM_NEWADDR)
                break;
            next += nextifm->ifm_msglen;
        }

        if (ifm->ifm_flags & IFF_UP)
        {
            sdl = (struct sockaddr_dl *)(ifm + 1);
            /* search for the right network interface */
            if (sdl->sdl_family != AF_LINK)
                continue;
            if (strcmp(sdl->sdl_data, data->ifdata.if_name) != 0)
                continue;
            strncpy(s, sdl->sdl_data, sdl->sdl_nlen);
            s[sdl->sdl_nlen] = '\0';

            rx_o = data->stats.rx_bytes; tx_o = data->stats.tx_bytes;
            /* write stats */
            data->stats.tx_packets = ifm->ifm_data.ifi_opackets;
            data->stats.rx_packets = ifm->ifm_data.ifi_ipackets;
            data->stats.rx_bytes = ifm->ifm_data.ifi_ibytes;
            data->stats.tx_bytes = ifm->ifm_data.ifi_obytes;
            data->stats.rx_errors = ifm->ifm_data.ifi_ierrors;
            data->stats.tx_errors = ifm->ifm_data.ifi_oerrors;

            if (rx_o > data->stats.rx_bytes)
                data->stats.rx_over++;
            if (tx_o > data->stats.tx_bytes)
                data->stats.tx_over++;
        }
    }
    return 0;
}
