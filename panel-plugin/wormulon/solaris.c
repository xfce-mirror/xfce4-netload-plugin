/* $Id$ */


/*****************************************************************************
 *
 * init_osspecific()
 *
 * Init function
 *
 ****************************************************************************/

void init_osspecific(netdata* data)
{
    /* nothing */
    
#ifdef DEBUG
    fprintf( stderr, "The netload plugin was initialized for Sun Solaris.\n" );
#endif

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
    int sockfd, i, numifs, numifreqs;
    size_t bufsize;
    char *buf;
    struct ifreq ifr, *ifrp;
    struct ifconf ifc;
    unsigned long rx_o, tx_o;

    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {   
        perror("socket");
        return FALSE;
    }
    if (ioctl(sockfd, SIOCGIFNUM, (char *) &numifs) < 0)
    {   
        perror("SIOCGIFNUM");
        close(sockfd);
        return FALSE;
    }
    bufsize = ((size_t) numifs) * sizeof(struct ifreq);
    buf = (char *) malloc(bufsize);
    if (!buf)
    {   
        perror("malloc");
        close(sockfd);
        return FALSE;
    }

    ifc.ifc_len = bufsize;
    ifc.ifc_buf = buf;

    if (ioctl(sockfd, SIOCGIFCONF, (char *) &ifc) < 0)
    {   
        perror("SIOCGIFCONF");
        close(sockfd);
        free(buf);
        return FALSE;
    }

    ifrp = ifc.ifc_req;
    numifreqs = ifc.ifc_len / sizeof(struct ifreq);

    for (i = 0; i < numifreqs; i++, ifrp++)
    {
        memset((char *)&ifr, 0, sizeof(ifr));
        strncpy(ifr.ifr_name, ifrp->ifr_name, sizeof(ifr.ifr_name));
        /* do not check for loopback device as it cannot be monitored */
        if (!strncmp(ifr.ifr_name, "lo", 2))
            continue;
        if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0)
        {   
            perror("SIOCGIFFLAGS");
            continue;
        }
        if (!strcmp(data->ifdata.if_name, ifr.ifr_name) && (ifr.ifr_flags & IFF_UP))
        {
            validinterface = TRUE;
            break;
        }
    }
    free(buf);
    close(sockfd);

    return validinterface;
}

/*****************************************************************************
 *
 * get_stat()
 *
 * use the Solaris kstat_*() interface to gather statistics
 * We have to open/close *kc each time :(
 *
 ****************************************************************************/

int get_stat(netdata* data)
{
    kstat_t *ksp;
    kstat_named_t *knp;
    kstat_ctl_t *kc;
    unsigned long rx_o, tx_o;

    if ((kc = kstat_open()) == NULL)
    {   
        perror("kstat_open()");
        return 1;
    }

    rx_o = data->stats.rx_bytes; tx_o = data->stats.tx_bytes;

    ksp = kstat_lookup(kc, NULL, -1, data->ifdata.if_name);
    if (ksp && kstat_read(kc, ksp, NULL) >= 0)
    {   
        knp = (kstat_named_t *)kstat_data_lookup(ksp, "opackets");
        if (knp)
            data->stats.tx_packets = knp->value.ui32;
        knp = (kstat_named_t *)kstat_data_lookup(ksp, "ipackets");
        if (knp)
            data->stats.rx_packets = knp->value.ui32;
        knp = (kstat_named_t *)kstat_data_lookup(ksp, "obytes");
        if (knp)
            data->stats.tx_bytes = knp->value.ui32;
        knp = (kstat_named_t *)kstat_data_lookup(ksp, "rbytes");
        if (knp)
            data->stats.rx_bytes = knp->value.ui32;
        knp = (kstat_named_t *)kstat_data_lookup(ksp, "oerrors");
        if (knp)
            data->stats.tx_errors = knp->value.ui32;
        knp = (kstat_named_t *)kstat_data_lookup(ksp, "ierrors");
        if (knp)
            data->stats.rx_errors = knp->value.ui32;
    }

    kstat_close(kc);

    /* check for overflows */
    if (rx_o > data->stats.rx_bytes)
        data->stats.rx_over++;
    if (tx_o > data->stats.tx_bytes)
        data->stats.tx_over++;

    return 0;
}
