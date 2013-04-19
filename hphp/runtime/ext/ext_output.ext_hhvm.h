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

bool fh_ob_start(TypedValue* output_callback, int chunk_size, bool erase) asm("_ZN4HPHP10f_ob_startERKNS_7VariantEib");

void fh_ob_clean() asm("_ZN4HPHP10f_ob_cleanEv");

void fh_ob_flush() asm("_ZN4HPHP10f_ob_flushEv");

bool fh_ob_end_clean() asm("_ZN4HPHP14f_ob_end_cleanEv");

bool fh_ob_end_flush() asm("_ZN4HPHP14f_ob_end_flushEv");

void fh_flush() asm("_ZN4HPHP7f_flushEv");

Value* fh_ob_get_contents(Value* _rv) asm("_ZN4HPHP17f_ob_get_contentsEv");

Value* fh_ob_get_clean(Value* _rv) asm("_ZN4HPHP14f_ob_get_cleanEv");

Value* fh_ob_get_flush(Value* _rv) asm("_ZN4HPHP14f_ob_get_flushEv");

long fh_ob_get_length() asm("_ZN4HPHP15f_ob_get_lengthEv");

long fh_ob_get_level() asm("_ZN4HPHP14f_ob_get_levelEv");

Value* fh_ob_get_status(Value* _rv, bool full_status) asm("_ZN4HPHP15f_ob_get_statusEb");

Value* fh_ob_gzhandler(Value* _rv, Value* buffer, int mode) asm("_ZN4HPHP14f_ob_gzhandlerERKNS_6StringEi");

void fh_ob_implicit_flush(bool flag) asm("_ZN4HPHP19f_ob_implicit_flushEb");

Value* fh_ob_list_handlers(Value* _rv) asm("_ZN4HPHP18f_ob_list_handlersEv");

bool fh_output_add_rewrite_var(Value* name, Value* value) asm("_ZN4HPHP24f_output_add_rewrite_varERKNS_6StringES2_");

bool fh_output_reset_rewrite_vars() asm("_ZN4HPHP27f_output_reset_rewrite_varsEv");

void fh_hphp_crash_log(Value* name, Value* value) asm("_ZN4HPHP16f_hphp_crash_logERKNS_6StringES2_");

void fh_hphp_stats(Value* name, long value) asm("_ZN4HPHP12f_hphp_statsERKNS_6StringEl");

long fh_hphp_get_stats(Value* name) asm("_ZN4HPHP16f_hphp_get_statsERKNS_6StringE");

Value* fh_hphp_get_status(Value* _rv) asm("_ZN4HPHP17f_hphp_get_statusEv");

Value* fh_hphp_get_iostatus(Value* _rv) asm("_ZN4HPHP19f_hphp_get_iostatusEv");

void fh_hphp_set_iostatus_address(Value* name) asm("_ZN4HPHP27f_hphp_set_iostatus_addressERKNS_6StringE");

TypedValue* fh_hphp_get_timers(TypedValue* _rv, bool get_as_float) asm("_ZN4HPHP17f_hphp_get_timersEb");

TypedValue* fh_hphp_output_global_state(TypedValue* _rv, bool serialize) asm("_ZN4HPHP26f_hphp_output_global_stateEb");

long fh_hphp_instruction_counter() asm("_ZN4HPHP26f_hphp_instruction_counterEv");

TypedValue* fh_hphp_get_hardware_counters(TypedValue* _rv) asm("_ZN4HPHP28f_hphp_get_hardware_countersEv");

bool fh_hphp_set_hardware_events(Value* events) asm("_ZN4HPHP26f_hphp_set_hardware_eventsERKNS_6StringE");

void fh_hphp_clear_hardware_events() asm("_ZN4HPHP28f_hphp_clear_hardware_eventsEv");

} // namespace HPHP
