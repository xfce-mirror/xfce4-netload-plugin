#ifndef OPENBSD_H
#define OPENBSD_H

#include "../net.h"

void init_osspecific(netdata* data);
int checkinterface(netdata* data);
int get_stat(netdata* data);

#endif /* OPENBSD_H */
