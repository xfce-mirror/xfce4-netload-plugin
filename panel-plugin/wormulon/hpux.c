#include <netio.h>
#define WAIT_PCKS_COUNTER   15

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
    wait_pcks_counter = WAIT_PCKS_COUNTER+1;
    
#ifdef DEBUG
    fprintf( stderr, "The netload plugin was initialized for HP_UX.\n" );
#endif

}



/*****************************************************************************
 *
 * _countinterfaces()
 *
 * count all network interfaces in the system. This function is intended to 
 * use it only in hpux.c
 *
 ****************************************************************************/

int _countinterfaces(void)
{
    int     val, num_iface=-1, sd;
    if ((sd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        return (-1);

    if (ioctl(sd, SIOCGIFNUM, &val) != -1)
        num_iface = val;
    close(sd);

    return num_iface;
}

/*****************************************************************************
 *
 * _getifdata()
 *
 * get the Interface-ID, the Interface-Speed, and over all, check if the
 * given interface really exists. This function is intended to use it only in
 * hpux.c
 *
 ****************************************************************************/

void _getifdata()
{
    int buffer, fd, val, ret = -1;
    unsigned int len, i;
    char tmpinterfacestring[sizeof(data->ifdata.if_name)+1],*strstrmatch;
    struct nmparms params;
    mib_ifEntry * if_buf;

    
    /*
     * The interface description is more then the pure devicename.
     * Let's do some formating to allow a propper pattern matching
     */
    strcpy(tmpinterfacestring,data->ifdata.if_name);
    strcat(tmpinterfacestring," ");

    for (i=0; i <= data->ifdata.if_amount; i++)
    {
        if ((fd = open_mib("/dev/lan", O_RDWR, i, 0)) >= 0)
        {
            if ((if_buf = (mib_ifEntry *) malloc (sizeof(mib_ifEntry))) != 0) {
                params.objid  = ID_ifEntry;
                params.buffer = if_buf;
                len = sizeof(mib_ifEntry);
                params.len    = &len;
                if_buf->ifIndex = i+1;
                if ((ret = get_mib_info(fd, &params)) == 0) {
                    /*
                     * The interface given by the user must start at the
                     * beginning of if_buf->ifDescr. If that's the case,
                     * strstrmatch is equal to if_buf->ifDescr. If not,
                     * strstrmatch might be a subset of if_buf->ifDescr,
                     * or NULL
                     */
                    strstrmatch = strstr(if_buf->ifDescr, (char *)tmpinterfacestring);
                    if ( strstrmatch && (strcmp(strstrmatch,if_buf->ifDescr)== 0))
                    {
                        data->ifdata.if_valid = 1;
                        data->ifdata.if_id = i+1;
                        break;
                    }
                }
            }
        }
        free(if_buf);
        close_mib(fd);
    }
    return;
}

/*****************************************************************************
 *
 * checkinterface()
 *
 * check if a given interface exists, return 1 if it does and 0 if not (This
 * function is a wrapper function for _countinterfaces && _getifdata.)
 *
 ****************************************************************************/
int checkinterface(netdata* data)
{
    /* ==  0 no network interfaces, -1 sth. went wrong */
    if ((data->ifdata.if_amount =_countinterfaces()) > 0)
        _getifdata();
    return data->ifdata.if_valid;
}

/******************************************************************************
 *
 * get_stat()
 *
 * stub function for all unsupported operating systems
 *
 *****************************************************************************/

int get_stat(netdata* data)
{
    int             i,fd, ret=-1;
    unsigned int    len;
    unsigned long   rx_o, tx_o;
    struct          nmparms params, params2;
    mib_ifEntry     *if_buf;

    if (data->ifdata.if_valid == 1 && (fd = open_mib("/dev/lan", O_RDWR, 0, 0)) >= 0)
    {
        if ((if_buf = (mib_ifEntry *) malloc (sizeof(mib_ifEntry))) != 0)
        {
            if_buf->ifIndex = data->ifdata.if_id;
            params.objid  = ID_ifEntry;
            params.buffer = if_buf;
            len = (unsigned int) sizeof(mib_ifEntry);
            params.len    = &len;
            if ((ret = get_mib_info(fd, &params)) == 0)
            {
                rx_o = data->stats.rx_bytes; tx_o = data->stats.tx_bytes;

                data->stats.tx_bytes = if_buf->ifOutOctets;
                data->stats.rx_bytes = if_buf->ifInOctets;
                data->stats.tx_errors = if_buf->ifOutErrors;
                data->stats.rx_errors = if_buf->ifInErrors;

                if (rx_o > data->stats.rx_bytes)
                    data->stats.rx_over++;
                if (tx_o > data->stats.tx_bytes)
                    data->stats.tx_over++;
            }
        }
        free(if_buf);

        /*
         * Getting the tx/rx packets every run often hurts to much performance
         * With WAIT_PCKS_COUNTER=15 i save on my system 43% cpu usage.instead of
         * WAIT_PCKS_COUNTER=0
         */
        if( data->wait_pcks_counter > WAIT_PCKS_COUNTER )
        {
            if ((if_ptr = (nmapi_logstat *) malloc(sizeof(nmapi_logstat) * data->ifdata.if_amount)) != 0 )
            {
                len = (unsigned int) data->ifdata.if_amount *sizeof(nmapi_logstat);
                if ((ret = get_logical_stat(if_ptr, &len)) == 0)
                {
                    for (i=0; i <= data->ifdata.if_amount; i++)
                    {
                        if(if_ptr[i].ifindex == data->ifdata.if_id)
                        {
                            data->stats.tx_packets = if_ptr[i].out_packets;
                            data->stats.rx_packets = if_ptr[i].in_packets;
                        }
                    }
                }
            }
            free(if_ptr);
            data->wait_pcks_counter = 0;
        }
        else
        {
            data->wait_pcks_counter++;
        }
    }
    close_mib(fd);

    return(0);
}
