#
# SYNOPSIS
#
#   TODO: AX_PD_EXTERNAL()
#
# DESCRIPTION
#
#   This macro provides tests, options, & flags for building a Pd
#   external.
#
# User flags (should already be set at call time)
#   UCFLAGS, UCPPFLAGS, ULDFLAGS
#
# Pd directories
#   AC_ARG_WITH(pd-dir)     : default: "\${prefix}/pd"
#   AC_ARG_WITH(pd-include) : sets IFLAGS, AM_IFLAGS
#   AC_ARG_WITH(pd-extdir)  : default $pddir/externs
#   AC_SUBST(pddir)
#   AC_SUBST(pddocdir)
#   AC_SUBST(pdextdir)
#    + aliases: pdexternsdir, pdexecdir
#
# Header tests:
#   AC_CHECK_HEADER(m_pd.h)
#
# Object Externals vs. multi-object libraries:
#   AC_ARG_ENABLE(object-externals, ...) ##-- set shell var want_object_externals
#   AC_DEFINE(WANT_OBJECT_EXTERNALS,...)
#   AM_CONDITIONAL(WANT_OBJECT_EXTERNALS)
#
# Debugging
#   AC_ARG_ENABLE(debug)
#   AC_DEFINE(ENABLE_DEBUG)
#   AC_SUBST(ENABLE_DEBUG)
#
# Build time:
#   AC_DEFINE_UNQUOTED(PACKAGE_BUILD_DATE)
#   AC_DEFINE_UNQUOTED(PACKAGE_BUILD_USER)
#
# Versioning
#   AC_SUBST(PACKAGE_VERSION,PACKAGE_NAME,BUGREPORT)
#
# Platform-dependent flags (shell vars and AC_SUBST)
#   LD            : linker
#   PDEXT_LFLAGS  : linker flags, appended to LDFLAGS
#   PDEXT_AFLAGS  : compiler flags for alpha, appended to CFLAGS
#   PDEXT_OFLAGS  : compiler optimization flags, appended to CFLAGS
#   PDEXT_DFLAGS  : preprocessor flags, appended to CPPFLAGS
#   PDEXT_WFLAGS  : compiler warning flags, appended to CFLAGS (gcc only)
#   PDEXT         : platform-specific external extension (without leading ".")
#
# EXEEXT hacks:
#  AC_SUBST(pd_buildext)
#   
#
#
# LAST MODIFICATION
#
#   Sat, 14 Feb 2009 10:06:12 +0100
#
# COPYLEFT
#
#   Copyright (c) 2009 Bryan Jurish <moocow@ling.uni-potsdam.de>
#
#   Copying and distribution of this file, with or without
#   modification, are permitted in any medium without royalty provided
#   the copyright notice and this notice are preserved.

