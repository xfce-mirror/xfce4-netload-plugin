dnl configure.ac
dnl
dnl xfce4-netload-plugin - Netload plugin for xfce4-panel
dnl
dnl 2003 Benedikt Meurer <benedikt.meurer@unix-ag.uni-siegen.de>
dnl

AC_INIT([panel-plugin/netload.c])

AM_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([xfce4-netload-plugin], [0.3.3])

AM_MAINTAINER_MODE

dnl Check for basic programs
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LIBTOOL

dnl Check for standard header files
AC_HEADER_STDC

dnl Substitute in Makefiles
AC_SUBST(OS)

AC_CHECK_FUNCS([gethostname])
AC_CHECK_FUNCS([memset])
AC_CHECK_FUNCS([socket])
AC_CHECK_FUNCS([strstr])
AC_CHECK_FUNCS([malloc])
AC_CHECK_FUNCS([bzero])
AC_CHECK_HEADERS([arpa/inet.h])
AC_CHECK_HEADERS([netinet/in.h])
AC_CHECK_HEADERS([stdlib.h])
AC_CHECK_HEADERS([string.h])
AC_CHECK_HEADERS([sys/ioctl.h])
AC_CHECK_HEADERS([sys/param.h])
AC_CHECK_HEADERS([sys/socket.h])
AC_CHECK_HEADERS([sys/time.h])
AC_CHECK_HEADERS([unistd.h])
AC_CHECK_HEADERS([sys/sockio.h])
AC_HEADER_SYS_WAIT
AC_PROG_GCC_TRADITIONAL
AC_TYPE_SIZE_T

AC_CHECK_LIB(kstat, kstat_open, SOLLIBS="-lkstat -lsocket", SOLLIBS="")
AC_CHECK_LIB(nsl, kstat_open, SOLLIBS="$SOLLIBS -linet_ntop", SOLLIBS="$SOLLIBS")
AC_SUBST(SOLLIBS)

dnl Check for i18n support
BM_I18N([xfce4-netload], [ca de eu fr hu ko lt pl uk ru es pt_BR])

dnl configure the panel plugin
XFCE_PANEL_PLUGIN([XFCE4_PANEL], [4.0.0])

dnl Check for debugging support
BM_DEBUG_SUPPORT

AC_OUTPUT([
Makefile
panel-plugin/Makefile
po/Makefile.in
])



