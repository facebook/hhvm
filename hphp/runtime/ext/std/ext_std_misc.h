/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_EXT_MISC_H_
#define incl_HPHP_EXT_MISC_H_

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/std/ext_std.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

int64_t HHVM_FUNCTION(connection_aborted);
int64_t HHVM_FUNCTION(connection_status);
int64_t HHVM_FUNCTION(connection_timeout);
Variant HHVM_FUNCTION(constant, const String& name);
bool HHVM_FUNCTION(define, const String& name, const Variant& value,
                   bool case_insensitive = false);
bool HHVM_FUNCTION(defined, const String& name, bool autoload = true);
int64_t HHVM_FUNCTION(ignore_user_abort, bool setting = false);
TypedValue* HHVM_FUNCTION(pack, ActRec* ar);
int64_t HHVM_FUNCTION(sleep, int seconds);
void HHVM_FUNCTION(usleep, int micro_seconds);
Variant HHVM_FUNCTION(time_nanosleep, int seconds, int nanoseconds);
bool HHVM_FUNCTION(time_sleep_until, double timestamp);
String HHVM_FUNCTION(uniqid, const String& prefix = null_string,
                     bool more_entropy = false);
Variant HHVM_FUNCTION(unpack, const String& format, const String& data);
Array HHVM_FUNCTION(sys_getloadavg);
Array HHVM_FUNCTION(token_get_all, const String& source);
String HHVM_FUNCTION(token_name, int64_t token);
String HHVM_FUNCTION(hphp_to_string, const Variant& v);
Variant HHVM_FUNCTION(SystemLib_max2, const Variant& arg1, const Variant& arg2);
Variant HHVM_FUNCTION(SystemLib_min2, const Variant& arg1, const Variant& arg2);
extern const double k_INF;
extern const double k_NAN;

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_MISC_H_
