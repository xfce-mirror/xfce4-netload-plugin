#ifndef NETBSD_H
#define NETBSD_H

#include "../net.h"

void init_osspecific(netdata* data);
int checkinterface(netdata* data);
int get_stat(netdata* data);

#endif /* NETBSD_H */
