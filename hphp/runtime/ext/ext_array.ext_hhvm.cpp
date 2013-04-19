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

TypedValue* fh_array_change_key_case(TypedValue* _rv, TypedValue* input, bool upper) asm("_ZN4HPHP23f_array_change_key_caseERKNS_7VariantEb");

void fg1_array_change_key_case(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_array_change_key_case(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToBooleanInPlace(args-1);
  fh_array_change_key_case(rv, (args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_array_change_key_case(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfBoolean)) {
      fh_array_change_key_case(rv, (args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_array_change_key_case(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("array_change_key_case", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_array_chunk(TypedValue* _rv, TypedValue* input, int size, bool preserve_keys) asm("_ZN4HPHP13f_array_chunkERKNS_7VariantEib");

void fg1_array_chunk(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_array_chunk(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-2);
    }
  case 2:
    break;
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  fh_array_chunk(rv, (args-0), (int)(args[-1].m_data.num), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_array_chunk(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfBoolean) &&
        (args - 1)->m_type == KindOfInt64) {
      fh_array_chunk(rv, (args-0), (int)(args[-1].m_data.num), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_array_chunk(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("array_chunk", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_array_column(TypedValue* _rv, TypedValue* arr, TypedValue* val_key, TypedValue* idx_key) asm("_ZN4HPHP14f_array_columnERKNS_7VariantES2_S2_");

TypedValue* fg_array_column(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    fh_array_column(rv, (args-0), (args-1), (count > 2) ? (args-2) : (TypedValue*)(&null_variant));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_wrong_arguments_nr("array_column", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_array_combine(TypedValue* _rv, TypedValue* keys, TypedValue* values) asm("_ZN4HPHP15f_array_combineERKNS_7VariantES2_");

TypedValue* fg_array_combine(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    fh_array_combine(rv, (args-0), (args-1));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_wrong_arguments_nr("array_combine", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_array_count_values(TypedValue* _rv, TypedValue* input) asm("_ZN4HPHP20f_array_count_valuesERKNS_7VariantE");

TypedValue* fg_array_count_values(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    fh_array_count_values(rv, (args-0));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_wrong_arguments_nr("array_count_values", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_array_fill_keys(TypedValue* _rv, TypedValue* keys, TypedValue* value) asm("_ZN4HPHP17f_array_fill_keysERKNS_7VariantES2_");

TypedValue* fg_array_fill_keys(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    fh_array_fill_keys(rv, (args-0), (args-1));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_wrong_arguments_nr("array_fill_keys", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_array_fill(TypedValue* _rv, int start_index, int num, TypedValue* value) asm("_ZN4HPHP12f_array_fillEiiRKNS_7VariantE");

void fg1_array_fill(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_array_fill(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  fh_array_fill(rv, (int)(args[-0].m_data.num), (int)(args[-1].m_data.num), (args-2));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_array_fill(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if ((args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfInt64) {
      fh_array_fill(rv, (int)(args[-0].m_data.num), (int)(args[-1].m_data.num), (args-2));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_array_fill(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("array_fill", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_array_filter(TypedValue* _rv, TypedValue* input, TypedValue* callback) asm("_ZN4HPHP14f_array_filterERKNS_7VariantES2_");

TypedValue* fg_array_filter(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    fh_array_filter(rv, (args-0), (count > 1) ? (args-1) : (TypedValue*)(&null_variant));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_wrong_arguments_nr("array_filter", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_array_flip(TypedValue* _rv, TypedValue* trans) asm("_ZN4HPHP12f_array_flipERKNS_7VariantE");

TypedValue* fg_array_flip(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    fh_array_flip(rv, (args-0));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_wrong_arguments_nr("array_flip", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_array_key_exists(TypedValue* key, TypedValue* search) asm("_ZN4HPHP18f_array_key_existsERKNS_7VariantES2_");

TypedValue* fg_array_key_exists(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    rv->m_type = KindOfBoolean;
    rv->m_data.num = (fh_array_key_exists((args-0), (args-1))) ? 1LL : 0LL;
  } else {
    throw_wrong_arguments_nr("array_key_exists", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_key_exists(TypedValue* key, TypedValue* search) asm("_ZN4HPHP12f_key_existsERKNS_7VariantES2_");

TypedValue* fg_key_exists(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    rv->m_type = KindOfBoolean;
    rv->m_data.num = (fh_key_exists((args-0), (args-1))) ? 1LL : 0LL;
  } else {
    throw_wrong_arguments_nr("key_exists", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_array_keys(TypedValue* _rv, TypedValue* input, TypedValue* search_value, bool strict) asm("_ZN4HPHP12f_array_keysERKNS_7VariantES2_b");

void fg1_array_keys(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_array_keys(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToBooleanInPlace(args-2);
  fh_array_keys(rv, (args-0), (count > 1) ? (args-1) : (TypedValue*)(&null_variant), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_array_keys(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfBoolean)) {
      fh_array_keys(rv, (args-0), (count > 1) ? (args-1) : (TypedValue*)(&null_variant), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_array_keys(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("array_keys", count, 1, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_array_map(TypedValue* _rv, int64_t _argc, TypedValue* callback, TypedValue* arr1, Value* _argv) asm("_ZN4HPHP11f_array_mapEiRKNS_7VariantES2_RKNS_5ArrayE");

TypedValue* fg_array_map(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2) {

    Array extraArgs;
    {
      ArrayInit ai(count-2);
      for (int32_t i = 2; i < count; ++i) {
        TypedValue* extraArg = ar->getExtraArg(i-2);
        if (tvIsStronglyBound(extraArg)) {
          ai.setRef(i-2, tvAsVariant(extraArg));
        } else {
          ai.set(i-2, tvAsVariant(extraArg));
        }
      }
      extraArgs = ai.create();
    }
    fh_array_map(rv, count, (args-0), (args-1), (Value*)(&extraArgs));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_missing_arguments_nr("array_map", 2, count, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_array_merge(TypedValue* _rv, int64_t _argc, TypedValue* array1, Value* _argv) asm("_ZN4HPHP13f_array_mergeEiRKNS_7VariantERKNS_5ArrayE");

TypedValue* fg_array_merge(ActRec* ar) {
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
    fh_array_merge(rv, count, (args-0), (Value*)(&extraArgs));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_missing_arguments_nr("array_merge", 1, count, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_array_merge_recursive(TypedValue* _rv, int64_t _argc, TypedValue* array1, Value* _argv) asm("_ZN4HPHP23f_array_merge_recursiveEiRKNS_7VariantERKNS_5ArrayE");

TypedValue* fg_array_merge_recursive(ActRec* ar) {
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
    fh_array_merge_recursive(rv, count, (args-0), (Value*)(&extraArgs));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_missing_arguments_nr("array_merge_recursive", 1, count, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_array_replace(TypedValue* _rv, int64_t _argc, TypedValue* array1, Value* _argv) asm("_ZN4HPHP15f_array_replaceEiRKNS_7VariantERKNS_5ArrayE");

TypedValue* fg_array_replace(ActRec* ar) {
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
    fh_array_replace(rv, count, (args-0), (Value*)(&extraArgs));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_missing_arguments_nr("array_replace", 1, count, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_array_replace_recursive(TypedValue* _rv, int64_t _argc, TypedValue* array1, Value* _argv) asm("_ZN4HPHP25f_array_replace_recursiveEiRKNS_7VariantERKNS_5ArrayE");

TypedValue* fg_array_replace_recursive(ActRec* ar) {
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
    fh_array_replace_recursive(rv, count, (args-0), (Value*)(&extraArgs));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_missing_arguments_nr("array_replace_recursive", 1, count, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_array_pad(TypedValue* _rv, TypedValue* input, int pad_size, TypedValue* pad_value) asm("_ZN4HPHP11f_array_padERKNS_7VariantEiS2_");

void fg1_array_pad(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_array_pad(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-1);
  fh_array_pad(rv, (args-0), (int)(args[-1].m_data.num), (args-2));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_array_pad(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if ((args - 1)->m_type == KindOfInt64) {
      fh_array_pad(rv, (args-0), (int)(args[-1].m_data.num), (args-2));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_array_pad(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("array_pad", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_array_pop(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP11f_array_popERKNS_14VRefParamValueE");

TypedValue* fg_array_pop(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    fh_array_pop(rv, (args-0));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_wrong_arguments_nr("array_pop", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_array_product(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP15f_array_productERKNS_7VariantE");

TypedValue* fg_array_product(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    fh_array_product(rv, (args-0));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_wrong_arguments_nr("array_product", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_array_push(TypedValue* _rv, int64_t _argc, TypedValue* array, TypedValue* var, Value* _argv) asm("_ZN4HPHP12f_array_pushEiRKNS_14VRefParamValueERKNS_7VariantERKNS_5ArrayE");

TypedValue* fg_array_push(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2) {

    Array extraArgs;
    {
      ArrayInit ai(count-2);
      for (int32_t i = 2; i < count; ++i) {
        TypedValue* extraArg = ar->getExtraArg(i-2);
        if (tvIsStronglyBound(extraArg)) {
          ai.setRef(i-2, tvAsVariant(extraArg));
        } else {
          ai.set(i-2, tvAsVariant(extraArg));
        }
      }
      extraArgs = ai.create();
    }
    fh_array_push(rv, count, (args-0), (args-1), (Value*)(&extraArgs));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_missing_arguments_nr("array_push", 2, count, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_array_rand(TypedValue* _rv, TypedValue* input, int num_req) asm("_ZN4HPHP12f_array_randERKNS_7VariantEi");

void fg1_array_rand(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_array_rand(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-1);
  fh_array_rand(rv, (args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_array_rand(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfInt64)) {
      fh_array_rand(rv, (args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(1));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_array_rand(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("array_rand", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_array_reduce(TypedValue* _rv, TypedValue* input, TypedValue* callback, TypedValue* initial) asm("_ZN4HPHP14f_array_reduceERKNS_7VariantES2_S2_");

TypedValue* fg_array_reduce(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    fh_array_reduce(rv, (args-0), (args-1), (count > 2) ? (args-2) : (TypedValue*)(&null_variant));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_wrong_arguments_nr("array_reduce", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_array_reverse(TypedValue* _rv, TypedValue* array, bool preserve_keys) asm("_ZN4HPHP15f_array_reverseERKNS_7VariantEb");

void fg1_array_reverse(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_array_reverse(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToBooleanInPlace(args-1);
  fh_array_reverse(rv, (args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_array_reverse(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfBoolean)) {
      fh_array_reverse(rv, (args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_array_reverse(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("array_reverse", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_array_search(TypedValue* _rv, TypedValue* needle, TypedValue* haystack, bool strict) asm("_ZN4HPHP14f_array_searchERKNS_7VariantES2_b");

void fg1_array_search(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_array_search(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToBooleanInPlace(args-2);
  fh_array_search(rv, (args-0), (args-1), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_array_search(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfBoolean)) {
      fh_array_search(rv, (args-0), (args-1), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_array_search(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("array_search", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_array_shift(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP13f_array_shiftERKNS_14VRefParamValueE");

TypedValue* fg_array_shift(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    fh_array_shift(rv, (args-0));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_wrong_arguments_nr("array_shift", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_array_slice(TypedValue* _rv, TypedValue* array, int offset, TypedValue* length, bool preserve_keys) asm("_ZN4HPHP13f_array_sliceERKNS_7VariantEiS2_b");

void fg1_array_slice(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_array_slice(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-3);
    }
  case 3:
  case 2:
    break;
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  fh_array_slice(rv, (args-0), (int)(args[-1].m_data.num), (count > 2) ? (args-2) : (TypedValue*)(&null_variant), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_array_slice(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 4) {
    if ((count <= 3 || (args - 3)->m_type == KindOfBoolean) &&
        (args - 1)->m_type == KindOfInt64) {
      fh_array_slice(rv, (args-0), (int)(args[-1].m_data.num), (count > 2) ? (args-2) : (TypedValue*)(&null_variant), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_array_slice(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("array_slice", count, 2, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_array_splice(TypedValue* _rv, TypedValue* input, int offset, TypedValue* length, TypedValue* replacement) asm("_ZN4HPHP14f_array_spliceERKNS_14VRefParamValueEiRKNS_7VariantES5_");

void fg1_array_splice(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_array_splice(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-1);
  fh_array_splice(rv, (args-0), (int)(args[-1].m_data.num), (count > 2) ? (args-2) : (TypedValue*)(&null_variant), (count > 3) ? (args-3) : (TypedValue*)(&null_variant));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_array_splice(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 4) {
    if ((args - 1)->m_type == KindOfInt64) {
      fh_array_splice(rv, (args-0), (int)(args[-1].m_data.num), (count > 2) ? (args-2) : (TypedValue*)(&null_variant), (count > 3) ? (args-3) : (TypedValue*)(&null_variant));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_array_splice(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("array_splice", count, 2, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_array_sum(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP11f_array_sumERKNS_7VariantE");

TypedValue* fg_array_sum(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    fh_array_sum(rv, (args-0));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_wrong_arguments_nr("array_sum", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_array_unshift(int64_t _argc, TypedValue* array, TypedValue* var, Value* _argv) asm("_ZN4HPHP15f_array_unshiftEiRKNS_14VRefParamValueERKNS_7VariantERKNS_5ArrayE");

TypedValue* fg_array_unshift(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2) {
    rv->m_type = KindOfInt64;

    Array extraArgs;
    {
      ArrayInit ai(count-2);
      for (int32_t i = 2; i < count; ++i) {
        TypedValue* extraArg = ar->getExtraArg(i-2);
        if (tvIsStronglyBound(extraArg)) {
          ai.setRef(i-2, tvAsVariant(extraArg));
        } else {
          ai.set(i-2, tvAsVariant(extraArg));
        }
      }
      extraArgs = ai.create();
    }
    rv->m_data.num = (int64_t)fh_array_unshift(count, (args-0), (args-1), (Value*)(&extraArgs));
  } else {
    throw_missing_arguments_nr("array_unshift", 2, count, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_array_values(TypedValue* _rv, TypedValue* input) asm("_ZN4HPHP14f_array_valuesERKNS_7VariantE");

TypedValue* fg_array_values(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    fh_array_values(rv, (args-0));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_wrong_arguments_nr("array_values", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_array_walk_recursive(TypedValue* input, TypedValue* funcname, TypedValue* userdata) asm("_ZN4HPHP22f_array_walk_recursiveERKNS_14VRefParamValueERKNS_7VariantES5_");

TypedValue* fg_array_walk_recursive(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    rv->m_type = KindOfBoolean;
    rv->m_data.num = (fh_array_walk_recursive((args-0), (args-1), (count > 2) ? (args-2) : (TypedValue*)(&null_variant))) ? 1LL : 0LL;
  } else {
    throw_wrong_arguments_nr("array_walk_recursive", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_array_walk(TypedValue* input, TypedValue* funcname, TypedValue* userdata) asm("_ZN4HPHP12f_array_walkERKNS_14VRefParamValueERKNS_7VariantES5_");

TypedValue* fg_array_walk(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    rv->m_type = KindOfBoolean;
    rv->m_data.num = (fh_array_walk((args-0), (args-1), (count > 2) ? (args-2) : (TypedValue*)(&null_variant))) ? 1LL : 0LL;
  } else {
    throw_wrong_arguments_nr("array_walk", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_compact(Value* _rv, int64_t _argc, TypedValue* varname, Value* _argv) asm("_ZN4HPHP9f_compactEiRKNS_7VariantERKNS_5ArrayE");

TypedValue* fg_compact(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1) {
    rv->m_type = KindOfArray;

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
    fh_compact(&(rv->m_data), count, (args-0), (Value*)(&extraArgs));
    if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  } else {
    throw_missing_arguments_nr("compact", 1, count, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_shuffle(TypedValue* array) asm("_ZN4HPHP9f_shuffleERKNS_14VRefParamValueE");

TypedValue* fg_shuffle(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    rv->m_type = KindOfBoolean;
    rv->m_data.num = (fh_shuffle((args-0))) ? 1LL : 0LL;
  } else {
    throw_wrong_arguments_nr("shuffle", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_count(TypedValue* var, bool recursive) asm("_ZN4HPHP7f_countERKNS_7VariantEb");

void fg1_count(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_count(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToBooleanInPlace(args-1);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_count((args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
}

TypedValue* fg_count(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfBoolean)) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_count((args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
    } else {
      fg1_count(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("count", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_sizeof(TypedValue* var, bool recursive) asm("_ZN4HPHP8f_sizeofERKNS_7VariantEb");

void fg1_sizeof(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_sizeof(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToBooleanInPlace(args-1);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_sizeof((args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
}

TypedValue* fg_sizeof(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfBoolean)) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_sizeof((args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
    } else {
      fg1_sizeof(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("sizeof", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_each(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP6f_eachERKNS_14VRefParamValueE");

TypedValue* fg_each(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    fh_each(rv, (args-0));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_wrong_arguments_nr("each", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_current(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP9f_currentERKNS_14VRefParamValueE");

TypedValue* fg_current(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    fh_current(rv, (args-0));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_wrong_arguments_nr("current", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_hphp_current_ref(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP18f_hphp_current_refERKNS_14VRefParamValueE");

TypedValue* fg_hphp_current_ref(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    fh_hphp_current_ref(rv, (args-0));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_wrong_arguments_nr("hphp_current_ref", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_next(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP6f_nextERKNS_14VRefParamValueE");

TypedValue* fg_next(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    fh_next(rv, (args-0));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_wrong_arguments_nr("next", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_pos(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP5f_posERKNS_14VRefParamValueE");

TypedValue* fg_pos(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    fh_pos(rv, (args-0));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_wrong_arguments_nr("pos", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_prev(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP6f_prevERKNS_14VRefParamValueE");

TypedValue* fg_prev(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    fh_prev(rv, (args-0));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_wrong_arguments_nr("prev", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_reset(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP7f_resetERKNS_14VRefParamValueE");

TypedValue* fg_reset(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    fh_reset(rv, (args-0));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_wrong_arguments_nr("reset", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_end(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP5f_endERKNS_14VRefParamValueE");

TypedValue* fg_end(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    fh_end(rv, (args-0));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_wrong_arguments_nr("end", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_key(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP5f_keyERKNS_14VRefParamValueE");

TypedValue* fg_key(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    fh_key(rv, (args-0));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_wrong_arguments_nr("key", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_hphp_get_iterator(TypedValue* _rv, TypedValue* iterable) asm("_ZN4HPHP19f_hphp_get_iteratorERKNS_7VariantE");

TypedValue* fg_hphp_get_iterator(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    fh_hphp_get_iterator(rv, (args-0));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_wrong_arguments_nr("hphp_get_iterator", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_hphp_get_mutable_iterator(TypedValue* _rv, TypedValue* iterable) asm("_ZN4HPHP27f_hphp_get_mutable_iteratorERKNS_14VRefParamValueE");

TypedValue* fg_hphp_get_mutable_iterator(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    fh_hphp_get_mutable_iterator(rv, (args-0));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_wrong_arguments_nr("hphp_get_mutable_iterator", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_in_array(TypedValue* needle, TypedValue* haystack, bool strict) asm("_ZN4HPHP10f_in_arrayERKNS_7VariantES2_b");

void fg1_in_array(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_in_array(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToBooleanInPlace(args-2);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_in_array((args-0), (args-1), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false))) ? 1LL : 0LL;
}

TypedValue* fg_in_array(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfBoolean)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_in_array((args-0), (args-1), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false))) ? 1LL : 0LL;
    } else {
      fg1_in_array(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("in_array", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_range(TypedValue* _rv, TypedValue* low, TypedValue* high, TypedValue* step) asm("_ZN4HPHP7f_rangeERKNS_7VariantES2_S2_");

TypedValue* fg_range(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    Variant defVal2 = 1;
    fh_range(rv, (args-0), (args-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_wrong_arguments_nr("range", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_array_diff(TypedValue* _rv, int64_t _argc, TypedValue* array1, TypedValue* array2, Value* _argv) asm("_ZN4HPHP12f_array_diffEiRKNS_7VariantES2_RKNS_5ArrayE");

TypedValue* fg_array_diff(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2) {

    Array extraArgs;
    {
      ArrayInit ai(count-2);
      for (int32_t i = 2; i < count; ++i) {
        TypedValue* extraArg = ar->getExtraArg(i-2);
        if (tvIsStronglyBound(extraArg)) {
          ai.setRef(i-2, tvAsVariant(extraArg));
        } else {
          ai.set(i-2, tvAsVariant(extraArg));
        }
      }
      extraArgs = ai.create();
    }
    fh_array_diff(rv, count, (args-0), (args-1), (Value*)(&extraArgs));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_missing_arguments_nr("array_diff", 2, count, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_array_udiff(TypedValue* _rv, int64_t _argc, TypedValue* array1, TypedValue* array2, TypedValue* data_compare_func, Value* _argv) asm("_ZN4HPHP13f_array_udiffEiRKNS_7VariantES2_S2_RKNS_5ArrayE");

TypedValue* fg_array_udiff(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3) {

    Array extraArgs;
    {
      ArrayInit ai(count-3);
      for (int32_t i = 3; i < count; ++i) {
        TypedValue* extraArg = ar->getExtraArg(i-3);
        if (tvIsStronglyBound(extraArg)) {
          ai.setRef(i-3, tvAsVariant(extraArg));
        } else {
          ai.set(i-3, tvAsVariant(extraArg));
        }
      }
      extraArgs = ai.create();
    }
    fh_array_udiff(rv, count, (args-0), (args-1), (args-2), (Value*)(&extraArgs));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_missing_arguments_nr("array_udiff", 3, count, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_array_diff_assoc(TypedValue* _rv, int64_t _argc, TypedValue* array1, TypedValue* array2, Value* _argv) asm("_ZN4HPHP18f_array_diff_assocEiRKNS_7VariantES2_RKNS_5ArrayE");

TypedValue* fg_array_diff_assoc(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2) {

    Array extraArgs;
    {
      ArrayInit ai(count-2);
      for (int32_t i = 2; i < count; ++i) {
        TypedValue* extraArg = ar->getExtraArg(i-2);
        if (tvIsStronglyBound(extraArg)) {
          ai.setRef(i-2, tvAsVariant(extraArg));
        } else {
          ai.set(i-2, tvAsVariant(extraArg));
        }
      }
      extraArgs = ai.create();
    }
    fh_array_diff_assoc(rv, count, (args-0), (args-1), (Value*)(&extraArgs));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_missing_arguments_nr("array_diff_assoc", 2, count, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_array_diff_uassoc(TypedValue* _rv, int64_t _argc, TypedValue* array1, TypedValue* array2, TypedValue* key_compare_func, Value* _argv) asm("_ZN4HPHP19f_array_diff_uassocEiRKNS_7VariantES2_S2_RKNS_5ArrayE");

TypedValue* fg_array_diff_uassoc(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3) {

    Array extraArgs;
    {
      ArrayInit ai(count-3);
      for (int32_t i = 3; i < count; ++i) {
        TypedValue* extraArg = ar->getExtraArg(i-3);
        if (tvIsStronglyBound(extraArg)) {
          ai.setRef(i-3, tvAsVariant(extraArg));
        } else {
          ai.set(i-3, tvAsVariant(extraArg));
        }
      }
      extraArgs = ai.create();
    }
    fh_array_diff_uassoc(rv, count, (args-0), (args-1), (args-2), (Value*)(&extraArgs));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_missing_arguments_nr("array_diff_uassoc", 3, count, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_array_udiff_assoc(TypedValue* _rv, int64_t _argc, TypedValue* array1, TypedValue* array2, TypedValue* data_compare_func, Value* _argv) asm("_ZN4HPHP19f_array_udiff_assocEiRKNS_7VariantES2_S2_RKNS_5ArrayE");

TypedValue* fg_array_udiff_assoc(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3) {

    Array extraArgs;
    {
      ArrayInit ai(count-3);
      for (int32_t i = 3; i < count; ++i) {
        TypedValue* extraArg = ar->getExtraArg(i-3);
        if (tvIsStronglyBound(extraArg)) {
          ai.setRef(i-3, tvAsVariant(extraArg));
        } else {
          ai.set(i-3, tvAsVariant(extraArg));
        }
      }
      extraArgs = ai.create();
    }
    fh_array_udiff_assoc(rv, count, (args-0), (args-1), (args-2), (Value*)(&extraArgs));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_missing_arguments_nr("array_udiff_assoc", 3, count, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_array_udiff_uassoc(TypedValue* _rv, int64_t _argc, TypedValue* array1, TypedValue* array2, TypedValue* data_compare_func, TypedValue* key_compare_func, Value* _argv) asm("_ZN4HPHP20f_array_udiff_uassocEiRKNS_7VariantES2_S2_S2_RKNS_5ArrayE");

TypedValue* fg_array_udiff_uassoc(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 4) {

    Array extraArgs;
    {
      ArrayInit ai(count-4);
      for (int32_t i = 4; i < count; ++i) {
        TypedValue* extraArg = ar->getExtraArg(i-4);
        if (tvIsStronglyBound(extraArg)) {
          ai.setRef(i-4, tvAsVariant(extraArg));
        } else {
          ai.set(i-4, tvAsVariant(extraArg));
        }
      }
      extraArgs = ai.create();
    }
    fh_array_udiff_uassoc(rv, count, (args-0), (args-1), (args-2), (args-3), (Value*)(&extraArgs));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_missing_arguments_nr("array_udiff_uassoc", 4, count, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_array_diff_key(TypedValue* _rv, int64_t _argc, TypedValue* array1, TypedValue* array2, Value* _argv) asm("_ZN4HPHP16f_array_diff_keyEiRKNS_7VariantES2_RKNS_5ArrayE");

TypedValue* fg_array_diff_key(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2) {

    Array extraArgs;
    {
      ArrayInit ai(count-2);
      for (int32_t i = 2; i < count; ++i) {
        TypedValue* extraArg = ar->getExtraArg(i-2);
        if (tvIsStronglyBound(extraArg)) {
          ai.setRef(i-2, tvAsVariant(extraArg));
        } else {
          ai.set(i-2, tvAsVariant(extraArg));
        }
      }
      extraArgs = ai.create();
    }
    fh_array_diff_key(rv, count, (args-0), (args-1), (Value*)(&extraArgs));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_missing_arguments_nr("array_diff_key", 2, count, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_array_diff_ukey(TypedValue* _rv, int64_t _argc, TypedValue* array1, TypedValue* array2, TypedValue* key_compare_func, Value* _argv) asm("_ZN4HPHP17f_array_diff_ukeyEiRKNS_7VariantES2_S2_RKNS_5ArrayE");

TypedValue* fg_array_diff_ukey(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3) {

    Array extraArgs;
    {
      ArrayInit ai(count-3);
      for (int32_t i = 3; i < count; ++i) {
        TypedValue* extraArg = ar->getExtraArg(i-3);
        if (tvIsStronglyBound(extraArg)) {
          ai.setRef(i-3, tvAsVariant(extraArg));
        } else {
          ai.set(i-3, tvAsVariant(extraArg));
        }
      }
      extraArgs = ai.create();
    }
    fh_array_diff_ukey(rv, count, (args-0), (args-1), (args-2), (Value*)(&extraArgs));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_missing_arguments_nr("array_diff_ukey", 3, count, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_array_intersect(TypedValue* _rv, int64_t _argc, TypedValue* array1, TypedValue* array2, Value* _argv) asm("_ZN4HPHP17f_array_intersectEiRKNS_7VariantES2_RKNS_5ArrayE");

TypedValue* fg_array_intersect(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2) {

    Array extraArgs;
    {
      ArrayInit ai(count-2);
      for (int32_t i = 2; i < count; ++i) {
        TypedValue* extraArg = ar->getExtraArg(i-2);
        if (tvIsStronglyBound(extraArg)) {
          ai.setRef(i-2, tvAsVariant(extraArg));
        } else {
          ai.set(i-2, tvAsVariant(extraArg));
        }
      }
      extraArgs = ai.create();
    }
    fh_array_intersect(rv, count, (args-0), (args-1), (Value*)(&extraArgs));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_missing_arguments_nr("array_intersect", 2, count, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_array_uintersect(TypedValue* _rv, int64_t _argc, TypedValue* array1, TypedValue* array2, TypedValue* data_compare_func, Value* _argv) asm("_ZN4HPHP18f_array_uintersectEiRKNS_7VariantES2_S2_RKNS_5ArrayE");

TypedValue* fg_array_uintersect(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3) {

    Array extraArgs;
    {
      ArrayInit ai(count-3);
      for (int32_t i = 3; i < count; ++i) {
        TypedValue* extraArg = ar->getExtraArg(i-3);
        if (tvIsStronglyBound(extraArg)) {
          ai.setRef(i-3, tvAsVariant(extraArg));
        } else {
          ai.set(i-3, tvAsVariant(extraArg));
        }
      }
      extraArgs = ai.create();
    }
    fh_array_uintersect(rv, count, (args-0), (args-1), (args-2), (Value*)(&extraArgs));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_missing_arguments_nr("array_uintersect", 3, count, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_array_intersect_assoc(TypedValue* _rv, int64_t _argc, TypedValue* array1, TypedValue* array2, Value* _argv) asm("_ZN4HPHP23f_array_intersect_assocEiRKNS_7VariantES2_RKNS_5ArrayE");

TypedValue* fg_array_intersect_assoc(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2) {

    Array extraArgs;
    {
      ArrayInit ai(count-2);
      for (int32_t i = 2; i < count; ++i) {
        TypedValue* extraArg = ar->getExtraArg(i-2);
        if (tvIsStronglyBound(extraArg)) {
          ai.setRef(i-2, tvAsVariant(extraArg));
        } else {
          ai.set(i-2, tvAsVariant(extraArg));
        }
      }
      extraArgs = ai.create();
    }
    fh_array_intersect_assoc(rv, count, (args-0), (args-1), (Value*)(&extraArgs));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_missing_arguments_nr("array_intersect_assoc", 2, count, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_array_intersect_uassoc(TypedValue* _rv, int64_t _argc, TypedValue* array1, TypedValue* array2, TypedValue* key_compare_func, Value* _argv) asm("_ZN4HPHP24f_array_intersect_uassocEiRKNS_7VariantES2_S2_RKNS_5ArrayE");

TypedValue* fg_array_intersect_uassoc(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3) {

    Array extraArgs;
    {
      ArrayInit ai(count-3);
      for (int32_t i = 3; i < count; ++i) {
        TypedValue* extraArg = ar->getExtraArg(i-3);
        if (tvIsStronglyBound(extraArg)) {
          ai.setRef(i-3, tvAsVariant(extraArg));
        } else {
          ai.set(i-3, tvAsVariant(extraArg));
        }
      }
      extraArgs = ai.create();
    }
    fh_array_intersect_uassoc(rv, count, (args-0), (args-1), (args-2), (Value*)(&extraArgs));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_missing_arguments_nr("array_intersect_uassoc", 3, count, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_array_uintersect_assoc(TypedValue* _rv, int64_t _argc, TypedValue* array1, TypedValue* array2, TypedValue* data_compare_func, Value* _argv) asm("_ZN4HPHP24f_array_uintersect_assocEiRKNS_7VariantES2_S2_RKNS_5ArrayE");

TypedValue* fg_array_uintersect_assoc(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3) {

    Array extraArgs;
    {
      ArrayInit ai(count-3);
      for (int32_t i = 3; i < count; ++i) {
        TypedValue* extraArg = ar->getExtraArg(i-3);
        if (tvIsStronglyBound(extraArg)) {
          ai.setRef(i-3, tvAsVariant(extraArg));
        } else {
          ai.set(i-3, tvAsVariant(extraArg));
        }
      }
      extraArgs = ai.create();
    }
    fh_array_uintersect_assoc(rv, count, (args-0), (args-1), (args-2), (Value*)(&extraArgs));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_missing_arguments_nr("array_uintersect_assoc", 3, count, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_array_uintersect_uassoc(TypedValue* _rv, int64_t _argc, TypedValue* array1, TypedValue* array2, TypedValue* data_compare_func, TypedValue* key_compare_func, Value* _argv) asm("_ZN4HPHP25f_array_uintersect_uassocEiRKNS_7VariantES2_S2_S2_RKNS_5ArrayE");

TypedValue* fg_array_uintersect_uassoc(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 4) {

    Array extraArgs;
    {
      ArrayInit ai(count-4);
      for (int32_t i = 4; i < count; ++i) {
        TypedValue* extraArg = ar->getExtraArg(i-4);
        if (tvIsStronglyBound(extraArg)) {
          ai.setRef(i-4, tvAsVariant(extraArg));
        } else {
          ai.set(i-4, tvAsVariant(extraArg));
        }
      }
      extraArgs = ai.create();
    }
    fh_array_uintersect_uassoc(rv, count, (args-0), (args-1), (args-2), (args-3), (Value*)(&extraArgs));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_missing_arguments_nr("array_uintersect_uassoc", 4, count, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_array_intersect_key(TypedValue* _rv, int64_t _argc, TypedValue* array1, TypedValue* array2, Value* _argv) asm("_ZN4HPHP21f_array_intersect_keyEiRKNS_7VariantES2_RKNS_5ArrayE");

TypedValue* fg_array_intersect_key(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2) {

    Array extraArgs;
    {
      ArrayInit ai(count-2);
      for (int32_t i = 2; i < count; ++i) {
        TypedValue* extraArg = ar->getExtraArg(i-2);
        if (tvIsStronglyBound(extraArg)) {
          ai.setRef(i-2, tvAsVariant(extraArg));
        } else {
          ai.set(i-2, tvAsVariant(extraArg));
        }
      }
      extraArgs = ai.create();
    }
    fh_array_intersect_key(rv, count, (args-0), (args-1), (Value*)(&extraArgs));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_missing_arguments_nr("array_intersect_key", 2, count, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_array_intersect_ukey(TypedValue* _rv, int64_t _argc, TypedValue* array1, TypedValue* array2, TypedValue* key_compare_func, Value* _argv) asm("_ZN4HPHP22f_array_intersect_ukeyEiRKNS_7VariantES2_S2_RKNS_5ArrayE");

TypedValue* fg_array_intersect_ukey(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3) {

    Array extraArgs;
    {
      ArrayInit ai(count-3);
      for (int32_t i = 3; i < count; ++i) {
        TypedValue* extraArg = ar->getExtraArg(i-3);
        if (tvIsStronglyBound(extraArg)) {
          ai.setRef(i-3, tvAsVariant(extraArg));
        } else {
          ai.set(i-3, tvAsVariant(extraArg));
        }
      }
      extraArgs = ai.create();
    }
    fh_array_intersect_ukey(rv, count, (args-0), (args-1), (args-2), (Value*)(&extraArgs));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_missing_arguments_nr("array_intersect_ukey", 3, count, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_sort(TypedValue* array, int sort_flags, bool use_collator) asm("_ZN4HPHP6f_sortERKNS_14VRefParamValueEib");

void fg1_sort(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_sort(TypedValue* rv, ActRec* ar, int32_t count) {
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
    break;
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_sort((args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false))) ? 1LL : 0LL;
}

TypedValue* fg_sort(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfBoolean) &&
        (count <= 1 || (args - 1)->m_type == KindOfInt64)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_sort((args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false))) ? 1LL : 0LL;
    } else {
      fg1_sort(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("sort", count, 1, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_rsort(TypedValue* array, int sort_flags, bool use_collator) asm("_ZN4HPHP7f_rsortERKNS_14VRefParamValueEib");

void fg1_rsort(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_rsort(TypedValue* rv, ActRec* ar, int32_t count) {
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
    break;
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_rsort((args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false))) ? 1LL : 0LL;
}

TypedValue* fg_rsort(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfBoolean) &&
        (count <= 1 || (args - 1)->m_type == KindOfInt64)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_rsort((args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false))) ? 1LL : 0LL;
    } else {
      fg1_rsort(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("rsort", count, 1, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_asort(TypedValue* array, int sort_flags, bool use_collator) asm("_ZN4HPHP7f_asortERKNS_14VRefParamValueEib");

void fg1_asort(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_asort(TypedValue* rv, ActRec* ar, int32_t count) {
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
    break;
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_asort((args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false))) ? 1LL : 0LL;
}

TypedValue* fg_asort(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfBoolean) &&
        (count <= 1 || (args - 1)->m_type == KindOfInt64)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_asort((args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false))) ? 1LL : 0LL;
    } else {
      fg1_asort(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("asort", count, 1, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_arsort(TypedValue* array, int sort_flags, bool use_collator) asm("_ZN4HPHP8f_arsortERKNS_14VRefParamValueEib");

void fg1_arsort(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_arsort(TypedValue* rv, ActRec* ar, int32_t count) {
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
    break;
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_arsort((args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false))) ? 1LL : 0LL;
}

TypedValue* fg_arsort(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfBoolean) &&
        (count <= 1 || (args - 1)->m_type == KindOfInt64)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_arsort((args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false))) ? 1LL : 0LL;
    } else {
      fg1_arsort(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("arsort", count, 1, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_ksort(TypedValue* array, int sort_flags) asm("_ZN4HPHP7f_ksortERKNS_14VRefParamValueEi");

void fg1_ksort(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_ksort(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-1);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_ksort((args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0))) ? 1LL : 0LL;
}

TypedValue* fg_ksort(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfInt64)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_ksort((args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0))) ? 1LL : 0LL;
    } else {
      fg1_ksort(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("ksort", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_krsort(TypedValue* array, int sort_flags) asm("_ZN4HPHP8f_krsortERKNS_14VRefParamValueEi");

void fg1_krsort(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_krsort(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-1);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_krsort((args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0))) ? 1LL : 0LL;
}

TypedValue* fg_krsort(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfInt64)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_krsort((args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0))) ? 1LL : 0LL;
    } else {
      fg1_krsort(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("krsort", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_natsort(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP9f_natsortERKNS_14VRefParamValueE");

TypedValue* fg_natsort(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    fh_natsort(rv, (args-0));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_wrong_arguments_nr("natsort", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_natcasesort(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP13f_natcasesortERKNS_14VRefParamValueE");

TypedValue* fg_natcasesort(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    fh_natcasesort(rv, (args-0));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_wrong_arguments_nr("natcasesort", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_usort(TypedValue* array, TypedValue* cmp_function) asm("_ZN4HPHP7f_usortERKNS_14VRefParamValueERKNS_7VariantE");

TypedValue* fg_usort(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    rv->m_type = KindOfBoolean;
    rv->m_data.num = (fh_usort((args-0), (args-1))) ? 1LL : 0LL;
  } else {
    throw_wrong_arguments_nr("usort", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_uasort(TypedValue* array, TypedValue* cmp_function) asm("_ZN4HPHP8f_uasortERKNS_14VRefParamValueERKNS_7VariantE");

TypedValue* fg_uasort(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    rv->m_type = KindOfBoolean;
    rv->m_data.num = (fh_uasort((args-0), (args-1))) ? 1LL : 0LL;
  } else {
    throw_wrong_arguments_nr("uasort", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_uksort(TypedValue* array, TypedValue* cmp_function) asm("_ZN4HPHP8f_uksortERKNS_14VRefParamValueERKNS_7VariantE");

TypedValue* fg_uksort(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    rv->m_type = KindOfBoolean;
    rv->m_data.num = (fh_uksort((args-0), (args-1))) ? 1LL : 0LL;
  } else {
    throw_wrong_arguments_nr("uksort", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_array_multisort(int64_t _argc, TypedValue* ar1, Value* _argv) asm("_ZN4HPHP17f_array_multisortEiRKNS_14VRefParamValueERKNS_5ArrayE");

TypedValue* fg_array_multisort(ActRec* ar) {
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
    rv->m_data.num = (fh_array_multisort(count, (args-0), (Value*)(&extraArgs))) ? 1LL : 0LL;
  } else {
    throw_missing_arguments_nr("array_multisort", 1, count, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_array_unique(TypedValue* _rv, TypedValue* array, int sort_flags) asm("_ZN4HPHP14f_array_uniqueERKNS_7VariantEi");

void fg1_array_unique(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_array_unique(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-1);
  fh_array_unique(rv, (args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(2));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_array_unique(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfInt64)) {
      fh_array_unique(rv, (args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(2));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_array_unique(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("array_unique", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_i18n_loc_get_default(Value* _rv) asm("_ZN4HPHP22f_i18n_loc_get_defaultEv");

TypedValue* fg_i18n_loc_get_default(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfString;
    fh_i18n_loc_get_default(&(rv->m_data));
    if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("i18n_loc_get_default", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_i18n_loc_set_default(Value* locale) asm("_ZN4HPHP22f_i18n_loc_set_defaultERKNS_6StringE");

void fg1_i18n_loc_set_default(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_i18n_loc_set_default(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_i18n_loc_set_default(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_i18n_loc_set_default(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_i18n_loc_set_default(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_i18n_loc_set_default(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("i18n_loc_set_default", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_i18n_loc_set_attribute(long attr, long val) asm("_ZN4HPHP24f_i18n_loc_set_attributeEll");

void fg1_i18n_loc_set_attribute(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_i18n_loc_set_attribute(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_i18n_loc_set_attribute((long)(args[-0].m_data.num), (long)(args[-1].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_i18n_loc_set_attribute(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfInt64) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_i18n_loc_set_attribute((long)(args[-0].m_data.num), (long)(args[-1].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_i18n_loc_set_attribute(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("i18n_loc_set_attribute", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_i18n_loc_set_strength(long strength) asm("_ZN4HPHP23f_i18n_loc_set_strengthEl");

void fg1_i18n_loc_set_strength(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_i18n_loc_set_strength(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_i18n_loc_set_strength((long)(args[-0].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_i18n_loc_set_strength(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfInt64) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_i18n_loc_set_strength((long)(args[-0].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_i18n_loc_set_strength(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("i18n_loc_set_strength", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_i18n_loc_get_error_code(TypedValue* _rv) asm("_ZN4HPHP25f_i18n_loc_get_error_codeEv");

TypedValue* fg_i18n_loc_get_error_code(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    fh_i18n_loc_get_error_code(rv);
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("i18n_loc_get_error_code", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

} // namespace HPHP
