plugindir = $(libdir)/xfce4/panel/plugins
plugin_LTLIBRARIES = libnetload.la

LIBS = @LIBS@ @SOLLIBS@

libnetload_la_SOURCES =							\
	netload.c							\
	monitor-label.c							\
	monitor-label.h							\
	utils.c								\
	utils.h								\
	net.h								\
	net.c								\
	os.h								\
	wormulon.h							\
	slurm.h

libnetload_la_CFLAGS =							\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"				\
	@GLIB_CFLAGS@							\
	@GTK_CFLAGS@							\
	@LIBXFCE4PANEL_CFLAGS@						\
	@LIBXFCE4UI_CFLAGS@						\
	@LIBXFCE4UTIL_CFLAGS@

libnetload_la_LDFLAGS =							\
	-avoid-version							\
	-module								\
	-no-undefined							\
	-export-symbols-regex '^xfce_panel_module_(preinit|init|construct)' \
	$(PLATFORM_LDFLAGS)

libnetload_la_LIBADD =							\
	@SOLLIBS@							\
	@GLIB_LIBS@							\
	@GTK_LIBS@							\
	@LIBXFCE4PANEL_LIBS@						\
	@LIBXFCE4UI_LIBS@						\
	@LIBXFCE4UTIL_LIBS@

check_PROGRAMS = commandline
commandline_SOURCES = commandline.c utils.c utils.h net.c net.h
commandline_CFLAGS =							\
	@GLIB_CFLAGS@							\
	@LIBXFCE4UTIL_CFLAGS@
commandline_LDADD =							\
	@GLIB_LIBS@							\
	@LIBXFCE4UTIL_LIBS@

# .desktop file
#
desktop_in_files = netload.desktop.in
	
desktopdir = $(datadir)/xfce4/panel/plugins
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)

EXTRA_DIST = 								\
	wormulon commandline.c						\
	meson.build							\
	$(desktop_in_files)

DISTCLEANFILES =							\
	$(desktop_DATA)
	
%.desktop: %.desktop.in Makefile
	$(AM_V_GEN)$(MSGFMT) --desktop --template $< -d $(top_srcdir)/po -o $@
