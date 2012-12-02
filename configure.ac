AC_PREREQ([2.0])
AC_INIT(webrtc2sip, 0.1, diopmamadou(at)doubango(DOT)org)
AM_INIT_AUTOMAKE
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_SRCDIR([mp_mediaproxy.cc])

dnl find suitable C and C++ compilers
AC_PROG_CC
AC_PROG_CXX

AC_PROG_INSTALL

AC_PROG_SED

AC_PROG_MAKE_SET

# CFLAGS
CFLAGS="${CFLAGS=}"

#echo Setting up build environment for ${target_cpu}${target_os}

### ENABLE-DEBUG
AC_MSG_CHECKING(whether to enable debugging)
debug_default="yes"
AC_ARG_ENABLE(debug, 
[  --enable-debug[=no/yes] turn on/off debugging 
                       [[default=$debug_default]]],
[ if test "x$enableval" = "xyes" ; then
	CFLAGS="$CFLAGS -O0 -g3 -DDEBUG"
	AC_MSG_RESULT(yes)
  else
	CFLAGS="$CFLAGS -O3 -NDEBUG"
	AC_MSG_RESULT(no)
  fi
],
[])

### DEBUG-LEVEL
AC_ARG_WITH(debug-level, 
[  --with-debug-level[=level] Set debug-level to level [[default=info]]
                       info: informational
                       warn: warnings
                       error [default]: errors
                       fatal: fatal],
[ if test "x$withval" = "xinfo" ; then
	AC_DEFINE(DEBUG_LEVEL, DEBUG_LEVEL_INFO, [info debug])
	AC_MSG_RESULT(info)
  elif test "x$withval" = "xwarn" ; then
	AC_DEFINE(DEBUG_LEVEL, DEBUG_LEVEL_INFO, [warn debug])
	AC_MSG_RESULT(warn)
  elif test "x$withval" = "xerror" ; then
	AC_DEFINE(DEBUG_LEVEL, DEBUG_LEVEL_ERROR, [error debug])
	AC_MSG_RESULT(error)
  elif test "x$withval" = "xfatal" ; then
	AC_DEFINE(DEBUG_LEVEL, DEBUG_LEVEL_FATAL, [fatal debug])
	AC_MSG_RESULT(fatal)
  fi
])

AH_TEMPLATE([HAVE_DNS_H], [Define if <dns.h> header exist])
AC_CHECK_HEADER([dns.h], AC_DEFINE(HAVE_DNS_H, 1), AC_DEFINE(HAVE_DNS_H, 0), [])

AH_TEMPLATE([TNET_HAVE_SS_LEN], [Define if sockaddr_storage.ss_len exists])
AC_CHECK_MEMBER([struct sockaddr_storage.ss_len], AC_DEFINE(TNET_HAVE_SS_LEN, 1), AC_DEFINE(TNET_HAVE_SS_LEN,0), [#include <sys/socket.h>])

AH_TEMPLATE([TNET_HAVE_SA_LEN], [Define if sockaddr.sa_len exists])
AC_CHECK_MEMBER([struct sockaddr.sa_len], AC_DEFINE(TNET_HAVE_SA_LEN, 1), AC_DEFINE(TNET_HAVE_SA_LEN, 0), [#include <netinet/in.h>])

# Configure path to doubango includes and libs
AC_MSG_CHECKING([for doubango])
AC_ARG_WITH([doubango], [AC_HELP_STRING([--with-doubango=PATH], [path to installed doubango [default=/usr]])], [
  DOUBANGO_INCLUDE="${withval}/include"
  DOUBANGO_LIB="${withval}/lib"
  AC_MSG_RESULT("${withval}")
], [
  DOUBANGO_INCLUDE="/usr/include/doubango"
  DOUBANGO_LIB="/usr/lib"
  AC_MSG_RESULT(/usr)
])
AC_SUBST(DOUBANGO_INCLUDE)
AC_SUBST(DOUBANGO_LIB)

AC_MSG_CHECKING([for libxml2])
AC_ARG_WITH([libxml2], [AC_HELP_STRING([--with-libxml2=PATH], [path to installed libxml2 [default=/usr]])], [
  LIBXML2_INCLUDE="${withval}/include/libxml2"
  LIBXML2_LIB="${withval}/lib"
  AC_MSG_RESULT("${withval}")
], [
  LIBXML2_INCLUDE="/usr/include/libxml2"
  LIBXML2_LIB="/usr/lib"
  AC_MSG_RESULT(/usr)
])
AC_SUBST(LIBXML2_INCLUDE)
AC_SUBST(LIBXML2_LIB)

AC_MSG_CHECKING([for libpthread])
AC_ARG_WITH([libpthread], [AC_HELP_STRING([--with-libpthread=PATH], [path to installed libpthread [default=/usr]])], [
  LIBPTHREAD_INCLUDE="${withval}/include"
  LIBPTHREAD_LIB="${withval}/lib"
  AC_MSG_RESULT("${withval}")
], [
  LIBPTHREAD_INCLUDE="/usr/include"
  LIBPTHREAD_LIB="/usr/lib"
  AC_MSG_RESULT(/usr)
])
AC_SUBST(LIBPTHREAD_INCLUDE)
AC_SUBST(LIBPTHREAD_LIB)

AC_OUTPUT(
Makefile
)