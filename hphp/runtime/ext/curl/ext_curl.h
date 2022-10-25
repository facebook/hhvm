/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#pragma once

#include "hphp/runtime/ext/extension.h"

#include <curl/curl.h>

#define CURLOPT_RETURNTRANSFER 19913
#define CURLOPT_BINARYTRANSFER 19914

#define CURLOPT_SAFE_UPLOAD -1

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Variant HHVM_FUNCTION(curl_init, const Variant& url = null_string);
Variant HHVM_FUNCTION(curl_version, int64_t uversion = CURLVERSION_NOW);
bool HHVM_FUNCTION(curl_setopt, const Resource& ch, int64_t option, const Variant& value);
bool HHVM_FUNCTION(curl_setopt_array, const Resource& ch, const Array& options);
Variant HHVM_FUNCTION(curl_exec, const Resource& ch);
Variant HHVM_FUNCTION(curl_getinfo, const Resource& ch, int64_t opt = 0);
Variant HHVM_FUNCTION(curl_errno, const Resource& ch);
Variant HHVM_FUNCTION(curl_error, const Resource& ch);
Variant HHVM_FUNCTION(curl_close, const Resource& ch);
Resource HHVM_FUNCTION(curl_multi_init);
Variant HHVM_FUNCTION(curl_multi_add_handle, const Resource& mh, const Resource& ch);
Variant HHVM_FUNCTION(curl_multi_remove_handle, const Resource& mh, const Resource& ch);
Variant HHVM_FUNCTION(curl_multi_exec, const Resource& mh, int64_t& still_running);
Variant HHVM_FUNCTION(curl_multi_select, const Resource& mh, double timeout = 1.0);
Variant HHVM_FUNCTION(curl_multi_getcontent, const Resource& ch);
Variant HHVM_FUNCTION(curl_multi_info_read, const Resource& mh,
                               int64_t& msgs_in_queue);
Variant HHVM_FUNCTION(curl_multi_close, const Resource& mh);

///////////////////////////////////////////////////////////////////////////////
}
