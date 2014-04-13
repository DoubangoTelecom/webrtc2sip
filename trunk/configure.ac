#
# Copyright (C) 2012-2013 Mamadou DIOP
# Copyright (C) 2012-2013 Doubango Telecom <http://www.doubango.org>
# License: GPLv3 or commercial
# This file is part of webrtc2sip project <https://code.google.com/p/webrtc2sip/>
#

AC_PREREQ([2.0])
AC_INIT(webrtc2sip, 2.0.0, doubango[at]googlegroups(dot)com)
AM_INIT_AUTOMAKE
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_SRCDIR([mp_mediaproxy.cc])

dnl find suitable C and C++ compilers
AC_PROG_CC
AC_PROG_CXX
AM_PROG_CC_C_O

AC_PROG_INSTALL

AC_PROG_SED

AC_PROG_MAKE_SET

PKG_PROG_PKG_CONFIG

# DOUBANGO MINI REQUIRED VERSION
AC_SUBST(DOUBANGO_MINVER, 2.0.1050)

#echo Setting up build environment for ${target_cpu}${target_os}

### ENABLE-DEBUG
AC_MSG_CHECKING(whether to enable debugging)
debug_default="no"
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

### DEBUG-LEVEL: could be updated using config.xml
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

AC_CHECK_LIB(c, dlopen, AC_SUBST(LIBDL,""), AC_CHECK_LIB(dl, dlopen, AC_SUBST(LIBDL,-ldl)))

AH_TEMPLATE([HAVE_DNS_H], [Define if <dns.h> header exist])
AC_CHECK_HEADER([dns.h], AC_DEFINE(HAVE_DNS_H, 1), AC_DEFINE(HAVE_DNS_H, 0), [])

