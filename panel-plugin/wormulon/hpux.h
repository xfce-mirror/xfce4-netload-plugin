#ifndef HPUX_H
#define HPUX_H

#include "../net.h"

void init_osspecific(netdata* data);
int checkinterface(netdata* data);
int get_stat(netdata* data);

#endif /* HPUX_H */
