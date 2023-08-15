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

// Run a single step of hphp_debug_caller_info. This function is called on
// each frame in the backtrace, starting at the deepest one. We stop when it
// returns true (i.e. when it has found a non-skip-frame caller.)
bool hphp_debug_caller_info_impl(
    Array& result, bool& skipped, const Func* func, Offset offset);

bool hphp_debug_caller_identifier_impl(
    String& result, bool& skipped, const Func* func);

Array HHVM_FUNCTION(debug_backtrace,
                    int64_t options = k_DEBUG_BACKTRACE_PROVIDE_OBJECT,
                    int64_t limit = 0);
int64_t HHVM_FUNCTION(error_reporting, const Variant& level = uninit_variant);

ArrayData* debug_backtrace_jit(int64_t options);
String debug_string_backtrace(bool skip, bool ignore_args = false,
                              int64_t limit = 0);
String stringify_backtrace(const Array& bt, bool ignore_args);

///////////////////////////////////////////////////////////////////////////////
}
