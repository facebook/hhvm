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

#include "runtime/ext_hhvm/ext_hhvm.h"
#include "runtime/base/builtin_functions.h"
#include "runtime/base/array/array_init.h"
#include "runtime/ext/ext.h"
#include "runtime/vm/class.h"
#include "runtime/vm/runtime.h"
#include <exception>

namespace HPHP {

void fh_fb_setprofile(TypedValue* callback) asm("_ZN4HPHP15f_fb_setprofileERKNS_7VariantE");

TypedValue* fg_fb_setprofile(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    rv->m_type = KindOfNull;
    fh_fb_setprofile((args-0));
  } else {
    throw_wrong_arguments_nr("fb_setprofile", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_xhprof_frame_begin(Value* name) asm("_ZN4HPHP20f_xhprof_frame_beginERKNS_6StringE");

void fg1_xhprof_frame_begin(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xhprof_frame_begin(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfNull;
  fh_xhprof_frame_begin(&args[-0].m_data);
}

TypedValue* fg_xhprof_frame_begin(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfNull;
      fh_xhprof_frame_begin(&args[-0].m_data);
    } else {
      fg1_xhprof_frame_begin(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xhprof_frame_begin", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_xhprof_frame_end() asm("_ZN4HPHP18f_xhprof_frame_endEv");

TypedValue* fg_xhprof_frame_end(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfNull;
    fh_xhprof_frame_end();
  } else {
    throw_toomany_arguments_nr("xhprof_frame_end", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_xhprof_enable(int flags, Value* args) asm("_ZN4HPHP15f_xhprof_enableEiRKNS_5ArrayE");

void fg1_xhprof_enable(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xhprof_enable(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfArray) {
      tvCastToArrayInPlace(args-1);
    }
  case 1:
    if ((args-0)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-0);
    }
  case 0:
    break;
  }
  rv->m_type = KindOfNull;
  fh_xhprof_enable((count > 0) ? (int)(args[-0].m_data.num) : (int)(0), (count > 1) ? &args[-1].m_data : (Value*)(&null_array));
}

TypedValue* fg_xhprof_enable(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfArray) &&
        (count <= 0 || (args - 0)->m_type == KindOfInt64)) {
      rv->m_type = KindOfNull;
      fh_xhprof_enable((count > 0) ? (int)(args[-0].m_data.num) : (int)(0), (count > 1) ? &args[-1].m_data : (Value*)(&null_array));
    } else {
      fg1_xhprof_enable(rv, ar, count);
    }
  } else {
    throw_toomany_arguments_nr("xhprof_enable", 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_xhprof_disable(TypedValue* _rv) asm("_ZN4HPHP16f_xhprof_disableEv");

TypedValue* fg_xhprof_disable(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    fh_xhprof_disable(rv);
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("xhprof_disable", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_xhprof_network_enable() asm("_ZN4HPHP23f_xhprof_network_enableEv");

TypedValue* fg_xhprof_network_enable(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfNull;
    fh_xhprof_network_enable();
  } else {
    throw_toomany_arguments_nr("xhprof_network_enable", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_xhprof_network_disable(TypedValue* _rv) asm("_ZN4HPHP24f_xhprof_network_disableEv");

TypedValue* fg_xhprof_network_disable(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    fh_xhprof_network_disable(rv);
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("xhprof_network_disable", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_xhprof_sample_enable() asm("_ZN4HPHP22f_xhprof_sample_enableEv");

TypedValue* fg_xhprof_sample_enable(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfNull;
    fh_xhprof_sample_enable();
  } else {
    throw_toomany_arguments_nr("xhprof_sample_enable", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_xhprof_sample_disable(TypedValue* _rv) asm("_ZN4HPHP23f_xhprof_sample_disableEv");

TypedValue* fg_xhprof_sample_disable(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    fh_xhprof_sample_disable(rv);
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("xhprof_sample_disable", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_xhprof_run_trace(TypedValue* _rv, Value* packedTrace, int flags) asm("_ZN4HPHP18f_xhprof_run_traceERKNS_6StringEi");

void fg1_xhprof_run_trace(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xhprof_run_trace(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_xhprof_run_trace(rv, &args[-0].m_data, (int)(args[-1].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_xhprof_run_trace(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64 &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_xhprof_run_trace(rv, &args[-0].m_data, (int)(args[-1].m_data.num));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_xhprof_run_trace(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xhprof_run_trace", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

} // namespace HPHP
