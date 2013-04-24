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
void HPHP::f_asio_enter_context()
_ZN4HPHP20f_asio_enter_contextEv

*/

void fh_asio_enter_context() asm("_ZN4HPHP20f_asio_enter_contextEv");

TypedValue* fg_asio_enter_context(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_data.num = 0LL;
      rv.m_type = KindOfNull;
      fh_asio_enter_context();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("asio_enter_context", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_asio_exit_context()
_ZN4HPHP19f_asio_exit_contextEv

*/

void fh_asio_exit_context() asm("_ZN4HPHP19f_asio_exit_contextEv");

TypedValue* fg_asio_exit_context(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_data.num = 0LL;
      rv.m_type = KindOfNull;
      fh_asio_exit_context();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("asio_exit_context", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
int HPHP::f_asio_get_current_context_idx()
_ZN4HPHP30f_asio_get_current_context_idxEv

(return value) => rax
*/

int fh_asio_get_current_context_idx() asm("_ZN4HPHP30f_asio_get_current_context_idxEv");

TypedValue* fg_asio_get_current_context_idx(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_type = KindOfInt64;
      rv.m_data.num = (int64_t)fh_asio_get_current_context_idx();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("asio_get_current_context_idx", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Object HPHP::f_asio_get_running_in_context(int)
_ZN4HPHP29f_asio_get_running_in_contextEi

(return value) => rax
_rv => rdi
ctx_idx => rsi
*/

Value* fh_asio_get_running_in_context(Value* _rv, int ctx_idx) asm("_ZN4HPHP29f_asio_get_running_in_contextEi");

TypedValue * fg1_asio_get_running_in_context(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_asio_get_running_in_context(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  tvCastToInt64InPlace(args-0);
  fh_asio_get_running_in_context((&rv->m_data), (int)(args[-0].m_data.num));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_asio_get_running_in_context(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfInt64) {
        rv.m_type = KindOfObject;
        fh_asio_get_running_in_context((&rv.m_data), (int)(args[-0].m_data.num));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_asio_get_running_in_context(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("asio_get_running_in_context", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Object HPHP::f_asio_get_running()
_ZN4HPHP18f_asio_get_runningEv

(return value) => rax
_rv => rdi
*/

Value* fh_asio_get_running(Value* _rv) asm("_ZN4HPHP18f_asio_get_runningEv");

TypedValue* fg_asio_get_running(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_type = KindOfObject;
      fh_asio_get_running((&rv.m_data));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("asio_get_running", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Object HPHP::f_asio_get_current()
_ZN4HPHP18f_asio_get_currentEv

(return value) => rax
_rv => rdi
*/

Value* fh_asio_get_current(Value* _rv) asm("_ZN4HPHP18f_asio_get_currentEv");

TypedValue* fg_asio_get_current(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_type = KindOfObject;
      fh_asio_get_current((&rv.m_data));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("asio_get_current", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_asio_set_on_failed_callback(HPHP::Variant const&)
_ZN4HPHP29f_asio_set_on_failed_callbackERKNS_7VariantE

on_failed_cb => rdi
*/

void fh_asio_set_on_failed_callback(TypedValue* on_failed_cb) asm("_ZN4HPHP29f_asio_set_on_failed_callbackERKNS_7VariantE");

TypedValue* fg_asio_set_on_failed_callback(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv.m_data.num = 0LL;
      rv.m_type = KindOfNull;
      fh_asio_set_on_failed_callback((args-0));
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("asio_set_on_failed_callback", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_asio_set_on_started_callback(HPHP::Variant const&)
_ZN4HPHP30f_asio_set_on_started_callbackERKNS_7VariantE

on_started_cb => rdi
*/

void fh_asio_set_on_started_callback(TypedValue* on_started_cb) asm("_ZN4HPHP30f_asio_set_on_started_callbackERKNS_7VariantE");

TypedValue* fg_asio_set_on_started_callback(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv.m_data.num = 0LL;
      rv.m_type = KindOfNull;
      fh_asio_set_on_started_callback((args-0));
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("asio_set_on_started_callback", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}




} // !HPHP

