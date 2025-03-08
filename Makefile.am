ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}

@SET_MAKE@

SUBDIRS = panel-plugin po icons

distclean-local:
	rm -rf *.cache *~

.PHONY: ChangeLog

ChangeLog: Makefile
	(GIT_DIR=$(top_srcdir)/.git git log > .changelog.tmp \
	&& mv .changelog.tmp ChangeLog; rm -f .changelog.tmp) \
	|| (touch ChangeLog; echo 'Git directory not found: installing possibly empty changelog.' >&2)

dist-hook: ChangeLog

EXTRA_DIST = 								\
	README								\
	meson.build							\
	meson_options.txt						\
	po/meson.build							\
	xfce-revision.h.in						\
	$(NULL)
