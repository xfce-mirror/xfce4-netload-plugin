/* $Id: linux.c,v 1.3 2003/08/31 12:54:36 bwalle Exp $ */



/*****************************************************************************
 *
 * init_osspecific()
 *
 * Init function
 *
 ****************************************************************************/

void init_osspecific(netdata* data)
{
#ifdef DEBUG
    fprintf( stderr, "The netload plugin was initialized for Linux.\n" );
#endif
}


/*****************************************************************************
 *
 * checkinterface()
 *
 * check if a given interface exists and is up.
 * return TRUE if found, FALSE if not
 *
 ****************************************************************************/

int checkinterface(netdata* data)
{
	int interfacefound = FALSE;
	unsigned int i;
	struct if_nameindex *ifs;

#ifdef DEBUG
    fprintf( stderr, "Checking the interface '%s' now ...\n", data->ifdata.if_name );
#endif

    
	if ((ifs = if_nameindex()) == NULL)
		return FALSE;

	for (i = 0; ifs[i].if_index; i++)
	{
		if (strcmp(ifs[i].if_name, data->ifdata.if_name) == 0)
		{
			interfacefound = TRUE;
			break;
		}
	}

	return interfacefound;
}

/******************************************************************************
 *
 * get_stat()
 *
 * read the network statistics from /proc/net/dev (PATH_NET_DEV)
 * if the file is not open open it. fseek() to the beginning and parse
 * each line until we've found the right interface
 *
 * returns 0 if successful, 1 in case of error
 *
 *****************************************************************************/

int get_stat(netdata* data)
{
    /* bwalle: Instead of the original code we open the file each time new. The
     * performance difference is _very_ minimal. But I don't think that it's a good
     * idea to keep the file open for a very long time for _each_ plugin instance.
     */
    char buffer[BUFSIZE];
    char *ptr;
    char *devname;
    int dump;
    int interfacefound;
    FILE* proc_net_dev;
    unsigned long rx_o, tx_o;

    if ((proc_net_dev = fopen(PATH_NET_DEV, "r")) == NULL)
    {
        fprintf(stderr, "cannot open %s!\nnot running Linux?\n",
            PATH_NET_DEV);
        return 1;
    }

    /* backup old rx/tx values */
    rx_o = data->stats.rx_bytes; tx_o = data->stats.tx_bytes;

    /* do not parse the first two lines as they only contain static garbage */
    fseek(proc_net_dev, 0, SEEK_SET);
    fgets(buffer, BUFSIZ-1, proc_net_dev);
    fgets(buffer, BUFSIZ-1, proc_net_dev);

    interfacefound = 0;
    while (fgets(buffer, BUFSIZ-1, proc_net_dev) != NULL)
    {
        /* find the device name and substitute ':' with '\0' */
        ptr = buffer;
        while (*ptr == ' ')
            ptr++;
        devname = ptr;
        while (*ptr != ':')
            ptr++;
        *ptr = '\0';
        ptr++;
        if (!strcmp(devname, (char *) data->ifdata.if_name))
        {
            /* read stats and fill struct */
            sscanf(ptr, "%lg %lu %lu %d %d %d %d %d %lg %lu %lu %d %d %d %d %d",
                &(data->stats.rx_bytes), &(data->stats.rx_packets), &(data->stats.rx_errors),
                &dump, &dump, &dump, &dump, &dump,
                &(data->stats.tx_bytes), &(data->stats.tx_packets), &(data->stats.tx_errors),
                &dump, &dump, &dump, &dump, &dump);
            interfacefound = 1;
            continue; /* break, as we won't get any new information */
        }
    }
    fclose( proc_net_dev );
    if (interfacefound)
    {
        if (rx_o > data->stats.rx_bytes)
            data->stats.rx_over++;
        if (tx_o > data->stats.tx_bytes)
            data->stats.tx_over++;
    }
    return (interfacefound == 1)? 0 : 1;
}
