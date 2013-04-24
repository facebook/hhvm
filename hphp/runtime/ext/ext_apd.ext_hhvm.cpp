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
bool HPHP::f_override_function(HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP19f_override_functionERKNS_6StringES2_S2_

(return value) => rax
name => rdi
args => rsi
code => rdx
*/

bool fh_override_function(Value* name, Value* args, Value* code) asm("_ZN4HPHP19f_override_functionERKNS_6StringES2_S2_");

TypedValue * fg1_override_function(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_override_function(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (fh_override_function(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data)) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_override_function(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if (IS_STRING_TYPE((args-2)->m_type) && IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_override_function(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data)) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_override_function(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("override_function", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_rename_function(HPHP::String const&, HPHP::String const&)
_ZN4HPHP17f_rename_functionERKNS_6StringES2_

(return value) => rax
orig_name => rdi
new_name => rsi
*/

bool fh_rename_function(Value* orig_name, Value* new_name) asm("_ZN4HPHP17f_rename_functionERKNS_6StringES2_");

TypedValue * fg1_rename_function(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_rename_function(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (fh_rename_function(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_rename_function(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_rename_function(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_rename_function(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("rename_function", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_apd_set_browser_trace()
_ZN4HPHP23f_apd_set_browser_traceEv

*/

void fh_apd_set_browser_trace() asm("_ZN4HPHP23f_apd_set_browser_traceEv");

TypedValue* fg_apd_set_browser_trace(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_data.num = 0LL;
      rv.m_type = KindOfNull;
      fh_apd_set_browser_trace();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("apd_set_browser_trace", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_apd_set_pprof_trace(HPHP::String const&, HPHP::String const&)
_ZN4HPHP21f_apd_set_pprof_traceERKNS_6StringES2_

(return value) => rax
_rv => rdi
dumpdir => rsi
frament => rdx
*/

Value* fh_apd_set_pprof_trace(Value* _rv, Value* dumpdir, Value* frament) asm("_ZN4HPHP21f_apd_set_pprof_traceERKNS_6StringES2_");

TypedValue * fg1_apd_set_pprof_trace(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_apd_set_pprof_trace(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfString;
  switch (count) {
  default: // count >= 2
    if (!IS_STRING_TYPE((args-1)->m_type)) {
      tvCastToStringInPlace(args-1);
    }
  case 1:
    if (!IS_STRING_TYPE((args-0)->m_type)) {
      tvCastToStringInPlace(args-0);
    }
  case 0:
    break;
  }
  fh_apd_set_pprof_trace((&rv->m_data), (count > 0) ? &args[-0].m_data : (Value*)(&null_string), (count > 1) ? &args[-1].m_data : (Value*)(&null_string));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_apd_set_pprof_trace(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 2LL) {
      if ((count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && (count <= 0 || IS_STRING_TYPE((args-0)->m_type))) {
        rv.m_type = KindOfString;
        fh_apd_set_pprof_trace((&rv.m_data), (count > 0) ? &args[-0].m_data : (Value*)(&null_string), (count > 1) ? &args[-1].m_data : (Value*)(&null_string));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_apd_set_pprof_trace(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("apd_set_pprof_trace", 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_apd_set_session_trace_socket(HPHP::String const&, int, int, int)
_ZN4HPHP30f_apd_set_session_trace_socketERKNS_6StringEiii

(return value) => rax
ip_or_filename => rdi
domain => rsi
port => rdx
mask => rcx
*/

bool fh_apd_set_session_trace_socket(Value* ip_or_filename, int domain, int port, int mask) asm("_ZN4HPHP30f_apd_set_session_trace_socketERKNS_6StringEiii");

TypedValue * fg1_apd_set_session_trace_socket(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_apd_set_session_trace_socket(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (fh_apd_set_session_trace_socket(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_apd_set_session_trace_socket(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 4LL) {
      if ((args-3)->m_type == KindOfInt64 && (args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64 && IS_STRING_TYPE((args-0)->m_type)) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_apd_set_session_trace_socket(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_apd_set_session_trace_socket(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("apd_set_session_trace_socket", count, 4, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_apd_stop_trace()
_ZN4HPHP16f_apd_stop_traceEv

*/

void fh_apd_stop_trace() asm("_ZN4HPHP16f_apd_stop_traceEv");

TypedValue* fg_apd_stop_trace(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_data.num = 0LL;
      rv.m_type = KindOfNull;
      fh_apd_stop_trace();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("apd_stop_trace", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_apd_breakpoint()
_ZN4HPHP16f_apd_breakpointEv

(return value) => rax
*/

bool fh_apd_breakpoint() asm("_ZN4HPHP16f_apd_breakpointEv");

TypedValue* fg_apd_breakpoint(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_type = KindOfBoolean;
      rv.m_data.num = (fh_apd_breakpoint()) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("apd_breakpoint", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_apd_continue()
_ZN4HPHP14f_apd_continueEv

(return value) => rax
*/

bool fh_apd_continue() asm("_ZN4HPHP14f_apd_continueEv");

TypedValue* fg_apd_continue(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_type = KindOfBoolean;
      rv.m_data.num = (fh_apd_continue()) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("apd_continue", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_apd_echo(HPHP::String const&)
_ZN4HPHP10f_apd_echoERKNS_6StringE

(return value) => rax
output => rdi
*/

bool fh_apd_echo(Value* output) asm("_ZN4HPHP10f_apd_echoERKNS_6StringE");

TypedValue * fg1_apd_echo(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_apd_echo(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToStringInPlace(args-0);
  rv->m_data.num = (fh_apd_echo(&args[-0].m_data)) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_apd_echo(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_apd_echo(&args[-0].m_data)) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_apd_echo(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("apd_echo", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}




} // !HPHP