AH_TEMPLATE([TNET_HAVE_SS_LEN], [Define if sockaddr_storage.ss_len exists])
AC_CHECK_MEMBER([struct sockaddr_storage.ss_len], AC_DEFINE(TNET_HAVE_SS_LEN, 1), AC_DEFINE(TNET_HAVE_SS_LEN,0), [#include <sys/socket.h>])

AH_TEMPLATE([TNET_HAVE_SA_LEN], [Define if sockaddr.sa_len exists])
AC_CHECK_MEMBER([struct sockaddr.sa_len], AC_DEFINE(TNET_HAVE_SA_LEN, 1), AC_DEFINE(TNET_HAVE_SA_LEN, 0), [#include <netinet/in.h>])

###########
# DOUBANGO
###########
AC_SUBST(have_tinysak, no)
AC_SUBST(have_tinynet, no)
AC_SUBST(have_tinyhttp, no)
AC_SUBST(have_tinysip, no)
AC_SUBST(have_tinydav, no)
AC_SUBST(have_tinysdp, no)
AC_SUBST(have_tinysigcomp, no)
AC_SUBST(have_tinymedia, no)
AC_SUBST(have_tinyxcap, no)
AC_SUBST(have_tinysms, no)
AC_SUBST(have_tinymsrp, no)
AC_SUBST(have_tinyrtp, no)
AC_SUBST(have_tinyipsec, no)
PKG_CHECK_MODULES(TINYSAK, tinySAK >= ${DOUBANGO_MINVER}, AC_SUBST(have_tinysak, yes), AC_MSG_WARN(Cannot find tinySAK >= ${DOUBANGO_MINVER} using pkg-config))
PKG_CHECK_MODULES(TINYNET, tinyNET >= ${DOUBANGO_MINVER}, AC_SUBST(have_tinynet, yes), AC_MSG_WARN(Cannot find tinyNET >= ${DOUBANGO_MINVER} using pkg-config))
PKG_CHECK_MODULES(TINYHTTP, tinyHTTP >= ${DOUBANGO_MINVER}, AC_SUBST(have_tinyhttp, yes), AC_MSG_WARN(Cannot find tinyHTTP >= ${DOUBANGO_MINVER} using pkg-config))
PKG_CHECK_MODULES(TINYSIP, tinySIP >= ${DOUBANGO_MINVER}, AC_SUBST(have_tinysip, yes), AC_MSG_WARN(Cannot find tinySIP >= ${DOUBANGO_MINVER} using pkg-config))
PKG_CHECK_MODULES(TINYDAV, tinyDAV >= ${DOUBANGO_MINVER}, AC_SUBST(have_tinydav, yes), AC_MSG_WARN(Cannot find tinyDAV >= ${DOUBANGO_MINVER} using pkg-config))
PKG_CHECK_MODULES(TINYSDP, tinySDP >= ${DOUBANGO_MINVER}, AC_SUBST(have_tinysdp, yes), AC_MSG_WARN(Cannot find tinySDP >= ${DOUBANGO_MINVER} using pkg-config))
PKG_CHECK_MODULES(TINYSIGCOMP, tinySIGCOMP >= ${DOUBANGO_MINVER}, AC_SUBST(have_tinysigcomp, yes), AC_MSG_WARN(Cannot find tinySIGCOMP >= ${DOUBANGO_MINVER} using pkg-config))
PKG_CHECK_MODULES(TINYMEDIA, tinyMEDIA >= ${DOUBANGO_MINVER}, AC_SUBST(have_tinymedia, yes), AC_MSG_WARN(Cannot find tinyMEDIA >= ${DOUBANGO_MINVER} using pkg-config))
PKG_CHECK_MODULES(TINYXCAP, tinyXCAP >= ${DOUBANGO_MINVER}, AC_SUBST(have_tinyxcap, yes), AC_MSG_WARN(Cannot find tinyxcap >= ${DOUBANGO_MINVER} using pkg-config))
PKG_CHECK_MODULES(TINYSMS, tinySMS >= ${DOUBANGO_MINVER}, AC_SUBST(have_tinysms, yes), AC_MSG_WARN(Cannot find tinySMS >= ${DOUBANGO_MINVER} using pkg-config))
PKG_CHECK_MODULES(TINYMSRP, tinyMSRP >= ${DOUBANGO_MINVER}, AC_SUBST(have_tinymsrp, yes), AC_MSG_WARN(Cannot find tinyMSRP >= ${DOUBANGO_MINVER} using pkg-config))
PKG_CHECK_MODULES(TINYRTP, tinyRTP >= ${DOUBANGO_MINVER}, AC_SUBST(have_tinyrtp, yes), AC_MSG_WARN(Cannot find tinyRTP >= ${DOUBANGO_MINVER} using pkg-config))
PKG_CHECK_MODULES(TINYIPSEC, tinyIPSec >= ${DOUBANGO_MINVER}, AC_SUBST(have_tinyipsec, yes), AC_MSG_WARN(Cannot find tinyIPSec >= ${DOUBANGO_MINVER} using pkg-config))

if test $have_tinysak:$have_tinynet:$have_tinyhttp:$have_tinysip:$have_tinydav:$have_tinysdp:$have_tinysigcomp:$have_tinymedia:$have_tinyxcap:$have_tinysms:$have_tinymsrp:$have_tinyrtp:$have_tinyipsec != yes:yes:yes:yes:yes:yes:yes:yes:yes:yes:yes:yes:yes ; then
	AC_MSG_WARN([
***
*** Failed to find all DOUBANGO libraries using pkg-config.
*** This most likely means the DOUBANGO pkg files (*.pc) are not 
*** in the default path where pkg-config expect them.
***
*** Please set your PKG_CONFIG_PATH environment variable to include the
*** directory in which the *.pc files are installed. Most likely
*** these files are installed in /usr/local/lib/pkgconfig; if that is the
*** case, try:
***
***     export PKG_CONFIG_PATH="\$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig"
***
*** then run this ./configure <your options here> again.
***
*** or, try:
***
***   PKG_CONFIG_PATH=/usr/local/lib/pkgconfig ./configure <your options here>
***
*** Anyways, we're trying another way to find DOUBANGO.
***
]);

AC_SUBST(DOUBANGO_INCLUDES_PATH,/usr/local/include)
AC_SUBST(DOUBANGO_LIBS_FALLBACK,-L/usr/local/lib)
AC_SUBST(TINYSAK_LIBS,-ltinySAK) AC_SUBST(TINYSAK_CFLAGS,-I${DOUBANGO_INCLUDES_PATH}/tinysak)
AC_SUBST(TINYNET_LIBS,-ltinyNET) AC_SUBST(TINYNET_CFLAGS,-I${DOUBANGO_INCLUDES_PATH}/tinynet)
AC_SUBST(TINYHTTP_LIBS,-ltinyHTTP) AC_SUBST(TINYHTTP_CFLAGS,-I${DOUBANGO_INCLUDES_PATH}/tinyhttp)
AC_SUBST(TINYSIP_LIBS,-ltinySIP) AC_SUBST(TINYSIP_CFLAGS,-I${DOUBANGO_INCLUDES_PATH}/tinysip)
AC_SUBST(TINYDAV_LIBS,-ltinyDAV) AC_SUBST(TINYDAV_CFLAGS,-I${DOUBANGO_INCLUDES_PATH}/tinydav)
AC_SUBST(TINYSDP_LIBS,-ltinySDP) AC_SUBST(TINYSDP_CFLAGS,-I${DOUBANGO_INCLUDES_PATH}/tinysdp)
AC_SUBST(TINYSIGCOMP_LIBS,-ltinySIGCOMP) AC_SUBST(TINYSIGCOMP_CFLAGS,-I${DOUBANGO_INCLUDES_PATH}/tinysigcomp)
AC_SUBST(TINYMEDIA_LIBS,-ltinyMEDIA) AC_SUBST(TINYMEDIA_CFLAGS,-I${DOUBANGO_INCLUDES_PATH}/tinymedia)
AC_SUBST(TINYXCAP_LIBS,-ltinyXCAP) AC_SUBST(TINYXCAP_CFLAGS,-I${DOUBANGO_INCLUDES_PATH}/tinyxcap)
AC_SUBST(TINYSMS_LIBS,-ltinySMS) AC_SUBST(TINYSMS_CFLAGS,-I${DOUBANGO_INCLUDES_PATH}/tinysms)
AC_SUBST(TINYMSRP_LIBS,-ltinyMSRP) AC_SUBST(TINYMSRP_CFLAGS,-I${DOUBANGO_INCLUDES_PATH}/tinymsrp)
AC_SUBST(TINYRTP_LIBS,-ltinyRTP) AC_SUBST(TINYRTP_CFLAGS,-I${DOUBANGO_INCLUDES_PATH}/tinyrtp)
AC_SUBST(TINYIPSEC_LIBS,-ltinyIPSec) AC_SUBST(TINYIPSEC_CFLAGS,-I${DOUBANGO_INCLUDES_PATH}/tinyipsec)
fi

if test $have_tinysak != yes; then
	AC_CHECK_HEADERS(tinysak/tinysak_config.h, 
		AC_CHECK_LIB(tinySAK, tsk_object_new, AC_SUBST(have_tinysak, yes), AC_MSG_ERROR(Failed to find libtinySAK), [${LIBDL}])
	)
fi
if test $have_tinynet != yes; then
	AC_CHECK_HEADERS(tinynet/tinynet_config.h, 
		AC_CHECK_LIB(tinyNET, tnet_startup, AC_SUBST(have_tinynet, yes), AC_MSG_ERROR(Failed to find libtinyNET), [-ltinySAK ${LIBDL}])
	)
fi
if test $have_tinyhttp != yes; then
	AC_CHECK_HEADERS(tinyhttp/tinyhttp_config.h, 
		AC_CHECK_LIB(tinyHTTP, thttp_stack_create, AC_SUBST(have_tinyhttp, yes), AC_MSG_ERROR(Failed to find libtinyHTTP), [-ltinySAK -ltinyNET ${LIBDL}])
	)
fi
if test $have_tinysip != yes; then
	AC_CHECK_HEADERS(tinysip/tinysip_config.h, 
		AC_CHECK_LIB(tinySIP, tsip_stack_create, AC_SUBST(have_tinysip, yes), AC_MSG_ERROR(Failed to find libtinySIP), [-ltinySAK -ltinyNET -ltinyHTTP -ltinyIPSec -ltinySDP -ltinyMEDIA -ltinySIGCOMP ${LIBDL} -lm])
	)
fi
if test $have_tinydav != yes; then
	AC_CHECK_HEADERS(tinydav/tinydav_config.h, 
		AC_CHECK_LIB(tinyDAV, tdav_init, AC_SUBST(have_tinydav, yes), AC_MSG_ERROR(Failed to find libtinyDAV), [-ltinySAK -ltinyNET -ltinySDP -ltinyRTP -ltinyMEDIA -ltinyMSRP ${LIBDL}])
	)
fi
if test $have_tinysdp != yes; then
	AC_CHECK_HEADERS(tinysdp/tinysdp_config.h, 
		AC_CHECK_LIB(tinySDP, tsdp_message_create, AC_SUBST(have_tinysdp, yes), AC_MSG_ERROR(Failed to find libtinySDP), [-ltinySAK ${LIBDL}])
	)
fi
if test $have_tinysigcomp != yes; then
	AC_CHECK_HEADERS(tinysigcomp/tinysigcomp_config.h, 
		AC_CHECK_LIB(tinySIGCOMP, tcomp_manager_create, AC_SUBST(have_tinysigcomp, yes), AC_MSG_ERROR(Failed to find libtinySIGCOMP), [-ltinySAK ${LIBDL} -lm])
	)
fi
if test $have_tinymedia != yes; then
	AC_CHECK_HEADERS(tinymedia/tinymedia_config.h, 
		AC_CHECK_LIB(tinyMEDIA, tmedia_session_mgr_create, AC_SUBST(have_tinymedia, yes), AC_MSG_ERROR(Failed to find libtinyMEDIA), [-ltinySAK -ltinyNET -ltinySDP ${LIBDL}])
	)
fi
if test $have_tinyxcap != yes; then
	AC_CHECK_HEADERS(tinyxcap/tinyxcap_config.h, 
		AC_CHECK_LIB(tinyXCAP, txcap_stack_create, AC_SUBST(have_tinyxcap, yes), AC_MSG_ERROR(Failed to find libtinyXCAP), [-ltinySAK -ltinyHTTP -ltinyNET ${LIBDL}])
	)
fi
if test $have_tinysms != yes; then
	AC_CHECK_HEADERS(tinysms/tinysms_config.h, 
		AC_CHECK_LIB(tinySMS, tsms_tpdu_message_init, AC_SUBST(have_tinysms, yes), AC_MSG_ERROR(Failed to find libtinySMS), [-ltinySAK ${LIBDL}])
	)
fi
if test $have_tinymsrp != yes; then
	AC_CHECK_HEADERS(tinymsrp/tinymsrp_config.h, 
		AC_CHECK_LIB(tinyMSRP, tmsrp_create_report, AC_SUBST(have_tinymsrp, yes), AC_MSG_ERROR(Failed to find libtinyMSRP), [-ltinySAK -ltinyNET -ltinySDP -ltinyHTTP ${LIBDL}])
	)
fi
if test $have_tinyrtp != yes; then
	AC_CHECK_HEADERS(tinyrtp/tinyrtp_config.h, 
		AC_CHECK_LIB(tinyRTP, trtp_manager_create, AC_SUBST(have_tinyrtp, yes), AC_MSG_ERROR(Failed to find libtinyRTP), [-ltinySAK -ltinyNET -ltinyMEDIA ${LIBDL}])
	)
fi
if test $have_tinyipsec != yes; then
	AC_CHECK_HEADERS(tinyipsec/tinyipsec_config.h, 
		AC_CHECK_LIB(tinyIPSec, tipsec_ctx_create, AC_SUBST(have_tinyipsec, yes) , AC_MSG_ERROR(Failed to find libtinyIPSec), [-ltinySAK ${LIBDL}])
	)
fi

#############
# LIBXML2
#############
AC_SUBST(LIBXML2_INCLUDE, "/usr/include/libxml2")
AC_SUBST(LIBXML2_LIB, "/usr/lib")
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

##########
# PTHREAD
##########
AC_SUBST(LIBPTHREAD_INCLUDE, "/usr/include")
AC_SUBST(LIBPTHREAD_LIB, "/usr/lib")
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


AC_OUTPUT(
Makefile
)




AC_MSG_NOTICE([
 
**************************************************************************
 				CONGRATULATIONS
**************************************************************************
Host setup:                  ${host}
Install prefix:              ${prefix}
Compiler:                    ${CC}

DOUBANGO
     MIN_VER:                ${DOUBANGO_MINVER}
     TINYSAK:                ${have_tinysak}
     TINYNET:                ${have_tinynet}
     TINYHTTP:               ${have_tinyhttp}
     TINYSIP:                ${have_tinysip}
     TINYDAV:                ${have_tinydav}
     TINYSDP:                ${have_tinysdp}
     TINYSIGCOMP:            ${have_tinysigcomp}
     TINYMEDIA:              ${have_tinymedia}
     TINYMEDIA:              ${have_tinyxcap}
     TINYSMS:                ${have_tinysms}
     TINYMSRP:               ${have_tinymsrp}
     TINYRTP:                ${have_tinyrtp}
     TINYIPSEC:              ${have_tinyipsec}

LIBXML2_INCLUDE:             ${LIBXML2_INCLUDE}
LIBXML2_LIB:                 ${LIBXML2_LIB}

LIBPTHREAD_INCLUDE:          ${LIBPTHREAD_INCLUDE}
LIBPTHREAD_LIB:              ${LIBPTHREAD_LIB}

Report issues at https://groups.google.com/group/doubango

-------------------------------------------------------------------------
Next steps
  1) run 'make' to build the source
  2) run 'make install' to install
-------------------------------------------------------------------------

])





