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
HPHP::Array HPHP::f_debug_backtrace(bool)
_ZN4HPHP17f_debug_backtraceEb

(return value) => rax
_rv => rdi
provide_object => rsi
*/

Value* fh_debug_backtrace(Value* _rv, bool provide_object) asm("_ZN4HPHP17f_debug_backtraceEb");

TypedValue * fg1_debug_backtrace(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_debug_backtrace(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfArray;
  tvCastToBooleanInPlace(args-0);
  fh_debug_backtrace((Value*)(rv), (count > 0) ? (bool)(args[-0].m_data.num) : (bool)(true));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_debug_backtrace(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      if ((count <= 0 || (args-0)->m_type == KindOfBoolean)) {
        rv._count = 0;
        rv.m_type = KindOfArray;
        fh_debug_backtrace((Value*)(&(rv)), (count > 0) ? (bool)(args[-0].m_data.num) : (bool)(true));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_debug_backtrace(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("debug_backtrace", 1, 1);
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
void HPHP::f_debug_print_backtrace()
_ZN4HPHP23f_debug_print_backtraceEv

*/

void fh_debug_print_backtrace() asm("_ZN4HPHP23f_debug_print_backtraceEv");

TypedValue* fg_debug_print_backtrace(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_data.num = 0LL;
      rv._count = 0;
      rv.m_type = KindOfNull;
      fh_debug_print_backtrace();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("debug_print_backtrace", 0, 1);
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
HPHP::Array HPHP::f_error_get_last()
_ZN4HPHP16f_error_get_lastEv

(return value) => rax
_rv => rdi
*/

Value* fh_error_get_last(Value* _rv) asm("_ZN4HPHP16f_error_get_lastEv");

TypedValue* fg_error_get_last(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfArray;
      fh_error_get_last((Value*)(&(rv)));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("error_get_last", 0, 1);
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
bool HPHP::f_error_log(HPHP::String const&, int, HPHP::String const&, HPHP::String const&)
_ZN4HPHP11f_error_logERKNS_6StringEiS2_S2_

(return value) => rax
message => rdi
message_type => rsi
destination => rdx
extra_headers => rcx
*/

bool fh_error_log(Value* message, int message_type, Value* destination, Value* extra_headers) asm("_ZN4HPHP11f_error_logERKNS_6StringEiS2_S2_");

TypedValue * fg1_error_log(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_error_log(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 4
    if (!IS_STRING_TYPE((args-3)->m_type)) {
      tvCastToStringInPlace(args-3);
    }
  case 3:
    if (!IS_STRING_TYPE((args-2)->m_type)) {
      tvCastToStringInPlace(args-2);
    }
  case 2:
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (fh_error_log((Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (Value*)(args-2) : (Value*)(&null_string), (count > 3) ? (Value*)(args-3) : (Value*)(&null_string))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_error_log(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 4LL) {
      if ((count <= 3 || IS_STRING_TYPE((args-3)->m_type)) && (count <= 2 || IS_STRING_TYPE((args-2)->m_type)) && (count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_error_log((Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (Value*)(args-2) : (Value*)(&null_string), (count > 3) ? (Value*)(args-3) : (Value*)(&null_string))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_error_log(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("error_log", count, 1, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_error_reporting(HPHP::Variant const&)
_ZN4HPHP17f_error_reportingERKNS_7VariantE

(return value) => rax
level => rdi
*/

long fh_error_reporting(TypedValue* level) asm("_ZN4HPHP17f_error_reportingERKNS_7VariantE");

TypedValue* fg_error_reporting(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      rv._count = 0;
      rv.m_type = KindOfInt64;
      Variant defVal0;
      rv.m_data.num = (int64_t)fh_error_reporting((count > 0) ? (args-0) : (TypedValue*)(&defVal0));
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("error_reporting", 1, 1);
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
bool HPHP::f_restore_error_handler()
_ZN4HPHP23f_restore_error_handlerEv

(return value) => rax
*/

bool fh_restore_error_handler() asm("_ZN4HPHP23f_restore_error_handlerEv");

TypedValue* fg_restore_error_handler(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfBoolean;
      rv.m_data.num = (fh_restore_error_handler()) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("restore_error_handler", 0, 1);
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
bool HPHP::f_restore_exception_handler()
_ZN4HPHP27f_restore_exception_handlerEv

(return value) => rax
*/

bool fh_restore_exception_handler() asm("_ZN4HPHP27f_restore_exception_handlerEv");

TypedValue* fg_restore_exception_handler(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfBoolean;
      rv.m_data.num = (fh_restore_exception_handler()) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("restore_exception_handler", 0, 1);
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
HPHP::Variant HPHP::f_set_error_handler(HPHP::Variant const&, int)
_ZN4HPHP19f_set_error_handlerERKNS_7VariantEi

(return value) => rax
_rv => rdi
error_handler => rsi
error_types => rdx
*/

TypedValue* fh_set_error_handler(TypedValue* _rv, TypedValue* error_handler, int error_types) asm("_ZN4HPHP19f_set_error_handlerERKNS_7VariantEi");

TypedValue * fg1_set_error_handler(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_set_error_handler(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-1);
  fh_set_error_handler((rv), (args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(k_E_ALL));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_set_error_handler(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64)) {
        fh_set_error_handler((&(rv)), (args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(k_E_ALL));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_set_error_handler(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("set_error_handler", count, 1, 2, 1);
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
HPHP::Variant HPHP::f_set_exception_handler(HPHP::Variant const&)
_ZN4HPHP23f_set_exception_handlerERKNS_7VariantE

(return value) => rax
_rv => rdi
exception_handler => rsi
*/

TypedValue* fh_set_exception_handler(TypedValue* _rv, TypedValue* exception_handler) asm("_ZN4HPHP23f_set_exception_handlerERKNS_7VariantE");

TypedValue* fg_set_exception_handler(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      fh_set_exception_handler((&(rv)), (args-0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("set_exception_handler", count, 1, 1, 1);
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
void HPHP::f_hphp_set_error_page(HPHP::String const&)
_ZN4HPHP21f_hphp_set_error_pageERKNS_6StringE

page => rdi
*/

void fh_hphp_set_error_page(Value* page) asm("_ZN4HPHP21f_hphp_set_error_pageERKNS_6StringE");

TypedValue * fg1_hphp_set_error_page(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_set_error_page(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  tvCastToStringInPlace(args-0);
  fh_hphp_set_error_page((Value*)(args-0));
  return rv;
}

TypedValue* fg_hphp_set_error_page(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        fh_hphp_set_error_page((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_set_error_page(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_set_error_page", count, 1, 1, 1);
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
void HPHP::f_hphp_throw_fatal_error(HPHP::String const&)
_ZN4HPHP24f_hphp_throw_fatal_errorERKNS_6StringE

error_msg => rdi
*/

void fh_hphp_throw_fatal_error(Value* error_msg) asm("_ZN4HPHP24f_hphp_throw_fatal_errorERKNS_6StringE");

TypedValue * fg1_hphp_throw_fatal_error(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_throw_fatal_error(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  tvCastToStringInPlace(args-0);
  fh_hphp_throw_fatal_error((Value*)(args-0));
  return rv;
}

TypedValue* fg_hphp_throw_fatal_error(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        fh_hphp_throw_fatal_error((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_throw_fatal_error(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_throw_fatal_error", count, 1, 1, 1);
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
void HPHP::f_hphp_clear_unflushed()
_ZN4HPHP22f_hphp_clear_unflushedEv

*/

void fh_hphp_clear_unflushed() asm("_ZN4HPHP22f_hphp_clear_unflushedEv");

TypedValue* fg_hphp_clear_unflushed(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_data.num = 0LL;
      rv._count = 0;
      rv.m_type = KindOfNull;
      fh_hphp_clear_unflushed();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("hphp_clear_unflushed", 0, 1);
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
HPHP::Array HPHP::f_hphp_debug_caller_info()
_ZN4HPHP24f_hphp_debug_caller_infoEv

(return value) => rax
_rv => rdi
*/

Value* fh_hphp_debug_caller_info(Value* _rv) asm("_ZN4HPHP24f_hphp_debug_caller_infoEv");

TypedValue* fg_hphp_debug_caller_info(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfArray;
      fh_hphp_debug_caller_info((Value*)(&(rv)));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("hphp_debug_caller_info", 0, 1);
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
bool HPHP::f_trigger_error(HPHP::String const&, int)
_ZN4HPHP15f_trigger_errorERKNS_6StringEi

(return value) => rax
error_msg => rdi
error_type => rsi
*/

bool fh_trigger_error(Value* error_msg, int error_type) asm("_ZN4HPHP15f_trigger_errorERKNS_6StringEi");

TypedValue * fg1_trigger_error(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_trigger_error(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (fh_trigger_error((Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(k_E_USER_NOTICE))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_trigger_error(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_trigger_error((Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(k_E_USER_NOTICE))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_trigger_error(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("trigger_error", count, 1, 2, 1);
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
bool HPHP::f_user_error(HPHP::String const&, int)
_ZN4HPHP12f_user_errorERKNS_6StringEi

(return value) => rax
error_msg => rdi
error_type => rsi
*/

bool fh_user_error(Value* error_msg, int error_type) asm("_ZN4HPHP12f_user_errorERKNS_6StringEi");

TypedValue * fg1_user_error(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_user_error(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (fh_user_error((Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(k_E_USER_NOTICE))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_user_error(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_user_error((Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(k_E_USER_NOTICE))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_user_error(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("user_error", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}




} // !HPHP

