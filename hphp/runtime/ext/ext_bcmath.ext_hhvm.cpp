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

bool fh_bcscale(long scale) asm("_ZN4HPHP9f_bcscaleEl");

void fg1_bcscale(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_bcscale(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_bcscale((long)(args[-0].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_bcscale(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfInt64) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_bcscale((long)(args[-0].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_bcscale(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("bcscale", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_bcadd(Value* _rv, Value* left, Value* right, long scale) asm("_ZN4HPHP7f_bcaddERKNS_6StringES2_l");

void fg1_bcadd(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_bcadd(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfString;
  fh_bcadd(&(rv->m_data), &args[-0].m_data, &args[-1].m_data, (count > 2) ? (long)(args[-2].m_data.num) : (long)(-1));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_bcadd(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfString;
      fh_bcadd(&(rv->m_data), &args[-0].m_data, &args[-1].m_data, (count > 2) ? (long)(args[-2].m_data.num) : (long)(-1));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_bcadd(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("bcadd", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_bcsub(Value* _rv, Value* left, Value* right, long scale) asm("_ZN4HPHP7f_bcsubERKNS_6StringES2_l");

void fg1_bcsub(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_bcsub(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfString;
  fh_bcsub(&(rv->m_data), &args[-0].m_data, &args[-1].m_data, (count > 2) ? (long)(args[-2].m_data.num) : (long)(-1));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_bcsub(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfString;
      fh_bcsub(&(rv->m_data), &args[-0].m_data, &args[-1].m_data, (count > 2) ? (long)(args[-2].m_data.num) : (long)(-1));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_bcsub(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("bcsub", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_bccomp(Value* left, Value* right, long scale) asm("_ZN4HPHP8f_bccompERKNS_6StringES2_l");

void fg1_bccomp(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_bccomp(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_bccomp(&args[-0].m_data, &args[-1].m_data, (count > 2) ? (long)(args[-2].m_data.num) : (long)(-1));
}

TypedValue* fg_bccomp(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_bccomp(&args[-0].m_data, &args[-1].m_data, (count > 2) ? (long)(args[-2].m_data.num) : (long)(-1));
    } else {
      fg1_bccomp(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("bccomp", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_bcmul(Value* _rv, Value* left, Value* right, long scale) asm("_ZN4HPHP7f_bcmulERKNS_6StringES2_l");

void fg1_bcmul(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_bcmul(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfString;
  fh_bcmul(&(rv->m_data), &args[-0].m_data, &args[-1].m_data, (count > 2) ? (long)(args[-2].m_data.num) : (long)(-1));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_bcmul(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfString;
      fh_bcmul(&(rv->m_data), &args[-0].m_data, &args[-1].m_data, (count > 2) ? (long)(args[-2].m_data.num) : (long)(-1));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_bcmul(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("bcmul", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_bcdiv(Value* _rv, Value* left, Value* right, long scale) asm("_ZN4HPHP7f_bcdivERKNS_6StringES2_l");

void fg1_bcdiv(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_bcdiv(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfString;
  fh_bcdiv(&(rv->m_data), &args[-0].m_data, &args[-1].m_data, (count > 2) ? (long)(args[-2].m_data.num) : (long)(-1));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_bcdiv(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfString;
      fh_bcdiv(&(rv->m_data), &args[-0].m_data, &args[-1].m_data, (count > 2) ? (long)(args[-2].m_data.num) : (long)(-1));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_bcdiv(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("bcdiv", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_bcmod(Value* _rv, Value* left, Value* right) asm("_ZN4HPHP7f_bcmodERKNS_6StringES2_");

void fg1_bcmod(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_bcmod(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfString;
  fh_bcmod(&(rv->m_data), &args[-0].m_data, &args[-1].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_bcmod(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfString;
      fh_bcmod(&(rv->m_data), &args[-0].m_data, &args[-1].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_bcmod(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("bcmod", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_bcpow(Value* _rv, Value* left, Value* right, long scale) asm("_ZN4HPHP7f_bcpowERKNS_6StringES2_l");

void fg1_bcpow(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_bcpow(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfString;
  fh_bcpow(&(rv->m_data), &args[-0].m_data, &args[-1].m_data, (count > 2) ? (long)(args[-2].m_data.num) : (long)(-1));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_bcpow(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfString;
      fh_bcpow(&(rv->m_data), &args[-0].m_data, &args[-1].m_data, (count > 2) ? (long)(args[-2].m_data.num) : (long)(-1));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_bcpow(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("bcpow", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_bcpowmod(TypedValue* _rv, Value* left, Value* right, Value* modulus, long scale) asm("_ZN4HPHP10f_bcpowmodERKNS_6StringES2_S2_l");

void fg1_bcpowmod(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_bcpowmod(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
    break;
  }
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_bcpowmod(rv, &args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (count > 3) ? (long)(args[-3].m_data.num) : (long)(-1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_bcpowmod(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3 && count <= 4) {
    if ((count <= 3 || (args - 3)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 2)->m_type) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_bcpowmod(rv, &args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (count > 3) ? (long)(args[-3].m_data.num) : (long)(-1));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_bcpowmod(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("bcpowmod", count, 3, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_bcsqrt(TypedValue* _rv, Value* operand, long scale) asm("_ZN4HPHP8f_bcsqrtERKNS_6StringEl");

void fg1_bcsqrt(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_bcsqrt(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_bcsqrt(rv, &args[-0].m_data, (count > 1) ? (long)(args[-1].m_data.num) : (long)(-1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_bcsqrt(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_bcsqrt(rv, &args[-0].m_data, (count > 1) ? (long)(args[-1].m_data.num) : (long)(-1));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_bcsqrt(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("bcsqrt", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

} // namespace HPHP
