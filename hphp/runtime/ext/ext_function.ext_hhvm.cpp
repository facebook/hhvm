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

Value* fh_get_defined_functions(Value* _rv) asm("_ZN4HPHP23f_get_defined_functionsEv");

TypedValue* fg_get_defined_functions(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfArray;
    fh_get_defined_functions(&(rv->m_data));
    if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("get_defined_functions", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_function_exists(Value* function_name, bool autoload) asm("_ZN4HPHP17f_function_existsERKNS_6StringEb");

void fg1_function_exists(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_function_exists(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_data.num = (fh_function_exists(&args[-0].m_data, (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true))) ? 1LL : 0LL;
}

TypedValue* fg_function_exists(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfBoolean) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_function_exists(&args[-0].m_data, (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true))) ? 1LL : 0LL;
    } else {
      fg1_function_exists(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("function_exists", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_is_callable(TypedValue* v, bool syntax, TypedValue* name) asm("_ZN4HPHP13f_is_callableERKNS_7VariantEbRKNS_14VRefParamValueE");

void fg1_is_callable(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_is_callable(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToBooleanInPlace(args-1);
  rv->m_type = KindOfBoolean;
  VRefParamValue defVal2 = uninit_null();
  rv->m_data.num = (fh_is_callable((args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false), (count > 2) ? (args-2) : (TypedValue*)(&defVal2))) ? 1LL : 0LL;
}

TypedValue* fg_is_callable(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 3) {
    if ((count <= 1 || (args - 1)->m_type == KindOfBoolean)) {
      rv->m_type = KindOfBoolean;
      VRefParamValue defVal2 = uninit_null();
      rv->m_data.num = (fh_is_callable((args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false), (count > 2) ? (args-2) : (TypedValue*)(&defVal2))) ? 1LL : 0LL;
    } else {
      fg1_is_callable(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("is_callable", count, 1, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_call_user_func(TypedValue* _rv, int64_t _argc, TypedValue* function, Value* _argv) asm("_ZN4HPHP16f_call_user_funcEiRKNS_7VariantERKNS_5ArrayE");

TypedValue* fg_call_user_func(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1) {

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
    fh_call_user_func(rv, count, (args-0), (Value*)(&extraArgs));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_missing_arguments_nr("call_user_func", 1, count, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_call_user_func_array(TypedValue* _rv, TypedValue* function, Value* params) asm("_ZN4HPHP22f_call_user_func_arrayERKNS_7VariantERKNS_5ArrayE");

void fg1_call_user_func_array(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_call_user_func_array(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToArrayInPlace(args-1);
  fh_call_user_func_array(rv, (args-0), &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_call_user_func_array(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfArray) {
      fh_call_user_func_array(rv, (args-0), &args[-1].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_call_user_func_array(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("call_user_func_array", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_call_user_func_array_async(Value* _rv, TypedValue* function, Value* params) asm("_ZN4HPHP28f_call_user_func_array_asyncERKNS_7VariantERKNS_5ArrayE");

void fg1_call_user_func_array_async(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_call_user_func_array_async(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToArrayInPlace(args-1);
  rv->m_type = KindOfObject;
  fh_call_user_func_array_async(&(rv->m_data), (args-0), &args[-1].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_call_user_func_array_async(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfArray) {
      rv->m_type = KindOfObject;
      fh_call_user_func_array_async(&(rv->m_data), (args-0), &args[-1].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_call_user_func_array_async(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("call_user_func_array_async", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_call_user_func_async(Value* _rv, int64_t _argc, TypedValue* function, Value* _argv) asm("_ZN4HPHP22f_call_user_func_asyncEiRKNS_7VariantERKNS_5ArrayE");

TypedValue* fg_call_user_func_async(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1) {
    rv->m_type = KindOfObject;

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
    fh_call_user_func_async(&(rv->m_data), count, (args-0), (Value*)(&extraArgs));
    if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  } else {
    throw_missing_arguments_nr("call_user_func_async", 1, count, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_check_user_func_async(TypedValue* _rv, TypedValue* handles, int timeout) asm("_ZN4HPHP23f_check_user_func_asyncERKNS_7VariantEi");

void fg1_check_user_func_async(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_check_user_func_async(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-1);
  fh_check_user_func_async(rv, (args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(-1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_check_user_func_async(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfInt64)) {
      fh_check_user_func_async(rv, (args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(-1));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_check_user_func_async(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("check_user_func_async", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_end_user_func_async(TypedValue* _rv, Value* handle, int default_strategy, TypedValue* additional_strategies) asm("_ZN4HPHP21f_end_user_func_asyncERKNS_6ObjectEiRKNS_7VariantE");

void fg1_end_user_func_async(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_end_user_func_async(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
  case 2:
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
    break;
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  Variant defVal2;
  fh_end_user_func_async(rv, &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(k_GLOBAL_STATE_IGNORE), (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_end_user_func_async(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 3) {
    if ((count <= 1 || (args - 1)->m_type == KindOfInt64) &&
        (args - 0)->m_type == KindOfObject) {
      Variant defVal2;
      fh_end_user_func_async(rv, &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(k_GLOBAL_STATE_IGNORE), (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_end_user_func_async(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("end_user_func_async", count, 1, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_call_user_func_serialized(Value* _rv, Value* input) asm("_ZN4HPHP27f_call_user_func_serializedERKNS_6StringE");

void fg1_call_user_func_serialized(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_call_user_func_serialized(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfString;
  fh_call_user_func_serialized(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_call_user_func_serialized(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfString;
      fh_call_user_func_serialized(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_call_user_func_serialized(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("call_user_func_serialized", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_call_user_func_array_rpc(TypedValue* _rv, Value* host, int port, Value* auth, int timeout, TypedValue* function, Value* params) asm("_ZN4HPHP26f_call_user_func_array_rpcERKNS_6StringEiS2_iRKNS_7VariantERKNS_5ArrayE");

void fg1_call_user_func_array_rpc(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_call_user_func_array_rpc(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-5)->m_type != KindOfArray) {
    tvCastToArrayInPlace(args-5);
  }
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
  }
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_call_user_func_array_rpc(rv, &args[-0].m_data, (int)(args[-1].m_data.num), &args[-2].m_data, (int)(args[-3].m_data.num), (args-4), &args[-5].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_call_user_func_array_rpc(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 6) {
    if ((args - 5)->m_type == KindOfArray &&
        (args - 3)->m_type == KindOfInt64 &&
        IS_STRING_TYPE((args - 2)->m_type) &&
        (args - 1)->m_type == KindOfInt64 &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_call_user_func_array_rpc(rv, &args[-0].m_data, (int)(args[-1].m_data.num), &args[-2].m_data, (int)(args[-3].m_data.num), (args-4), &args[-5].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_call_user_func_array_rpc(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("call_user_func_array_rpc", count, 6, 6, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 6);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_call_user_func_rpc(TypedValue* _rv, int64_t _argc, Value* host, int port, Value* auth, int timeout, TypedValue* function, Value* _argv) asm("_ZN4HPHP20f_call_user_func_rpcEiRKNS_6StringEiS2_iRKNS_7VariantERKNS_5ArrayE");

void fg1_call_user_func_rpc(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_call_user_func_rpc(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
  }
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }

  Array extraArgs;
  {
    ArrayInit ai(count-5);
    for (int32_t i = 5; i < count; ++i) {
      TypedValue* extraArg = ar->getExtraArg(i-5);
      if (tvIsStronglyBound(extraArg)) {
        ai.setRef(i-5, tvAsVariant(extraArg));
      } else {
        ai.set(i-5, tvAsVariant(extraArg));
      }
    }
    extraArgs = ai.create();
  }
  fh_call_user_func_rpc(rv, count, &args[-0].m_data, (int)(args[-1].m_data.num), &args[-2].m_data, (int)(args[-3].m_data.num), (args-4), (Value*)(&extraArgs));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_call_user_func_rpc(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 5) {
    if ((args - 3)->m_type == KindOfInt64 &&
        IS_STRING_TYPE((args - 2)->m_type) &&
        (args - 1)->m_type == KindOfInt64 &&
        IS_STRING_TYPE((args - 0)->m_type)) {

      Array extraArgs;
      {
        ArrayInit ai(count-5);
        for (int32_t i = 5; i < count; ++i) {
          TypedValue* extraArg = ar->getExtraArg(i-5);
          if (tvIsStronglyBound(extraArg)) {
            ai.setRef(i-5, tvAsVariant(extraArg));
          } else {
            ai.set(i-5, tvAsVariant(extraArg));
          }
        }
        extraArgs = ai.create();
      }
      fh_call_user_func_rpc(rv, count, &args[-0].m_data, (int)(args[-1].m_data.num), &args[-2].m_data, (int)(args[-3].m_data.num), (args-4), (Value*)(&extraArgs));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_call_user_func_rpc(rv, ar, count);
    }
  } else {
    throw_missing_arguments_nr("call_user_func_rpc", 5, count, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_forward_static_call_array(TypedValue* _rv, TypedValue* function, Value* params) asm("_ZN4HPHP27f_forward_static_call_arrayERKNS_7VariantERKNS_5ArrayE");

void fg1_forward_static_call_array(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_forward_static_call_array(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToArrayInPlace(args-1);
  fh_forward_static_call_array(rv, (args-0), &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_forward_static_call_array(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfArray) {
      fh_forward_static_call_array(rv, (args-0), &args[-1].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_forward_static_call_array(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("forward_static_call_array", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_forward_static_call(TypedValue* _rv, int64_t _argc, TypedValue* function, Value* _argv) asm("_ZN4HPHP21f_forward_static_callEiRKNS_7VariantERKNS_5ArrayE");

TypedValue* fg_forward_static_call(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1) {

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
    fh_forward_static_call(rv, count, (args-0), (Value*)(&extraArgs));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_missing_arguments_nr("forward_static_call", 1, count, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_get_called_class(TypedValue* _rv) asm("_ZN4HPHP18f_get_called_classEv");

TypedValue* fg_get_called_class(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    fh_get_called_class(rv);
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("get_called_class", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_create_function(Value* _rv, Value* args, Value* code) asm("_ZN4HPHP17f_create_functionERKNS_6StringES2_");

void fg1_create_function(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_create_function(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfString;
  fh_create_function(&(rv->m_data), &args[-0].m_data, &args[-1].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_create_function(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfString;
      fh_create_function(&(rv->m_data), &args[-0].m_data, &args[-1].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_create_function(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("create_function", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_func_get_arg(TypedValue* _rv, int arg_num) asm("_ZN4HPHP14f_func_get_argEi");

void fg1_func_get_arg(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_func_get_arg(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  fh_func_get_arg(rv, (int)(args[-0].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_func_get_arg(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfInt64) {
      fh_func_get_arg(rv, (int)(args[-0].m_data.num));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_func_get_arg(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("func_get_arg", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_func_get_args(TypedValue* _rv) asm("_ZN4HPHP15f_func_get_argsEv");

TypedValue* fg_func_get_args(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    fh_func_get_args(rv);
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("func_get_args", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_func_num_args() asm("_ZN4HPHP15f_func_num_argsEv");

TypedValue* fg_func_num_args(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfInt64;
    rv->m_data.num = (int64_t)fh_func_num_args();
  } else {
    throw_toomany_arguments_nr("func_num_args", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_register_postsend_function(int64_t _argc, TypedValue* function, Value* _argv) asm("_ZN4HPHP28f_register_postsend_functionEiRKNS_7VariantERKNS_5ArrayE");

TypedValue* fg_register_postsend_function(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1) {
    rv->m_type = KindOfNull;

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
    fh_register_postsend_function(count, (args-0), (Value*)(&extraArgs));
  } else {
    throw_missing_arguments_nr("register_postsend_function", 1, count, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_register_shutdown_function(int64_t _argc, TypedValue* function, Value* _argv) asm("_ZN4HPHP28f_register_shutdown_functionEiRKNS_7VariantERKNS_5ArrayE");

TypedValue* fg_register_shutdown_function(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1) {
    rv->m_type = KindOfNull;

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
    fh_register_shutdown_function(count, (args-0), (Value*)(&extraArgs));
  } else {
    throw_missing_arguments_nr("register_shutdown_function", 1, count, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_register_cleanup_function(int64_t _argc, TypedValue* function, Value* _argv) asm("_ZN4HPHP27f_register_cleanup_functionEiRKNS_7VariantERKNS_5ArrayE");

TypedValue* fg_register_cleanup_function(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1) {
    rv->m_type = KindOfNull;

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
    fh_register_cleanup_function(count, (args-0), (Value*)(&extraArgs));
  } else {
    throw_missing_arguments_nr("register_cleanup_function", 1, count, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_register_tick_function(int64_t _argc, TypedValue* function, Value* _argv) asm("_ZN4HPHP24f_register_tick_functionEiRKNS_7VariantERKNS_5ArrayE");

TypedValue* fg_register_tick_function(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1) {
    rv->m_type = KindOfBoolean;

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
    rv->m_data.num = (fh_register_tick_function(count, (args-0), (Value*)(&extraArgs))) ? 1LL : 0LL;
  } else {
    throw_missing_arguments_nr("register_tick_function", 1, count, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_unregister_tick_function(TypedValue* function_name) asm("_ZN4HPHP26f_unregister_tick_functionERKNS_7VariantE");

TypedValue* fg_unregister_tick_function(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    rv->m_type = KindOfNull;
    fh_unregister_tick_function((args-0));
  } else {
    throw_wrong_arguments_nr("unregister_tick_function", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

} // namespace HPHP
