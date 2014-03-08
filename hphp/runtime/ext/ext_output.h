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

#include "hphp/runtime/base/base-includes.h"
#include "hphp/runtime/server/server-stats.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

bool f_ob_start(const Variant& output_callback = uninit_null(), int chunk_size = 0,
                bool erase = true);
void f_ob_clean();
void f_ob_flush();
bool f_ob_end_clean();
bool f_ob_end_flush();
void f_flush();
Variant f_ob_get_contents();
Variant f_ob_get_clean();
Variant f_ob_get_flush();
int64_t f_ob_get_length();
int64_t f_ob_get_level();
Array f_ob_get_status(bool full_status = false);
void f_ob_implicit_flush(bool flag = true);
Array f_ob_list_handlers();
bool f_output_add_rewrite_var(const String& name, const String& value);
bool f_output_reset_rewrite_vars();

void f_hphp_crash_log(const String& name, const String& value);

void f_hphp_stats(const String& name, int64_t value);
int64_t f_hphp_get_stats(const String& name);
Array f_hphp_get_status();
Array f_hphp_get_iostatus();
void f_hphp_set_iostatus_address(const String& name);
Variant f_hphp_get_timers(bool get_as_float = true);
Variant f_hphp_output_global_state(bool serialize = true);
int64_t f_hphp_instruction_counter(void);
Variant f_hphp_get_hardware_counters(void);
bool f_hphp_set_hardware_events(const String& events);
void f_hphp_clear_hardware_events(void);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_OUTPUT_H_
