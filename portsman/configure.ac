dnl Process this file with autoconf to produce a configure script.
AC_INIT(src/main.c)
AM_INIT_AUTOMAKE(portsman, 0.2-rc2, anselmg@t-online,de)
dnl Prevents automake to use depcomp which breaks currently
dnl the make process in make dist tarball
AC_PROG_CC
AC_CHECK_HEADERS([curses.h dirent.h libintl.h locale.h signal.h stdlib.h string.h stdio.h stdlib.h sys/ioctl.h sys/stat.h sys/types.h time.h unistd.h])
AC_OUTPUT(Makefile)
AC_OUTPUT(src/Makefile)
AC_OUTPUT(doc/Makefile)
AC_OUTPUT(config/Makefile)
