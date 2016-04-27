dnl config.m4 for extension apm

dnl    APM stands for Alternative PHP Monitor
dnl    Copyright (C) 2008-2010  Davide Mendolia, Patrick Allaert
dnl
dnl    This file is part of APM.
dnl
dnl    This source file is subject to version 3.01 of the PHP license,
dnl    that is bundled with this package in the file LICENSE, and is
dnl    available through the world-wide-web at the following url:
dnl    http://www.php.net/license/3_01.txt
dnl    If you did not receive a copy of the PHP license and are unable to
dnl    obtain it through the world-wide-web, please send a note to
dnl    license@php.net so we can mail you a copy immediately.
dnl
dnl    Authors: Davide Mendolia <dmendolia@php.net>
dnl             Patrick Allaert <patrickallaert@php.net>

PHP_ARG_ENABLE(apm, whether to enable apm support,
[  --enable-apm            Enable apm support], yes)
PHP_ARG_WITH(curl, for cURL support,
[  --with-curl[=DIR]       Include cURL support for HTTP reporting])

if test -z "$PHP_ZLIB_DIR"; then
  PHP_ARG_WITH(zlib-dir, for the location of libz,
  [  --with-zlib-dir[=DIR]     MySQL: Set the path to libz install prefix], no, no)
fi

if test "$PHP_APM" != "no"; then

  AC_CONFIG_HEADERS()

  if test "$PHP_CURL" != "no"; then
    http_driver="driver_http.c"
    AC_DEFINE(APM_DRIVER_HTTP, 1, [activate HTTP sending driver])
    AC_DEFINE(HAVE_HTTP, 1, [HTTP found and included])

    if test -r $PHP_CURL/include/curl/easy.h; then
      CURL_DIR=$PHP_CURL
    else
      AC_MSG_CHECKING(for cURL in default path)
      for i in /usr/local /usr; do
        if test -r $i/include/curl/easy.h; then
          CURL_DIR=$i
          AC_MSG_RESULT(found in $i)
          break
        fi
      done
    fi

    PHP_ADD_INCLUDE($CURL_DIR/include)
    PHP_EVAL_LIBLINE($CURL_LIBS, APM_SHARED_LIBADD)
    PHP_ADD_LIBRARY_WITH_PATH(curl, $CURL_DIR/$PHP_LIBDIR, APM_SHARED_LIBADD)

    PHP_CHECK_LIBRARY(curl,curl_easy_perform,
      [
        AC_DEFINE(HAVE_CURL,1,[ ])
      ],[
        AC_MSG_ERROR(There is something wrong. Please check config.log for more information.)
      ],[
        $CURL_LIBS -L$CURL_DIR/$PHP_LIBDIR
      ])

  fi

  PHP_NEW_EXTENSION(apm, apm.c backtrace.c $http_driver, $ext_shared)
  PHP_SUBST(APM_SHARED_LIBADD)
fi
