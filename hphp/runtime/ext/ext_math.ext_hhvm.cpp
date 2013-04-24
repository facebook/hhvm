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
double HPHP::f_pi()
_ZN4HPHP4f_piEv

(return value) => xmm0
*/

double fh_pi() asm("_ZN4HPHP4f_piEv");

TypedValue* fg_pi(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_type = KindOfDouble;
      rv.m_data.dbl = fh_pi();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("pi", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



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

TypedValue* fg_min(ActRec *ar) {
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
      throw_missing_arguments_nr("min", 1, count, 1);
    }
    rv.m_data.num = 0LL;
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

TypedValue* fg_max(ActRec *ar) {
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
      throw_missing_arguments_nr("max", 1, count, 1);
    }
    rv.m_data.num = 0LL;
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

TypedValue* fg_abs(ActRec *ar) {
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
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_is_finite(double)
_ZN4HPHP11f_is_finiteEd

(return value) => rax
val => xmm0
*/

bool fh_is_finite(double val) asm("_ZN4HPHP11f_is_finiteEd");

TypedValue * fg1_is_finite(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_is_finite(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToDoubleInPlace(args-0);
  rv->m_data.num = (fh_is_finite((args[-0].m_data.dbl))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_is_finite(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfDouble) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_is_finite((args[-0].m_data.dbl))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_is_finite(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("is_finite", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_is_infinite(double)
_ZN4HPHP13f_is_infiniteEd

(return value) => rax
val => xmm0
*/

bool fh_is_infinite(double val) asm("_ZN4HPHP13f_is_infiniteEd");

TypedValue * fg1_is_infinite(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_is_infinite(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToDoubleInPlace(args-0);
  rv->m_data.num = (fh_is_infinite((args[-0].m_data.dbl))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_is_infinite(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfDouble) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_is_infinite((args[-0].m_data.dbl))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_is_infinite(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("is_infinite", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_is_nan(double)
_ZN4HPHP8f_is_nanEd

(return value) => rax
val => xmm0
*/

bool fh_is_nan(double val) asm("_ZN4HPHP8f_is_nanEd");

TypedValue * fg1_is_nan(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_is_nan(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToDoubleInPlace(args-0);
  rv->m_data.num = (fh_is_nan((args[-0].m_data.dbl))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_is_nan(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfDouble) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_is_nan((args[-0].m_data.dbl))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_is_nan(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("is_nan", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_ceil(double)
_ZN4HPHP6f_ceilEd

(return value) => xmm0
value => xmm0
*/

double fh_ceil(double value) asm("_ZN4HPHP6f_ceilEd");

TypedValue * fg1_ceil(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_ceil(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToDoubleInPlace(args-0);
  rv->m_data.dbl = fh_ceil((args[-0].m_data.dbl));
  return rv;
}

TypedValue* fg_ceil(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfDouble) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_ceil((args[-0].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_ceil(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("ceil", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_floor(double)
_ZN4HPHP7f_floorEd

(return value) => xmm0
value => xmm0
*/

double fh_floor(double value) asm("_ZN4HPHP7f_floorEd");

TypedValue * fg1_floor(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_floor(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToDoubleInPlace(args-0);
  rv->m_data.dbl = fh_floor((args[-0].m_data.dbl));
  return rv;
}

TypedValue* fg_floor(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfDouble) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_floor((args[-0].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_floor(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("floor", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
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

TypedValue * fg1_round(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_round(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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

TypedValue* fg_round(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfInt64) && (count <= 1 || (args-1)->m_type == KindOfInt64)) {
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
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_deg2rad(double)
_ZN4HPHP9f_deg2radEd

(return value) => xmm0
number => xmm0
*/

double fh_deg2rad(double number) asm("_ZN4HPHP9f_deg2radEd");

TypedValue * fg1_deg2rad(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_deg2rad(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToDoubleInPlace(args-0);
  rv->m_data.dbl = fh_deg2rad((args[-0].m_data.dbl));
  return rv;
}

TypedValue* fg_deg2rad(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfDouble) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_deg2rad((args[-0].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_deg2rad(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("deg2rad", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_rad2deg(double)
_ZN4HPHP9f_rad2degEd

(return value) => xmm0
number => xmm0
*/

double fh_rad2deg(double number) asm("_ZN4HPHP9f_rad2degEd");

TypedValue * fg1_rad2deg(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_rad2deg(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToDoubleInPlace(args-0);
  rv->m_data.dbl = fh_rad2deg((args[-0].m_data.dbl));
  return rv;
}

TypedValue* fg_rad2deg(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfDouble) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_rad2deg((args[-0].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_rad2deg(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("rad2deg", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_decbin(long)
_ZN4HPHP8f_decbinEl

(return value) => rax
_rv => rdi
number => rsi
*/

Value* fh_decbin(Value* _rv, long number) asm("_ZN4HPHP8f_decbinEl");

TypedValue * fg1_decbin(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_decbin(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfString;
  tvCastToInt64InPlace(args-0);
  fh_decbin((&rv->m_data), (long)(args[-0].m_data.num));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_decbin(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfInt64) {
        rv.m_type = KindOfString;
        fh_decbin((&rv.m_data), (long)(args[-0].m_data.num));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_decbin(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("decbin", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_dechex(long)
_ZN4HPHP8f_dechexEl

(return value) => rax
_rv => rdi
number => rsi
*/

Value* fh_dechex(Value* _rv, long number) asm("_ZN4HPHP8f_dechexEl");

TypedValue * fg1_dechex(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dechex(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfString;
  tvCastToInt64InPlace(args-0);
  fh_dechex((&rv->m_data), (long)(args[-0].m_data.num));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dechex(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfInt64) {
        rv.m_type = KindOfString;
        fh_dechex((&rv.m_data), (long)(args[-0].m_data.num));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dechex(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dechex", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_decoct(long)
_ZN4HPHP8f_decoctEl

(return value) => rax
_rv => rdi
number => rsi
*/

Value* fh_decoct(Value* _rv, long number) asm("_ZN4HPHP8f_decoctEl");

TypedValue * fg1_decoct(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_decoct(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfString;
  tvCastToInt64InPlace(args-0);
  fh_decoct((&rv->m_data), (long)(args[-0].m_data.num));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_decoct(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfInt64) {
        rv.m_type = KindOfString;
        fh_decoct((&rv.m_data), (long)(args[-0].m_data.num));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_decoct(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("decoct", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_bindec(HPHP::String const&)
_ZN4HPHP8f_bindecERKNS_6StringE

(return value) => rax
_rv => rdi
binary_string => rsi
*/

TypedValue* fh_bindec(TypedValue* _rv, Value* binary_string) asm("_ZN4HPHP8f_bindecERKNS_6StringE");

TypedValue * fg1_bindec(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_bindec(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_bindec((rv), &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_bindec(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        fh_bindec((&(rv)), &args[-0].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_bindec(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("bindec", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_hexdec(HPHP::String const&)
_ZN4HPHP8f_hexdecERKNS_6StringE

(return value) => rax
_rv => rdi
hex_string => rsi
*/

TypedValue* fh_hexdec(TypedValue* _rv, Value* hex_string) asm("_ZN4HPHP8f_hexdecERKNS_6StringE");

TypedValue * fg1_hexdec(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hexdec(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_hexdec((rv), &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_hexdec(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        fh_hexdec((&(rv)), &args[-0].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hexdec(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hexdec", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_octdec(HPHP::String const&)
_ZN4HPHP8f_octdecERKNS_6StringE

(return value) => rax
_rv => rdi
octal_string => rsi
*/

TypedValue* fh_octdec(TypedValue* _rv, Value* octal_string) asm("_ZN4HPHP8f_octdecERKNS_6StringE");

TypedValue * fg1_octdec(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_octdec(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_octdec((rv), &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_octdec(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        fh_octdec((&(rv)), &args[-0].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_octdec(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("octdec", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
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

TypedValue * fg1_base_convert(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_base_convert(TypedValue* rv, ActRec* ar, int64_t count) {
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
  fh_base_convert((rv), &args[-0].m_data, (long)(args[-1].m_data.num), (long)(args[-2].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_base_convert(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64 && IS_STRING_TYPE((args-0)->m_type)) {
        fh_base_convert((&(rv)), &args[-0].m_data, (long)(args[-1].m_data.num), (long)(args[-2].m_data.num));
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

TypedValue* fg_pow(ActRec *ar) {
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
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_exp(double)
_ZN4HPHP5f_expEd

(return value) => xmm0
arg => xmm0
*/

double fh_exp(double arg) asm("_ZN4HPHP5f_expEd");

TypedValue * fg1_exp(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_exp(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToDoubleInPlace(args-0);
  rv->m_data.dbl = fh_exp((args[-0].m_data.dbl));
  return rv;
}

TypedValue* fg_exp(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfDouble) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_exp((args[-0].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_exp(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("exp", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_expm1(double)
_ZN4HPHP7f_expm1Ed

(return value) => xmm0
arg => xmm0
*/

double fh_expm1(double arg) asm("_ZN4HPHP7f_expm1Ed");

TypedValue * fg1_expm1(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_expm1(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToDoubleInPlace(args-0);
  rv->m_data.dbl = fh_expm1((args[-0].m_data.dbl));
  return rv;
}

TypedValue* fg_expm1(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfDouble) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_expm1((args[-0].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_expm1(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("expm1", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_log10(double)
_ZN4HPHP7f_log10Ed

(return value) => xmm0
arg => xmm0
*/

double fh_log10(double arg) asm("_ZN4HPHP7f_log10Ed");

TypedValue * fg1_log10(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_log10(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToDoubleInPlace(args-0);
  rv->m_data.dbl = fh_log10((args[-0].m_data.dbl));
  return rv;
}

TypedValue* fg_log10(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfDouble) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_log10((args[-0].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_log10(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("log10", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_log1p(double)
_ZN4HPHP7f_log1pEd

(return value) => xmm0
number => xmm0
*/

double fh_log1p(double number) asm("_ZN4HPHP7f_log1pEd");

TypedValue * fg1_log1p(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_log1p(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToDoubleInPlace(args-0);
  rv->m_data.dbl = fh_log1p((args[-0].m_data.dbl));
  return rv;
}

TypedValue* fg_log1p(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfDouble) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_log1p((args[-0].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_log1p(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("log1p", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_log(double, double)
_ZN4HPHP5f_logEdd

(return value) => xmm0
arg => xmm0
base => xmm1
*/

double fh_log(double arg, double base) asm("_ZN4HPHP5f_logEdd");

TypedValue * fg1_log(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_log(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfDouble) {
      tvCastToDoubleInPlace(args-1);
    }
  case 1:
    break;
  }
  if ((args-0)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-0);
  }
  rv->m_data.dbl = fh_log((args[-0].m_data.dbl), (count > 1) ? (args[-1].m_data.dbl) : (double)(0));
  return rv;
}

TypedValue* fg_log(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfDouble) && (args-0)->m_type == KindOfDouble) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_log((args[-0].m_data.dbl), (count > 1) ? (args[-1].m_data.dbl) : (double)(0));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_log(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("log", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_cos(double)
_ZN4HPHP5f_cosEd

(return value) => xmm0
arg => xmm0
*/

double fh_cos(double arg) asm("_ZN4HPHP5f_cosEd");

TypedValue * fg1_cos(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_cos(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToDoubleInPlace(args-0);
  rv->m_data.dbl = fh_cos((args[-0].m_data.dbl));
  return rv;
}

TypedValue* fg_cos(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfDouble) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_cos((args[-0].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_cos(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("cos", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_cosh(double)
_ZN4HPHP6f_coshEd

(return value) => xmm0
arg => xmm0
*/

double fh_cosh(double arg) asm("_ZN4HPHP6f_coshEd");

TypedValue * fg1_cosh(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_cosh(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToDoubleInPlace(args-0);
  rv->m_data.dbl = fh_cosh((args[-0].m_data.dbl));
  return rv;
}

TypedValue* fg_cosh(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfDouble) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_cosh((args[-0].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_cosh(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("cosh", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_sin(double)
_ZN4HPHP5f_sinEd

(return value) => xmm0
arg => xmm0
*/

double fh_sin(double arg) asm("_ZN4HPHP5f_sinEd");

TypedValue * fg1_sin(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_sin(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToDoubleInPlace(args-0);
  rv->m_data.dbl = fh_sin((args[-0].m_data.dbl));
  return rv;
}

TypedValue* fg_sin(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfDouble) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_sin((args[-0].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_sin(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("sin", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_sinh(double)
_ZN4HPHP6f_sinhEd

(return value) => xmm0
arg => xmm0
*/

double fh_sinh(double arg) asm("_ZN4HPHP6f_sinhEd");

TypedValue * fg1_sinh(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_sinh(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToDoubleInPlace(args-0);
  rv->m_data.dbl = fh_sinh((args[-0].m_data.dbl));
  return rv;
}

TypedValue* fg_sinh(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfDouble) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_sinh((args[-0].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_sinh(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("sinh", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_tan(double)
_ZN4HPHP5f_tanEd

(return value) => xmm0
arg => xmm0
*/

double fh_tan(double arg) asm("_ZN4HPHP5f_tanEd");

TypedValue * fg1_tan(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_tan(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToDoubleInPlace(args-0);
  rv->m_data.dbl = fh_tan((args[-0].m_data.dbl));
  return rv;
}

TypedValue* fg_tan(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfDouble) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_tan((args[-0].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_tan(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("tan", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_tanh(double)
_ZN4HPHP6f_tanhEd

(return value) => xmm0
arg => xmm0
*/

double fh_tanh(double arg) asm("_ZN4HPHP6f_tanhEd");

TypedValue * fg1_tanh(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_tanh(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToDoubleInPlace(args-0);
  rv->m_data.dbl = fh_tanh((args[-0].m_data.dbl));
  return rv;
}

TypedValue* fg_tanh(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfDouble) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_tanh((args[-0].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_tanh(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("tanh", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_acos(double)
_ZN4HPHP6f_acosEd

(return value) => xmm0
arg => xmm0
*/

double fh_acos(double arg) asm("_ZN4HPHP6f_acosEd");

TypedValue * fg1_acos(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_acos(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToDoubleInPlace(args-0);
  rv->m_data.dbl = fh_acos((args[-0].m_data.dbl));
  return rv;
}

TypedValue* fg_acos(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfDouble) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_acos((args[-0].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_acos(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("acos", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_acosh(double)
_ZN4HPHP7f_acoshEd

(return value) => xmm0
arg => xmm0
*/

double fh_acosh(double arg) asm("_ZN4HPHP7f_acoshEd");

TypedValue * fg1_acosh(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_acosh(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToDoubleInPlace(args-0);
  rv->m_data.dbl = fh_acosh((args[-0].m_data.dbl));
  return rv;
}

TypedValue* fg_acosh(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfDouble) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_acosh((args[-0].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_acosh(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("acosh", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_asin(double)
_ZN4HPHP6f_asinEd

(return value) => xmm0
arg => xmm0
*/

double fh_asin(double arg) asm("_ZN4HPHP6f_asinEd");

TypedValue * fg1_asin(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_asin(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToDoubleInPlace(args-0);
  rv->m_data.dbl = fh_asin((args[-0].m_data.dbl));
  return rv;
}

TypedValue* fg_asin(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfDouble) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_asin((args[-0].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_asin(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("asin", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_asinh(double)
_ZN4HPHP7f_asinhEd

(return value) => xmm0
arg => xmm0
*/

double fh_asinh(double arg) asm("_ZN4HPHP7f_asinhEd");

TypedValue * fg1_asinh(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_asinh(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToDoubleInPlace(args-0);
  rv->m_data.dbl = fh_asinh((args[-0].m_data.dbl));
  return rv;
}

TypedValue* fg_asinh(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfDouble) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_asinh((args[-0].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_asinh(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("asinh", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_atan(double)
_ZN4HPHP6f_atanEd

(return value) => xmm0
arg => xmm0
*/

double fh_atan(double arg) asm("_ZN4HPHP6f_atanEd");

TypedValue * fg1_atan(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_atan(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToDoubleInPlace(args-0);
  rv->m_data.dbl = fh_atan((args[-0].m_data.dbl));
  return rv;
}

TypedValue* fg_atan(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfDouble) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_atan((args[-0].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_atan(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("atan", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_atanh(double)
_ZN4HPHP7f_atanhEd

(return value) => xmm0
arg => xmm0
*/

double fh_atanh(double arg) asm("_ZN4HPHP7f_atanhEd");

TypedValue * fg1_atanh(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_atanh(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToDoubleInPlace(args-0);
  rv->m_data.dbl = fh_atanh((args[-0].m_data.dbl));
  return rv;
}

TypedValue* fg_atanh(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfDouble) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_atanh((args[-0].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_atanh(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("atanh", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_atan2(double, double)
_ZN4HPHP7f_atan2Edd

(return value) => xmm0
y => xmm0
x => xmm1
*/

double fh_atan2(double y, double x) asm("_ZN4HPHP7f_atan2Edd");

TypedValue * fg1_atan2(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_atan2(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-0);
  }
  rv->m_data.dbl = fh_atan2((args[-0].m_data.dbl), (args[-1].m_data.dbl));
  return rv;
}

TypedValue* fg_atan2(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfDouble) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_atan2((args[-0].m_data.dbl), (args[-1].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_atan2(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("atan2", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_hypot(double, double)
_ZN4HPHP7f_hypotEdd

(return value) => xmm0
x => xmm0
y => xmm1
*/

double fh_hypot(double x, double y) asm("_ZN4HPHP7f_hypotEdd");

TypedValue * fg1_hypot(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hypot(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-0);
  }
  rv->m_data.dbl = fh_hypot((args[-0].m_data.dbl), (args[-1].m_data.dbl));
  return rv;
}

TypedValue* fg_hypot(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfDouble) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_hypot((args[-0].m_data.dbl), (args[-1].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hypot(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hypot", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_fmod(double, double)
_ZN4HPHP6f_fmodEdd

(return value) => xmm0
x => xmm0
y => xmm1
*/

double fh_fmod(double x, double y) asm("_ZN4HPHP6f_fmodEdd");

TypedValue * fg1_fmod(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_fmod(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-0);
  }
  rv->m_data.dbl = fh_fmod((args[-0].m_data.dbl), (args[-1].m_data.dbl));
  return rv;
}

TypedValue* fg_fmod(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfDouble) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_fmod((args[-0].m_data.dbl), (args[-1].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_fmod(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("fmod", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_sqrt(double)
_ZN4HPHP6f_sqrtEd

(return value) => xmm0
arg => xmm0
*/

double fh_sqrt(double arg) asm("_ZN4HPHP6f_sqrtEd");

TypedValue * fg1_sqrt(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_sqrt(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToDoubleInPlace(args-0);
  rv->m_data.dbl = fh_sqrt((args[-0].m_data.dbl));
  return rv;
}

TypedValue* fg_sqrt(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfDouble) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_sqrt((args[-0].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_sqrt(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("sqrt", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_getrandmax()
_ZN4HPHP12f_getrandmaxEv

(return value) => rax
*/

long fh_getrandmax() asm("_ZN4HPHP12f_getrandmaxEv");

TypedValue* fg_getrandmax(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_type = KindOfInt64;
      rv.m_data.num = (int64_t)fh_getrandmax();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("getrandmax", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
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

TypedValue* fg_srand(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      rv.m_data.num = 0LL;
      rv.m_type = KindOfNull;
      fh_srand((count > 0) ? (args-0) : (TypedValue*)(&null_variant));
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("srand", 1, 1);
    }
    rv.m_data.num = 0LL;
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

TypedValue * fg1_rand(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_rand(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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

TypedValue* fg_rand(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64) && (count <= 0 || (args-0)->m_type == KindOfInt64)) {
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
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_mt_getrandmax()
_ZN4HPHP15f_mt_getrandmaxEv

(return value) => rax
*/

long fh_mt_getrandmax() asm("_ZN4HPHP15f_mt_getrandmaxEv");

TypedValue* fg_mt_getrandmax(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_type = KindOfInt64;
      rv.m_data.num = (int64_t)fh_mt_getrandmax();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("mt_getrandmax", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
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

TypedValue* fg_mt_srand(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      rv.m_data.num = 0LL;
      rv.m_type = KindOfNull;
      fh_mt_srand((count > 0) ? (args-0) : (TypedValue*)(&null_variant));
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("mt_srand", 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_mt_rand(long, long)
_ZN4HPHP9f_mt_randEll

(return value) => rax
min => rdi
max => rsi
*/

long fh_mt_rand(long min, long max) asm("_ZN4HPHP9f_mt_randEll");

TypedValue * fg1_mt_rand(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mt_rand(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_data.num = (int64_t)fh_mt_rand((count > 0) ? (long)(args[-0].m_data.num) : (long)(0), (count > 1) ? (long)(args[-1].m_data.num) : (long)(RAND_MAX));
  return rv;
}

TypedValue* fg_mt_rand(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64) && (count <= 0 || (args-0)->m_type == KindOfInt64)) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_mt_rand((count > 0) ? (long)(args[-0].m_data.num) : (long)(0), (count > 1) ? (long)(args[-1].m_data.num) : (long)(RAND_MAX));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mt_rand(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("mt_rand", 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_lcg_value()
_ZN4HPHP11f_lcg_valueEv

(return value) => xmm0
*/

double fh_lcg_value() asm("_ZN4HPHP11f_lcg_valueEv");

TypedValue* fg_lcg_value(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_type = KindOfDouble;
      rv.m_data.dbl = fh_lcg_value();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("lcg_value", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}




} // !HPHP

