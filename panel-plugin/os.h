/*
 * Copyright 2003, 2005, 2006 Bernhard Walle <bernhard@bwalle.de>
 * Copyright 2010 Florian Rivoal <frivoal@gmail.com>
 * -------------------------------------------------------------------------------------------------
 * 
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the Free
 *  Software Foundation; either version 2 of the License, or (at your option)
 *  any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 675 Mass
 * Ave, Cambridge, MA 02139, USA.
 *
 * ------------------------------------------------------------------------------------------------- 
 */
#ifndef _OS_H
#define _OS_H

#if defined (__sun__)
#  define __Solaris__ 1
#endif

#ifdef __HPUX__             /* H P U X */
#  define _XOPEN_SOURCE_EXTENDED
#  include <stdio.h>
#  include <sys/param.h>
#  include <stdlib.h>
#  include <stdarg.h>
#  include <stropts.h>
#  include <unistd.h>
#  include <string.h>
#  include <time.h>
#  include <ctype.h>
#  include <signal.h>
#  include <sys/stdsyms.h>
#  include <sys/wait.h>
#  include <sys/stat.h>
#  include <sys/ioctl.h>
#  include <sys/types.h>
#  include <sys/socket.h>
#  include <sys/time.h>
#  include <netinet/in.h>
#  include <net/if.h>
#  include <sys/mib.h>
#  include <arpa/inet.h>
#  include <net/if.h>
#elif __APPLE__             /* Mac OS X */
#  include <stdio.h>
#  include <stdlib.h>
#  include <stdarg.h>
#  include <unistd.h>
#  include <string.h>
#  include <time.h>
#  include <ctype.h>
#  include <signal.h>
#  include <curses.h>
#  include <ifaddrs.h>
#  include <sys/param.h>
#  include <sys/sysctl.h>
#  include <sys/wait.h>
#  include <sys/stat.h>
#  include <sys/ioctl.h>
#  include <sys/types.h>
#  include <sys/socket.h>
#  include <sys/time.h>
#  include <netinet/in.h>
#  include <net/route.h>
#  include <net/if_dl.h>
#  include <net/if.h>
#  include <net/if_media.h>
#  include <net/if_mib.h>
#  include <arpa/inet.h>
#elif __DragonFly__           /* D R A G O N F L Y */
#  include <stdio.h>
#  include <stdlib.h>
#  include <stdarg.h>
#  include <unistd.h>
#  include <string.h>
#  include <time.h>
#  include <ctype.h>
#  include <signal.h>
#  include <sys/param.h>
#  include <sys/sysctl.h>
#  include <sys/wait.h>
#  include <sys/stat.h>
#  include <sys/ioctl.h>
#  include <sys/types.h>
#  include <sys/socket.h>
#  include <sys/time.h>
#  include <netinet/in.h>
#  include <net/if.h>
#  include <net/if_media.h>
#  include <net/if_mib.h>
#  include <arpa/inet.h>
#elif __FreeBSD__ || __FreeBSD_kernel__           /* F R E E B S D */
#  include <stdio.h>
#  include <stdlib.h>
#  include <stdarg.h>
#  include <unistd.h>
#  include <string.h>
#  include <time.h>
#  include <ctype.h>
#  include <signal.h>
#  include <sys/param.h>
#  include <sys/sysctl.h>
#  include <sys/wait.h>
#  include <sys/stat.h>
#  include <sys/ioctl.h>
#  include <sys/types.h>
#  include <sys/socket.h>
#  include <sys/time.h>
#  include <netinet/in.h>
#  include <net/if.h>
#  include <net/if_media.h>
#  include <net/if_mib.h>
#  include <arpa/inet.h>
#elif __NetBSD__            /* N E T B S D */
#  include <stdio.h>
#  include <sys/param.h>
#  include <sys/sysctl.h>
#  include <stdlib.h>
#  include <stdarg.h>
#  include <unistd.h>
#  include <string.h>
#  include <time.h>
#  include <ctype.h>
#  include <signal.h>
#  include <sys/wait.h>
#  include <sys/stat.h>
#  include <sys/ioctl.h>
#  include <sys/types.h>
#  include <sys/socket.h>
#  include <sys/time.h>
#  include <netinet/in.h>
#  include <net/if.h>
#  include <net/if_media.h>
#  include <net/route.h>
#  include <net/if_dl.h>
#  include <arpa/inet.h>
#  include <net/if.h>
#elif __OpenBSD__ || __MicroBSD__           /* O P E N B S D */
#  include <stdio.h>
#  include <sys/param.h>
#  include <sys/sysctl.h>
#  include <stdlib.h>
#  include <stdarg.h>
#  include <unistd.h>
#  include <string.h>
#  include <time.h>
#  include <ctype.h>
#  include <signal.h>
#  include <sys/wait.h>
#  include <sys/stat.h>
#  include <sys/ioctl.h>
#  include <sys/types.h>
#  include <sys/socket.h>
#  include <sys/time.h>
#  include <netinet/in.h>
#  include <net/if.h>
#  include <net/if_media.h>
#  include <net/if_dl.h>
#  include <net/route.h>
#  include <arpa/inet.h>
#elif __linux__             /* L I N U X */
#  include <stdio.h>
#  include <sys/param.h>
#  include <sys/sysctl.h>
#  include <stdlib.h>
#  include <stdarg.h>
#  include <unistd.h>
#  include <string.h>
#  include <time.h>
#  include <ctype.h>
#  include <signal.h>
#  include <sys/wait.h>
#  include <sys/stat.h>
#  include <sys/ioctl.h>
#  include <sys/types.h>
#  include <sys/socket.h>
#  include <sys/time.h>
#  include <netinet/in.h>
#  include <net/if.h>
#  include <arpa/inet.h>
#elif __Solaris__           /* S O L A R I S */
#  include <stdio.h>
#  define _WIDEC_H
#  include <sys/param.h>
#  include <stdlib.h>
#  include <stdarg.h>
#  include <unistd.h>
#  include <string.h>
#  include <time.h>
#  include <ctype.h>
#  include <signal.h>
#  include <sys/wait.h>
#  include <sys/stat.h>
#  include <sys/ioctl.h>
#  include <sys/types.h>
#  include <sys/socket.h>
#  include <sys/time.h>
#  include <netinet/in.h>
#  include <net/if.h>
#  include <stropts.h>
#  include <sys/socket.h>
#  include <sys/sockio.h>
#  include <net/if.h>
#  include <kstat.h>
#  include <net/if.h>
#  include <sys/sockio.h>
#  include <arpa/inet.h>
#  include <net/if.h>
#else
#  error "OS not supported"
#endif

#endif
