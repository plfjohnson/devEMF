# Process this file with autoconf to produce a configure script.
AC_INIT(devEMF, 2)

# find R home and set CXX/CXXFLAGS
: ${R_HOME=`R RHOME`}
if test -z "${R_HOME}"; then
  echo "could not determine R_HOME"
  exit 1
fi
RBIN="${R_HOME}/bin/R"
CXX=`"${RBIN}" CMD config CXX`;
CXXFLAGS=`"${RBIN}" CMD config CXXFLAGS`
AC_SUBST(R_HOME)

AC_LANG(C++)

#-----------------------------------------------------------------------
AC_PATH_PROG([PKGCONF],[pkg-config],[],[$PATH:/usr/local/bin:ext/bin:ext:/sw/bin:/opt/bin])

if test "xx$FONTCONFIG_LIBS" = "xx"; then
   if test "xx$PKGCONF" != "xx"; then
      AC_MSG_CHECKING([whether pkg-config knows about Fontconfig])
      if "${PKGCONF}" --exists fontconfig; then
         AC_MSG_RESULT([yes])
         FONTCONFIG_CFLAGS=`"${PKGCONF}" --cflags fontconfig`
         FONTCONFIG_LIBS=`"${PKGCONF}" --libs fontconfig`
         AC_MSG_NOTICE([FONTCONFIG_CFLAGS=${FONTCONFIG_CFLAGS}])
         AC_MSG_NOTICE([FONTCONFIG_LIBS=${FONTCONFIG_LIBS}])
      else
         AC_MSG_RESULT([no])
      fi
   fi
fi


if test "xx$ZLIB_LIBS" = "xx"; then
   if test "xx$PKGCONF" != "xx"; then
      AC_MSG_CHECKING([whether pkg-config knows about zlib])
      if "${PKGCONF}" --exists zlib; then
         AC_MSG_RESULT([yes])
         ZLIB_CFLAGS=`"${PKGCONF}" --cflags zlib`
         ZLIB_LIBS=`"${PKGCONF}" --libs zlib`
         AC_MSG_NOTICE([ZLIB_CFLAGS=${ZLIB_CFLAGS}])
         AC_MSG_NOTICE([ZLIB_LIBS=${ZLIB_LIBS}])
      else
         AC_MSG_RESULT([no])
      fi
   fi
fi

# below only applies to Mac/OSX
if test "xx$OSX_LIBS" = "xx"  &&  test `uname -s` = "Darwin"; then
   OSX_LIBS="-framework CoreText"
fi

CPPFLAGS="${FONTCONFIG_CFLAGS} ${ZLIB_CFLAGS}"
AC_CHECK_HEADERS_ONCE(zlib.h)
AC_CHECK_HEADERS_ONCE(fontconfig/fontconfig.h)
AC_CHECK_HEADERS_ONCE(CoreText/CTFont.h)


GOTZ=0
GOTFC=0
GOTCT=0
if (test "x$ac_cv_header_CoreText_CTFont_h" = xyes); then
   GOODLIBS="${LIBS}"
   LIBS="${GOODLIBS} ${OSX_LIBS}"
   AC_SEARCH_LIBS(CTFontGetGlyphsForCharacters, ,
                  [CPPFLAGS="${CPPFLAGS} -DHAVE_CORETEXT"; GOTCT=1],
                  [LIBS="${GOODLIBS}"])
fi

# below GOTCT conditional works around case when compiling Mac has
# extra libraries installed that are not present (non-default) on
# installation computer
if test ${GOTCT} = 0; then
   if (test "x$ac_cv_header_zlib_h" = xyes); then
      GOODLIBS="${LIBS}"
      LIBS="${GOODLIBS} ${ZLIB_LIBS}"
      AC_SEARCH_LIBS(gzopen, z,
                     [CPPFLAGS="${CPPFLAGS} -DHAVE_ZLIB"; GOTZ=1],
                     [LIBS="${GOODLIBS}"])
   fi
   if (test "x$ac_cv_header_fontconfig_fontconfig_h" = xyes); then
      GOODLIBS="${LIBS}"
      LIBS="${GOODLIBS} ${FONTCONFIG_LIBS}"
      AC_SEARCH_LIBS(FcFontMatch, ,
                     [CPPFLAGS="${CPPFLAGS} -DHAVE_FONTCONFIG"; GOTFC=1],
                     [LIBS="${GOODLIBS}"])
   fi
fi

if test ${GOTZ} = 0  &&  test ${GOTFC} = 0  &&  test ${GOTCT} = 0; then
   #only warn about Z and X because moot if missing CoreText
   AC_MSG_ERROR([Cannot find fontconfig.h or zlib.h! Please install the fontconfig or Zlib development headers and/or set FONTCONFIG_CFLAGS/LIBS or ZLIB_CFLAGS/LIBS correspondingly.])
fi

AC_CONFIG_FILES([src/Makevars])
AC_OUTPUT
