plugindir = @XFCE4_PANEL_PLUGINSDIR@
LIBS = @LIBS@ @SOLLIBS@

EXTRA_DIST =								    \
	wormulon commandline.c

plugin_LTLIBRARIES =							\
	libnetload.la

libnetload_la_LDFLAGS =							\
	-avoid-version							    \
	-export-dynamic							    \
	-module

libnetload_la_SOURCES =							\
	netload.c							        \
    utils.c                                     \
    utils.h                                     \
	net.h								        \
	net.c                                       \
    os.h                                        \
    wormulon.h                                  \
    global.h                                    \
    slurm.h                                     

libnetload_la_CFLAGS =							\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"		\
	@XFCE4_PANEL_CFLAGS@

libnetload_la_LIBADD = 							\
    @SOLLIBS@                                   \
	@XFCE4_PANEL_LIBS@
