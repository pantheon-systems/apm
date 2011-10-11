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
 | Authors: Davide Mendolia <dmendolia@php.net>                         |
 |          Patrick Allaert <patrickallaert@php.net>                    |
 +----------------------------------------------------------------------+
*/

#include <curl/curl.h>
#include "php_apm.h"
#include "php_ini.h"
#include "driver_http.h"
#include <stdio.h>

ZEND_EXTERN_MODULE_GLOBALS(apm)

ZEND_DECLARE_MODULE_GLOBALS(apm_http)

APM_DRIVER_CREATE(http)

PHP_INI_BEGIN()
	/* Boolean controlling whether the driver is active or not */
	STD_PHP_INI_BOOLEAN("apm.http_enabled",                "1",                PHP_INI_PERDIR, OnUpdateBool,   enabled,         zend_apm_http_globals, apm_http_globals)
	/* error_reporting of the driver */
	STD_PHP_INI_ENTRY("apm.http_error_reporting",          NULL,               PHP_INI_ALL,    OnUpdateAPMhttpErrorReporting,   error_reporting, zend_apm_http_globals, apm_http_globals)
	/* Max timeout to wait for storing the event */
	STD_PHP_INI_ENTRY("apm.http_request_timeout",          "100",              PHP_INI_ALL,    OnUpdateLong,   timeout,         zend_apm_http_globals, apm_http_globals)

	//STD_PHP_INI_ENTRY("apm.http_server",                   "http://localhost", PHP_INI_ALL,    OnUpdateDBFile, db_path,         zend_apm_sqlite3_globals, apm_sqlite3_globals)
	//STD_PHP_INI_ENTRY("apm.https_client_certificate",      NULL,               PHP_INI_ALL,    OnUpdateDBFile, db_path,         zend_apm_sqlite3_globals, apm_sqlite3_globals)
	//STD_PHP_INI_ENTRY("apm.https_certificate_authorities", NULL,               PHP_INI_ALL,    OnUpdateDBFile, db_path,         zend_apm_sqlite3_globals, apm_sqlite3_globals)
PHP_INI_END()

void apm_test_http_internal() {
  printf("> apm_test_http_internal\n");
  
  CURL *curl;
  CURLcode res;
  
  curl = curl_easy_init();
  if(curl) {
    struct curl_slist *chunk = NULL;
    chunk = curl_slist_append(chunk, "Content-Type: application/json");
    
    curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:9000");
    
    //curl_easy_setopt(curl,CURLOPT_SSLCERT,pCertFile);
    
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "Hello, Twisted!");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
    res = curl_easy_perform(curl);
 
    /* always cleanup */ 
    curl_easy_cleanup(curl);
  }  
}

/* Insert an event in the backend */
void apm_driver_http_insert_event(int type, char * error_filename, uint error_lineno, char * msg, char * trace TSRMLS_DC)
{
  CURL *curl;
  CURLcode res;
  
  curl_global_init(CURL_GLOBAL_ALL);
  curl = curl_easy_init();
  if(curl) {
    struct curl_httppost *formpost = NULL;
    struct curl_httppost *lastptr = NULL;
    struct curl_slist *headerlist = NULL;
    static const char buf[] = "Expect:";
    char int2string[64];
    
    sprintf(int2string, "%d", type);
    curl_formadd(&formpost,
             &lastptr,
             CURLFORM_COPYNAME, "type",
             CURLFORM_COPYCONTENTS, int2string,
             CURLFORM_END);

    curl_formadd(&formpost,
             &lastptr,
             CURLFORM_COPYNAME, "file",
             CURLFORM_COPYCONTENTS, error_filename ? error_filename : "",
             CURLFORM_END);

    sprintf(int2string, "%d", error_lineno);
    curl_formadd(&formpost,
             &lastptr,
             CURLFORM_COPYNAME, "line",
             CURLFORM_COPYCONTENTS, int2string,
             CURLFORM_END);

    curl_formadd(&formpost,
             &lastptr,
             CURLFORM_COPYNAME, "message",
             CURLFORM_COPYCONTENTS, msg ? msg : "",
             CURLFORM_END);

    curl_formadd(&formpost,
             &lastptr,
             CURLFORM_COPYNAME, "backtrace",
             CURLFORM_COPYCONTENTS, trace ? trace : "",
             CURLFORM_END);
    
    headerlist = curl_slist_append(headerlist, buf);
    curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:9000");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
    res = curl_easy_perform(curl);
 
    /* always cleanup */ 
    curl_easy_cleanup(curl);
  }
}

void apm_driver_http_insert_events(apm_event_entry * event_entry TSRMLS_DC)
{
	apm_event_entry * event_entry_cursor = event_entry;
  apm_test_http_internal();
	while ((event_entry_cursor = event_entry_cursor->next) != NULL) {
		if (event_entry_cursor->event.type & apm_driver_http_error_reporting()) {
			apm_driver_http_insert_event(event_entry_cursor->event.type, event_entry_cursor->event.error_filename, event_entry_cursor->event.error_lineno, event_entry_cursor->event.msg, event_entry_cursor->event.trace TSRMLS_CC);
		}
	}
}

int apm_driver_http_minit(int module_number)
{
	REGISTER_INI_ENTRIES();
	return SUCCESS;
}

int apm_driver_http_rinit()
{
	return SUCCESS;
}

int apm_driver_http_mshutdown()
{
	return SUCCESS;
}

int apm_driver_http_rshutdown()
{
	return SUCCESS;
}

void apm_driver_http_insert_slow_request(float duration, char * script_filename)
{
  // @TODO: Record slow request here.
}

/* {{{ proto void apm_test_http()
   Sends a test POST to the configured HTTP server. */
PHP_FUNCTION(apm_test_http)
{
  apm_test_http_internal();
}
