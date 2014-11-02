plugindir = $(libdir)/xfce4/panel-plugins
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
	global.h							\
	slurm.h

libnetload_la_CFLAGS =							\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"				\
	@LIBXFCE4PANEL_CFLAGS@						\
	@LIBXFCE4UI_CFLAGS@

libnetload_la_LDFLAGS =							\
	-avoid-version							\
	-module								\
	-no-undefined							\
	-export-symbols-regex '^xfce_panel_module_(preinit|init|construct)' \
	$(PLATFORM_LDFLAGS)

libnetload_la_LIBADD =							\
	@SOLLIBS@							\
	@LIBXFCE4PANEL_LIBS@						\
	@LIBXFCE4UI_LIBS@

# .desktop file
#
# Some automake trickery here. Because we cannot use $(libexecdir) in the
# automake stage, we'll use sed to get the full path into the .desktop file.
# We also need to let intltool merge the translated fields, so we add an
# additional level of indirection: a <name>.desktop.in.in file.
# 
desktop_in_in_files = netload.desktop.in.in
desktop_in_files = $(desktop_in_in_files:.desktop.in.in=.desktop.in)
	
desktopdir = $(datadir)/xfce4/panel-plugins
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST = 								\
	wormulon commandline.c						\
	$(desktop_in_in_files)

DISTCLEANFILES =							\
	$(desktop_DATA) $(desktop_in_files)
	
# get full path into .desktop file
%.desktop.in: %.desktop.in.in
	sed -e "s^@PLUGIN_PATH@^$(libexecdir)/xfce4/panel-plugins^" \
		$< > $@

