
#include "net.h"

void init_osspecific(netdata* data);
int checkinterface(netdata* data);
int get_stat(netdata* data);

#ifdef __linux__
#define BUFSIZE 256
#endif
