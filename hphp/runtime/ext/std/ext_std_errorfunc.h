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

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

extern const int64_t k_DEBUG_BACKTRACE_PROVIDE_OBJECT;
extern const int64_t k_DEBUG_BACKTRACE_IGNORE_ARGS;
extern const int64_t k_DEBUG_BACKTRACE_PROVIDE_METADATA;

extern const int64_t k_DEBUG_BACKTRACE_HASH_CONSIDER_METADATA;

Array HHVM_FUNCTION(debug_backtrace,
                    int64_t options = k_DEBUG_BACKTRACE_PROVIDE_OBJECT,
                    int64_t limit = 0);
Array HHVM_FUNCTION(hphp_debug_caller_info);
int64_t HHVM_FUNCTION(hphp_debug_backtrace_hash, int64_t options = 0);
void HHVM_FUNCTION(debug_print_backtrace, int64_t options = 0,
                                          int64_t limit = 0);
Array HHVM_FUNCTION(error_get_last);
bool HHVM_FUNCTION(error_log, const String& message, int message_type = 0,
                              const Variant& destination = uninit_variant,
                              const Variant& extra_headers = uninit_variant);
int64_t HHVM_FUNCTION(error_reporting, const Variant& level = uninit_variant);
bool HHVM_FUNCTION(restore_error_handler);
bool HHVM_FUNCTION(restore_exception_handler);
Variant HHVM_FUNCTION(set_error_handler, const Variant& error_handler,
                      int error_types = ((int)ErrorMode::PHP_ALL |
                                         (int)ErrorMode::STRICT));
Variant HHVM_FUNCTION(set_exception_handler, const Variant& exception_handler);
void HHVM_FUNCTION(hphp_set_error_page, const String& page);
void HHVM_FUNCTION(hphp_throw_fatal_error, const String& error_msg);
void HHVM_FUNCTION(hphp_clear_unflushed);
bool HHVM_FUNCTION(trigger_error, const String& error_msg,
                                  int error_type = (int)ErrorMode::USER_NOTICE);
bool HHVM_FUNCTION(trigger_sampled_error, const String& error_msg,
                   int sample_rate,
                   int error_type = (int)ErrorMode::USER_NOTICE);
bool HHVM_FUNCTION(user_error, const String& error_msg,
                               int error_type = (int)ErrorMode::USER_NOTICE);

ArrayData* debug_backtrace_jit(int64_t options);
String debug_string_backtrace(bool skip, bool ignore_args = false,
                              int64_t limit = 0);
String stringify_backtrace(const Array& bt, bool ignore_args);

///////////////////////////////////////////////////////////////////////////////
}

