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

void fh_session_write_close() asm("_ZN4HPHP21f_session_write_closeEv");

TypedValue* fg_session_write_close(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfNull;
    fh_session_write_close();
  } else {
    throw_toomany_arguments_nr("session_write_close", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_session_set_cookie_params(long lifetime, Value* path, Value* domain, TypedValue* secure, TypedValue* httponly) asm("_ZN4HPHP27f_session_set_cookie_paramsElRKNS_6StringES2_RKNS_7VariantES5_");

void fg1_session_set_cookie_params(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_session_set_cookie_params(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 5
  case 4:
  case 3:
    if (!IS_STRING_TYPE((args-2)->m_type)) {
      tvCastToStringInPlace(args-2);
    }
  case 2:
    if (!IS_STRING_TYPE((args-1)->m_type)) {
      tvCastToStringInPlace(args-1);
    }
  case 1:
    break;
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  rv->m_type = KindOfNull;
  Variant defVal3;
  Variant defVal4;
  fh_session_set_cookie_params((long)(args[-0].m_data.num), (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? &args[-2].m_data : (Value*)(&null_string), (count > 3) ? (args-3) : (TypedValue*)(&defVal3), (count > 4) ? (args-4) : (TypedValue*)(&defVal4));
}

TypedValue* fg_session_set_cookie_params(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 5) {
    if ((count <= 2 || IS_STRING_TYPE((args - 2)->m_type)) &&
        (count <= 1 || IS_STRING_TYPE((args - 1)->m_type)) &&
        (args - 0)->m_type == KindOfInt64) {
      rv->m_type = KindOfNull;
      Variant defVal3;
      Variant defVal4;
      fh_session_set_cookie_params((long)(args[-0].m_data.num), (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? &args[-2].m_data : (Value*)(&null_string), (count > 3) ? (args-3) : (TypedValue*)(&defVal3), (count > 4) ? (args-4) : (TypedValue*)(&defVal4));
    } else {
      fg1_session_set_cookie_params(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("session_set_cookie_params", count, 1, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_session_get_cookie_params(Value* _rv) asm("_ZN4HPHP27f_session_get_cookie_paramsEv");

TypedValue* fg_session_get_cookie_params(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfArray;
    fh_session_get_cookie_params(&(rv->m_data));
    if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("session_get_cookie_params", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_session_name(Value* _rv, Value* newname) asm("_ZN4HPHP14f_session_nameERKNS_6StringE");

void fg1_session_name(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_session_name(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfString;
  fh_session_name(&(rv->m_data), (count > 0) ? &args[-0].m_data : (Value*)(&null_string));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_session_name(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 1) {
    if ((count <= 0 || IS_STRING_TYPE((args - 0)->m_type))) {
      rv->m_type = KindOfString;
      fh_session_name(&(rv->m_data), (count > 0) ? &args[-0].m_data : (Value*)(&null_string));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_session_name(rv, ar, count);
    }
  } else {
    throw_toomany_arguments_nr("session_name", 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_session_module_name(TypedValue* _rv, Value* newname) asm("_ZN4HPHP21f_session_module_nameERKNS_6StringE");

void fg1_session_module_name(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_session_module_name(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_session_module_name(rv, (count > 0) ? &args[-0].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_session_module_name(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 1) {
    if ((count <= 0 || IS_STRING_TYPE((args - 0)->m_type))) {
      fh_session_module_name(rv, (count > 0) ? &args[-0].m_data : (Value*)(&null_string));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_session_module_name(rv, ar, count);
    }
  } else {
    throw_toomany_arguments_nr("session_module_name", 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_session_set_save_handler(Value* open, Value* close, Value* read, Value* write, Value* destroy, Value* gc) asm("_ZN4HPHP26f_session_set_save_handlerERKNS_6StringES2_S2_S2_S2_S2_");

void fg1_session_set_save_handler(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_session_set_save_handler(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-5)->m_type)) {
    tvCastToStringInPlace(args-5);
  }
  if (!IS_STRING_TYPE((args-4)->m_type)) {
    tvCastToStringInPlace(args-4);
  }
  if (!IS_STRING_TYPE((args-3)->m_type)) {
    tvCastToStringInPlace(args-3);
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_session_set_save_handler(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data, &args[-3].m_data, &args[-4].m_data, &args[-5].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_session_set_save_handler(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 6) {
    if (IS_STRING_TYPE((args - 5)->m_type) &&
        IS_STRING_TYPE((args - 4)->m_type) &&
        IS_STRING_TYPE((args - 3)->m_type) &&
        IS_STRING_TYPE((args - 2)->m_type) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_session_set_save_handler(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data, &args[-3].m_data, &args[-4].m_data, &args[-5].m_data)) ? 1LL : 0LL;
    } else {
      fg1_session_set_save_handler(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("session_set_save_handler", count, 6, 6, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 6);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_session_save_path(Value* _rv, Value* newname) asm("_ZN4HPHP19f_session_save_pathERKNS_6StringE");

void fg1_session_save_path(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_session_save_path(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfString;
  fh_session_save_path(&(rv->m_data), (count > 0) ? &args[-0].m_data : (Value*)(&null_string));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_session_save_path(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 1) {
    if ((count <= 0 || IS_STRING_TYPE((args - 0)->m_type))) {
      rv->m_type = KindOfString;
      fh_session_save_path(&(rv->m_data), (count > 0) ? &args[-0].m_data : (Value*)(&null_string));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_session_save_path(rv, ar, count);
    }
  } else {
    throw_toomany_arguments_nr("session_save_path", 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_session_id(Value* _rv, Value* newid) asm("_ZN4HPHP12f_session_idERKNS_6StringE");

void fg1_session_id(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_session_id(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfString;
  fh_session_id(&(rv->m_data), (count > 0) ? &args[-0].m_data : (Value*)(&null_string));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_session_id(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 1) {
    if ((count <= 0 || IS_STRING_TYPE((args - 0)->m_type))) {
      rv->m_type = KindOfString;
      fh_session_id(&(rv->m_data), (count > 0) ? &args[-0].m_data : (Value*)(&null_string));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_session_id(rv, ar, count);
    }
  } else {
    throw_toomany_arguments_nr("session_id", 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_session_regenerate_id(bool delete_old_session) asm("_ZN4HPHP23f_session_regenerate_idEb");

void fg1_session_regenerate_id(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_session_regenerate_id(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToBooleanInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_session_regenerate_id((count > 0) ? (bool)(args[-0].m_data.num) : (bool)(false))) ? 1LL : 0LL;
}

TypedValue* fg_session_regenerate_id(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 1) {
    if ((count <= 0 || (args - 0)->m_type == KindOfBoolean)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_session_regenerate_id((count > 0) ? (bool)(args[-0].m_data.num) : (bool)(false))) ? 1LL : 0LL;
    } else {
      fg1_session_regenerate_id(rv, ar, count);
    }
  } else {
    throw_toomany_arguments_nr("session_regenerate_id", 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_session_cache_limiter(Value* _rv, Value* new_cache_limiter) asm("_ZN4HPHP23f_session_cache_limiterERKNS_6StringE");

void fg1_session_cache_limiter(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_session_cache_limiter(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfString;
  fh_session_cache_limiter(&(rv->m_data), (count > 0) ? &args[-0].m_data : (Value*)(&null_string));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_session_cache_limiter(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 1) {
    if ((count <= 0 || IS_STRING_TYPE((args - 0)->m_type))) {
      rv->m_type = KindOfString;
      fh_session_cache_limiter(&(rv->m_data), (count > 0) ? &args[-0].m_data : (Value*)(&null_string));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_session_cache_limiter(rv, ar, count);
    }
  } else {
    throw_toomany_arguments_nr("session_cache_limiter", 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_session_cache_expire(Value* new_cache_expire) asm("_ZN4HPHP22f_session_cache_expireERKNS_6StringE");

void fg1_session_cache_expire(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_session_cache_expire(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_session_cache_expire((count > 0) ? &args[-0].m_data : (Value*)(&null_string));
}

TypedValue* fg_session_cache_expire(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 1) {
    if ((count <= 0 || IS_STRING_TYPE((args - 0)->m_type))) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_session_cache_expire((count > 0) ? &args[-0].m_data : (Value*)(&null_string));
    } else {
      fg1_session_cache_expire(rv, ar, count);
    }
  } else {
    throw_toomany_arguments_nr("session_cache_expire", 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_session_encode(TypedValue* _rv) asm("_ZN4HPHP16f_session_encodeEv");

TypedValue* fg_session_encode(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    fh_session_encode(rv);
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("session_encode", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_session_decode(Value* data) asm("_ZN4HPHP16f_session_decodeERKNS_6StringE");

void fg1_session_decode(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_session_decode(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_session_decode(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_session_decode(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_session_decode(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_session_decode(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("session_decode", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_session_start() asm("_ZN4HPHP15f_session_startEv");

TypedValue* fg_session_start(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfBoolean;
    rv->m_data.num = (fh_session_start()) ? 1LL : 0LL;
  } else {
    throw_toomany_arguments_nr("session_start", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_session_destroy() asm("_ZN4HPHP17f_session_destroyEv");

TypedValue* fg_session_destroy(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfBoolean;
    rv->m_data.num = (fh_session_destroy()) ? 1LL : 0LL;
  } else {
    throw_toomany_arguments_nr("session_destroy", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_session_unset(TypedValue* _rv) asm("_ZN4HPHP15f_session_unsetEv");

TypedValue* fg_session_unset(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    fh_session_unset(rv);
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("session_unset", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_session_commit() asm("_ZN4HPHP16f_session_commitEv");

TypedValue* fg_session_commit(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfNull;
    fh_session_commit();
  } else {
    throw_toomany_arguments_nr("session_commit", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_session_register(int64_t _argc, TypedValue* var_names, Value* _argv) asm("_ZN4HPHP18f_session_registerEiRKNS_7VariantERKNS_5ArrayE");

TypedValue* fg_session_register(ActRec* ar) {
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
    rv->m_data.num = (fh_session_register(count, (args-0), (Value*)(&extraArgs))) ? 1LL : 0LL;
  } else {
    throw_missing_arguments_nr("session_register", 1, count, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_session_unregister(Value* varname) asm("_ZN4HPHP20f_session_unregisterERKNS_6StringE");

void fg1_session_unregister(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_session_unregister(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_session_unregister(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_session_unregister(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_session_unregister(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_session_unregister(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("session_unregister", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_session_is_registered(Value* varname) asm("_ZN4HPHP23f_session_is_registeredERKNS_6StringE");

void fg1_session_is_registered(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_session_is_registered(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_session_is_registered(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_session_is_registered(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_session_is_registered(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_session_is_registered(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("session_is_registered", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

} // namespace HPHP
