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
#include <runtime/ext_hhvm/ext_hhvm.h>
#include <runtime/base/builtin_functions.h>
#include <runtime/base/array/array_init.h>
#include <runtime/ext/ext.h>
#include <runtime/vm/class.h>
#include <runtime/vm/runtime.h>
#include <exception>

namespace HPHP {

/*
void HPHP::f_hphp_crash_log(HPHP::String const&, HPHP::String const&)
_ZN4HPHP16f_hphp_crash_logERKNS_6StringES2_

name => rdi
value => rsi
*/

void fh_hphp_crash_log(Value* name, Value* value) asm("_ZN4HPHP16f_hphp_crash_logERKNS_6StringES2_");

TypedValue * fg1_hphp_crash_log(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_crash_log(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_hphp_crash_log((Value*)(args-0), (Value*)(args-1));
  return rv;
}

TypedValue* fg_hphp_crash_log(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        fh_hphp_crash_log((Value*)(args-0), (Value*)(args-1));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_crash_log(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_crash_log", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_hphp_get_status()
_ZN4HPHP17f_hphp_get_statusEv

(return value) => rax
_rv => rdi
*/

Value* fh_hphp_get_status(Value* _rv) asm("_ZN4HPHP17f_hphp_get_statusEv");

TypedValue* fg_hphp_get_status(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfArray;
      fh_hphp_get_status((Value*)(&(rv)));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("hphp_get_status", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_hphp_get_timers(bool)
_ZN4HPHP17f_hphp_get_timersEb

(return value) => rax
_rv => rdi
get_as_float => rsi
*/

TypedValue* fh_hphp_get_timers(TypedValue* _rv, bool get_as_float) asm("_ZN4HPHP17f_hphp_get_timersEb");

TypedValue * fg1_hphp_get_timers(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_get_timers(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToBooleanInPlace(args-0);
  fh_hphp_get_timers((rv), (count > 0) ? (bool)(args[-0].m_data.num) : (bool)(true));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_hphp_get_timers(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      if ((count <= 0 || (args-0)->m_type == KindOfBoolean)) {
        fh_hphp_get_timers((&(rv)), (count > 0) ? (bool)(args[-0].m_data.num) : (bool)(true));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_get_timers(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("hphp_get_timers", 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_hphp_output_global_state(bool)
_ZN4HPHP26f_hphp_output_global_stateEb

(return value) => rax
_rv => rdi
serialize => rsi
*/

TypedValue* fh_hphp_output_global_state(TypedValue* _rv, bool serialize) asm("_ZN4HPHP26f_hphp_output_global_stateEb");

TypedValue * fg1_hphp_output_global_state(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_output_global_state(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToBooleanInPlace(args-0);
  fh_hphp_output_global_state((rv), (count > 0) ? (bool)(args[-0].m_data.num) : (bool)(true));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_hphp_output_global_state(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      if ((count <= 0 || (args-0)->m_type == KindOfBoolean)) {
        fh_hphp_output_global_state((&(rv)), (count > 0) ? (bool)(args[-0].m_data.num) : (bool)(true));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_output_global_state(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("hphp_output_global_state", 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long long HPHP::f_hphp_instruction_counter()
_ZN4HPHP26f_hphp_instruction_counterEv

(return value) => rax
*/

long long fh_hphp_instruction_counter() asm("_ZN4HPHP26f_hphp_instruction_counterEv");

TypedValue* fg_hphp_instruction_counter(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfInt64;
      rv.m_data.num = (long long)fh_hphp_instruction_counter();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("hphp_instruction_counter", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_hphp_get_hardware_counters()
_ZN4HPHP28f_hphp_get_hardware_countersEv

(return value) => rax
_rv => rdi
*/

TypedValue* fh_hphp_get_hardware_counters(TypedValue* _rv) asm("_ZN4HPHP28f_hphp_get_hardware_countersEv");

TypedValue* fg_hphp_get_hardware_counters(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      fh_hphp_get_hardware_counters((&(rv)));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("hphp_get_hardware_counters", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_hphp_set_hardware_events(HPHP::String const&)
_ZN4HPHP26f_hphp_set_hardware_eventsERKNS_6StringE

(return value) => rax
events => rdi
*/

bool fh_hphp_set_hardware_events(Value* events) asm("_ZN4HPHP26f_hphp_set_hardware_eventsERKNS_6StringE");

TypedValue * fg1_hphp_set_hardware_events(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_set_hardware_events(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToStringInPlace(args-0);
  String defVal0 = null;
  rv->m_data.num = (fh_hphp_set_hardware_events((count > 0) ? (Value*)(args-0) : (Value*)(&defVal0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_hphp_set_hardware_events(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      if ((count <= 0 || IS_STRING_TYPE((args-0)->m_type))) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        String defVal0 = null;
        rv.m_data.num = (fh_hphp_set_hardware_events((count > 0) ? (Value*)(args-0) : (Value*)(&defVal0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_set_hardware_events(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("hphp_set_hardware_events", 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_hphp_clear_hardware_events()
_ZN4HPHP28f_hphp_clear_hardware_eventsEv

*/

void fh_hphp_clear_hardware_events() asm("_ZN4HPHP28f_hphp_clear_hardware_eventsEv");

TypedValue* fg_hphp_clear_hardware_events(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_data.num = 0LL;
      rv._count = 0;
      rv.m_type = KindOfNull;
      fh_hphp_clear_hardware_events();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("hphp_clear_hardware_events", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}




} // !HPHP