AC_DEFUN([AX_PD_EXTERNAL],
[
 ##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
 ## prerequisites
 AC_PREREQ(2.5)

 ##-- save user's CFLAGS,CPPFLAGS (do this before calling AX_PD_EXTERNAL!)
 #test -z "$UCPPFLAGS" && UCPPFLAGS="$CPPFLAGS"
 #test -z "$UCFLAGS"   &&   UCFLAGS="$CFLAGS"
 #test -z "$ULDFLAGS"   && ULDFLAGS="$LDFLAGS"

 ##-- Programs, prefix
 AC_PROG_CC
 AC_PROG_INSTALL
 AC_PROG_LN_S
 dnl AC_PREFIX_DEFAULT(/usr/local)

 ##-- use libtool (but don't build static libraries)
 ##  + in Makefile.am, do:
 ##    pdexterns_LTLIBRARIES = ext1.la ...
 ##    ext1_la_SOURCES = ...
 ##    ext1_la_LDFLAGS = -module
 ##  + still unclear how to get *.$(PDEXT) targets built from *.la
 dnl AC_DISABLE_STATIC
 dnl AC_LIBTOOL_DLOPEN
 dnl AC_PROG_LIBTOOL

 dnl ----- maintainer mode
 dnl  + enables "maintainer mode" only with ./configure --enable-maintainer-mode
 dnl    - causes make __never__ to invoke 'config/missing' (i.e. any autotools)
 dnl    - basically a hack to avoid version mismatches in autoconf, automake, etc.
 dnl      for autobuilds from SVN
 dnl  + maintainer should call ./configure --enable-maintainer-mode, and must keep
 dnl    SVN sources consistent
 dnl AM_MAINTAINER_MODE
 dnl /---- maintainer mode

 ## /prerequisites
 ##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

 ##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
 ## versioning
 AC_SUBST(PACKAGE_VERSION)
 AC_SUBST(PACKAGE_NAME)
 AC_SUBST(BUGREPORT)
 ## /versioning
 ##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

 ##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
 ## hack EXEEXT (alternative to using libtool)
 AC_MSG_CHECKING([how to hack automake EXEEXT conventions])
 case "${am__api_version}" in
  1.[[0-4]]*)
    AC_MSG_RESULT([automake v${ap__api_version}: on install])
    pd_buildext=""
    ;;
  *)
    AC_MSG_RESULT([automake v${am__api_version}: on build])
    pd_buildext="\$(EXEEXT)"
    ;;
 esac
 AC_SUBST(pd_buildext)
 ## /hack EXEEXT
 ##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

 ##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
 ## pd-directory/ies
 AC_ARG_WITH(pd-dir,
	AC_HELP_STRING([--with-pd-dir=DIR], [Pd base directory (default=PREFIX/pd)]),
	[pddir="$withval"],
	[pddir="\${prefix}/pd"])
 AC_SUBST(pddir)

 pddocdir="${pddir}/doc/5.reference"
 AC_SUBST(pddocdir)

 ##-- pdincludedir
 AC_ARG_WITH(pd-include,
	AC_HELP_STRING([--with-pd-include=DIR], [Pd include directory (default=NONE)]),
	[pdincludedir="$withval"],
	[pdincludedir=""])
 if test -n "$pdincludedir" ; then
  IFLAGS="$IFLAGS -I$pdincludedir"
 fi
 AC_SUBST(pdincludedir)

 ##-- pdextdir
 AC_ARG_WITH(pd-extdir,
	AC_HELP_STRING([--with-pd-extdir=DIR], [Directory for Pd externals (default=PDDIR/externs)]),
	[pdextdir="$withval"],
	[pdextdir="$pddir/externs"])
 AC_SUBST(pdextdir)

 ##-- pdextdir: aliases
 pdexternsdir="$pdextdir"
 pdexecdir="$pdextdir"
 AC_SUBST(pdexternsdir)
 AC_SUBST(pdexecdir)
 ## pd-directory/ies
 ##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

 ##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
 ## single-object-externals?
 AC_ARG_ENABLE(object-externals,
	AC_HELP_STRING([--enable-object-externals], [Whether to build single-object externals (default=no)]),
	[want_object_externals="$enableval"],
	[want_object_externals="no"])

 AC_MSG_CHECKING([whether to build single-object externals])
 if test "$want_object_externals" != "no" ; then
  ##-- single-objects
  AC_MSG_RESULT(yes)
  AC_DEFINE(WANT_OBJECT_EXTERNALS,1,
     [Define this if you are building single-object externals])
 else
  ##-- multi-lib only
  AC_MSG_RESULT(no)
 fi

 ##-- add automake conditional for object externals
 AM_CONDITIONAL(WANT_OBJECT_EXTERNALS, [test "$want_object_externals" != "no"])
 ##
 ## single-object-externals?
 ##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


 ##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
 ## check: m_pd.h
 CPPFLAGS="$CPPFLAGS $IFLAGS"
 AC_CHECK_HEADER(m_pd.h,[],
	AC_MSG_ERROR([could not find Pd header file 'm_pd.h' - bailing out]),
	[/* nonempty includes: compile only */])
 ##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

 ##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
 ## debugging
 AC_MSG_CHECKING([whether to build a debug version])
 AC_ARG_ENABLE([debug],
	AC_HELP_STRING([--enable-debug],[build debug version (default=no)]))
 if test "$enable_debug" = "yes" ; then
  AC_MSG_RESULT(yes)
  ENABLE_DEBUG="yes"
 else
  AC_MSG_RESULT(no)
  ENABLE_DEBUG="no"
 fi
 AC_SUBST(ENABLE_DEBUG)
 ## debugging
 ##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

 ##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
 ## warning flags, gcc
 if test "$GCC" = "yes"; then
   AC_MSG_CHECKING([whether to set default gcc warning flags])
   case "$UCFLAGS" in
     *-W*)
       AC_MSG_RESULT(no)
       ;;
     *)
       AC_MSG_RESULT(yes)
       PDEXT_WFLAGS="$WFLAGS -Wall -Winline -W -Wno-unused"
       ;;
   esac
 fi
 ## warning flags, gcc
 ##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

 ##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
 ## compiled
 AC_DEFINE_UNQUOTED(PACKAGE_BUILD_DATE,  "`date`",   [Date this package was configured])
 AC_DEFINE_UNQUOTED(PACKAGE_BUILD_USER,  "$USER",    [User who configured this package])
 ## /compiled
 ##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

 ##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
 ## BEGIN platform-dependent variables
 ##

 ##-- Defaults
 LD=ld

 ##----------- `uname -m`: machine hardware name
 AC_MSG_CHECKING([target machine])
 uname_m="`uname -m`"

 ##-- alpha
 if test "$uname_m" = alpha; 
 then
  AC_MSG_RESULT(alpha)
  PDEXT_AFLAGS="-mieee -mcpu=ev56"; 
 else
  AC_MSG_RESULT([$uname_m])
 fi

 ##----------- `uname -s`: kernel name
 AC_MSG_CHECKING([target system])
 uname_s="`uname -s`"

 case "$uname_s" in
   Linux)
     AC_MSG_RESULT(linux)
     ;;
   Darwin)
     AC_MSG_RESULT(darwin)
     ;;
   IRIX64)
     AC_MSG_RESULT(irix64)
     ;;
   IRIX32)
     AC_MSG_RESULT(irix32)
     ;;
   *)
     AC_MSG_RESULT([$uname_s])
     AC_MSG_WARN([Unknown kernel type "$uname_s" defaults to "Linux"])
     uname_s="Linux"
     ;;
  esac

 ##-- Linux
 if test "$uname_s" = "Linux"
 then
  PDEXT_LFLAGS="-Wl,-export-dynamic -shared"
  if test "$ENABLE_DEBUG" = "no" -a -z "$UCFLAGS"; then
    ##-- only set OFLAGS if user CFLAGS are empty
    PDEXT_OFLAGS="-O2 -pipe" 
  else
    PDEXT_OFLAGS="-g"
  fi
  PDEXT_OFLAGS="$PDEXT_OFLAGS -fPIC"
  PDEXT_DFLAGS="$PDEXT_DFLAGS -DPIC"
  PDEXT=pd_linux
 fi

 ##-- MacOSX (darwin)
 if test "$uname_s" = "Darwin"
 then
  LD=cc
  PDEXT_LFLAGS="-bundle -undefined suppress  -flat_namespace"
  PDEXT_DFLAGS="-DMACOSX"
  if test "$ENABLE_DEBUG" = "no" -a -z "$UCFLAGS"; then
    ##-- only set OFLAGS if user CFLAGS are empty
    PDEXT_OFLAGS="-O2"
  else
    PDEXT_OFLAGS="-g"
  fi
  PDEXT=pd_darwin
 fi

 ##-- irix64
 if test "$uname_s" = "IRIX64"
 then
  PDEXT_LFLAGS="-n32 -DUNIX -DIRIX -DN32 -woff 1080,1064,1185 \
	-OPT:roundoff=3 -OPT:IEEE_arithmetic=3 -OPT:cray_ivdep=true \
	-shared -rdata_shared"
  PDEXT=pd_irix6
 fi

 ##-- irix32
 if test "$uname_s" = "IRIX32"
 then
  PDEXT_LFLAGS="-o32 -DUNIX -DIRIX -O2 -shared -rdata_shared"
  PDEXT=pd_irix5
 fi

 ##----------- report pd extension
 AC_MSG_NOTICE([will use pd extension ".$PDEXT" for pd externals])

 EXT=$PDEXT

 ##----------- substitute
 AC_SUBST(LD)
 AC_SUBST(PDEXT)
 AC_SUBST(PDEXT_AFLAGS)
 AC_SUBST(PDEXT_DFLAGS)
 AC_SUBST(PDEXT_IFLAGS)
 AC_SUBST(PDEXT_LFLAGS)
 AC_SUBST(PDEXT_OFLAGS)
 AC_SUBST(PDEXT_WFLAGS)

 ##-- add defaults to user flags
 CPPFLAGS="$UCPPFLAGS $PDEXT_IFLAGS $PDEXT_DFLAGS"
 CFLAGS="$UCFLAGS $PDEXT_FLAGS $PDEXT_AFLAGS $PDEXT_WFLAGS"
 LDFLAGS="$ULDFLAGS $PDEXT_LFLAGS" 

 ## END platform-dependent variables
 ##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
])