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

#ifndef incl_HPHP_EXT_ERROR_H_
#define incl_HPHP_EXT_ERROR_H_

#include "hphp/runtime/ext/std/ext_std.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

extern const int64_t k_DEBUG_BACKTRACE_PROVIDE_OBJECT;
extern const int64_t k_DEBUG_BACKTRACE_IGNORE_ARGS;
extern const int64_t k_DEBUG_BACKTRACE_PROVIDE_METADATA;
extern const int64_t k_E_ERROR;
extern const int64_t k_E_WARNING;
extern const int64_t k_E_PARSE;
extern const int64_t k_E_NOTICE;
extern const int64_t k_E_CORE_ERROR;
extern const int64_t k_E_CORE_WARNING;
extern const int64_t k_E_COMPILE_ERROR;
extern const int64_t k_E_COMPILE_WARNING;
extern const int64_t k_E_USER_ERROR;
extern const int64_t k_E_USER_WARNING;
extern const int64_t k_E_USER_NOTICE;
extern const int64_t k_E_STRICT;
extern const int64_t k_E_RECOVERABLE_ERROR;
extern const int64_t k_E_DEPRECATED;
extern const int64_t k_E_USER_DEPRECATED;
extern const int64_t k_E_ALL;

Array HHVM_FUNCTION(debug_backtrace,
                    int64_t options = k_DEBUG_BACKTRACE_PROVIDE_OBJECT,
                    int64_t limit = 0);
Array HHVM_FUNCTION(hphp_debug_caller_info);
void HHVM_FUNCTION(debug_print_backtrace, int64_t options = 0,
                                          int64_t limit = 0);
Array HHVM_FUNCTION(error_get_last);
bool HHVM_FUNCTION(error_log, const String& message, int message_type = 0,
                              const Variant& destination = null_variant,
                              const Variant& extra_headers = null_variant);
int64_t HHVM_FUNCTION(error_reporting, const Variant& level = null_variant);
bool HHVM_FUNCTION(restore_error_handler);
bool HHVM_FUNCTION(restore_exception_handler);
Variant HHVM_FUNCTION(set_error_handler, const Variant& error_handler,
                                         int error_types = k_E_ALL);
Variant HHVM_FUNCTION(set_exception_handler, const Variant& exception_handler);
void HHVM_FUNCTION(hphp_set_error_page, const String& page);
void HHVM_FUNCTION(hphp_throw_fatal_error, const String& error_msg);
void HHVM_FUNCTION(hphp_clear_unflushed);
bool HHVM_FUNCTION(trigger_error, const String& error_msg,
                                  int error_type = k_E_USER_NOTICE);
bool HHVM_FUNCTION(user_error, const String& error_msg,
                               int error_type = k_E_USER_NOTICE);

String debug_string_backtrace(bool skip, bool ignore_args = false,
                              int64_t limit = 0);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_ERROR_H_
