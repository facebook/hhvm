/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#ifndef incl_HPHP_EXT_CURL_H_
#define incl_HPHP_EXT_CURL_H_

#include "hphp/runtime/ext/extension.h"

#include <curl/curl.h>

#define CURLOPT_RETURNTRANSFER 19913
#define CURLOPT_BINARYTRANSFER 19914

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Variant HHVM_FUNCTION(curl_init, const Variant& url = null_string);
Variant HHVM_FUNCTION(curl_init_pooled, const String& poolName,
                              const Variant& url = null_string);
Variant HHVM_FUNCTION(curl_copy_handle, const Resource& ch);
Variant HHVM_FUNCTION(curl_version, int uversion = CURLVERSION_NOW);
bool HHVM_FUNCTION(curl_setopt, const Resource& ch, int option, const Variant& value);
bool HHVM_FUNCTION(curl_setopt_array, const Resource& ch, const Array& options);
Variant HHVM_FUNCTION(fb_curl_getopt, const Resource& ch, int64_t opt = 0);
Variant HHVM_FUNCTION(curl_exec, const Resource& ch);
Variant HHVM_FUNCTION(curl_getinfo, const Resource& ch, int opt = 0);
Variant HHVM_FUNCTION(curl_errno, const Resource& ch);
Variant HHVM_FUNCTION(curl_error, const Resource& ch);
String HHVM_FUNCTION(curl_strerror, int code);
Variant HHVM_FUNCTION(curl_close, const Resource& ch);
void HHVM_FUNCTION(curl_reset, const Resource& ch);
Resource HHVM_FUNCTION(curl_multi_init);
Variant HHVM_FUNCTION(curl_multi_strerror, int64_t code);
Variant HHVM_FUNCTION(curl_multi_add_handle, const Resource& mh, const Resource& ch);
Variant HHVM_FUNCTION(curl_multi_remove_handle, const Resource& mh, const Resource& ch);
Variant HHVM_FUNCTION(curl_multi_exec, const Resource& mh, VRefParam still_running);
Variant HHVM_FUNCTION(curl_multi_select, const Resource& mh, double timeout = 1.0);
Variant HHVM_FUNCTION(curl_multi_getcontent, const Resource& ch);
Variant HHVM_FUNCTION(fb_curl_multi_fdset, const Resource& mh,
                              VRefParam read_fd_set,
                              VRefParam write_fd_set,
                              VRefParam exc_fd_set,
                              VRefParam max_fd = null_object);
Variant HHVM_FUNCTION(curl_multi_info_read, const Resource& mh,
                               VRefParam msgs_in_queue = null_object);
Variant HHVM_FUNCTION(curl_multi_close, const Resource& mh);
bool HHVM_FUNCTION(curl_multi_setopt, const Resource& mh,
                              int option,
                              const Variant& value);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_CURL_H_
