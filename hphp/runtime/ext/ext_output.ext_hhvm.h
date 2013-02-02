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
void HPHP::f_hphp_crash_log(HPHP::String const&, HPHP::String const&)
_ZN4HPHP16f_hphp_crash_logERKNS_6StringES2_

name => rdi
value => rsi
*/

void fh_hphp_crash_log(Value* name, Value* value) asm("_ZN4HPHP16f_hphp_crash_logERKNS_6StringES2_");

/*
HPHP::Array HPHP::f_hphp_get_status()
_ZN4HPHP17f_hphp_get_statusEv

(return value) => rax
_rv => rdi
*/

Value* fh_hphp_get_status(Value* _rv) asm("_ZN4HPHP17f_hphp_get_statusEv");

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
long long HPHP::f_hphp_instruction_counter()
_ZN4HPHP26f_hphp_instruction_counterEv

(return value) => rax
*/

long long fh_hphp_instruction_counter() asm("_ZN4HPHP26f_hphp_instruction_counterEv");

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

