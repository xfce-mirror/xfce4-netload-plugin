dnl configure.ac
dnl
dnl xfce4-netload-plugin - Netload plugin for xfce4-panel
dnl
dnl 2003 Benedikt Meurer <benedikt.meurer@unix-ag.uni-siegen.de>
dnl

AC_INIT([panel-plugin/netload.c])

AM_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([xfce4-netload-plugin], [0.1.3])

AM_MAINTAINER_MODE

dnl Check for basic programs
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LIBTOOL

dnl Check for standard header files
AC_HEADER_STDC

dnl Determine operating system
case $host_os in
*linux*)
	OS=linux
	;;
*freebsd*)
	OS=freebsd
	AC_CHECK_HEADERS(	
		[fcntl.h sys/socket.h sys/sysctl.h net/if.h net/if_mib.h],
		[],
		[AC_MSG_ERROR([You must have fcntl.h sys/socket.h sys/sysctl.h net/if.h net/if_mib.h])]
	)
	;;
*)
	echo ""
	echo "Sorry, $host_os is not supported yet."
	echo ""
	exit 1
	;;
esac

dnl Substitute in Makefiles
AC_SUBST(OS)


AC_CHECK_HEADERS(	[sys/time.h time.h],
					[],
					[AC_MSG_ERROR([We need sys/time.h and time.h])]
				)


dnl Check for i18n support
BM_I18N([xfce4-netload], [de])

dnl configure the panel plugin
XFCE_PANEL_PLUGIN([XFCE4_PANEL], [3.99.2])

dnl Check for debugging support
BM_DEBUG_SUPPORT

AC_OUTPUT([
Makefile
panel-plugin/Makefile
po/Makefile.in
])



