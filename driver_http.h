/*
 +----------------------------------------------------------------------+
 |  APM stands for Alternative PHP Monitor                              |
 +----------------------------------------------------------------------+
 | Copyright (c) 2008-2010  Davide Mendolia, Patrick Allaert            |
 +----------------------------------------------------------------------+
 | This source file is subject to version 3.01 of the PHP license,      |
 | that is bundled with this package in the file LICENSE, and is        |
 | available through the world-wide-web at the following url:           |
 | http://www.php.net/license/3_01.txt                                  |
 | If you did not receive a copy of the PHP license and are unable to   |
 | obtain it through the world-wide-web, please send a note to          |
 | license@php.net so we can mail you a copy immediately.               |
 +----------------------------------------------------------------------+
 | Authors: David Strauss <david@davidstrauss.net>                      |
 |          Patrick Allaert <patrickallaert@php.net>                    |
 +----------------------------------------------------------------------+
*/

#ifndef DRIVER_HTTP_H
#define DRIVER_HTTP_H

#include <curl/curl.h>

#define APM_E_http APM_E_ALL

apm_driver_entry * apm_driver_http_create();
void apm_driver_http_insert_event(int type, char * error_filename, uint error_lineno, char * msg, char * trace TSRMLS_DC);
void apm_driver_http_insert_events(apm_event_entry * event_entry TSRMLS_DC);
int apm_driver_http_minit(int);
int apm_driver_http_rinit();
int apm_driver_http_mshutdown();
int apm_driver_http_rshutdown();
void apm_driver_http_insert_slow_request(float duration, char * script_filename);

/* Extension globals */
ZEND_BEGIN_MODULE_GLOBALS(apm_http)
	/* Boolean controlling whether the driver is active or not */
	zend_bool enabled;

	/* Driver error reporting */
	int     error_reporting;

	/* Max timeout to wait for the HTTP POST request */
	long      timeout;
ZEND_END_MODULE_GLOBALS(apm_http)

#ifdef ZTS
#define APM_S3_G(v) TSRMG(apm_http_globals_id, zend_apm_http_globals *, v)
#else
#define APM_S3_G(v) (apm_http_globals.v)
#endif

#endif
