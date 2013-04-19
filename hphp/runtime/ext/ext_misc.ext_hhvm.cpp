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

long fh_connection_aborted() asm("_ZN4HPHP20f_connection_abortedEv");

TypedValue* fg_connection_aborted(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfInt64;
    rv->m_data.num = (int64_t)fh_connection_aborted();
  } else {
    throw_toomany_arguments_nr("connection_aborted", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_connection_status() asm("_ZN4HPHP19f_connection_statusEv");

TypedValue* fg_connection_status(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfInt64;
    rv->m_data.num = (int64_t)fh_connection_status();
  } else {
    throw_toomany_arguments_nr("connection_status", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_connection_timeout() asm("_ZN4HPHP20f_connection_timeoutEv");

TypedValue* fg_connection_timeout(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfInt64;
    rv->m_data.num = (int64_t)fh_connection_timeout();
  } else {
    throw_toomany_arguments_nr("connection_timeout", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_constant(TypedValue* _rv, Value* name) asm("_ZN4HPHP10f_constantERKNS_6StringE");

void fg1_constant(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_constant(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_constant(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_constant(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_constant(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_constant(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("constant", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_define(Value* name, TypedValue* value, bool case_insensitive) asm("_ZN4HPHP8f_defineERKNS_6StringERKNS_7VariantEb");

void fg1_define(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_define(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-2);
    }
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_define(&args[-0].m_data, (args-1), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false))) ? 1LL : 0LL;
}

TypedValue* fg_define(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfBoolean) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_define(&args[-0].m_data, (args-1), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false))) ? 1LL : 0LL;
    } else {
      fg1_define(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("define", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_defined(Value* name, bool autoload) asm("_ZN4HPHP9f_definedERKNS_6StringEb");

void fg1_defined(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_defined(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_defined(&args[-0].m_data, (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true))) ? 1LL : 0LL;
}

TypedValue* fg_defined(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfBoolean) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_defined(&args[-0].m_data, (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true))) ? 1LL : 0LL;
    } else {
      fg1_defined(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("defined", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_die(TypedValue* _rv, TypedValue* status) asm("_ZN4HPHP5f_dieERKNS_7VariantE");

TypedValue* fg_die(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 1) {
    fh_die(rv, (count > 0) ? (args-0) : (TypedValue*)(&null_variant));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("die", 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_exit(TypedValue* _rv, TypedValue* status) asm("_ZN4HPHP6f_exitERKNS_7VariantE");

TypedValue* fg_exit(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 1) {
    fh_exit(rv, (count > 0) ? (args-0) : (TypedValue*)(&null_variant));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("exit", 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_eval(TypedValue* _rv, Value* code_str) asm("_ZN4HPHP6f_evalERKNS_6StringE");

void fg1_eval(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_eval(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_eval(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_eval(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_eval(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_eval(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("eval", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_get_browser(TypedValue* _rv, Value* user_agent, bool return_array) asm("_ZN4HPHP13f_get_browserERKNS_6StringEb");

void fg1_get_browser(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_get_browser(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-1);
    }
  case 1:
    if (!IS_STRING_TYPE((args-0)->m_type)) {
      tvCastToStringInPlace(args-0);
    }
  case 0:
    break;
  }
  fh_get_browser(rv, (count > 0) ? &args[-0].m_data : (Value*)(&null_string), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_get_browser(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfBoolean) &&
        (count <= 0 || IS_STRING_TYPE((args - 0)->m_type))) {
      fh_get_browser(rv, (count > 0) ? &args[-0].m_data : (Value*)(&null_string), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_get_browser(rv, ar, count);
    }
  } else {
    throw_toomany_arguments_nr("get_browser", 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh___halt_compiler() asm("_ZN4HPHP17f___halt_compilerEv");

TypedValue* fg___halt_compiler(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfNull;
    fh___halt_compiler();
  } else {
    throw_toomany_arguments_nr("__halt_compiler", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_highlight_file(TypedValue* _rv, Value* filename, bool ret) asm("_ZN4HPHP16f_highlight_fileERKNS_6StringEb");

void fg1_highlight_file(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_highlight_file(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_highlight_file(rv, &args[-0].m_data, (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_highlight_file(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfBoolean) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_highlight_file(rv, &args[-0].m_data, (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_highlight_file(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("highlight_file", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_show_source(TypedValue* _rv, Value* filename, bool ret) asm("_ZN4HPHP13f_show_sourceERKNS_6StringEb");

void fg1_show_source(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_show_source(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_show_source(rv, &args[-0].m_data, (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_show_source(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfBoolean) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_show_source(rv, &args[-0].m_data, (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_show_source(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("show_source", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_highlight_string(TypedValue* _rv, Value* str, bool ret) asm("_ZN4HPHP18f_highlight_stringERKNS_6StringEb");

void fg1_highlight_string(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_highlight_string(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_highlight_string(rv, &args[-0].m_data, (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_highlight_string(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfBoolean) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_highlight_string(rv, &args[-0].m_data, (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_highlight_string(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("highlight_string", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_ignore_user_abort(bool setting) asm("_ZN4HPHP19f_ignore_user_abortEb");

void fg1_ignore_user_abort(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_ignore_user_abort(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToBooleanInPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_ignore_user_abort((count > 0) ? (bool)(args[-0].m_data.num) : (bool)(false));
}

TypedValue* fg_ignore_user_abort(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 1) {
    if ((count <= 0 || (args - 0)->m_type == KindOfBoolean)) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_ignore_user_abort((count > 0) ? (bool)(args[-0].m_data.num) : (bool)(false));
    } else {
      fg1_ignore_user_abort(rv, ar, count);
    }
  } else {
    throw_toomany_arguments_nr("ignore_user_abort", 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_pack(TypedValue* _rv, int64_t _argc, Value* format, Value* _argv) asm("_ZN4HPHP6f_packEiRKNS_6StringERKNS_5ArrayE");

void fg1_pack(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pack(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);

  Array extraArgs;
  {
    ArrayInit ai(count-1);
    for (int32_t i = 1; i < count; ++i) {
      TypedValue* extraArg = ar->getExtraArg(i-1);
      if (tvIsStronglyBound(extraArg)) {
        ai.setRef(i-1, tvAsVariant(extraArg));
      } else {
        ai.set(i-1, tvAsVariant(extraArg));
      }
    }
    extraArgs = ai.create();
  }
  fh_pack(rv, count, &args[-0].m_data, (Value*)(&extraArgs));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_pack(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {

      Array extraArgs;
      {
        ArrayInit ai(count-1);
        for (int32_t i = 1; i < count; ++i) {
          TypedValue* extraArg = ar->getExtraArg(i-1);
          if (tvIsStronglyBound(extraArg)) {
            ai.setRef(i-1, tvAsVariant(extraArg));
          } else {
            ai.set(i-1, tvAsVariant(extraArg));
          }
        }
        extraArgs = ai.create();
      }
      fh_pack(rv, count, &args[-0].m_data, (Value*)(&extraArgs));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_pack(rv, ar, count);
    }
  } else {
    throw_missing_arguments_nr("pack", 1, count, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_php_check_syntax(Value* filename, TypedValue* error_message) asm("_ZN4HPHP18f_php_check_syntaxERKNS_6StringERKNS_14VRefParamValueE");

void fg1_php_check_syntax(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_php_check_syntax(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfBoolean;
  VRefParamValue defVal1 = uninit_null();
  rv->m_data.num = (fh_php_check_syntax(&args[-0].m_data, (count > 1) ? (args-1) : (TypedValue*)(&defVal1))) ? 1LL : 0LL;
}

TypedValue* fg_php_check_syntax(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      VRefParamValue defVal1 = uninit_null();
      rv->m_data.num = (fh_php_check_syntax(&args[-0].m_data, (count > 1) ? (args-1) : (TypedValue*)(&defVal1))) ? 1LL : 0LL;
    } else {
      fg1_php_check_syntax(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("php_check_syntax", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_php_strip_whitespace(Value* _rv, Value* filename) asm("_ZN4HPHP22f_php_strip_whitespaceERKNS_6StringE");

void fg1_php_strip_whitespace(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_php_strip_whitespace(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfString;
  fh_php_strip_whitespace(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_php_strip_whitespace(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfString;
      fh_php_strip_whitespace(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_php_strip_whitespace(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("php_strip_whitespace", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_sleep(int seconds) asm("_ZN4HPHP7f_sleepEi");

void fg1_sleep(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_sleep(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_sleep((int)(args[-0].m_data.num));
}

TypedValue* fg_sleep(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfInt64) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_sleep((int)(args[-0].m_data.num));
    } else {
      fg1_sleep(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("sleep", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_usleep(int micro_seconds) asm("_ZN4HPHP8f_usleepEi");

void fg1_usleep(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_usleep(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  rv->m_type = KindOfNull;
  fh_usleep((int)(args[-0].m_data.num));
}

TypedValue* fg_usleep(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfInt64) {
      rv->m_type = KindOfNull;
      fh_usleep((int)(args[-0].m_data.num));
    } else {
      fg1_usleep(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("usleep", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_time_nanosleep(TypedValue* _rv, int seconds, int nanoseconds) asm("_ZN4HPHP16f_time_nanosleepEii");

void fg1_time_nanosleep(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_time_nanosleep(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  fh_time_nanosleep(rv, (int)(args[-0].m_data.num), (int)(args[-1].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_time_nanosleep(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfInt64) {
      fh_time_nanosleep(rv, (int)(args[-0].m_data.num), (int)(args[-1].m_data.num));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_time_nanosleep(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("time_nanosleep", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_time_sleep_until(double timestamp) asm("_ZN4HPHP18f_time_sleep_untilEd");

void fg1_time_sleep_until(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_time_sleep_until(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToDoubleInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_time_sleep_until((args[-0].m_data.dbl))) ? 1LL : 0LL;
}

TypedValue* fg_time_sleep_until(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfDouble) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_time_sleep_until((args[-0].m_data.dbl))) ? 1LL : 0LL;
    } else {
      fg1_time_sleep_until(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("time_sleep_until", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_uniqid(Value* _rv, Value* prefix, bool more_entropy) asm("_ZN4HPHP8f_uniqidERKNS_6StringEb");

void fg1_uniqid(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_uniqid(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-1);
    }
  case 1:
    if (!IS_STRING_TYPE((args-0)->m_type)) {
      tvCastToStringInPlace(args-0);
    }
  case 0:
    break;
  }
  rv->m_type = KindOfString;
  fh_uniqid(&(rv->m_data), (count > 0) ? &args[-0].m_data : (Value*)(&null_string), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_uniqid(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfBoolean) &&
        (count <= 0 || IS_STRING_TYPE((args - 0)->m_type))) {
      rv->m_type = KindOfString;
      fh_uniqid(&(rv->m_data), (count > 0) ? &args[-0].m_data : (Value*)(&null_string), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_uniqid(rv, ar, count);
    }
  } else {
    throw_toomany_arguments_nr("uniqid", 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_unpack(TypedValue* _rv, Value* format, Value* data) asm("_ZN4HPHP8f_unpackERKNS_6StringES2_");

void fg1_unpack(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_unpack(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_unpack(rv, &args[-0].m_data, &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_unpack(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_unpack(rv, &args[-0].m_data, &args[-1].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_unpack(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("unpack", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_sys_getloadavg(Value* _rv) asm("_ZN4HPHP16f_sys_getloadavgEv");

TypedValue* fg_sys_getloadavg(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfArray;
    fh_sys_getloadavg(&(rv->m_data));
    if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("sys_getloadavg", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_token_get_all(Value* _rv, Value* source) asm("_ZN4HPHP15f_token_get_allERKNS_6StringE");

void fg1_token_get_all(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_token_get_all(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfArray;
  fh_token_get_all(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_token_get_all(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfArray;
      fh_token_get_all(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_token_get_all(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("token_get_all", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_token_name(Value* _rv, long token) asm("_ZN4HPHP12f_token_nameEl");

void fg1_token_name(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_token_name(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  rv->m_type = KindOfString;
  fh_token_name(&(rv->m_data), (long)(args[-0].m_data.num));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_token_name(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfInt64) {
      rv->m_type = KindOfString;
      fh_token_name(&(rv->m_data), (long)(args[-0].m_data.num));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_token_name(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("token_name", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_hphp_process_abort(TypedValue* _rv, TypedValue* magic) asm("_ZN4HPHP20f_hphp_process_abortERKNS_7VariantE");

TypedValue* fg_hphp_process_abort(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    fh_hphp_process_abort(rv, (args-0));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_wrong_arguments_nr("hphp_process_abort", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_hphp_to_string(Value* _rv, TypedValue* v) asm("_ZN4HPHP16f_hphp_to_stringERKNS_7VariantE");

TypedValue* fg_hphp_to_string(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    rv->m_type = KindOfString;
    fh_hphp_to_string(&(rv->m_data), (args-0));
    if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  } else {
    throw_wrong_arguments_nr("hphp_to_string", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

} // namespace HPHP
