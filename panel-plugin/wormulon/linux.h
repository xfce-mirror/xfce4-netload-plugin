#ifndef LINUX_H
#define LINUX_H

#include "../net.h"

void init_osspecific(netdata* data);
int checkinterface(netdata* data);
int get_stat(netdata* data);

#ifdef __linux__
#define BUFSIZE 256
#endif /* _linux_ */

#endif /* LINUX_H */
