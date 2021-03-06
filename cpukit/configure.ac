## Process this file with autoconf to produce a configure script.
## 
## $Id$

AC_PREREQ([2.68])
AC_INIT([rtems-cpukit],[_RTEMS_VERSION],[http://www.rtems.org/bugzilla])
AC_CONFIG_SRCDIR([score])
RTEMS_TOP([..],[])

RTEMS_CANONICAL_TARGET_CPU

AM_INIT_AUTOMAKE([no-define nostdinc subdir-objects foreign 1.11.1])
AM_MAINTAINER_MODE

RTEMS_ENABLE_MULTILIB
RTEMS_ENABLE_MULTIPROCESSING
RTEMS_ENABLE_POSIX
RTEMS_ENABLE_ITRON
RTEMS_ENABLE_RTEMS_DEBUG
RTEMS_ENABLE_NETWORKING
RTEMS_ENABLE_HTTPD
RTEMS_ENABLE_SHTTPD
RTEMS_ENABLE_DRVMGR

RTEMS_ENV_RTEMSCPU
RTEMS_CHECK_RTEMS_DEBUG

# Is this a supported CPU?
AC_MSG_CHECKING([if cpu $RTEMS_CPU is supported])
if test -d "$srcdir/score/cpu/$RTEMS_CPU"; then
  AC_MSG_RESULT(yes)
else
  AC_MSG_ERROR(no)
fi

RTEMS_PROG_CC_FOR_TARGET
RTEMS_PROG_CCAS
RTEMS_CANONICALIZE_TOOLS
AM_PROG_CC_C_O
AC_PROG_RANLIB

RTEMS_CHECK_NEWLIB

# BSD-isms, used throughout the sources
# Not really used by this configure script
# FIXME: They should be eliminated if possible.
AC_CHECK_FUNCS([strsep strcasecmp snprintf])
AC_CHECK_FUNCS([strdup strndup strncasecmp])
AC_CHECK_FUNCS([bcopy bcmp])
AC_CHECK_FUNCS([isascii fileno])

# <FIXME>
#   Check for functions supplied by newlib >= 1.17.0
# Newlib's posix/ directory
AC_CHECK_FUNCS([sleep usleep \
  opendir closedir readdir readdir_r \
  rewinddir scandir seekdir telldir isatty])
AC_CHECK_FUNCS([creat])
AC_CHECK_FUNCS([__assert])
AC_CHECK_FUNCS([execl execlp execle execv execvp execve])
AC_CHECK_FUNCS([regcomp regexec regerror regfree])
# Mandated by POSIX but not present in newlib
AC_CHECK_DECLS([flockfile],[AC_CHECK_FUNCS([flockfile])],,[#include <stdio.h>])
AC_CHECK_DECLS([funlockfile],[AC_CHECK_FUNCS([funlockfile])],,[#include <stdio.h>])
AC_CHECK_DECLS([ftrylockfile],[AC_CHECK_FUNCS([ftrylockfile])],,[#include <stdio.h>])

# Newlib's unix/ directory
AC_CHECK_FUNCS([ttyname getcwd])
# </FIXME>

## Check if the installed toolchain has these headers
AC_CHECK_HEADERS([tar.h errno.h sched.h])

## error out if libc doesn't provide stdint.h
AS_IF([test x"${ac_cv_header_stdint_h}" != xyes],
[AC_MSG_ERROR([Required header stdint.h not found])])

## error out if libc doesn't provide inttypes.h
AS_IF([test x"${ac_cv_header_inttypes_h}" != xyes],
[AC_MSG_ERROR([Required header inttypes.h not found])])

## error out if libc doesn't provide errno.h
AS_IF([test x"${ac_cv_header_errno_h}" != xyes],
[AC_MSG_ERROR([Required header errno.h not found])])

## error out if libc doesn't provide sched.h
AS_IF([test x"${ac_cv_header_sched_h}" != xyes],
[AC_MSG_ERROR([Required header sched.h not found])])

## error out if libc doesn't provide tar.h
AS_IF([test x"${ac_cv_header_tar_h}" != xyes],
[AC_MSG_ERROR([Required header tar.h not found])])

AC_HEADER_STDBOOL
AS_IF([test x"${ac_cv_header_stdbool_h}" != xyes],
[AC_MSG_ERROR([No sufficient stdbool.h found])])

AC_CHECK_TYPES([ uint8_t,  int8_t])
AC_CHECK_TYPES([uint16_t, int16_t])
AC_CHECK_TYPES([uint32_t, int32_t])
AC_CHECK_TYPES([uint64_t, int64_t])
AC_CHECK_TYPES([uintmax_t, intmax_t])
AC_CHECK_TYPES([uintptr_t, intptr_t])

# Some toolchain sanity checks and diagnostics
RTEMS_CHECK_GCC_SANITY

# These are conditionally defined by the toolchain
# FIXME: we should either conditionally compile those parts in
# RTEMS depending on them, or abort - For now, simply check.
AC_CHECK_HEADER([pthread.h],[
  AC_CHECK_TYPES([pthread_rwlock_t])
  AC_CHECK_TYPES([pthread_barrier_t])
  AC_CHECK_TYPES([pthread_spinlock_t])
])

RTEMS_CHECK_MULTIPROCESSING
RTEMS_CHECK_POSIX_API
RTEMS_CHECK_ITRON_API
RTEMS_CHECK_NETWORKING

rtems_major=`echo _RTEMS_VERSION | sed "s/\..*//"`
rtems_minor=`echo _RTEMS_VERSION | sed "s/[[0-9]][[0-9]]*\.//;s/\..*//"`
rtems_revision=`echo _RTEMS_VERSION | sed "s/[[0-9]][[0-9]]*\.//;s/[[0-9]][[0-9]]*\.//;s/\..*//"`

_RTEMS_CPUOPT_INIT

RTEMS_CPUOPT([RTEMS_DEBUG],
  [test x"${enable_rtems_debug}" = x"yes"],
  [1],
  [if RTEMS_DEBUG is enabled])

RTEMS_CPUOPT([RTEMS_ITRON_API],
  [test x"$rtems_cv_HAS_ITRON_API" = x"yes"],
  [1],
  [if itron api is supported])

RTEMS_CPUOPT([RTEMS_MULTIPROCESSING],
  [test x"$enable_multiprocessing" = xyes],
  [1],
  [if multiprocessing is enabled])

RTEMS_CPUOPT([RTEMS_NEWLIB],
  [test x"$RTEMS_USE_NEWLIB" = xyes],
  [1],
  [if using newlib])

RTEMS_CPUOPT([RTEMS_POSIX_API],
  [test x"$rtems_cv_HAS_POSIX_API" = xyes],
  [1],
  [if posix api is supported])

RTEMS_CPUOPT([RTEMS_NETWORKING],
  [test x"$rtems_cv_HAS_NETWORKING" = xyes],
  [1],
  [if networking is enabled])

RTEMS_CPUOPT([RTEMS_DRVMGR_STARTUP],
  [test x"$enable_drvmgr" = xyes],
  [1],
  [if driver manager api is supported])

RTEMS_CPUOPT([RTEMS_VERSION],
  [true],
  ["]_RTEMS_VERSION["],
  [RTEMS version string])

RTEMS_CPUOPT([__RTEMS_USE_TICKS_FOR_STATISTICS__],
  [test x"${USE_TICKS_FOR_STATISTICS}" = x"1"],
  [1],
  [disable nanosecond granularity for statistics])

RTEMS_CPUOPT([__RTEMS_USE_TICKS_CPU_USAGE_STATISTICS__],
  [test x"${USE_TICKS_FOR_CPU_USAGE_STATISTICS}" = x"1"],
  [1],
  [disable nanosecond granularity for cpu usage statistics])

RTEMS_CPUOPT([__RTEMS_USE_TICKS_RATE_MONOTONIC_STATISTICS__],
  [test x"${USE_TICKS_FOR_RATE_MONOTONIC_STATISTICS}" = x"1"],
  [1],
  [disable nanosecond granularity for period statistics])

## This improves both the size and coverage analysis.
RTEMS_CPUOPT([__RTEMS_DO_NOT_INLINE_THREAD_ENABLE_DISPATCH__],
  [test x"${RTEMS_DO_NOT_INLINE_THREAD_ENABLE_DISPATCH}" = x"1"],
  [1],
  [disable inlining _Thread_Enable_dispatch])

## This improves both the size and coverage analysis.
RTEMS_CPUOPT([__RTEMS_DO_NOT_INLINE_CORE_MUTEX_SEIZE__],
  [test x"${RTEMS_DO_NOT_INLINE_CORE_MUTEX_SEIZE}" = x"1"],
  [1],
  [disable inlining _Thread_Enable_dispatch])

## This improves both the size and coverage analysis.
RTEMS_CPUOPT([__RTEMS_DO_NOT_UNROLL_THREADQ_ENQUEUE_PRIORITY__],
  [test x"${RTEMS_DO_NOT_UNROLL_THREADQ_ENQUEUE_PRIORITY}" = x"1"],
  [1],
  [disable inlining _Thread_queue_Enqueue_priority])

## This gives the same behavior as 4.8 and older
RTEMS_CPUOPT([__RTEMS_STRICT_ORDER_MUTEX__],
  [test x"${ENABLE_STRICT_ORDER_MUTEX}" = x"1"],
  [1],
  [disable strict order mutex])

## Deactivate ada bindings
RTEMS_CPUOPT([__RTEMS_ADA__],
  [test x"${enable_ada}" = x"yes"],
  [1],
  [Define to 1 if ada/gnat bindings are built-in])

RTEMS_CPUOPT([__RTEMS_MAJOR__],
  [true],
  [$rtems_major],
  [major version portion of an RTEMS release])

RTEMS_CPUOPT([__RTEMS_MINOR__],
  [true],
  [$rtems_minor],
  [minor version portion of an RTEMS release])

RTEMS_CPUOPT([__RTEMS_REVISION__],
  [true],
  [$rtems_revision],
  [revision version portion of an RTEMS release])

_RTEMS_CPUOPT_FINI

AC_ENABLE_MULTILIB([Makefile],[..])

# libmisc/shell/* wants to assign file descriptors to stdio file descriptors.
AC_MSG_CHECKING([for assignable stdio])
AC_COMPILE_IFELSE(
  [AC_LANG_PROGRAM(
    [#include <stdio.h>], 
    [stdin = fopen("/tmp", "r")])],
  [HAVE_ASSIGNABLE_STDIO=yes],
  [HAVE_ASSIGNABLE_STDIO=no])
AC_MSG_RESULT([$HAVE_ASSIGNABLE_STDIO])

# libmisc/serdbg exploits weak symbols
RTEMS_CHECK_GCC_WEAK

AC_CHECK_HEADERS([sys/cdefs.h],[NEED_SYS_CDEFS_H=no],[NEED_SYS_CDEFS_H=yes])
AC_CHECK_HEADERS([sys/queue.h],[NEED_SYS_QUEUE_H=no],[NEED_SYS_QUEUE_H=yes])

# FIXME: These checks are only in here to provide
# configuration-time diagnostics and are not really used.
AC_CHECK_DECLS([_POSIX_LOGIN_NAME_MAX],,,[#include <limits.h>])
AC_CHECK_DECLS([CHAR_BIT],,,[#include <limits.h>])

# FIXME: We should get rid of this.
# So far, only used in libfs/src/nfsclient/src/dirutils.c
AC_CHECK_SIZEOF([mode_t])
AC_CHECK_SIZEOF([off_t])

# FIXME: Mandatory in SUSv4, optional in SUSv3. 
#   Not implemented in GCC/newlib, so far.
AC_CHECK_DECLS([WORD_BIT],,,[#include <limits.h>])
AC_CHECK_DECLS([LONG_BIT],,,[#include <limits.h>])

## BSD-ism, excluded from POSIX, but available on most platforms
AC_CHECK_DECLS([sbrk],,,[#include <unistd.h>])

## Check if libc provides BSD's strlcpy/strlcat
AC_CHECK_FUNCS(strlcpy strlcat)


# ... far too many conditionals ...
AM_CONDITIONAL(LIBRPC,[test x"$rtems_cv_HAS_NETWORKING" = x"yes"])
AM_CONDITIONAL(NEWLIB,test x"$RTEMS_USE_NEWLIB" = x"yes")

AM_CONDITIONAL(HAS_MP,test x"$enable_multiprocessing" = x"yes" )

AM_CONDITIONAL(HAS_PTHREADS,test x"$rtems_cv_HAS_POSIX_API" = x"yes")
AM_CONDITIONAL(LIBITRON,test x"$rtems_cv_HAS_ITRON_API" = x"yes")
AM_CONDITIONAL(LIBNETWORKING,test x"$rtems_cv_HAS_NETWORKING" = x"yes")
AM_CONDITIONAL([LIBHTTPD],[test x"$rtems_cv_HAS_NETWORKING" = xyes \
&& test x"$enable_httpd" = xyes])
AM_CONDITIONAL([LIBSHTTPD],[test x"$rtems_cv_HAS_NETWORKING" = xyes \
&& test x"$enable_shttpd" = xyes])

AM_CONDITIONAL([NEED_SYS_CDEFS_H],[test x"$NEED_SYS_CDEFS_H" = x"yes"])
AM_CONDITIONAL([NEED_SYS_QUEUE_H],[test x"$NEED_SYS_QUEUE_H" = x"yes"])

AM_CONDITIONAL([LIBSHELL],[test x"$HAVE_ASSIGNABLE_STDIO" = x"yes"])
AM_CONDITIONAL([LIBSERDBG],[test x"$rtems_cv_cc_attribute_weak" = x"yes"])
AM_CONDITIONAL([LIBGNAT],[test x"$rtems_cv_HAS_POSIX_API" = x"yes" \
&& test x"$enable_ada" = x"yes"])

AM_CONDITIONAL([LIBDOSFS],[dnl
test x"$ac_cv_type_uint8_t" = xyes \
&& test x"$ac_cv_type_uint16_t" = xyes])

AC_CONFIG_HEADER(config.h)

## These are needed by the NFS Client
AC_CHECK_PROG(RPCGEN,rpcgen,rpcgen)
AM_CONDITIONAL([RPCTOOLS],[test "$RPCGEN" = rpcgen \
&& test -n "$AWK" \
&& test "$enable_rpcgen" = yes])

RTEMS_AMPOLISH3

# Explicitly list all Makefiles here
AC_CONFIG_FILES([
Doxyfile
Makefile
rtems/Makefile
sapi/Makefile
score/Makefile
score/cpu/Makefile
score/cpu/arm/Makefile
score/cpu/bfin/Makefile
score/cpu/avr/Makefile
score/cpu/h8300/Makefile
score/cpu/i386/Makefile
score/cpu/lm32/Makefile
score/cpu/m68k/Makefile
score/cpu/m32c/Makefile
score/cpu/m32r/Makefile
score/cpu/mips/Makefile
score/cpu/powerpc/Makefile
score/cpu/sh/Makefile
score/cpu/sparc/Makefile
score/cpu/no_cpu/Makefile
posix/Makefile
itron/Makefile
libblock/Makefile
libdrvmgr/Makefile
libfs/Makefile
libfs/src/nfsclient/Makefile
libgnat/Makefile
libcsupport/Makefile
libnetworking/Makefile
libpci/Makefile
librpc/Makefile
libmisc/Makefile
libi2c/Makefile
libmd/Makefile
zlib/Makefile
httpd/Makefile
ftpd/Makefile
telnetd/Makefile
pppd/Makefile
shttpd/Makefile
mghttpd/Makefile
wrapup/Makefile])

AC_CONFIG_COMMANDS([preinstall-stamp],
[test -z "$with_multisubdir" && ${MAKE} preinstall-stamp],
[MAKE=${MAKE}
with_multisubdir="$with_multisubdir"])

AC_OUTPUT
