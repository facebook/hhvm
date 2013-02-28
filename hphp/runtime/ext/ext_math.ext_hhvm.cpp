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
HPHP::Variant HPHP::f_min(int, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP5f_minEiRKNS_7VariantERKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
value => rdx
_argv => rcx
*/

TypedValue* fh_min(TypedValue* _rv, int64_t _argc, TypedValue* value, Value* _argv) asm("_ZN4HPHP5f_minEiRKNS_7VariantERKNS_5ArrayE");

TypedValue* fg_min(HPHP::VM::ActRec *ar) {
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
      fh_min((&(rv)), (count), (args-0), (Value*)(&extraArgs));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_missing_arguments_nr("min", count+1, 1);
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
HPHP::Variant HPHP::f_max(int, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP5f_maxEiRKNS_7VariantERKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
value => rdx
_argv => rcx
*/

TypedValue* fh_max(TypedValue* _rv, int64_t _argc, TypedValue* value, Value* _argv) asm("_ZN4HPHP5f_maxEiRKNS_7VariantERKNS_5ArrayE");

TypedValue* fg_max(HPHP::VM::ActRec *ar) {
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
      fh_max((&(rv)), (count), (args-0), (Value*)(&extraArgs));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_missing_arguments_nr("max", count+1, 1);
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
HPHP::Variant HPHP::f_abs(HPHP::Variant const&)
_ZN4HPHP5f_absERKNS_7VariantE

(return value) => rax
_rv => rdi
number => rsi
*/

TypedValue* fh_abs(TypedValue* _rv, TypedValue* number) asm("_ZN4HPHP5f_absERKNS_7VariantE");

TypedValue* fg_abs(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      fh_abs((&(rv)), (args-0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("abs", count, 1, 1, 1);
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
double HPHP::f_round(HPHP::Variant const&, long, long)
_ZN4HPHP7f_roundERKNS_7VariantEll

(return value) => xmm0
val => rdi
precision => rsi
mode => rdx
*/

double fh_round(TypedValue* val, long precision, long mode) asm("_ZN4HPHP7f_roundERKNS_7VariantEll");

TypedValue * fg1_round(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_round(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfDouble;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
    break;
  }
  rv->m_data.dbl = fh_round((args-0), (count > 1) ? (long)(args[-1].m_data.num) : (long)(0), (count > 2) ? (long)(args[-2].m_data.num) : (long)(1));
  return rv;
}

TypedValue* fg_round(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfInt64) && (count <= 1 || (args-1)->m_type == KindOfInt64)) {
        rv._count = 0;
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_round((args-0), (count > 1) ? (long)(args[-1].m_data.num) : (long)(0), (count > 2) ? (long)(args[-2].m_data.num) : (long)(1));
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_round(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("round", count, 1, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_base_convert(HPHP::String const&, long, long)
_ZN4HPHP14f_base_convertERKNS_6StringEll

(return value) => rax
_rv => rdi
number => rsi
frombase => rdx
tobase => rcx
*/

TypedValue* fh_base_convert(TypedValue* _rv, Value* number, long frombase, long tobase) asm("_ZN4HPHP14f_base_convertERKNS_6StringEll");

TypedValue * fg1_base_convert(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_base_convert(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_base_convert((rv), (Value*)(args-0), (long)(args[-1].m_data.num), (long)(args[-2].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_base_convert(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64 && IS_STRING_TYPE((args-0)->m_type)) {
        fh_base_convert((&(rv)), (Value*)(args-0), (long)(args[-1].m_data.num), (long)(args[-2].m_data.num));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_base_convert(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("base_convert", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_pow(HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP5f_powERKNS_7VariantES2_

(return value) => rax
_rv => rdi
base => rsi
exp => rdx
*/

TypedValue* fh_pow(TypedValue* _rv, TypedValue* base, TypedValue* exp) asm("_ZN4HPHP5f_powERKNS_7VariantES2_");

TypedValue* fg_pow(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      fh_pow((&(rv)), (args-0), (args-1));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 2);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("pow", count, 2, 2, 1);
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
void HPHP::f_srand(HPHP::Variant const&)
_ZN4HPHP7f_srandERKNS_7VariantE

seed => rdi
*/

void fh_srand(TypedValue* seed) asm("_ZN4HPHP7f_srandERKNS_7VariantE");

TypedValue* fg_srand(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      rv.m_data.num = 0LL;
      rv._count = 0;
      rv.m_type = KindOfNull;
      fh_srand((count > 0) ? (args-0) : (TypedValue*)(&null_variant));
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("srand", 1, 1);
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
long HPHP::f_rand(long, long)
_ZN4HPHP6f_randEll

(return value) => rax
min => rdi
max => rsi
*/

long fh_rand(long min, long max) asm("_ZN4HPHP6f_randEll");

TypedValue * fg1_rand(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_rand(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfInt64;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
    if ((args-0)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-0);
    }
  case 0:
    break;
  }
  rv->m_data.num = (int64_t)fh_rand((count > 0) ? (long)(args[-0].m_data.num) : (long)(0), (count > 1) ? (long)(args[-1].m_data.num) : (long)(RAND_MAX));
  return rv;
}

TypedValue* fg_rand(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64) && (count <= 0 || (args-0)->m_type == KindOfInt64)) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_rand((count > 0) ? (long)(args[-0].m_data.num) : (long)(0), (count > 1) ? (long)(args[-1].m_data.num) : (long)(RAND_MAX));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_rand(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("rand", 2, 1);
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
void HPHP::f_mt_srand(HPHP::Variant const&)
_ZN4HPHP10f_mt_srandERKNS_7VariantE

seed => rdi
*/

void fh_mt_srand(TypedValue* seed) asm("_ZN4HPHP10f_mt_srandERKNS_7VariantE");

TypedValue* fg_mt_srand(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      rv.m_data.num = 0LL;
      rv._count = 0;
      rv.m_type = KindOfNull;
      fh_mt_srand((count > 0) ? (args-0) : (TypedValue*)(&null_variant));
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("mt_srand", 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}




} // !HPHP

