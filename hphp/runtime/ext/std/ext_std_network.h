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

#include "hphp/runtime/ext/std/ext_std.h"
#include "hphp/runtime/ext/stream/ext_stream.h"
#include "hphp/util/network.h"

#include <folly/portability/Syslog.h>

namespace HPHP {

OptString HHVM_FUNCTION(gethostbyname, const OptString& hostname);
Variant HHVM_FUNCTION(getservbyname, const OptString& service,
                                     const OptString& protocol);
bool HHVM_FUNCTION(checkdnsrr, const OptString& host,
                               const OptString& type = null_string);
Variant HHVM_FUNCTION(dns_get_record, const OptString& hostname, int64_t type,
                                      Variant& authnsRef,
                                      Variant& addtlRef);
bool HHVM_FUNCTION(getmxrr, const OptString& hostname,
                            Variant& mxhostsRef,
                            Variant& weightsRef);
void HHVM_FUNCTION(header, const OptString& str, bool replace = true,
                   int64_t http_response_code = 0);
bool HHVM_FUNCTION(headers_sent);

bool validate_dns_arguments(const OptString& host, const OptString& type,
                            int& ntype);
}
