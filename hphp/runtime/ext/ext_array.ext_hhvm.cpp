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
HPHP::Variant HPHP::f_array_change_key_case(HPHP::Variant const&, bool)
_ZN4HPHP23f_array_change_key_caseERKNS_7VariantEb

(return value) => rax
_rv => rdi
input => rsi
upper => rdx
*/

TypedValue* fh_array_change_key_case(TypedValue* _rv, TypedValue* input, bool upper) asm("_ZN4HPHP23f_array_change_key_caseERKNS_7VariantEb");

TypedValue * fg1_array_change_key_case(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_array_change_key_case(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToBooleanInPlace(args-1);
  fh_array_change_key_case((rv), (args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_array_change_key_case(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfBoolean)) {
        fh_array_change_key_case((&(rv)), (args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_array_change_key_case(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("array_change_key_case", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_array_chunk(HPHP::Variant const&, int, bool)
_ZN4HPHP13f_array_chunkERKNS_7VariantEib

(return value) => rax
_rv => rdi
input => rsi
size => rdx
preserve_keys => rcx
*/

TypedValue* fh_array_chunk(TypedValue* _rv, TypedValue* input, int size, bool preserve_keys) asm("_ZN4HPHP13f_array_chunkERKNS_7VariantEib");

TypedValue * fg1_array_chunk(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_array_chunk(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
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
  fh_array_chunk((rv), (args-0), (int)(args[-1].m_data.num), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_array_chunk(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfBoolean) && (args-1)->m_type == KindOfInt64) {
        fh_array_chunk((&(rv)), (args-0), (int)(args[-1].m_data.num), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_array_chunk(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("array_chunk", count, 2, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_array_combine(HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP15f_array_combineERKNS_7VariantES2_

(return value) => rax
_rv => rdi
keys => rsi
values => rdx
*/

TypedValue* fh_array_combine(TypedValue* _rv, TypedValue* keys, TypedValue* values) asm("_ZN4HPHP15f_array_combineERKNS_7VariantES2_");

TypedValue* fg_array_combine(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      fh_array_combine((&(rv)), (args-0), (args-1));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 2);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("array_combine", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_array_count_values(HPHP::Variant const&)
_ZN4HPHP20f_array_count_valuesERKNS_7VariantE

(return value) => rax
_rv => rdi
input => rsi
*/

TypedValue* fh_array_count_values(TypedValue* _rv, TypedValue* input) asm("_ZN4HPHP20f_array_count_valuesERKNS_7VariantE");

TypedValue* fg_array_count_values(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      fh_array_count_values((&(rv)), (args-0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("array_count_values", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_array_fill_keys(HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP17f_array_fill_keysERKNS_7VariantES2_

(return value) => rax
_rv => rdi
keys => rsi
value => rdx
*/

TypedValue* fh_array_fill_keys(TypedValue* _rv, TypedValue* keys, TypedValue* value) asm("_ZN4HPHP17f_array_fill_keysERKNS_7VariantES2_");

TypedValue* fg_array_fill_keys(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      fh_array_fill_keys((&(rv)), (args-0), (args-1));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 2);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("array_fill_keys", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_array_fill(int, int, HPHP::Variant const&)
_ZN4HPHP12f_array_fillEiiRKNS_7VariantE

(return value) => rax
_rv => rdi
start_index => rsi
num => rdx
value => rcx
*/

TypedValue* fh_array_fill(TypedValue* _rv, int start_index, int num, TypedValue* value) asm("_ZN4HPHP12f_array_fillEiiRKNS_7VariantE");

TypedValue * fg1_array_fill(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_array_fill(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  fh_array_fill((rv), (int)(args[-0].m_data.num), (int)(args[-1].m_data.num), (args-2));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_array_fill(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfInt64) {
        fh_array_fill((&(rv)), (int)(args[-0].m_data.num), (int)(args[-1].m_data.num), (args-2));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_array_fill(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("array_fill", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_array_filter(HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP14f_array_filterERKNS_7VariantES2_

(return value) => rax
_rv => rdi
input => rsi
callback => rdx
*/

TypedValue* fh_array_filter(TypedValue* _rv, TypedValue* input, TypedValue* callback) asm("_ZN4HPHP14f_array_filterERKNS_7VariantES2_");

TypedValue* fg_array_filter(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      fh_array_filter((&(rv)), (args-0), (count > 1) ? (args-1) : (TypedValue*)(&null_variant));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 2);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("array_filter", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_array_flip(HPHP::Variant const&)
_ZN4HPHP12f_array_flipERKNS_7VariantE

(return value) => rax
_rv => rdi
trans => rsi
*/

TypedValue* fh_array_flip(TypedValue* _rv, TypedValue* trans) asm("_ZN4HPHP12f_array_flipERKNS_7VariantE");

TypedValue* fg_array_flip(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      fh_array_flip((&(rv)), (args-0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("array_flip", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_array_key_exists(HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP18f_array_key_existsERKNS_7VariantES2_

(return value) => rax
key => rdi
search => rsi
*/

bool fh_array_key_exists(TypedValue* key, TypedValue* search) asm("_ZN4HPHP18f_array_key_existsERKNS_7VariantES2_");

TypedValue* fg_array_key_exists(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      rv.m_type = KindOfBoolean;
      rv.m_data.num = (fh_array_key_exists((args-0), (args-1))) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 2);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("array_key_exists", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_key_exists(HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP12f_key_existsERKNS_7VariantES2_

(return value) => rax
key => rdi
search => rsi
*/

bool fh_key_exists(TypedValue* key, TypedValue* search) asm("_ZN4HPHP12f_key_existsERKNS_7VariantES2_");

TypedValue* fg_key_exists(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      rv.m_type = KindOfBoolean;
      rv.m_data.num = (fh_key_exists((args-0), (args-1))) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 2);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("key_exists", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_array_keys(HPHP::Variant const&, HPHP::Variant const&, bool)
_ZN4HPHP12f_array_keysERKNS_7VariantES2_b

(return value) => rax
_rv => rdi
input => rsi
search_value => rdx
strict => rcx
*/

TypedValue* fh_array_keys(TypedValue* _rv, TypedValue* input, TypedValue* search_value, bool strict) asm("_ZN4HPHP12f_array_keysERKNS_7VariantES2_b");

TypedValue * fg1_array_keys(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_array_keys(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToBooleanInPlace(args-2);
  fh_array_keys((rv), (args-0), (count > 1) ? (args-1) : (TypedValue*)(&null_variant), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_array_keys(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfBoolean)) {
        fh_array_keys((&(rv)), (args-0), (count > 1) ? (args-1) : (TypedValue*)(&null_variant), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_array_keys(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("array_keys", count, 1, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_array_map(int, HPHP::Variant const&, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP11f_array_mapEiRKNS_7VariantES2_RKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
callback => rdx
arr1 => rcx
_argv => r8
*/

TypedValue* fh_array_map(TypedValue* _rv, int64_t _argc, TypedValue* callback, TypedValue* arr1, Value* _argv) asm("_ZN4HPHP11f_array_mapEiRKNS_7VariantES2_RKNS_5ArrayE");

TypedValue* fg_array_map(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL) {
      Array extraArgs;
      {
        ArrayInit ai(count-2);
        for (int64_t i = 2; i < count; ++i) {
          TypedValue* extraArg = ar->getExtraArg(i-2);
          if (tvIsStronglyBound(extraArg)) {
            ai.setRef(i-2, tvAsVariant(extraArg));
          } else {
            ai.set(i-2, tvAsVariant(extraArg));
          }
        }
        extraArgs = ai.create();
      }
      fh_array_map((&(rv)), (count), (args-0), (args-1), (Value*)(&extraArgs));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 2);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_missing_arguments_nr("array_map", count+1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_array_merge_recursive(int, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP23f_array_merge_recursiveEiRKNS_7VariantERKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
array1 => rdx
_argv => rcx
*/

TypedValue* fh_array_merge_recursive(TypedValue* _rv, int64_t _argc, TypedValue* array1, Value* _argv) asm("_ZN4HPHP23f_array_merge_recursiveEiRKNS_7VariantERKNS_5ArrayE");

TypedValue* fg_array_merge_recursive(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL) {
      Array extraArgs;
      {
        ArrayInit ai(count-1);
        for (int64_t i = 1; i < count; ++i) {
          TypedValue* extraArg = ar->getExtraArg(i-1);
          if (tvIsStronglyBound(extraArg)) {
            ai.setRef(i-1, tvAsVariant(extraArg));
          } else {
            ai.set(i-1, tvAsVariant(extraArg));
          }
        }
        extraArgs = ai.create();
      }
      fh_array_merge_recursive((&(rv)), (count), (args-0), (Value*)(&extraArgs));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_missing_arguments_nr("array_merge_recursive", count+1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_array_merge(int, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP13f_array_mergeEiRKNS_7VariantERKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
array1 => rdx
_argv => rcx
*/

TypedValue* fh_array_merge(TypedValue* _rv, int64_t _argc, TypedValue* array1, Value* _argv) asm("_ZN4HPHP13f_array_mergeEiRKNS_7VariantERKNS_5ArrayE");

TypedValue* fg_array_merge(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL) {
      Array extraArgs;
      {
        ArrayInit ai(count-1);
        for (int64_t i = 1; i < count; ++i) {
          TypedValue* extraArg = ar->getExtraArg(i-1);
          if (tvIsStronglyBound(extraArg)) {
            ai.setRef(i-1, tvAsVariant(extraArg));
          } else {
            ai.set(i-1, tvAsVariant(extraArg));
          }
        }
        extraArgs = ai.create();
      }
      fh_array_merge((&(rv)), (count), (args-0), (Value*)(&extraArgs));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_missing_arguments_nr("array_merge", count+1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_array_replace_recursive(int, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP25f_array_replace_recursiveEiRKNS_7VariantERKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
array1 => rdx
_argv => rcx
*/

TypedValue* fh_array_replace_recursive(TypedValue* _rv, int64_t _argc, TypedValue* array1, Value* _argv) asm("_ZN4HPHP25f_array_replace_recursiveEiRKNS_7VariantERKNS_5ArrayE");

TypedValue* fg_array_replace_recursive(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL) {
      Array extraArgs;
      {
        ArrayInit ai(count-1);
        for (int64_t i = 1; i < count; ++i) {
          TypedValue* extraArg = ar->getExtraArg(i-1);
          if (tvIsStronglyBound(extraArg)) {
            ai.setRef(i-1, tvAsVariant(extraArg));
          } else {
            ai.set(i-1, tvAsVariant(extraArg));
          }
        }
        extraArgs = ai.create();
      }
      fh_array_replace_recursive((&(rv)), (count), (args-0), (Value*)(&extraArgs));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_missing_arguments_nr("array_replace_recursive", count+1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_array_replace(int, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP15f_array_replaceEiRKNS_7VariantERKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
array1 => rdx
_argv => rcx
*/

TypedValue* fh_array_replace(TypedValue* _rv, int64_t _argc, TypedValue* array1, Value* _argv) asm("_ZN4HPHP15f_array_replaceEiRKNS_7VariantERKNS_5ArrayE");

TypedValue* fg_array_replace(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL) {
      Array extraArgs;
      {
        ArrayInit ai(count-1);
        for (int64_t i = 1; i < count; ++i) {
          TypedValue* extraArg = ar->getExtraArg(i-1);
          if (tvIsStronglyBound(extraArg)) {
            ai.setRef(i-1, tvAsVariant(extraArg));
          } else {
            ai.set(i-1, tvAsVariant(extraArg));
          }
        }
        extraArgs = ai.create();
      }
      fh_array_replace((&(rv)), (count), (args-0), (Value*)(&extraArgs));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_missing_arguments_nr("array_replace", count+1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_array_multisort(int, HPHP::VRefParamValue const&, HPHP::Array const&)
_ZN4HPHP17f_array_multisortEiRKNS_14VRefParamValueERKNS_5ArrayE

(return value) => rax
_argc => rdi
ar1 => rsi
_argv => rdx
*/

bool fh_array_multisort(int64_t _argc, TypedValue* ar1, Value* _argv) asm("_ZN4HPHP17f_array_multisortEiRKNS_14VRefParamValueERKNS_5ArrayE");

TypedValue* fg_array_multisort(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL) {
      rv.m_type = KindOfBoolean;
      Array extraArgs;
      {
        ArrayInit ai(count-1);
        for (int64_t i = 1; i < count; ++i) {
          TypedValue* extraArg = ar->getExtraArg(i-1);
          if (tvIsStronglyBound(extraArg)) {
            ai.setRef(i-1, tvAsVariant(extraArg));
          } else {
            ai.set(i-1, tvAsVariant(extraArg));
          }
        }
        extraArgs = ai.create();
      }
      rv.m_data.num = (fh_array_multisort((count), (args-0), (Value*)(&extraArgs))) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_missing_arguments_nr("array_multisort", count+1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_array_pad(HPHP::Variant const&, int, HPHP::Variant const&)
_ZN4HPHP11f_array_padERKNS_7VariantEiS2_

(return value) => rax
_rv => rdi
input => rsi
pad_size => rdx
pad_value => rcx
*/

TypedValue* fh_array_pad(TypedValue* _rv, TypedValue* input, int pad_size, TypedValue* pad_value) asm("_ZN4HPHP11f_array_padERKNS_7VariantEiS2_");

TypedValue * fg1_array_pad(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_array_pad(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-1);
  fh_array_pad((rv), (args-0), (int)(args[-1].m_data.num), (args-2));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_array_pad(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-1)->m_type == KindOfInt64) {
        fh_array_pad((&(rv)), (args-0), (int)(args[-1].m_data.num), (args-2));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_array_pad(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("array_pad", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_array_pop(HPHP::VRefParamValue const&)
_ZN4HPHP11f_array_popERKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
array => rsi
*/

TypedValue* fh_array_pop(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP11f_array_popERKNS_14VRefParamValueE");

TypedValue* fg_array_pop(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      fh_array_pop((&(rv)), (args-0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("array_pop", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_array_product(HPHP::Variant const&)
_ZN4HPHP15f_array_productERKNS_7VariantE

(return value) => rax
_rv => rdi
array => rsi
*/

TypedValue* fh_array_product(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP15f_array_productERKNS_7VariantE");

TypedValue* fg_array_product(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      fh_array_product((&(rv)), (args-0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("array_product", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_array_push(int, HPHP::VRefParamValue const&, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP12f_array_pushEiRKNS_14VRefParamValueERKNS_7VariantERKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
array => rdx
var => rcx
_argv => r8
*/

TypedValue* fh_array_push(TypedValue* _rv, int64_t _argc, TypedValue* array, TypedValue* var, Value* _argv) asm("_ZN4HPHP12f_array_pushEiRKNS_14VRefParamValueERKNS_7VariantERKNS_5ArrayE");

TypedValue* fg_array_push(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL) {
      Array extraArgs;
      {
        ArrayInit ai(count-2);
        for (int64_t i = 2; i < count; ++i) {
          TypedValue* extraArg = ar->getExtraArg(i-2);
          if (tvIsStronglyBound(extraArg)) {
            ai.setRef(i-2, tvAsVariant(extraArg));
          } else {
            ai.set(i-2, tvAsVariant(extraArg));
          }
        }
        extraArgs = ai.create();
      }
      fh_array_push((&(rv)), (count), (args-0), (args-1), (Value*)(&extraArgs));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 2);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_missing_arguments_nr("array_push", count+1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_array_rand(HPHP::Variant const&, int)
_ZN4HPHP12f_array_randERKNS_7VariantEi

(return value) => rax
_rv => rdi
input => rsi
num_req => rdx
*/

TypedValue* fh_array_rand(TypedValue* _rv, TypedValue* input, int num_req) asm("_ZN4HPHP12f_array_randERKNS_7VariantEi");

TypedValue * fg1_array_rand(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_array_rand(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-1);
  fh_array_rand((rv), (args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_array_rand(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64)) {
        fh_array_rand((&(rv)), (args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(1));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_array_rand(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("array_rand", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_array_reduce(HPHP::Variant const&, HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP14f_array_reduceERKNS_7VariantES2_S2_

(return value) => rax
_rv => rdi
input => rsi
callback => rdx
initial => rcx
*/

TypedValue* fh_array_reduce(TypedValue* _rv, TypedValue* input, TypedValue* callback, TypedValue* initial) asm("_ZN4HPHP14f_array_reduceERKNS_7VariantES2_S2_");

TypedValue* fg_array_reduce(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      fh_array_reduce((&(rv)), (args-0), (args-1), (count > 2) ? (args-2) : (TypedValue*)(&null_variant));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 3);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("array_reduce", count, 2, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_array_reverse(HPHP::Variant const&, bool)
_ZN4HPHP15f_array_reverseERKNS_7VariantEb

(return value) => rax
_rv => rdi
array => rsi
preserve_keys => rdx
*/

TypedValue* fh_array_reverse(TypedValue* _rv, TypedValue* array, bool preserve_keys) asm("_ZN4HPHP15f_array_reverseERKNS_7VariantEb");

TypedValue * fg1_array_reverse(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_array_reverse(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToBooleanInPlace(args-1);
  fh_array_reverse((rv), (args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_array_reverse(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfBoolean)) {
        fh_array_reverse((&(rv)), (args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_array_reverse(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("array_reverse", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_array_search(HPHP::Variant const&, HPHP::Variant const&, bool)
_ZN4HPHP14f_array_searchERKNS_7VariantES2_b

(return value) => rax
_rv => rdi
needle => rsi
haystack => rdx
strict => rcx
*/

TypedValue* fh_array_search(TypedValue* _rv, TypedValue* needle, TypedValue* haystack, bool strict) asm("_ZN4HPHP14f_array_searchERKNS_7VariantES2_b");

TypedValue * fg1_array_search(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_array_search(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToBooleanInPlace(args-2);
  fh_array_search((rv), (args-0), (args-1), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_array_search(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfBoolean)) {
        fh_array_search((&(rv)), (args-0), (args-1), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_array_search(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("array_search", count, 2, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_array_shift(HPHP::VRefParamValue const&)
_ZN4HPHP13f_array_shiftERKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
array => rsi
*/

TypedValue* fh_array_shift(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP13f_array_shiftERKNS_14VRefParamValueE");

TypedValue* fg_array_shift(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      fh_array_shift((&(rv)), (args-0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("array_shift", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_array_slice(HPHP::Variant const&, int, HPHP::Variant const&, bool)
_ZN4HPHP13f_array_sliceERKNS_7VariantEiS2_b

(return value) => rax
_rv => rdi
array => rsi
offset => rdx
length => rcx
preserve_keys => r8
*/

TypedValue* fh_array_slice(TypedValue* _rv, TypedValue* array, int offset, TypedValue* length, bool preserve_keys) asm("_ZN4HPHP13f_array_sliceERKNS_7VariantEiS2_b");

TypedValue * fg1_array_slice(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_array_slice(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
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
  fh_array_slice((rv), (args-0), (int)(args[-1].m_data.num), (count > 2) ? (args-2) : (TypedValue*)(&null_variant), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_array_slice(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 4LL) {
      if ((count <= 3 || (args-3)->m_type == KindOfBoolean) && (args-1)->m_type == KindOfInt64) {
        fh_array_slice((&(rv)), (args-0), (int)(args[-1].m_data.num), (count > 2) ? (args-2) : (TypedValue*)(&null_variant), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_array_slice(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("array_slice", count, 2, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_array_splice(HPHP::VRefParamValue const&, int, HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP14f_array_spliceERKNS_14VRefParamValueEiRKNS_7VariantES5_

(return value) => rax
_rv => rdi
input => rsi
offset => rdx
length => rcx
replacement => r8
*/

TypedValue* fh_array_splice(TypedValue* _rv, TypedValue* input, int offset, TypedValue* length, TypedValue* replacement) asm("_ZN4HPHP14f_array_spliceERKNS_14VRefParamValueEiRKNS_7VariantES5_");

TypedValue * fg1_array_splice(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_array_splice(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-1);
  fh_array_splice((rv), (args-0), (int)(args[-1].m_data.num), (count > 2) ? (args-2) : (TypedValue*)(&null_variant), (count > 3) ? (args-3) : (TypedValue*)(&null_variant));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_array_splice(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 4LL) {
      if ((args-1)->m_type == KindOfInt64) {
        fh_array_splice((&(rv)), (args-0), (int)(args[-1].m_data.num), (count > 2) ? (args-2) : (TypedValue*)(&null_variant), (count > 3) ? (args-3) : (TypedValue*)(&null_variant));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_array_splice(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("array_splice", count, 2, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_array_sum(HPHP::Variant const&)
_ZN4HPHP11f_array_sumERKNS_7VariantE

(return value) => rax
_rv => rdi
array => rsi
*/

TypedValue* fh_array_sum(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP11f_array_sumERKNS_7VariantE");

TypedValue* fg_array_sum(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      fh_array_sum((&(rv)), (args-0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("array_sum", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_array_unique(HPHP::Variant const&, int)
_ZN4HPHP14f_array_uniqueERKNS_7VariantEi

(return value) => rax
_rv => rdi
array => rsi
sort_flags => rdx
*/

TypedValue* fh_array_unique(TypedValue* _rv, TypedValue* array, int sort_flags) asm("_ZN4HPHP14f_array_uniqueERKNS_7VariantEi");

TypedValue * fg1_array_unique(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_array_unique(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-1);
  fh_array_unique((rv), (args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(2));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_array_unique(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64)) {
        fh_array_unique((&(rv)), (args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(2));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_array_unique(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("array_unique", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_array_unshift(int, HPHP::VRefParamValue const&, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP15f_array_unshiftEiRKNS_14VRefParamValueERKNS_7VariantERKNS_5ArrayE

(return value) => rax
_argc => rdi
array => rsi
var => rdx
_argv => rcx
*/

long fh_array_unshift(int64_t _argc, TypedValue* array, TypedValue* var, Value* _argv) asm("_ZN4HPHP15f_array_unshiftEiRKNS_14VRefParamValueERKNS_7VariantERKNS_5ArrayE");

TypedValue* fg_array_unshift(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL) {
      rv.m_type = KindOfInt64;
      Array extraArgs;
      {
        ArrayInit ai(count-2);
        for (int64_t i = 2; i < count; ++i) {
          TypedValue* extraArg = ar->getExtraArg(i-2);
          if (tvIsStronglyBound(extraArg)) {
            ai.setRef(i-2, tvAsVariant(extraArg));
          } else {
            ai.set(i-2, tvAsVariant(extraArg));
          }
        }
        extraArgs = ai.create();
      }
      rv.m_data.num = (int64_t)fh_array_unshift((count), (args-0), (args-1), (Value*)(&extraArgs));
      frame_free_locals_no_this_inl(ar, 2);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_missing_arguments_nr("array_unshift", count+1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_array_values(HPHP::Variant const&)
_ZN4HPHP14f_array_valuesERKNS_7VariantE

(return value) => rax
_rv => rdi
input => rsi
*/

TypedValue* fh_array_values(TypedValue* _rv, TypedValue* input) asm("_ZN4HPHP14f_array_valuesERKNS_7VariantE");

TypedValue* fg_array_values(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      fh_array_values((&(rv)), (args-0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("array_values", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_array_walk_recursive(HPHP::VRefParamValue const&, HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP22f_array_walk_recursiveERKNS_14VRefParamValueERKNS_7VariantES5_

(return value) => rax
input => rdi
funcname => rsi
userdata => rdx
*/

bool fh_array_walk_recursive(TypedValue* input, TypedValue* funcname, TypedValue* userdata) asm("_ZN4HPHP22f_array_walk_recursiveERKNS_14VRefParamValueERKNS_7VariantES5_");

TypedValue* fg_array_walk_recursive(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      rv.m_type = KindOfBoolean;
      rv.m_data.num = (fh_array_walk_recursive((args-0), (args-1), (count > 2) ? (args-2) : (TypedValue*)(&null_variant))) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 3);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("array_walk_recursive", count, 2, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_array_walk(HPHP::VRefParamValue const&, HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP12f_array_walkERKNS_14VRefParamValueERKNS_7VariantES5_

(return value) => rax
input => rdi
funcname => rsi
userdata => rdx
*/

bool fh_array_walk(TypedValue* input, TypedValue* funcname, TypedValue* userdata) asm("_ZN4HPHP12f_array_walkERKNS_14VRefParamValueERKNS_7VariantES5_");

TypedValue* fg_array_walk(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      rv.m_type = KindOfBoolean;
      rv.m_data.num = (fh_array_walk((args-0), (args-1), (count > 2) ? (args-2) : (TypedValue*)(&null_variant))) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 3);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("array_walk", count, 2, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_compact(int, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP9f_compactEiRKNS_7VariantERKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
varname => rdx
_argv => rcx
*/

Value* fh_compact(Value* _rv, int64_t _argc, TypedValue* varname, Value* _argv) asm("_ZN4HPHP9f_compactEiRKNS_7VariantERKNS_5ArrayE");

TypedValue* fg_compact(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL) {
      rv.m_type = KindOfArray;
      Array extraArgs;
      {
        ArrayInit ai(count-1);
        for (int64_t i = 1; i < count; ++i) {
          TypedValue* extraArg = ar->getExtraArg(i-1);
          if (tvIsStronglyBound(extraArg)) {
            ai.setRef(i-1, tvAsVariant(extraArg));
          } else {
            ai.set(i-1, tvAsVariant(extraArg));
          }
        }
        extraArgs = ai.create();
      }
      fh_compact((&rv.m_data), (count), (args-0), (Value*)(&extraArgs));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_missing_arguments_nr("compact", count+1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_shuffle(HPHP::VRefParamValue const&)
_ZN4HPHP9f_shuffleERKNS_14VRefParamValueE

(return value) => rax
array => rdi
*/

bool fh_shuffle(TypedValue* array) asm("_ZN4HPHP9f_shuffleERKNS_14VRefParamValueE");

TypedValue* fg_shuffle(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv.m_type = KindOfBoolean;
      rv.m_data.num = (fh_shuffle((args-0))) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("shuffle", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_count(HPHP::Variant const&, bool)
_ZN4HPHP7f_countERKNS_7VariantEb

(return value) => rax
var => rdi
recursive => rsi
*/

long fh_count(TypedValue* var, bool recursive) asm("_ZN4HPHP7f_countERKNS_7VariantEb");

TypedValue * fg1_count(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_count(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfInt64;
  tvCastToBooleanInPlace(args-1);
  rv->m_data.num = (int64_t)fh_count((args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
  return rv;
}

TypedValue* fg_count(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfBoolean)) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_count((args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_count(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("count", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_sizeof(HPHP::Variant const&, bool)
_ZN4HPHP8f_sizeofERKNS_7VariantEb

(return value) => rax
var => rdi
recursive => rsi
*/

long fh_sizeof(TypedValue* var, bool recursive) asm("_ZN4HPHP8f_sizeofERKNS_7VariantEb");

TypedValue * fg1_sizeof(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_sizeof(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfInt64;
  tvCastToBooleanInPlace(args-1);
  rv->m_data.num = (int64_t)fh_sizeof((args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
  return rv;
}

TypedValue* fg_sizeof(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfBoolean)) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_sizeof((args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_sizeof(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("sizeof", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_each(HPHP::VRefParamValue const&)
_ZN4HPHP6f_eachERKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
array => rsi
*/

TypedValue* fh_each(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP6f_eachERKNS_14VRefParamValueE");

TypedValue* fg_each(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      fh_each((&(rv)), (args-0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("each", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_current(HPHP::VRefParamValue const&)
_ZN4HPHP9f_currentERKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
array => rsi
*/

TypedValue* fh_current(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP9f_currentERKNS_14VRefParamValueE");

TypedValue* fg_current(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      fh_current((&(rv)), (args-0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("current", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_hphp_current_ref(HPHP::VRefParamValue const&)
_ZN4HPHP18f_hphp_current_refERKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
array => rsi
*/

TypedValue* fh_hphp_current_ref(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP18f_hphp_current_refERKNS_14VRefParamValueE");

TypedValue* fg_hphp_current_ref(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      fh_hphp_current_ref((&(rv)), (args-0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("hphp_current_ref", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_next(HPHP::VRefParamValue const&)
_ZN4HPHP6f_nextERKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
array => rsi
*/

TypedValue* fh_next(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP6f_nextERKNS_14VRefParamValueE");

TypedValue* fg_next(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      fh_next((&(rv)), (args-0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("next", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_pos(HPHP::VRefParamValue const&)
_ZN4HPHP5f_posERKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
array => rsi
*/

TypedValue* fh_pos(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP5f_posERKNS_14VRefParamValueE");

TypedValue* fg_pos(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      fh_pos((&(rv)), (args-0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("pos", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_prev(HPHP::VRefParamValue const&)
_ZN4HPHP6f_prevERKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
array => rsi
*/

TypedValue* fh_prev(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP6f_prevERKNS_14VRefParamValueE");

TypedValue* fg_prev(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      fh_prev((&(rv)), (args-0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("prev", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_reset(HPHP::VRefParamValue const&)
_ZN4HPHP7f_resetERKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
array => rsi
*/

TypedValue* fh_reset(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP7f_resetERKNS_14VRefParamValueE");

TypedValue* fg_reset(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      fh_reset((&(rv)), (args-0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("reset", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_end(HPHP::VRefParamValue const&)
_ZN4HPHP5f_endERKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
array => rsi
*/

TypedValue* fh_end(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP5f_endERKNS_14VRefParamValueE");

TypedValue* fg_end(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      fh_end((&(rv)), (args-0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("end", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_key(HPHP::VRefParamValue const&)
_ZN4HPHP5f_keyERKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
array => rsi
*/

TypedValue* fh_key(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP5f_keyERKNS_14VRefParamValueE");

TypedValue* fg_key(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      fh_key((&(rv)), (args-0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("key", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_hphp_get_iterator(HPHP::Variant const&)
_ZN4HPHP19f_hphp_get_iteratorERKNS_7VariantE

(return value) => rax
_rv => rdi
iterable => rsi
*/

TypedValue* fh_hphp_get_iterator(TypedValue* _rv, TypedValue* iterable) asm("_ZN4HPHP19f_hphp_get_iteratorERKNS_7VariantE");

TypedValue* fg_hphp_get_iterator(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      fh_hphp_get_iterator((&(rv)), (args-0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("hphp_get_iterator", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_hphp_get_mutable_iterator(HPHP::VRefParamValue const&)
_ZN4HPHP27f_hphp_get_mutable_iteratorERKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
iterable => rsi
*/

TypedValue* fh_hphp_get_mutable_iterator(TypedValue* _rv, TypedValue* iterable) asm("_ZN4HPHP27f_hphp_get_mutable_iteratorERKNS_14VRefParamValueE");

TypedValue* fg_hphp_get_mutable_iterator(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      fh_hphp_get_mutable_iterator((&(rv)), (args-0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("hphp_get_mutable_iterator", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_in_array(HPHP::Variant const&, HPHP::Variant const&, bool)
_ZN4HPHP10f_in_arrayERKNS_7VariantES2_b

(return value) => rax
needle => rdi
haystack => rsi
strict => rdx
*/

bool fh_in_array(TypedValue* needle, TypedValue* haystack, bool strict) asm("_ZN4HPHP10f_in_arrayERKNS_7VariantES2_b");

TypedValue * fg1_in_array(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_in_array(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToBooleanInPlace(args-2);
  rv->m_data.num = (fh_in_array((args-0), (args-1), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_in_array(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfBoolean)) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_in_array((args-0), (args-1), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_in_array(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("in_array", count, 2, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_range(HPHP::Variant const&, HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP7f_rangeERKNS_7VariantES2_S2_

(return value) => rax
_rv => rdi
low => rsi
high => rdx
step => rcx
*/

TypedValue* fh_range(TypedValue* _rv, TypedValue* low, TypedValue* high, TypedValue* step) asm("_ZN4HPHP7f_rangeERKNS_7VariantES2_S2_");

TypedValue* fg_range(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      Variant defVal2 = 1;
      fh_range((&(rv)), (args-0), (args-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 3);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("range", count, 2, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_array_diff(int, HPHP::Variant const&, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP12f_array_diffEiRKNS_7VariantES2_RKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
array1 => rdx
array2 => rcx
_argv => r8
*/

TypedValue* fh_array_diff(TypedValue* _rv, int64_t _argc, TypedValue* array1, TypedValue* array2, Value* _argv) asm("_ZN4HPHP12f_array_diffEiRKNS_7VariantES2_RKNS_5ArrayE");

TypedValue* fg_array_diff(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL) {
      Array extraArgs;
      {
        ArrayInit ai(count-2);
        for (int64_t i = 2; i < count; ++i) {
          TypedValue* extraArg = ar->getExtraArg(i-2);
          if (tvIsStronglyBound(extraArg)) {
            ai.setRef(i-2, tvAsVariant(extraArg));
          } else {
            ai.set(i-2, tvAsVariant(extraArg));
          }
        }
        extraArgs = ai.create();
      }
      fh_array_diff((&(rv)), (count), (args-0), (args-1), (Value*)(&extraArgs));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 2);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_missing_arguments_nr("array_diff", count+1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_array_udiff(int, HPHP::Variant const&, HPHP::Variant const&, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP13f_array_udiffEiRKNS_7VariantES2_S2_RKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
array1 => rdx
array2 => rcx
data_compare_func => r8
_argv => r9
*/

TypedValue* fh_array_udiff(TypedValue* _rv, int64_t _argc, TypedValue* array1, TypedValue* array2, TypedValue* data_compare_func, Value* _argv) asm("_ZN4HPHP13f_array_udiffEiRKNS_7VariantES2_S2_RKNS_5ArrayE");

TypedValue* fg_array_udiff(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 3LL) {
      Array extraArgs;
      {
        ArrayInit ai(count-3);
        for (int64_t i = 3; i < count; ++i) {
          TypedValue* extraArg = ar->getExtraArg(i-3);
          if (tvIsStronglyBound(extraArg)) {
            ai.setRef(i-3, tvAsVariant(extraArg));
          } else {
            ai.set(i-3, tvAsVariant(extraArg));
          }
        }
        extraArgs = ai.create();
      }
      fh_array_udiff((&(rv)), (count), (args-0), (args-1), (args-2), (Value*)(&extraArgs));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 3);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_missing_arguments_nr("array_udiff", count+1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_array_diff_assoc(int, HPHP::Variant const&, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP18f_array_diff_assocEiRKNS_7VariantES2_RKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
array1 => rdx
array2 => rcx
_argv => r8
*/

TypedValue* fh_array_diff_assoc(TypedValue* _rv, int64_t _argc, TypedValue* array1, TypedValue* array2, Value* _argv) asm("_ZN4HPHP18f_array_diff_assocEiRKNS_7VariantES2_RKNS_5ArrayE");

TypedValue* fg_array_diff_assoc(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL) {
      Array extraArgs;
      {
        ArrayInit ai(count-2);
        for (int64_t i = 2; i < count; ++i) {
          TypedValue* extraArg = ar->getExtraArg(i-2);
          if (tvIsStronglyBound(extraArg)) {
            ai.setRef(i-2, tvAsVariant(extraArg));
          } else {
            ai.set(i-2, tvAsVariant(extraArg));
          }
        }
        extraArgs = ai.create();
      }
      fh_array_diff_assoc((&(rv)), (count), (args-0), (args-1), (Value*)(&extraArgs));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 2);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_missing_arguments_nr("array_diff_assoc", count+1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_array_diff_uassoc(int, HPHP::Variant const&, HPHP::Variant const&, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP19f_array_diff_uassocEiRKNS_7VariantES2_S2_RKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
array1 => rdx
array2 => rcx
key_compare_func => r8
_argv => r9
*/

TypedValue* fh_array_diff_uassoc(TypedValue* _rv, int64_t _argc, TypedValue* array1, TypedValue* array2, TypedValue* key_compare_func, Value* _argv) asm("_ZN4HPHP19f_array_diff_uassocEiRKNS_7VariantES2_S2_RKNS_5ArrayE");

TypedValue* fg_array_diff_uassoc(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 3LL) {
      Array extraArgs;
      {
        ArrayInit ai(count-3);
        for (int64_t i = 3; i < count; ++i) {
          TypedValue* extraArg = ar->getExtraArg(i-3);
          if (tvIsStronglyBound(extraArg)) {
            ai.setRef(i-3, tvAsVariant(extraArg));
          } else {
            ai.set(i-3, tvAsVariant(extraArg));
          }
        }
        extraArgs = ai.create();
      }
      fh_array_diff_uassoc((&(rv)), (count), (args-0), (args-1), (args-2), (Value*)(&extraArgs));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 3);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_missing_arguments_nr("array_diff_uassoc", count+1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_array_udiff_assoc(int, HPHP::Variant const&, HPHP::Variant const&, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP19f_array_udiff_assocEiRKNS_7VariantES2_S2_RKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
array1 => rdx
array2 => rcx
data_compare_func => r8
_argv => r9
*/

TypedValue* fh_array_udiff_assoc(TypedValue* _rv, int64_t _argc, TypedValue* array1, TypedValue* array2, TypedValue* data_compare_func, Value* _argv) asm("_ZN4HPHP19f_array_udiff_assocEiRKNS_7VariantES2_S2_RKNS_5ArrayE");

TypedValue* fg_array_udiff_assoc(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 3LL) {
      Array extraArgs;
      {
        ArrayInit ai(count-3);
        for (int64_t i = 3; i < count; ++i) {
          TypedValue* extraArg = ar->getExtraArg(i-3);
          if (tvIsStronglyBound(extraArg)) {
            ai.setRef(i-3, tvAsVariant(extraArg));
          } else {
            ai.set(i-3, tvAsVariant(extraArg));
          }
        }
        extraArgs = ai.create();
      }
      fh_array_udiff_assoc((&(rv)), (count), (args-0), (args-1), (args-2), (Value*)(&extraArgs));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 3);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_missing_arguments_nr("array_udiff_assoc", count+1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_array_udiff_uassoc(int, HPHP::Variant const&, HPHP::Variant const&, HPHP::Variant const&, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP20f_array_udiff_uassocEiRKNS_7VariantES2_S2_S2_RKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
array1 => rdx
array2 => rcx
data_compare_func => r8
key_compare_func => r9
_argv => st0
*/

TypedValue* fh_array_udiff_uassoc(TypedValue* _rv, int64_t _argc, TypedValue* array1, TypedValue* array2, TypedValue* data_compare_func, TypedValue* key_compare_func, Value* _argv) asm("_ZN4HPHP20f_array_udiff_uassocEiRKNS_7VariantES2_S2_S2_RKNS_5ArrayE");

TypedValue* fg_array_udiff_uassoc(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 4LL) {
      Array extraArgs;
      {
        ArrayInit ai(count-4);
        for (int64_t i = 4; i < count; ++i) {
          TypedValue* extraArg = ar->getExtraArg(i-4);
          if (tvIsStronglyBound(extraArg)) {
            ai.setRef(i-4, tvAsVariant(extraArg));
          } else {
            ai.set(i-4, tvAsVariant(extraArg));
          }
        }
        extraArgs = ai.create();
      }
      fh_array_udiff_uassoc((&(rv)), (count), (args-0), (args-1), (args-2), (args-3), (Value*)(&extraArgs));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 4);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_missing_arguments_nr("array_udiff_uassoc", count+1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_array_diff_key(int, HPHP::Variant const&, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP16f_array_diff_keyEiRKNS_7VariantES2_RKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
array1 => rdx
array2 => rcx
_argv => r8
*/

TypedValue* fh_array_diff_key(TypedValue* _rv, int64_t _argc, TypedValue* array1, TypedValue* array2, Value* _argv) asm("_ZN4HPHP16f_array_diff_keyEiRKNS_7VariantES2_RKNS_5ArrayE");

TypedValue* fg_array_diff_key(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL) {
      Array extraArgs;
      {
        ArrayInit ai(count-2);
        for (int64_t i = 2; i < count; ++i) {
          TypedValue* extraArg = ar->getExtraArg(i-2);
          if (tvIsStronglyBound(extraArg)) {
            ai.setRef(i-2, tvAsVariant(extraArg));
          } else {
            ai.set(i-2, tvAsVariant(extraArg));
          }
        }
        extraArgs = ai.create();
      }
      fh_array_diff_key((&(rv)), (count), (args-0), (args-1), (Value*)(&extraArgs));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 2);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_missing_arguments_nr("array_diff_key", count+1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_array_diff_ukey(int, HPHP::Variant const&, HPHP::Variant const&, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP17f_array_diff_ukeyEiRKNS_7VariantES2_S2_RKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
array1 => rdx
array2 => rcx
key_compare_func => r8
_argv => r9
*/

TypedValue* fh_array_diff_ukey(TypedValue* _rv, int64_t _argc, TypedValue* array1, TypedValue* array2, TypedValue* key_compare_func, Value* _argv) asm("_ZN4HPHP17f_array_diff_ukeyEiRKNS_7VariantES2_S2_RKNS_5ArrayE");

TypedValue* fg_array_diff_ukey(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 3LL) {
      Array extraArgs;
      {
        ArrayInit ai(count-3);
        for (int64_t i = 3; i < count; ++i) {
          TypedValue* extraArg = ar->getExtraArg(i-3);
          if (tvIsStronglyBound(extraArg)) {
            ai.setRef(i-3, tvAsVariant(extraArg));
          } else {
            ai.set(i-3, tvAsVariant(extraArg));
          }
        }
        extraArgs = ai.create();
      }
      fh_array_diff_ukey((&(rv)), (count), (args-0), (args-1), (args-2), (Value*)(&extraArgs));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 3);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_missing_arguments_nr("array_diff_ukey", count+1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_array_intersect(int, HPHP::Variant const&, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP17f_array_intersectEiRKNS_7VariantES2_RKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
array1 => rdx
array2 => rcx
_argv => r8
*/

TypedValue* fh_array_intersect(TypedValue* _rv, int64_t _argc, TypedValue* array1, TypedValue* array2, Value* _argv) asm("_ZN4HPHP17f_array_intersectEiRKNS_7VariantES2_RKNS_5ArrayE");

TypedValue* fg_array_intersect(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL) {
      Array extraArgs;
      {
        ArrayInit ai(count-2);
        for (int64_t i = 2; i < count; ++i) {
          TypedValue* extraArg = ar->getExtraArg(i-2);
          if (tvIsStronglyBound(extraArg)) {
            ai.setRef(i-2, tvAsVariant(extraArg));
          } else {
            ai.set(i-2, tvAsVariant(extraArg));
          }
        }
        extraArgs = ai.create();
      }
      fh_array_intersect((&(rv)), (count), (args-0), (args-1), (Value*)(&extraArgs));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 2);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_missing_arguments_nr("array_intersect", count+1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_array_uintersect(int, HPHP::Variant const&, HPHP::Variant const&, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP18f_array_uintersectEiRKNS_7VariantES2_S2_RKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
array1 => rdx
array2 => rcx
data_compare_func => r8
_argv => r9
*/

TypedValue* fh_array_uintersect(TypedValue* _rv, int64_t _argc, TypedValue* array1, TypedValue* array2, TypedValue* data_compare_func, Value* _argv) asm("_ZN4HPHP18f_array_uintersectEiRKNS_7VariantES2_S2_RKNS_5ArrayE");

TypedValue* fg_array_uintersect(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 3LL) {
      Array extraArgs;
      {
        ArrayInit ai(count-3);
        for (int64_t i = 3; i < count; ++i) {
          TypedValue* extraArg = ar->getExtraArg(i-3);
          if (tvIsStronglyBound(extraArg)) {
            ai.setRef(i-3, tvAsVariant(extraArg));
          } else {
            ai.set(i-3, tvAsVariant(extraArg));
          }
        }
        extraArgs = ai.create();
      }
      fh_array_uintersect((&(rv)), (count), (args-0), (args-1), (args-2), (Value*)(&extraArgs));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 3);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_missing_arguments_nr("array_uintersect", count+1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_array_intersect_assoc(int, HPHP::Variant const&, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP23f_array_intersect_assocEiRKNS_7VariantES2_RKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
array1 => rdx
array2 => rcx
_argv => r8
*/

TypedValue* fh_array_intersect_assoc(TypedValue* _rv, int64_t _argc, TypedValue* array1, TypedValue* array2, Value* _argv) asm("_ZN4HPHP23f_array_intersect_assocEiRKNS_7VariantES2_RKNS_5ArrayE");

TypedValue* fg_array_intersect_assoc(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL) {
      Array extraArgs;
      {
        ArrayInit ai(count-2);
        for (int64_t i = 2; i < count; ++i) {
          TypedValue* extraArg = ar->getExtraArg(i-2);
          if (tvIsStronglyBound(extraArg)) {
            ai.setRef(i-2, tvAsVariant(extraArg));
          } else {
            ai.set(i-2, tvAsVariant(extraArg));
          }
        }
        extraArgs = ai.create();
      }
      fh_array_intersect_assoc((&(rv)), (count), (args-0), (args-1), (Value*)(&extraArgs));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 2);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_missing_arguments_nr("array_intersect_assoc", count+1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_array_intersect_uassoc(int, HPHP::Variant const&, HPHP::Variant const&, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP24f_array_intersect_uassocEiRKNS_7VariantES2_S2_RKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
array1 => rdx
array2 => rcx
key_compare_func => r8
_argv => r9
*/

TypedValue* fh_array_intersect_uassoc(TypedValue* _rv, int64_t _argc, TypedValue* array1, TypedValue* array2, TypedValue* key_compare_func, Value* _argv) asm("_ZN4HPHP24f_array_intersect_uassocEiRKNS_7VariantES2_S2_RKNS_5ArrayE");

TypedValue* fg_array_intersect_uassoc(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 3LL) {
      Array extraArgs;
      {
        ArrayInit ai(count-3);
        for (int64_t i = 3; i < count; ++i) {
          TypedValue* extraArg = ar->getExtraArg(i-3);
          if (tvIsStronglyBound(extraArg)) {
            ai.setRef(i-3, tvAsVariant(extraArg));
          } else {
            ai.set(i-3, tvAsVariant(extraArg));
          }
        }
        extraArgs = ai.create();
      }
      fh_array_intersect_uassoc((&(rv)), (count), (args-0), (args-1), (args-2), (Value*)(&extraArgs));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 3);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_missing_arguments_nr("array_intersect_uassoc", count+1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_array_uintersect_assoc(int, HPHP::Variant const&, HPHP::Variant const&, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP24f_array_uintersect_assocEiRKNS_7VariantES2_S2_RKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
array1 => rdx
array2 => rcx
data_compare_func => r8
_argv => r9
*/

TypedValue* fh_array_uintersect_assoc(TypedValue* _rv, int64_t _argc, TypedValue* array1, TypedValue* array2, TypedValue* data_compare_func, Value* _argv) asm("_ZN4HPHP24f_array_uintersect_assocEiRKNS_7VariantES2_S2_RKNS_5ArrayE");

TypedValue* fg_array_uintersect_assoc(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 3LL) {
      Array extraArgs;
      {
        ArrayInit ai(count-3);
        for (int64_t i = 3; i < count; ++i) {
          TypedValue* extraArg = ar->getExtraArg(i-3);
          if (tvIsStronglyBound(extraArg)) {
            ai.setRef(i-3, tvAsVariant(extraArg));
          } else {
            ai.set(i-3, tvAsVariant(extraArg));
          }
        }
        extraArgs = ai.create();
      }
      fh_array_uintersect_assoc((&(rv)), (count), (args-0), (args-1), (args-2), (Value*)(&extraArgs));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 3);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_missing_arguments_nr("array_uintersect_assoc", count+1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_array_uintersect_uassoc(int, HPHP::Variant const&, HPHP::Variant const&, HPHP::Variant const&, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP25f_array_uintersect_uassocEiRKNS_7VariantES2_S2_S2_RKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
array1 => rdx
array2 => rcx
data_compare_func => r8
key_compare_func => r9
_argv => st0
*/

TypedValue* fh_array_uintersect_uassoc(TypedValue* _rv, int64_t _argc, TypedValue* array1, TypedValue* array2, TypedValue* data_compare_func, TypedValue* key_compare_func, Value* _argv) asm("_ZN4HPHP25f_array_uintersect_uassocEiRKNS_7VariantES2_S2_S2_RKNS_5ArrayE");

TypedValue* fg_array_uintersect_uassoc(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 4LL) {
      Array extraArgs;
      {
        ArrayInit ai(count-4);
        for (int64_t i = 4; i < count; ++i) {
          TypedValue* extraArg = ar->getExtraArg(i-4);
          if (tvIsStronglyBound(extraArg)) {
            ai.setRef(i-4, tvAsVariant(extraArg));
          } else {
            ai.set(i-4, tvAsVariant(extraArg));
          }
        }
        extraArgs = ai.create();
      }
      fh_array_uintersect_uassoc((&(rv)), (count), (args-0), (args-1), (args-2), (args-3), (Value*)(&extraArgs));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 4);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_missing_arguments_nr("array_uintersect_uassoc", count+1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_array_intersect_key(int, HPHP::Variant const&, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP21f_array_intersect_keyEiRKNS_7VariantES2_RKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
array1 => rdx
array2 => rcx
_argv => r8
*/

TypedValue* fh_array_intersect_key(TypedValue* _rv, int64_t _argc, TypedValue* array1, TypedValue* array2, Value* _argv) asm("_ZN4HPHP21f_array_intersect_keyEiRKNS_7VariantES2_RKNS_5ArrayE");

TypedValue* fg_array_intersect_key(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL) {
      Array extraArgs;
      {
        ArrayInit ai(count-2);
        for (int64_t i = 2; i < count; ++i) {
          TypedValue* extraArg = ar->getExtraArg(i-2);
          if (tvIsStronglyBound(extraArg)) {
            ai.setRef(i-2, tvAsVariant(extraArg));
          } else {
            ai.set(i-2, tvAsVariant(extraArg));
          }
        }
        extraArgs = ai.create();
      }
      fh_array_intersect_key((&(rv)), (count), (args-0), (args-1), (Value*)(&extraArgs));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 2);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_missing_arguments_nr("array_intersect_key", count+1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_array_intersect_ukey(int, HPHP::Variant const&, HPHP::Variant const&, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP22f_array_intersect_ukeyEiRKNS_7VariantES2_S2_RKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
array1 => rdx
array2 => rcx
key_compare_func => r8
_argv => r9
*/

TypedValue* fh_array_intersect_ukey(TypedValue* _rv, int64_t _argc, TypedValue* array1, TypedValue* array2, TypedValue* key_compare_func, Value* _argv) asm("_ZN4HPHP22f_array_intersect_ukeyEiRKNS_7VariantES2_S2_RKNS_5ArrayE");

TypedValue* fg_array_intersect_ukey(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 3LL) {
      Array extraArgs;
      {
        ArrayInit ai(count-3);
        for (int64_t i = 3; i < count; ++i) {
          TypedValue* extraArg = ar->getExtraArg(i-3);
          if (tvIsStronglyBound(extraArg)) {
            ai.setRef(i-3, tvAsVariant(extraArg));
          } else {
            ai.set(i-3, tvAsVariant(extraArg));
          }
        }
        extraArgs = ai.create();
      }
      fh_array_intersect_ukey((&(rv)), (count), (args-0), (args-1), (args-2), (Value*)(&extraArgs));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 3);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_missing_arguments_nr("array_intersect_ukey", count+1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_sort(HPHP::VRefParamValue const&, int, bool)
_ZN4HPHP6f_sortERKNS_14VRefParamValueEib

(return value) => rax
array => rdi
sort_flags => rsi
use_collator => rdx
*/

bool fh_sort(TypedValue* array, int sort_flags, bool use_collator) asm("_ZN4HPHP6f_sortERKNS_14VRefParamValueEib");

TypedValue * fg1_sort(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_sort(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
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
  rv->m_data.num = (fh_sort((args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_sort(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfBoolean) && (count <= 1 || (args-1)->m_type == KindOfInt64)) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_sort((args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_sort(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("sort", count, 1, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_rsort(HPHP::VRefParamValue const&, int, bool)
_ZN4HPHP7f_rsortERKNS_14VRefParamValueEib

(return value) => rax
array => rdi
sort_flags => rsi
use_collator => rdx
*/

bool fh_rsort(TypedValue* array, int sort_flags, bool use_collator) asm("_ZN4HPHP7f_rsortERKNS_14VRefParamValueEib");

TypedValue * fg1_rsort(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_rsort(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
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
  rv->m_data.num = (fh_rsort((args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_rsort(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfBoolean) && (count <= 1 || (args-1)->m_type == KindOfInt64)) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_rsort((args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_rsort(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("rsort", count, 1, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_asort(HPHP::VRefParamValue const&, int, bool)
_ZN4HPHP7f_asortERKNS_14VRefParamValueEib

(return value) => rax
array => rdi
sort_flags => rsi
use_collator => rdx
*/

bool fh_asort(TypedValue* array, int sort_flags, bool use_collator) asm("_ZN4HPHP7f_asortERKNS_14VRefParamValueEib");

TypedValue * fg1_asort(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_asort(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
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
  rv->m_data.num = (fh_asort((args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_asort(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfBoolean) && (count <= 1 || (args-1)->m_type == KindOfInt64)) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_asort((args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_asort(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("asort", count, 1, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_arsort(HPHP::VRefParamValue const&, int, bool)
_ZN4HPHP8f_arsortERKNS_14VRefParamValueEib

(return value) => rax
array => rdi
sort_flags => rsi
use_collator => rdx
*/

bool fh_arsort(TypedValue* array, int sort_flags, bool use_collator) asm("_ZN4HPHP8f_arsortERKNS_14VRefParamValueEib");

TypedValue * fg1_arsort(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_arsort(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
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
  rv->m_data.num = (fh_arsort((args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_arsort(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfBoolean) && (count <= 1 || (args-1)->m_type == KindOfInt64)) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_arsort((args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_arsort(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("arsort", count, 1, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_ksort(HPHP::VRefParamValue const&, int)
_ZN4HPHP7f_ksortERKNS_14VRefParamValueEi

(return value) => rax
array => rdi
sort_flags => rsi
*/

bool fh_ksort(TypedValue* array, int sort_flags) asm("_ZN4HPHP7f_ksortERKNS_14VRefParamValueEi");

TypedValue * fg1_ksort(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_ksort(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToInt64InPlace(args-1);
  rv->m_data.num = (fh_ksort((args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_ksort(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64)) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_ksort((args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_ksort(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("ksort", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_krsort(HPHP::VRefParamValue const&, int)
_ZN4HPHP8f_krsortERKNS_14VRefParamValueEi

(return value) => rax
array => rdi
sort_flags => rsi
*/

bool fh_krsort(TypedValue* array, int sort_flags) asm("_ZN4HPHP8f_krsortERKNS_14VRefParamValueEi");

TypedValue * fg1_krsort(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_krsort(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToInt64InPlace(args-1);
  rv->m_data.num = (fh_krsort((args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_krsort(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64)) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_krsort((args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_krsort(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("krsort", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_usort(HPHP::VRefParamValue const&, HPHP::Variant const&)
_ZN4HPHP7f_usortERKNS_14VRefParamValueERKNS_7VariantE

(return value) => rax
array => rdi
cmp_function => rsi
*/

bool fh_usort(TypedValue* array, TypedValue* cmp_function) asm("_ZN4HPHP7f_usortERKNS_14VRefParamValueERKNS_7VariantE");

TypedValue* fg_usort(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      rv.m_type = KindOfBoolean;
      rv.m_data.num = (fh_usort((args-0), (args-1))) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 2);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("usort", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_uasort(HPHP::VRefParamValue const&, HPHP::Variant const&)
_ZN4HPHP8f_uasortERKNS_14VRefParamValueERKNS_7VariantE

(return value) => rax
array => rdi
cmp_function => rsi
*/

bool fh_uasort(TypedValue* array, TypedValue* cmp_function) asm("_ZN4HPHP8f_uasortERKNS_14VRefParamValueERKNS_7VariantE");

TypedValue* fg_uasort(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      rv.m_type = KindOfBoolean;
      rv.m_data.num = (fh_uasort((args-0), (args-1))) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 2);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("uasort", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_uksort(HPHP::VRefParamValue const&, HPHP::Variant const&)
_ZN4HPHP8f_uksortERKNS_14VRefParamValueERKNS_7VariantE

(return value) => rax
array => rdi
cmp_function => rsi
*/

bool fh_uksort(TypedValue* array, TypedValue* cmp_function) asm("_ZN4HPHP8f_uksortERKNS_14VRefParamValueERKNS_7VariantE");

TypedValue* fg_uksort(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      rv.m_type = KindOfBoolean;
      rv.m_data.num = (fh_uksort((args-0), (args-1))) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 2);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("uksort", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_natsort(HPHP::VRefParamValue const&)
_ZN4HPHP9f_natsortERKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
array => rsi
*/

TypedValue* fh_natsort(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP9f_natsortERKNS_14VRefParamValueE");

TypedValue* fg_natsort(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      fh_natsort((&(rv)), (args-0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("natsort", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_natcasesort(HPHP::VRefParamValue const&)
_ZN4HPHP13f_natcasesortERKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
array => rsi
*/

TypedValue* fh_natcasesort(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP13f_natcasesortERKNS_14VRefParamValueE");

TypedValue* fg_natcasesort(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      fh_natcasesort((&(rv)), (args-0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("natcasesort", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_i18n_loc_get_default()
_ZN4HPHP22f_i18n_loc_get_defaultEv

(return value) => rax
_rv => rdi
*/

Value* fh_i18n_loc_get_default(Value* _rv) asm("_ZN4HPHP22f_i18n_loc_get_defaultEv");

TypedValue* fg_i18n_loc_get_default(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_type = KindOfString;
      fh_i18n_loc_get_default((&rv.m_data));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("i18n_loc_get_default", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_i18n_loc_set_default(HPHP::String const&)
_ZN4HPHP22f_i18n_loc_set_defaultERKNS_6StringE

(return value) => rax
locale => rdi
*/

bool fh_i18n_loc_set_default(Value* locale) asm("_ZN4HPHP22f_i18n_loc_set_defaultERKNS_6StringE");

TypedValue * fg1_i18n_loc_set_default(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_i18n_loc_set_default(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToStringInPlace(args-0);
  rv->m_data.num = (fh_i18n_loc_set_default(&args[-0].m_data)) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_i18n_loc_set_default(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_i18n_loc_set_default(&args[-0].m_data)) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_i18n_loc_set_default(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("i18n_loc_set_default", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_i18n_loc_set_attribute(long, long)
_ZN4HPHP24f_i18n_loc_set_attributeEll

(return value) => rax
attr => rdi
val => rsi
*/

bool fh_i18n_loc_set_attribute(long attr, long val) asm("_ZN4HPHP24f_i18n_loc_set_attributeEll");

TypedValue * fg1_i18n_loc_set_attribute(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_i18n_loc_set_attribute(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  rv->m_data.num = (fh_i18n_loc_set_attribute((long)(args[-0].m_data.num), (long)(args[-1].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_i18n_loc_set_attribute(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfInt64) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_i18n_loc_set_attribute((long)(args[-0].m_data.num), (long)(args[-1].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_i18n_loc_set_attribute(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("i18n_loc_set_attribute", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_i18n_loc_set_strength(long)
_ZN4HPHP23f_i18n_loc_set_strengthEl

(return value) => rax
strength => rdi
*/

bool fh_i18n_loc_set_strength(long strength) asm("_ZN4HPHP23f_i18n_loc_set_strengthEl");

TypedValue * fg1_i18n_loc_set_strength(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_i18n_loc_set_strength(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToInt64InPlace(args-0);
  rv->m_data.num = (fh_i18n_loc_set_strength((long)(args[-0].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_i18n_loc_set_strength(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfInt64) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_i18n_loc_set_strength((long)(args[-0].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_i18n_loc_set_strength(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("i18n_loc_set_strength", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_i18n_loc_get_error_code()
_ZN4HPHP25f_i18n_loc_get_error_codeEv

(return value) => rax
_rv => rdi
*/

TypedValue* fh_i18n_loc_get_error_code(TypedValue* _rv) asm("_ZN4HPHP25f_i18n_loc_get_error_codeEv");

TypedValue* fg_i18n_loc_get_error_code(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      fh_i18n_loc_get_error_code((&(rv)));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("i18n_loc_get_error_code", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}




} // !HPHP

