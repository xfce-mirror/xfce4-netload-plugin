#ifndef SOLARIS_H
#define SOLARIS_H

#include "../net.h"

void init_osspecific(netdata* data);
int checkinterface(netdata* data);
int get_stat(netdata* data);

#endif /* SOLARIS_H */
