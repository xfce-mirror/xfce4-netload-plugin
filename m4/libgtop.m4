
AC_DEFUN([INIT_LIBGTOP],
[

MINIMUM_LIBGTOP_VERSION=$1
AC_MSG_CHECKING([for LibGTop >= $MINIMUM_LIBGTOP_VERSION])

PC_FILE="libgtop-2.0"


# Make sure the version is sufficient

if $PKG_CONFIG --atleast-version=$MINIMUM_LIBGTOP_VERSION $PC_FILE ; then
  GTOP_VERSION=`$PKG_CONFIG $PC_FILE --modversion`
  AC_MSG_RESULT([yes ($GTOP_VERSION)])
else
  AC_MSG_RESULT([no])
  echo "$as_me: ERROR: LibGTop >= $MINIMUM_LIBGTOP_VERSION not found" >&2;
  echo "$as_me: ERROR: Cannot run without LibGTop."
  exit 1;
fi

#OK, we've got what we need, so get the necessary flags to compile
LIBGTOP_INCS=`$PKG_CONFIG $PC_FILE --cflags`
LIBGTOP_LIBS=`$PKG_CONFIG $PC_FILE --libs`

# And make sure they get plugged into the Makefile
AC_SUBST(LIBGTOP_INCS)
AC_SUBST(LIBGTOP_LIBS)

])
