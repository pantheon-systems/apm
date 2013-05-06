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
	STD_PHP_INI_BOOLEAN("apm.http_enabled",                "1",                PHP_INI_PERDIR, OnUpdateBool,                  enabled,                       zend_apm_http_globals, apm_http_globals)
	STD_PHP_INI_ENTRY("apm.http_error_reporting",          NULL,               PHP_INI_ALL,    OnUpdateAPMhttpErrorReporting, error_reporting,               zend_apm_http_globals, apm_http_globals)
	STD_PHP_INI_ENTRY("apm.http_request_timeout",          "1000",             PHP_INI_ALL,    OnUpdateLong,                  http_request_timeout,          zend_apm_http_globals, apm_http_globals)
	STD_PHP_INI_ENTRY("apm.http_server",                   "http://localhost", PHP_INI_ALL,    OnUpdateString,                http_server,                   zend_apm_http_globals, apm_http_globals)
	STD_PHP_INI_ENTRY("apm.https_client_certificate",      NULL,               PHP_INI_ALL,    OnUpdateString,                https_client_certificate,      zend_apm_http_globals, apm_http_globals)
	STD_PHP_INI_ENTRY("apm.https_client_key",              NULL,               PHP_INI_ALL,    OnUpdateString,                https_client_key,              zend_apm_http_globals, apm_http_globals)
	STD_PHP_INI_ENTRY("apm.https_certificate_authorities", NULL,               PHP_INI_ALL,    OnUpdateString,                https_certificate_authorities, zend_apm_http_globals, apm_http_globals)
	STD_PHP_INI_ENTRY("apm.http_max_backtrace_length",     "-1",               PHP_INI_ALL,    OnUpdateLong,                  http_max_backtrace_length,     zend_apm_http_globals, apm_http_globals)
PHP_INI_END()

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
             CURLFORM_COPYCONTENTS, truncate_trace(trace ? trace : "", APM_HTTP_G(http_max_backtrace_length)),
             CURLFORM_END);
    
    headerlist = curl_slist_append(headerlist, buf);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
    
    curl_easy_setopt(curl, CURLOPT_URL, APM_HTTP_G(http_server));    
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, APM_HTTP_G(http_request_timeout));
    if (APM_HTTP_G(https_client_certificate) != NULL) {
      curl_easy_setopt(curl, CURLOPT_SSLCERT, APM_HTTP_G(https_client_certificate));
    }
    if (APM_HTTP_G(https_client_key) != NULL) {
      curl_easy_setopt(curl, CURLOPT_SSLKEY, APM_HTTP_G(https_client_key));
    }
    if (APM_HTTP_G(https_certificate_authorities) != NULL) {
      curl_easy_setopt(curl, CURLOPT_CAINFO, APM_HTTP_G(https_certificate_authorities));
      curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    }
    
    res = curl_easy_perform(curl);
 
    /* always cleanup */ 
    curl_easy_cleanup(curl);
  }
}

void apm_driver_http_insert_events(apm_event_entry * event_entry TSRMLS_DC)
{
	apm_event_entry * event_entry_cursor = event_entry;
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

char * truncate_trace (const char * input_str, long str_len) {
    char *buff = NULL;

    /* Cannot do anything with NULL. */
    if (input_str == NULL) return input_str;

    /* Force negative lengths to return the input string */
    if (str_len < 0)
        return input_str;
        /* str_len = strlen (&input_str[0]); */

    /* Adjust length if source string too short. */
    if (str_len > strlen (&input_str[0]))
        str_len = strlen (&input_str[0]);

    /* Get long enough string from heap, return NULL if no go. */
    if ((buff = malloc (str_len + 1)) == NULL)
      return NULL;

    /* Transfer string section and return it. */
    memcpy (buff, &(input_str[0]), str_len);
    buff[str_len] = '\0';

    return buff;
}

/* {{{ proto void apm_test_http()
   Sends a test POST to the configured HTTP server. */
PHP_FUNCTION(apm_test_http)
{
  apm_driver_http_insert_event(1, "/path/to/file.php", 123, "Some Error Msg", "#0 some_function() called by [/path/to/file.php:123]");
}
