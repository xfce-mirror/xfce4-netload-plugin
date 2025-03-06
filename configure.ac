dnl configure.ac
dnl
dnl xfce4-netload-plugin - Netload plugin for xfce4-panel
dnl
dnl Copyright 2003 Benedikt Meurer <benedikt.meurer@unix-ag.uni-siegen.de>
dnl Copyright 2010 Florian Rivoal <frivoal@xfce.org>
dnl Copyright 2014 Mike Massonnet <mmassonnet@xfce.org>
dnl

dnl ***************************
dnl *** Version information ***
dnl ***************************
XDT_VERSION_INIT([1.4.2], [git])

AC_COPYRIGHT([Copyright (c) 2003-2024 The Xfce development team. All rights reserved.])
AC_INIT([xfce4-netload-plugin], [xdt_version], [https://gitlab.xfce.org/panel-plugins/xfce4-netload-plugin])
AC_REVISION([xdt_version_build])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIRS([m4])
AC_PREREQ([2.69])
AC_CANONICAL_TARGET()

AM_INIT_AUTOMAKE([1.8 no-dist-gzip dist-bzip2 foreign])
AM_MAINTAINER_MODE
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

LT_PREREQ([2.2.6])
LT_INIT([disable-static])

dnl Check for basic programs
AC_PROG_CC
AC_PROG_INSTALL

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
AC_CHECK_LIB(nsl, inet_ntop, SOLLIBS="$SOLLIBS -lnsl", SOLLIBS="$SOLLIBS")
AC_SUBST(SOLLIBS)

dnl configure the panel plugin
XDT_CHECK_PACKAGE([GLIB], [glib-2.0], [2.50.0])
XDT_CHECK_PACKAGE([GTK], [gtk+-3.0], [3.22.0])
XDT_CHECK_PACKAGE([LIBXFCE4PANEL], [libxfce4panel-2.0], [4.16.0])
XDT_CHECK_PACKAGE([LIBXFCE4UI], [libxfce4ui-2], [4.16.0])
XDT_CHECK_PACKAGE([LIBXFCE4UTIL], [libxfce4util-1.0], [4.16.0])

dnl Check for i18n support
GETTEXT_PACKAGE="$PACKAGE"
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["$GETTEXT_PACKAGE"], [Name of default gettext domain])
AC_SUBST([GETTEXT_PACKAGE])
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.19.8])

dnl Check for debugging support
XDT_FEATURE_DEBUG([xdt_debug_default])

AC_CONFIG_FILES([
Makefile
panel-plugin/Makefile
po/Makefile.in
icons/Makefile
icons/16x16/Makefile
icons/32x32/Makefile
icons/128x128/Makefile
icons/scalable/Makefile
])
AC_OUTPUT
