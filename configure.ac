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
if test `uname -s` == "Darwin"; then # on OSX
   LIBS="-framework CoreText"
else  # if linux/unix (not OSX)
   AC_PATH_PROG([PKGCONF],[pkg-config],[],[$PATH:/usr/local/bin:ext/bin:ext:/sw/bin:/opt/bin])

   if test "xx$XFT_LIBS" == "xx"; then
      if test "xx$PKGCONF" != "xx"; then
         AC_MSG_CHECKING([whether pkg-config knows about xft])
         if "${PKGCONF}" --exists xft; then
            AC_MSG_RESULT([yes])
            XFT_CFLAGS=`"${PKGCONF}" --cflags xft`
            XFT_LIBS=`"${PKGCONF}" --libs xft`
            AC_MSG_NOTICE([XFT_CFLAGS=${XFT_CFLAGS}])
            AC_MSG_NOTICE([XFT_LIBS=${XFT_LIBS}])
         else
            AC_MSG_RESULT([no])
         fi
      fi
   fi


   if test "xx$FREETYPE_LIBS" == "xx"; then
      if test "xx$PKGCONF" != "xx"; then
         AC_MSG_CHECKING([whether pkg-config knows about freetype2])
         if "${PKGCONF}" --exists freetype2; then
            AC_MSG_RESULT([yes])
            FREETYPE_CFLAGS=`"${PKGCONF}" --cflags freetype2`
            FREETYPE_LIBS=`"${PKGCONF}" --libs freetype2`
            AC_MSG_NOTICE([FREETYPE_CFLAGS=${FREETYPE_CFLAGS}])
            AC_MSG_NOTICE([FREETYPE_LIBS=${FREETYPE_LIBS}])
         else
            AC_MSG_RESULT([no])
         fi
      fi
   fi


   if test "xx$Z_LIBS" == "xx"; then
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


   CPPFLAGS="${XFT_CFLAGS} ${FREETYPE_CFLAGS} ${ZLIB_CFLAGS}"
   LIBS="${XFT_LIBS} ${FREETYPE_LIBS} ${ZLIB_LIBS}"

   AC_CHECK_HEADERS_ONCE(zlib.h)
   AC_CHECK_HEADERS_ONCE(ft2build.h)
   AC_CHECK_HEADERS_ONCE(X11/Xft/Xft.h)

   GOTZ=0
   GOTX=0
   if (test "x$ac_cv_header_zlib_h" == xyes); then
      AC_SEARCH_LIBS(gzopen, z,
                     [CPPFLAGS="${CPPFLAGS} -DHAVE_ZLIB"; GOTZ=1])
   fi
   if (test "x$ac_cv_header_X11_Xft_Xft_h" == xyes); then
      AC_SEARCH_LIBS(XftCharExists, Xft,
                     [CPPFLAGS="${CPPFLAGS} -DHAVE_XFT"; GOTX=1])
   fi

   if test ${GOTZ} = 0  &&  test ${GOTX} = 0; then
      AC_MSG_ERROR([Cannot find Xft.h or zlib.h! Please install the Xft/FreeType or Zlib development headers and/or set XFT_CFLAGS/LIBS or ZLIB_CFLAGS/LIBS correspondingly.])
   fi

fi

AC_CONFIG_FILES([src/Makevars])
AC_OUTPUT
