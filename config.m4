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

AC_DEFUN([MYSQL_LIB_CHK], [
  str="$MYSQL_DIR/$1/lib$MYSQL_LIBNAME.*"
  for j in `echo $str`; do
    if test -r $j; then
      MYSQL_LIB_DIR=$MYSQL_DIR/$1
      break 2
    fi
  done
])

PHP_ARG_ENABLE(apm, whether to enable apm support,
[  --enable-apm            Enable apm support], yes)
PHP_ARG_WITH(mysql, enable support for MySQL,
[  --with-mysql=MYSQLDIR        Location of MySQL base directory], no, no)
PHP_ARG_WITH(curl, for cURL support,
[  --with-curl[=DIR]       Include cURL support for HTTP reporting])

if test -z "$PHP_ZLIB_DIR"; then
  PHP_ARG_WITH(zlib-dir, for the location of libz, 
  [  --with-zlib-dir[=DIR]     MySQL: Set the path to libz install prefix], no, no)
fi

if test "$PHP_APM" != "no"; then

  AC_CONFIG_HEADERS()

  if test "$PHP_MYSQL" != "no"; then
    mysql_driver="driver_mysql.c"
    AC_DEFINE(APM_DRIVER_MYSQL, 1, [activate MySQL storage driver])

    MYSQL_DIR=
    MYSQL_INC_DIR=

    for i in $PHP_MYSQL /usr/local /usr; do
      if test -r $i/include/mysql/mysql.h; then
        MYSQL_DIR=$i
        MYSQL_INC_DIR=$i/include/mysql
        break
      elif test -r $i/include/mysql.h; then
        MYSQL_DIR=$i
        MYSQL_INC_DIR=$i/include
        break
      fi
    done

    if test -z "$MYSQL_DIR"; then
      AC_MSG_ERROR([Cannot find MySQL header files])
    fi

    if test "$enable_maintainer_zts" = "yes"; then
      MYSQL_LIBNAME=mysqlclient_r
    else
      MYSQL_LIBNAME=mysqlclient
    fi
    case $host_alias in
      *netware*[)]
        MYSQL_LIBNAME=mysql
        ;;
    esac

    for i in $PHP_LIBDIR $PHP_LIBDIR/mysql; do
      MYSQL_LIB_CHK($i)
    done

    if test -z "$MYSQL_LIB_DIR"; then
      AC_MSG_ERROR([Cannot find lib$MYSQL_LIBNAME under $MYSQL_DIR.])
    fi

    PHP_CHECK_LIBRARY($MYSQL_LIBNAME, mysql_close, [ ],
    [
      if test "$PHP_ZLIB_DIR" != "no"; then
        PHP_ADD_LIBRARY_WITH_PATH(z, $PHP_ZLIB_DIR, MYSQL_SHARED_LIBADD)
        PHP_CHECK_LIBRARY($MYSQL_LIBNAME, mysql_error, [], [
          AC_MSG_ERROR([mysql configure failed. Please check config.log for more information.])
        ], [
          -L$PHP_ZLIB_DIR/$PHP_LIBDIR -L$MYSQL_LIB_DIR 
        ])  
        MYSQL_LIBS="-L$PHP_ZLIB_DIR/$PHP_LIBDIR -lz"
      else
        PHP_ADD_LIBRARY(z,, MYSQL_SHARED_LIBADD)
        PHP_CHECK_LIBRARY($MYSQL_LIBNAME, mysql_errno, [], [
          AC_MSG_ERROR([Try adding --with-zlib-dir=<DIR>. Please check config.log for more information.])
        ], [
          -L$MYSQL_LIB_DIR
        ])   
        MYSQL_LIBS="-lz"
      fi
    ], [
      -L$MYSQL_LIB_DIR 
    ])

    PHP_ADD_LIBRARY_WITH_PATH($MYSQL_LIBNAME, $MYSQL_LIB_DIR, MYSQL_SHARED_LIBADD)
    PHP_ADD_INCLUDE($MYSQL_INC_DIR)

    MYSQL_MODULE_TYPE=external
    MYSQL_LIBS="-L$MYSQL_LIB_DIR -l$MYSQL_LIBNAME $MYSQL_LIBS"
    MYSQL_INCLUDE=-I$MYSQL_INC_DIR

    PHP_SUBST_OLD(MYSQL_MODULE_TYPE)
    PHP_SUBST_OLD(MYSQL_LIBS)
    PHP_SUBST_OLD(MYSQL_INCLUDE)

    AC_DEFINE(HAVE_MYSQL,1,[MySQL found and included])
  fi

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

  PHP_NEW_EXTENSION(apm, apm.c backtrace.c $mysql_driver $http_driver, $ext_shared)
  PHP_SUBST(APM_SHARED_LIBADD)
fi
