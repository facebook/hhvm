/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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
namespace HPHP {

/*
bool HPHP::f_ob_start(HPHP::Variant const&, int, bool)
_ZN4HPHP10f_ob_startERKNS_7VariantEib

(return value) => rax
output_callback => rdi
chunk_size => rsi
erase => rdx
*/

bool fh_ob_start(TypedValue* output_callback, int chunk_size, bool erase) asm("_ZN4HPHP10f_ob_startERKNS_7VariantEib");

/*
void HPHP::f_ob_clean()
_ZN4HPHP10f_ob_cleanEv

*/

void fh_ob_clean() asm("_ZN4HPHP10f_ob_cleanEv");

/*
void HPHP::f_ob_flush()
_ZN4HPHP10f_ob_flushEv

*/

void fh_ob_flush() asm("_ZN4HPHP10f_ob_flushEv");

/*
bool HPHP::f_ob_end_clean()
_ZN4HPHP14f_ob_end_cleanEv

(return value) => rax
*/

bool fh_ob_end_clean() asm("_ZN4HPHP14f_ob_end_cleanEv");

/*
bool HPHP::f_ob_end_flush()
_ZN4HPHP14f_ob_end_flushEv

(return value) => rax
*/

bool fh_ob_end_flush() asm("_ZN4HPHP14f_ob_end_flushEv");

/*
void HPHP::f_flush()
_ZN4HPHP7f_flushEv

*/

void fh_flush() asm("_ZN4HPHP7f_flushEv");

/*
HPHP::String HPHP::f_ob_get_clean()
_ZN4HPHP14f_ob_get_cleanEv

(return value) => rax
_rv => rdi
*/

Value* fh_ob_get_clean(Value* _rv) asm("_ZN4HPHP14f_ob_get_cleanEv");

/*
HPHP::String HPHP::f_ob_get_contents()
_ZN4HPHP17f_ob_get_contentsEv

(return value) => rax
_rv => rdi
*/

Value* fh_ob_get_contents(Value* _rv) asm("_ZN4HPHP17f_ob_get_contentsEv");

/*
HPHP::String HPHP::f_ob_get_flush()
_ZN4HPHP14f_ob_get_flushEv

(return value) => rax
_rv => rdi
*/

Value* fh_ob_get_flush(Value* _rv) asm("_ZN4HPHP14f_ob_get_flushEv");

/*
long HPHP::f_ob_get_length()
_ZN4HPHP15f_ob_get_lengthEv

(return value) => rax
*/

long fh_ob_get_length() asm("_ZN4HPHP15f_ob_get_lengthEv");

/*
long HPHP::f_ob_get_level()
_ZN4HPHP14f_ob_get_levelEv

(return value) => rax
*/

long fh_ob_get_level() asm("_ZN4HPHP14f_ob_get_levelEv");

/*
HPHP::Array HPHP::f_ob_get_status(bool)
_ZN4HPHP15f_ob_get_statusEb

(return value) => rax
_rv => rdi
full_status => rsi
*/

Value* fh_ob_get_status(Value* _rv, bool full_status) asm("_ZN4HPHP15f_ob_get_statusEb");

/*
HPHP::String HPHP::f_ob_gzhandler(HPHP::String const&, int)
_ZN4HPHP14f_ob_gzhandlerERKNS_6StringEi

(return value) => rax
_rv => rdi
buffer => rsi
mode => rdx
*/

Value* fh_ob_gzhandler(Value* _rv, Value* buffer, int mode) asm("_ZN4HPHP14f_ob_gzhandlerERKNS_6StringEi");

/*
void HPHP::f_ob_implicit_flush(bool)
_ZN4HPHP19f_ob_implicit_flushEb

flag => rdi
*/

void fh_ob_implicit_flush(bool flag) asm("_ZN4HPHP19f_ob_implicit_flushEb");

/*
HPHP::Array HPHP::f_ob_list_handlers()
_ZN4HPHP18f_ob_list_handlersEv

(return value) => rax
_rv => rdi
*/

Value* fh_ob_list_handlers(Value* _rv) asm("_ZN4HPHP18f_ob_list_handlersEv");

/*
bool HPHP::f_output_add_rewrite_var(HPHP::String const&, HPHP::String const&)
_ZN4HPHP24f_output_add_rewrite_varERKNS_6StringES2_

(return value) => rax
name => rdi
value => rsi
*/

bool fh_output_add_rewrite_var(Value* name, Value* value) asm("_ZN4HPHP24f_output_add_rewrite_varERKNS_6StringES2_");

/*
bool HPHP::f_output_reset_rewrite_vars()
_ZN4HPHP27f_output_reset_rewrite_varsEv

(return value) => rax
*/

bool fh_output_reset_rewrite_vars() asm("_ZN4HPHP27f_output_reset_rewrite_varsEv");

/*
void HPHP::f_hphp_crash_log(HPHP::String const&, HPHP::String const&)
_ZN4HPHP16f_hphp_crash_logERKNS_6StringES2_

name => rdi
value => rsi
*/

void fh_hphp_crash_log(Value* name, Value* value) asm("_ZN4HPHP16f_hphp_crash_logERKNS_6StringES2_");

/*
void HPHP::f_hphp_stats(HPHP::String const&, long)
_ZN4HPHP12f_hphp_statsERKNS_6StringEl

name => rdi
value => rsi
*/

void fh_hphp_stats(Value* name, long value) asm("_ZN4HPHP12f_hphp_statsERKNS_6StringEl");

/*
long HPHP::f_hphp_get_stats(HPHP::String const&)
_ZN4HPHP16f_hphp_get_statsERKNS_6StringE

(return value) => rax
name => rdi
*/

long fh_hphp_get_stats(Value* name) asm("_ZN4HPHP16f_hphp_get_statsERKNS_6StringE");

/*
HPHP::Array HPHP::f_hphp_get_status()
_ZN4HPHP17f_hphp_get_statusEv

(return value) => rax
_rv => rdi
*/

Value* fh_hphp_get_status(Value* _rv) asm("_ZN4HPHP17f_hphp_get_statusEv");

/*
HPHP::Array HPHP::f_hphp_get_iostatus()
_ZN4HPHP19f_hphp_get_iostatusEv

(return value) => rax
_rv => rdi
*/

Value* fh_hphp_get_iostatus(Value* _rv) asm("_ZN4HPHP19f_hphp_get_iostatusEv");

/*
void HPHP::f_hphp_set_iostatus_address(HPHP::String const&)
_ZN4HPHP27f_hphp_set_iostatus_addressERKNS_6StringE

name => rdi
*/

void fh_hphp_set_iostatus_address(Value* name) asm("_ZN4HPHP27f_hphp_set_iostatus_addressERKNS_6StringE");

/*
HPHP::Variant HPHP::f_hphp_get_timers(bool)
_ZN4HPHP17f_hphp_get_timersEb

(return value) => rax
_rv => rdi
get_as_float => rsi
*/

TypedValue* fh_hphp_get_timers(TypedValue* _rv, bool get_as_float) asm("_ZN4HPHP17f_hphp_get_timersEb");

/*
HPHP::Variant HPHP::f_hphp_output_global_state(bool)
_ZN4HPHP26f_hphp_output_global_stateEb

(return value) => rax
_rv => rdi
serialize => rsi
*/

TypedValue* fh_hphp_output_global_state(TypedValue* _rv, bool serialize) asm("_ZN4HPHP26f_hphp_output_global_stateEb");

/*
long HPHP::f_hphp_instruction_counter()
_ZN4HPHP26f_hphp_instruction_counterEv

(return value) => rax
*/

long fh_hphp_instruction_counter() asm("_ZN4HPHP26f_hphp_instruction_counterEv");

/*
HPHP::Variant HPHP::f_hphp_get_hardware_counters()
_ZN4HPHP28f_hphp_get_hardware_countersEv

(return value) => rax
_rv => rdi
*/

TypedValue* fh_hphp_get_hardware_counters(TypedValue* _rv) asm("_ZN4HPHP28f_hphp_get_hardware_countersEv");

/*
bool HPHP::f_hphp_set_hardware_events(HPHP::String const&)
_ZN4HPHP26f_hphp_set_hardware_eventsERKNS_6StringE

(return value) => rax
events => rdi
*/

bool fh_hphp_set_hardware_events(Value* events) asm("_ZN4HPHP26f_hphp_set_hardware_eventsERKNS_6StringE");

/*
void HPHP::f_hphp_clear_hardware_events()
_ZN4HPHP28f_hphp_clear_hardware_eventsEv

*/

void fh_hphp_clear_hardware_events() asm("_ZN4HPHP28f_hphp_clear_hardware_eventsEv");


} // !HPHP

