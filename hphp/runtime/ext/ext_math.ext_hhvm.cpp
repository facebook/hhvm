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

double fh_pi() asm("_ZN4HPHP4f_piEv");

TypedValue* fg_pi(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfDouble;
    rv->m_data.dbl = fh_pi();
  } else {
    throw_toomany_arguments_nr("pi", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_min(TypedValue* _rv, int64_t _argc, TypedValue* value, Value* _argv) asm("_ZN4HPHP5f_minEiRKNS_7VariantERKNS_5ArrayE");

TypedValue* fg_min(ActRec* ar) {
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
    fh_min(rv, count, (args-0), (Value*)(&extraArgs));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_missing_arguments_nr("min", 1, count, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_max(TypedValue* _rv, int64_t _argc, TypedValue* value, Value* _argv) asm("_ZN4HPHP5f_maxEiRKNS_7VariantERKNS_5ArrayE");

TypedValue* fg_max(ActRec* ar) {
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
    fh_max(rv, count, (args-0), (Value*)(&extraArgs));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_missing_arguments_nr("max", 1, count, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_abs(TypedValue* _rv, TypedValue* number) asm("_ZN4HPHP5f_absERKNS_7VariantE");

TypedValue* fg_abs(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    fh_abs(rv, (args-0));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_wrong_arguments_nr("abs", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_is_finite(double val) asm("_ZN4HPHP11f_is_finiteEd");

void fg1_is_finite(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_is_finite(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToDoubleInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_is_finite((args[-0].m_data.dbl))) ? 1LL : 0LL;
}

TypedValue* fg_is_finite(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfDouble) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_is_finite((args[-0].m_data.dbl))) ? 1LL : 0LL;
    } else {
      fg1_is_finite(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("is_finite", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_is_infinite(double val) asm("_ZN4HPHP13f_is_infiniteEd");

void fg1_is_infinite(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_is_infinite(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToDoubleInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_is_infinite((args[-0].m_data.dbl))) ? 1LL : 0LL;
}

TypedValue* fg_is_infinite(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfDouble) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_is_infinite((args[-0].m_data.dbl))) ? 1LL : 0LL;
    } else {
      fg1_is_infinite(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("is_infinite", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_is_nan(double val) asm("_ZN4HPHP8f_is_nanEd");

void fg1_is_nan(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_is_nan(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToDoubleInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_is_nan((args[-0].m_data.dbl))) ? 1LL : 0LL;
}

TypedValue* fg_is_nan(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfDouble) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_is_nan((args[-0].m_data.dbl))) ? 1LL : 0LL;
    } else {
      fg1_is_nan(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("is_nan", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_ceil(double value) asm("_ZN4HPHP6f_ceilEd");

void fg1_ceil(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_ceil(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToDoubleInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_ceil((args[-0].m_data.dbl));
}

TypedValue* fg_ceil(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfDouble) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_ceil((args[-0].m_data.dbl));
    } else {
      fg1_ceil(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("ceil", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_floor(double value) asm("_ZN4HPHP7f_floorEd");

void fg1_floor(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_floor(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToDoubleInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_floor((args[-0].m_data.dbl));
}

TypedValue* fg_floor(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfDouble) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_floor((args[-0].m_data.dbl));
    } else {
      fg1_floor(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("floor", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_round(TypedValue* val, long precision, long mode) asm("_ZN4HPHP7f_roundERKNS_7VariantEll");

void fg1_round(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_round(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_round((args-0), (count > 1) ? (long)(args[-1].m_data.num) : (long)(0), (count > 2) ? (long)(args[-2].m_data.num) : (long)(1));
}

TypedValue* fg_round(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        (count <= 1 || (args - 1)->m_type == KindOfInt64)) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_round((args-0), (count > 1) ? (long)(args[-1].m_data.num) : (long)(0), (count > 2) ? (long)(args[-2].m_data.num) : (long)(1));
    } else {
      fg1_round(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("round", count, 1, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_deg2rad(double number) asm("_ZN4HPHP9f_deg2radEd");

void fg1_deg2rad(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_deg2rad(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToDoubleInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_deg2rad((args[-0].m_data.dbl));
}

TypedValue* fg_deg2rad(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfDouble) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_deg2rad((args[-0].m_data.dbl));
    } else {
      fg1_deg2rad(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("deg2rad", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_rad2deg(double number) asm("_ZN4HPHP9f_rad2degEd");

void fg1_rad2deg(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_rad2deg(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToDoubleInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_rad2deg((args[-0].m_data.dbl));
}

TypedValue* fg_rad2deg(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfDouble) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_rad2deg((args[-0].m_data.dbl));
    } else {
      fg1_rad2deg(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("rad2deg", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_decbin(Value* _rv, long number) asm("_ZN4HPHP8f_decbinEl");

void fg1_decbin(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_decbin(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  rv->m_type = KindOfString;
  fh_decbin(&(rv->m_data), (long)(args[-0].m_data.num));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_decbin(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfInt64) {
      rv->m_type = KindOfString;
      fh_decbin(&(rv->m_data), (long)(args[-0].m_data.num));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_decbin(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("decbin", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_dechex(Value* _rv, long number) asm("_ZN4HPHP8f_dechexEl");

void fg1_dechex(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_dechex(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  rv->m_type = KindOfString;
  fh_dechex(&(rv->m_data), (long)(args[-0].m_data.num));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_dechex(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfInt64) {
      rv->m_type = KindOfString;
      fh_dechex(&(rv->m_data), (long)(args[-0].m_data.num));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_dechex(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("dechex", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_decoct(Value* _rv, long number) asm("_ZN4HPHP8f_decoctEl");

void fg1_decoct(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_decoct(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  rv->m_type = KindOfString;
  fh_decoct(&(rv->m_data), (long)(args[-0].m_data.num));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_decoct(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfInt64) {
      rv->m_type = KindOfString;
      fh_decoct(&(rv->m_data), (long)(args[-0].m_data.num));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_decoct(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("decoct", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_bindec(TypedValue* _rv, Value* binary_string) asm("_ZN4HPHP8f_bindecERKNS_6StringE");

void fg1_bindec(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_bindec(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_bindec(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_bindec(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_bindec(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_bindec(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("bindec", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_hexdec(TypedValue* _rv, Value* hex_string) asm("_ZN4HPHP8f_hexdecERKNS_6StringE");

void fg1_hexdec(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hexdec(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_hexdec(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_hexdec(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_hexdec(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_hexdec(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hexdec", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_octdec(TypedValue* _rv, Value* octal_string) asm("_ZN4HPHP8f_octdecERKNS_6StringE");

void fg1_octdec(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_octdec(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_octdec(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_octdec(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_octdec(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_octdec(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("octdec", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_base_convert(TypedValue* _rv, Value* number, long frombase, long tobase) asm("_ZN4HPHP14f_base_convertERKNS_6StringEll");

void fg1_base_convert(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_base_convert(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_base_convert(rv, &args[-0].m_data, (long)(args[-1].m_data.num), (long)(args[-2].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_base_convert(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if ((args - 2)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfInt64 &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_base_convert(rv, &args[-0].m_data, (long)(args[-1].m_data.num), (long)(args[-2].m_data.num));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_base_convert(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("base_convert", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_pow(TypedValue* _rv, TypedValue* base, TypedValue* exp) asm("_ZN4HPHP5f_powERKNS_7VariantES2_");

TypedValue* fg_pow(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    fh_pow(rv, (args-0), (args-1));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_wrong_arguments_nr("pow", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_exp(double arg) asm("_ZN4HPHP5f_expEd");

void fg1_exp(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_exp(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToDoubleInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_exp((args[-0].m_data.dbl));
}

TypedValue* fg_exp(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfDouble) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_exp((args[-0].m_data.dbl));
    } else {
      fg1_exp(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("exp", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_expm1(double arg) asm("_ZN4HPHP7f_expm1Ed");

void fg1_expm1(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_expm1(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToDoubleInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_expm1((args[-0].m_data.dbl));
}

TypedValue* fg_expm1(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfDouble) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_expm1((args[-0].m_data.dbl));
    } else {
      fg1_expm1(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("expm1", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_log10(double arg) asm("_ZN4HPHP7f_log10Ed");

void fg1_log10(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_log10(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToDoubleInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_log10((args[-0].m_data.dbl));
}

TypedValue* fg_log10(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfDouble) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_log10((args[-0].m_data.dbl));
    } else {
      fg1_log10(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("log10", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_log1p(double number) asm("_ZN4HPHP7f_log1pEd");

void fg1_log1p(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_log1p(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToDoubleInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_log1p((args[-0].m_data.dbl));
}

TypedValue* fg_log1p(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfDouble) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_log1p((args[-0].m_data.dbl));
    } else {
      fg1_log1p(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("log1p", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_log(double arg, double base) asm("_ZN4HPHP5f_logEdd");

void fg1_log(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_log(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_log((args[-0].m_data.dbl), (count > 1) ? (args[-1].m_data.dbl) : (double)(0));
}

TypedValue* fg_log(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfDouble) &&
        (args - 0)->m_type == KindOfDouble) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_log((args[-0].m_data.dbl), (count > 1) ? (args[-1].m_data.dbl) : (double)(0));
    } else {
      fg1_log(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("log", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_cos(double arg) asm("_ZN4HPHP5f_cosEd");

void fg1_cos(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_cos(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToDoubleInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_cos((args[-0].m_data.dbl));
}

TypedValue* fg_cos(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfDouble) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_cos((args[-0].m_data.dbl));
    } else {
      fg1_cos(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("cos", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_cosh(double arg) asm("_ZN4HPHP6f_coshEd");

void fg1_cosh(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_cosh(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToDoubleInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_cosh((args[-0].m_data.dbl));
}

TypedValue* fg_cosh(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfDouble) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_cosh((args[-0].m_data.dbl));
    } else {
      fg1_cosh(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("cosh", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_sin(double arg) asm("_ZN4HPHP5f_sinEd");

void fg1_sin(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_sin(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToDoubleInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_sin((args[-0].m_data.dbl));
}

TypedValue* fg_sin(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfDouble) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_sin((args[-0].m_data.dbl));
    } else {
      fg1_sin(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("sin", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_sinh(double arg) asm("_ZN4HPHP6f_sinhEd");

void fg1_sinh(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_sinh(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToDoubleInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_sinh((args[-0].m_data.dbl));
}

TypedValue* fg_sinh(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfDouble) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_sinh((args[-0].m_data.dbl));
    } else {
      fg1_sinh(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("sinh", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_tan(double arg) asm("_ZN4HPHP5f_tanEd");

void fg1_tan(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_tan(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToDoubleInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_tan((args[-0].m_data.dbl));
}

TypedValue* fg_tan(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfDouble) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_tan((args[-0].m_data.dbl));
    } else {
      fg1_tan(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("tan", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_tanh(double arg) asm("_ZN4HPHP6f_tanhEd");

void fg1_tanh(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_tanh(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToDoubleInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_tanh((args[-0].m_data.dbl));
}

TypedValue* fg_tanh(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfDouble) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_tanh((args[-0].m_data.dbl));
    } else {
      fg1_tanh(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("tanh", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_acos(double arg) asm("_ZN4HPHP6f_acosEd");

void fg1_acos(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_acos(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToDoubleInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_acos((args[-0].m_data.dbl));
}

TypedValue* fg_acos(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfDouble) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_acos((args[-0].m_data.dbl));
    } else {
      fg1_acos(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("acos", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_acosh(double arg) asm("_ZN4HPHP7f_acoshEd");

void fg1_acosh(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_acosh(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToDoubleInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_acosh((args[-0].m_data.dbl));
}

TypedValue* fg_acosh(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfDouble) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_acosh((args[-0].m_data.dbl));
    } else {
      fg1_acosh(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("acosh", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_asin(double arg) asm("_ZN4HPHP6f_asinEd");

void fg1_asin(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_asin(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToDoubleInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_asin((args[-0].m_data.dbl));
}

TypedValue* fg_asin(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfDouble) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_asin((args[-0].m_data.dbl));
    } else {
      fg1_asin(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("asin", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_asinh(double arg) asm("_ZN4HPHP7f_asinhEd");

void fg1_asinh(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_asinh(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToDoubleInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_asinh((args[-0].m_data.dbl));
}

TypedValue* fg_asinh(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfDouble) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_asinh((args[-0].m_data.dbl));
    } else {
      fg1_asinh(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("asinh", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_atan(double arg) asm("_ZN4HPHP6f_atanEd");

void fg1_atan(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_atan(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToDoubleInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_atan((args[-0].m_data.dbl));
}

TypedValue* fg_atan(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfDouble) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_atan((args[-0].m_data.dbl));
    } else {
      fg1_atan(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("atan", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_atanh(double arg) asm("_ZN4HPHP7f_atanhEd");

void fg1_atanh(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_atanh(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToDoubleInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_atanh((args[-0].m_data.dbl));
}

TypedValue* fg_atanh(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfDouble) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_atanh((args[-0].m_data.dbl));
    } else {
      fg1_atanh(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("atanh", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_atan2(double y, double x) asm("_ZN4HPHP7f_atan2Edd");

void fg1_atan2(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_atan2(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-0);
  }
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_atan2((args[-0].m_data.dbl), (args[-1].m_data.dbl));
}

TypedValue* fg_atan2(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfDouble) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_atan2((args[-0].m_data.dbl), (args[-1].m_data.dbl));
    } else {
      fg1_atan2(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("atan2", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_hypot(double x, double y) asm("_ZN4HPHP7f_hypotEdd");

void fg1_hypot(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hypot(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-0);
  }
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_hypot((args[-0].m_data.dbl), (args[-1].m_data.dbl));
}

TypedValue* fg_hypot(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfDouble) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_hypot((args[-0].m_data.dbl), (args[-1].m_data.dbl));
    } else {
      fg1_hypot(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hypot", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_fmod(double x, double y) asm("_ZN4HPHP6f_fmodEdd");

void fg1_fmod(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_fmod(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-0);
  }
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_fmod((args[-0].m_data.dbl), (args[-1].m_data.dbl));
}

TypedValue* fg_fmod(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfDouble) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_fmod((args[-0].m_data.dbl), (args[-1].m_data.dbl));
    } else {
      fg1_fmod(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("fmod", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_sqrt(double arg) asm("_ZN4HPHP6f_sqrtEd");

void fg1_sqrt(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_sqrt(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToDoubleInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_sqrt((args[-0].m_data.dbl));
}

TypedValue* fg_sqrt(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfDouble) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_sqrt((args[-0].m_data.dbl));
    } else {
      fg1_sqrt(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("sqrt", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_getrandmax() asm("_ZN4HPHP12f_getrandmaxEv");

TypedValue* fg_getrandmax(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfInt64;
    rv->m_data.num = (int64_t)fh_getrandmax();
  } else {
    throw_toomany_arguments_nr("getrandmax", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_srand(TypedValue* seed) asm("_ZN4HPHP7f_srandERKNS_7VariantE");

TypedValue* fg_srand(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 1) {
    rv->m_type = KindOfNull;
    fh_srand((count > 0) ? (args-0) : (TypedValue*)(&null_variant));
  } else {
    throw_toomany_arguments_nr("srand", 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_rand(long min, long max) asm("_ZN4HPHP6f_randEll");

void fg1_rand(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_rand(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_rand((count > 0) ? (long)(args[-0].m_data.num) : (long)(0), (count > 1) ? (long)(args[-1].m_data.num) : (long)(RAND_MAX));
}

TypedValue* fg_rand(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfInt64) &&
        (count <= 0 || (args - 0)->m_type == KindOfInt64)) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_rand((count > 0) ? (long)(args[-0].m_data.num) : (long)(0), (count > 1) ? (long)(args[-1].m_data.num) : (long)(RAND_MAX));
    } else {
      fg1_rand(rv, ar, count);
    }
  } else {
    throw_toomany_arguments_nr("rand", 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_mt_getrandmax() asm("_ZN4HPHP15f_mt_getrandmaxEv");

TypedValue* fg_mt_getrandmax(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfInt64;
    rv->m_data.num = (int64_t)fh_mt_getrandmax();
  } else {
    throw_toomany_arguments_nr("mt_getrandmax", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_mt_srand(TypedValue* seed) asm("_ZN4HPHP10f_mt_srandERKNS_7VariantE");

TypedValue* fg_mt_srand(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 1) {
    rv->m_type = KindOfNull;
    fh_mt_srand((count > 0) ? (args-0) : (TypedValue*)(&null_variant));
  } else {
    throw_toomany_arguments_nr("mt_srand", 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_mt_rand(long min, long max) asm("_ZN4HPHP9f_mt_randEll");

void fg1_mt_rand(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mt_rand(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_mt_rand((count > 0) ? (long)(args[-0].m_data.num) : (long)(0), (count > 1) ? (long)(args[-1].m_data.num) : (long)(RAND_MAX));
}

TypedValue* fg_mt_rand(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfInt64) &&
        (count <= 0 || (args - 0)->m_type == KindOfInt64)) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_mt_rand((count > 0) ? (long)(args[-0].m_data.num) : (long)(0), (count > 1) ? (long)(args[-1].m_data.num) : (long)(RAND_MAX));
    } else {
      fg1_mt_rand(rv, ar, count);
    }
  } else {
    throw_toomany_arguments_nr("mt_rand", 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_lcg_value() asm("_ZN4HPHP11f_lcg_valueEv");

TypedValue* fg_lcg_value(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfDouble;
    rv->m_data.dbl = fh_lcg_value();
  } else {
    throw_toomany_arguments_nr("lcg_value", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

} // namespace HPHP
