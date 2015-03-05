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

#ifndef incl_HPHP_EXT_OUTPUT_H_
#define incl_HPHP_EXT_OUTPUT_H_

#include "hphp/runtime/ext/std/ext_std.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

extern const int64_t k_PHP_OUTPUT_HANDLER_CONT;
extern const int64_t k_PHP_OUTPUT_HANDLER_WRITE;
extern const int64_t k_PHP_OUTPUT_HANDLER_START;
extern const int64_t k_PHP_OUTPUT_HANDLER_CLEAN;
extern const int64_t k_PHP_OUTPUT_HANDLER_FLUSH;
extern const int64_t k_PHP_OUTPUT_HANDLER_END;
extern const int64_t k_PHP_OUTPUT_HANDLER_FINAL;
extern const int64_t k_PHP_OUTPUT_HANDLER_CLEANABLE;
extern const int64_t k_PHP_OUTPUT_HANDLER_FLUSHABLE;
extern const int64_t k_PHP_OUTPUT_HANDLER_REMOVABLE;
extern const int64_t k_PHP_OUTPUT_HANDLER_STDFLAGS;

bool HHVM_FUNCTION(ob_start, const Variant& output_callback = uninit_null(),
                             int chunk_size = 0,
                             int flags = k_PHP_OUTPUT_HANDLER_STDFLAGS);
void HHVM_FUNCTION(ob_clean);
void HHVM_FUNCTION(ob_flush);
bool HHVM_FUNCTION(ob_end_clean);
bool HHVM_FUNCTION(ob_end_flush);
void HHVM_FUNCTION(flush);
Variant HHVM_FUNCTION(ob_get_contents);
Variant HHVM_FUNCTION(ob_get_clean);
Variant HHVM_FUNCTION(ob_get_flush);
int64_t HHVM_FUNCTION(ob_get_length);
int64_t HHVM_FUNCTION(ob_get_level);
Array HHVM_FUNCTION(ob_get_status, bool full_status = false);
void HHVM_FUNCTION(ob_implicit_flush, bool flag = true);
Array HHVM_FUNCTION(ob_list_handlers);
bool HHVM_FUNCTION(output_add_rewrite_var, const String& name,
                                           const String& value);
bool HHVM_FUNCTION(output_reset_rewrite_vars);

void HHVM_FUNCTION(hphp_crash_log, const String& name, const String& value);

void HHVM_FUNCTION(hphp_stats, const String& name, int64_t value);
int64_t HHVM_FUNCTION(hphp_get_stats, const String& name);
Array HHVM_FUNCTION(hphp_get_status);
Array HHVM_FUNCTION(hphp_get_iostatus);
void HHVM_FUNCTION(hphp_set_iostatus_address, const String& name);
Variant HHVM_FUNCTION(hphp_get_timers, bool get_as_float = true);
Variant HHVM_FUNCTION(hphp_output_global_state, bool serialize = true);
int64_t HHVM_FUNCTION(hphp_instruction_counter, void);
Variant HHVM_FUNCTION(hphp_get_hardware_counters, void);
bool HHVM_FUNCTION(hphp_set_hardware_events, const Variant& events);
void HHVM_FUNCTION(hphp_clear_hardware_events, void);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_OUTPUT_H_
