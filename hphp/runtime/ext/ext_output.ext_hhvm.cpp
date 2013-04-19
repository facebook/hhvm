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

bool fh_ob_start(TypedValue* output_callback, int chunk_size, bool erase) asm("_ZN4HPHP10f_ob_startERKNS_7VariantEib");

void fg1_ob_start(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_ob_start(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-2);
    }
  case 2:
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
  case 0:
    break;
  }
  rv->m_type = KindOfBoolean;
  Variant defVal0;
  rv->m_data.num = (fh_ob_start((count > 0) ? (args-0) : (TypedValue*)(&defVal0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(true))) ? 1LL : 0LL;
}

TypedValue* fg_ob_start(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfBoolean) &&
        (count <= 1 || (args - 1)->m_type == KindOfInt64)) {
      rv->m_type = KindOfBoolean;
      Variant defVal0;
      rv->m_data.num = (fh_ob_start((count > 0) ? (args-0) : (TypedValue*)(&defVal0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(true))) ? 1LL : 0LL;
    } else {
      fg1_ob_start(rv, ar, count);
    }
  } else {
    throw_toomany_arguments_nr("ob_start", 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_ob_clean() asm("_ZN4HPHP10f_ob_cleanEv");

TypedValue* fg_ob_clean(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfNull;
    fh_ob_clean();
  } else {
    throw_toomany_arguments_nr("ob_clean", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_ob_flush() asm("_ZN4HPHP10f_ob_flushEv");

TypedValue* fg_ob_flush(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfNull;
    fh_ob_flush();
  } else {
    throw_toomany_arguments_nr("ob_flush", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_ob_end_clean() asm("_ZN4HPHP14f_ob_end_cleanEv");

TypedValue* fg_ob_end_clean(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfBoolean;
    rv->m_data.num = (fh_ob_end_clean()) ? 1LL : 0LL;
  } else {
    throw_toomany_arguments_nr("ob_end_clean", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_ob_end_flush() asm("_ZN4HPHP14f_ob_end_flushEv");

TypedValue* fg_ob_end_flush(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfBoolean;
    rv->m_data.num = (fh_ob_end_flush()) ? 1LL : 0LL;
  } else {
    throw_toomany_arguments_nr("ob_end_flush", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_flush() asm("_ZN4HPHP7f_flushEv");

TypedValue* fg_flush(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfNull;
    fh_flush();
  } else {
    throw_toomany_arguments_nr("flush", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_ob_get_contents(Value* _rv) asm("_ZN4HPHP17f_ob_get_contentsEv");

TypedValue* fg_ob_get_contents(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfString;
    fh_ob_get_contents(&(rv->m_data));
    if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("ob_get_contents", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_ob_get_clean(Value* _rv) asm("_ZN4HPHP14f_ob_get_cleanEv");

TypedValue* fg_ob_get_clean(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfString;
    fh_ob_get_clean(&(rv->m_data));
    if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("ob_get_clean", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_ob_get_flush(Value* _rv) asm("_ZN4HPHP14f_ob_get_flushEv");

TypedValue* fg_ob_get_flush(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfString;
    fh_ob_get_flush(&(rv->m_data));
    if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("ob_get_flush", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_ob_get_length() asm("_ZN4HPHP15f_ob_get_lengthEv");

TypedValue* fg_ob_get_length(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfInt64;
    rv->m_data.num = (int64_t)fh_ob_get_length();
  } else {
    throw_toomany_arguments_nr("ob_get_length", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_ob_get_level() asm("_ZN4HPHP14f_ob_get_levelEv");

TypedValue* fg_ob_get_level(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfInt64;
    rv->m_data.num = (int64_t)fh_ob_get_level();
  } else {
    throw_toomany_arguments_nr("ob_get_level", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_ob_get_status(Value* _rv, bool full_status) asm("_ZN4HPHP15f_ob_get_statusEb");

void fg1_ob_get_status(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_ob_get_status(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToBooleanInPlace(args-0);
  rv->m_type = KindOfArray;
  fh_ob_get_status(&(rv->m_data), (count > 0) ? (bool)(args[-0].m_data.num) : (bool)(false));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_ob_get_status(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 1) {
    if ((count <= 0 || (args - 0)->m_type == KindOfBoolean)) {
      rv->m_type = KindOfArray;
      fh_ob_get_status(&(rv->m_data), (count > 0) ? (bool)(args[-0].m_data.num) : (bool)(false));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_ob_get_status(rv, ar, count);
    }
  } else {
    throw_toomany_arguments_nr("ob_get_status", 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_ob_gzhandler(Value* _rv, Value* buffer, int mode) asm("_ZN4HPHP14f_ob_gzhandlerERKNS_6StringEi");

void fg1_ob_gzhandler(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_ob_gzhandler(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfString;
  fh_ob_gzhandler(&(rv->m_data), &args[-0].m_data, (int)(args[-1].m_data.num));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_ob_gzhandler(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64 &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfString;
      fh_ob_gzhandler(&(rv->m_data), &args[-0].m_data, (int)(args[-1].m_data.num));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_ob_gzhandler(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("ob_gzhandler", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_ob_implicit_flush(bool flag) asm("_ZN4HPHP19f_ob_implicit_flushEb");

void fg1_ob_implicit_flush(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_ob_implicit_flush(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToBooleanInPlace(args-0);
  rv->m_type = KindOfNull;
  fh_ob_implicit_flush((count > 0) ? (bool)(args[-0].m_data.num) : (bool)(true));
}

TypedValue* fg_ob_implicit_flush(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 1) {
    if ((count <= 0 || (args - 0)->m_type == KindOfBoolean)) {
      rv->m_type = KindOfNull;
      fh_ob_implicit_flush((count > 0) ? (bool)(args[-0].m_data.num) : (bool)(true));
    } else {
      fg1_ob_implicit_flush(rv, ar, count);
    }
  } else {
    throw_toomany_arguments_nr("ob_implicit_flush", 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_ob_list_handlers(Value* _rv) asm("_ZN4HPHP18f_ob_list_handlersEv");

TypedValue* fg_ob_list_handlers(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfArray;
    fh_ob_list_handlers(&(rv->m_data));
    if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("ob_list_handlers", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_output_add_rewrite_var(Value* name, Value* value) asm("_ZN4HPHP24f_output_add_rewrite_varERKNS_6StringES2_");

void fg1_output_add_rewrite_var(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_output_add_rewrite_var(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_output_add_rewrite_var(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_output_add_rewrite_var(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_output_add_rewrite_var(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_output_add_rewrite_var(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("output_add_rewrite_var", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_output_reset_rewrite_vars() asm("_ZN4HPHP27f_output_reset_rewrite_varsEv");

TypedValue* fg_output_reset_rewrite_vars(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfBoolean;
    rv->m_data.num = (fh_output_reset_rewrite_vars()) ? 1LL : 0LL;
  } else {
    throw_toomany_arguments_nr("output_reset_rewrite_vars", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_hphp_crash_log(Value* name, Value* value) asm("_ZN4HPHP16f_hphp_crash_logERKNS_6StringES2_");

void fg1_hphp_crash_log(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_crash_log(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_hphp_crash_log(&args[-0].m_data, &args[-1].m_data);
}

TypedValue* fg_hphp_crash_log(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfNull;
      fh_hphp_crash_log(&args[-0].m_data, &args[-1].m_data);
    } else {
      fg1_hphp_crash_log(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_crash_log", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_hphp_stats(Value* name, long value) asm("_ZN4HPHP12f_hphp_statsERKNS_6StringEl");

void fg1_hphp_stats(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_stats(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_hphp_stats(&args[-0].m_data, (long)(args[-1].m_data.num));
}

TypedValue* fg_hphp_stats(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64 &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfNull;
      fh_hphp_stats(&args[-0].m_data, (long)(args[-1].m_data.num));
    } else {
      fg1_hphp_stats(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_stats", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_hphp_get_stats(Value* name) asm("_ZN4HPHP16f_hphp_get_statsERKNS_6StringE");

void fg1_hphp_get_stats(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_get_stats(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_hphp_get_stats(&args[-0].m_data);
}

TypedValue* fg_hphp_get_stats(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_hphp_get_stats(&args[-0].m_data);
    } else {
      fg1_hphp_get_stats(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_get_stats", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_hphp_get_status(Value* _rv) asm("_ZN4HPHP17f_hphp_get_statusEv");

TypedValue* fg_hphp_get_status(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfArray;
    fh_hphp_get_status(&(rv->m_data));
    if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("hphp_get_status", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_hphp_get_iostatus(Value* _rv) asm("_ZN4HPHP19f_hphp_get_iostatusEv");

TypedValue* fg_hphp_get_iostatus(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfArray;
    fh_hphp_get_iostatus(&(rv->m_data));
    if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("hphp_get_iostatus", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_hphp_set_iostatus_address(Value* name) asm("_ZN4HPHP27f_hphp_set_iostatus_addressERKNS_6StringE");

void fg1_hphp_set_iostatus_address(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_set_iostatus_address(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfNull;
  fh_hphp_set_iostatus_address(&args[-0].m_data);
}

TypedValue* fg_hphp_set_iostatus_address(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfNull;
      fh_hphp_set_iostatus_address(&args[-0].m_data);
    } else {
      fg1_hphp_set_iostatus_address(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_set_iostatus_address", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_hphp_get_timers(TypedValue* _rv, bool get_as_float) asm("_ZN4HPHP17f_hphp_get_timersEb");

void fg1_hphp_get_timers(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_get_timers(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToBooleanInPlace(args-0);
  fh_hphp_get_timers(rv, (count > 0) ? (bool)(args[-0].m_data.num) : (bool)(true));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_hphp_get_timers(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 1) {
    if ((count <= 0 || (args - 0)->m_type == KindOfBoolean)) {
      fh_hphp_get_timers(rv, (count > 0) ? (bool)(args[-0].m_data.num) : (bool)(true));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_hphp_get_timers(rv, ar, count);
    }
  } else {
    throw_toomany_arguments_nr("hphp_get_timers", 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_hphp_output_global_state(TypedValue* _rv, bool serialize) asm("_ZN4HPHP26f_hphp_output_global_stateEb");

void fg1_hphp_output_global_state(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_output_global_state(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToBooleanInPlace(args-0);
  fh_hphp_output_global_state(rv, (count > 0) ? (bool)(args[-0].m_data.num) : (bool)(true));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_hphp_output_global_state(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 1) {
    if ((count <= 0 || (args - 0)->m_type == KindOfBoolean)) {
      fh_hphp_output_global_state(rv, (count > 0) ? (bool)(args[-0].m_data.num) : (bool)(true));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_hphp_output_global_state(rv, ar, count);
    }
  } else {
    throw_toomany_arguments_nr("hphp_output_global_state", 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_hphp_instruction_counter() asm("_ZN4HPHP26f_hphp_instruction_counterEv");

TypedValue* fg_hphp_instruction_counter(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfInt64;
    rv->m_data.num = (int64_t)fh_hphp_instruction_counter();
  } else {
    throw_toomany_arguments_nr("hphp_instruction_counter", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_hphp_get_hardware_counters(TypedValue* _rv) asm("_ZN4HPHP28f_hphp_get_hardware_countersEv");

TypedValue* fg_hphp_get_hardware_counters(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    fh_hphp_get_hardware_counters(rv);
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("hphp_get_hardware_counters", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_hphp_set_hardware_events(Value* events) asm("_ZN4HPHP26f_hphp_set_hardware_eventsERKNS_6StringE");

void fg1_hphp_set_hardware_events(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_set_hardware_events(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfBoolean;
  String defVal0 = uninit_null();
  rv->m_data.num = (fh_hphp_set_hardware_events((count > 0) ? &args[-0].m_data : (Value*)(&defVal0))) ? 1LL : 0LL;
}

TypedValue* fg_hphp_set_hardware_events(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 1) {
    if ((count <= 0 || IS_STRING_TYPE((args - 0)->m_type))) {
      rv->m_type = KindOfBoolean;
      String defVal0 = uninit_null();
      rv->m_data.num = (fh_hphp_set_hardware_events((count > 0) ? &args[-0].m_data : (Value*)(&defVal0))) ? 1LL : 0LL;
    } else {
      fg1_hphp_set_hardware_events(rv, ar, count);
    }
  } else {
    throw_toomany_arguments_nr("hphp_set_hardware_events", 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_hphp_clear_hardware_events() asm("_ZN4HPHP28f_hphp_clear_hardware_eventsEv");

TypedValue* fg_hphp_clear_hardware_events(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfNull;
    fh_hphp_clear_hardware_events();
  } else {
    throw_toomany_arguments_nr("hphp_clear_hardware_events", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

} // namespace HPHP
