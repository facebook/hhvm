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

TypedValue* fh_mcrypt_module_open(TypedValue* _rv, Value* algorithm, Value* algorithm_directory, Value* mode, Value* mode_directory) asm("_ZN4HPHP20f_mcrypt_module_openERKNS_6StringES2_S2_S2_");

void fg1_mcrypt_module_open(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mcrypt_module_open(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  fh_mcrypt_module_open(rv, &args[-0].m_data, &args[-1].m_data, &args[-2].m_data, &args[-3].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mcrypt_module_open(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 4) {
    if (IS_STRING_TYPE((args - 3)->m_type) &&
        IS_STRING_TYPE((args - 2)->m_type) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_mcrypt_module_open(rv, &args[-0].m_data, &args[-1].m_data, &args[-2].m_data, &args[-3].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mcrypt_module_open(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mcrypt_module_open", count, 4, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_mcrypt_module_close(Value* td) asm("_ZN4HPHP21f_mcrypt_module_closeERKNS_6ObjectE");

void fg1_mcrypt_module_close(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mcrypt_module_close(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_mcrypt_module_close(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_mcrypt_module_close(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_mcrypt_module_close(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_mcrypt_module_close(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mcrypt_module_close", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_mcrypt_list_algorithms(Value* _rv, Value* lib_dir) asm("_ZN4HPHP24f_mcrypt_list_algorithmsERKNS_6StringE");

void fg1_mcrypt_list_algorithms(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mcrypt_list_algorithms(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfArray;
  fh_mcrypt_list_algorithms(&(rv->m_data), (count > 0) ? &args[-0].m_data : (Value*)(&null_string));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_mcrypt_list_algorithms(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 1) {
    if ((count <= 0 || IS_STRING_TYPE((args - 0)->m_type))) {
      rv->m_type = KindOfArray;
      fh_mcrypt_list_algorithms(&(rv->m_data), (count > 0) ? &args[-0].m_data : (Value*)(&null_string));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_mcrypt_list_algorithms(rv, ar, count);
    }
  } else {
    throw_toomany_arguments_nr("mcrypt_list_algorithms", 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_mcrypt_list_modes(Value* _rv, Value* lib_dir) asm("_ZN4HPHP19f_mcrypt_list_modesERKNS_6StringE");

void fg1_mcrypt_list_modes(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mcrypt_list_modes(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfArray;
  fh_mcrypt_list_modes(&(rv->m_data), (count > 0) ? &args[-0].m_data : (Value*)(&null_string));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_mcrypt_list_modes(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 1) {
    if ((count <= 0 || IS_STRING_TYPE((args - 0)->m_type))) {
      rv->m_type = KindOfArray;
      fh_mcrypt_list_modes(&(rv->m_data), (count > 0) ? &args[-0].m_data : (Value*)(&null_string));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_mcrypt_list_modes(rv, ar, count);
    }
  } else {
    throw_toomany_arguments_nr("mcrypt_list_modes", 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_mcrypt_module_get_algo_block_size(Value* algorithm, Value* lib_dir) asm("_ZN4HPHP35f_mcrypt_module_get_algo_block_sizeERKNS_6StringES2_");

void fg1_mcrypt_module_get_algo_block_size(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mcrypt_module_get_algo_block_size(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 2
    if (!IS_STRING_TYPE((args-1)->m_type)) {
      tvCastToStringInPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_mcrypt_module_get_algo_block_size(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string));
}

TypedValue* fg_mcrypt_module_get_algo_block_size(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || IS_STRING_TYPE((args - 1)->m_type)) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_mcrypt_module_get_algo_block_size(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string));
    } else {
      fg1_mcrypt_module_get_algo_block_size(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mcrypt_module_get_algo_block_size", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_mcrypt_module_get_algo_key_size(Value* algorithm, Value* lib_dir) asm("_ZN4HPHP33f_mcrypt_module_get_algo_key_sizeERKNS_6StringES2_");

void fg1_mcrypt_module_get_algo_key_size(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mcrypt_module_get_algo_key_size(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 2
    if (!IS_STRING_TYPE((args-1)->m_type)) {
      tvCastToStringInPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_mcrypt_module_get_algo_key_size(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string));
}

TypedValue* fg_mcrypt_module_get_algo_key_size(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || IS_STRING_TYPE((args - 1)->m_type)) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_mcrypt_module_get_algo_key_size(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string));
    } else {
      fg1_mcrypt_module_get_algo_key_size(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mcrypt_module_get_algo_key_size", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_mcrypt_module_get_supported_key_sizes(Value* _rv, Value* algorithm, Value* lib_dir) asm("_ZN4HPHP39f_mcrypt_module_get_supported_key_sizesERKNS_6StringES2_");

void fg1_mcrypt_module_get_supported_key_sizes(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mcrypt_module_get_supported_key_sizes(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 2
    if (!IS_STRING_TYPE((args-1)->m_type)) {
      tvCastToStringInPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfArray;
  fh_mcrypt_module_get_supported_key_sizes(&(rv->m_data), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_mcrypt_module_get_supported_key_sizes(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || IS_STRING_TYPE((args - 1)->m_type)) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfArray;
      fh_mcrypt_module_get_supported_key_sizes(&(rv->m_data), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_mcrypt_module_get_supported_key_sizes(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mcrypt_module_get_supported_key_sizes", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_mcrypt_module_is_block_algorithm_mode(Value* mode, Value* lib_dir) asm("_ZN4HPHP39f_mcrypt_module_is_block_algorithm_modeERKNS_6StringES2_");

void fg1_mcrypt_module_is_block_algorithm_mode(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mcrypt_module_is_block_algorithm_mode(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 2
    if (!IS_STRING_TYPE((args-1)->m_type)) {
      tvCastToStringInPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_mcrypt_module_is_block_algorithm_mode(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
}

TypedValue* fg_mcrypt_module_is_block_algorithm_mode(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || IS_STRING_TYPE((args - 1)->m_type)) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_mcrypt_module_is_block_algorithm_mode(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
    } else {
      fg1_mcrypt_module_is_block_algorithm_mode(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mcrypt_module_is_block_algorithm_mode", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_mcrypt_module_is_block_algorithm(Value* algorithm, Value* lib_dir) asm("_ZN4HPHP34f_mcrypt_module_is_block_algorithmERKNS_6StringES2_");

void fg1_mcrypt_module_is_block_algorithm(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mcrypt_module_is_block_algorithm(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 2
    if (!IS_STRING_TYPE((args-1)->m_type)) {
      tvCastToStringInPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_mcrypt_module_is_block_algorithm(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
}

TypedValue* fg_mcrypt_module_is_block_algorithm(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || IS_STRING_TYPE((args - 1)->m_type)) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_mcrypt_module_is_block_algorithm(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
    } else {
      fg1_mcrypt_module_is_block_algorithm(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mcrypt_module_is_block_algorithm", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_mcrypt_module_is_block_mode(Value* mode, Value* lib_dir) asm("_ZN4HPHP29f_mcrypt_module_is_block_modeERKNS_6StringES2_");

void fg1_mcrypt_module_is_block_mode(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mcrypt_module_is_block_mode(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 2
    if (!IS_STRING_TYPE((args-1)->m_type)) {
      tvCastToStringInPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_mcrypt_module_is_block_mode(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
}

TypedValue* fg_mcrypt_module_is_block_mode(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || IS_STRING_TYPE((args - 1)->m_type)) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_mcrypt_module_is_block_mode(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
    } else {
      fg1_mcrypt_module_is_block_mode(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mcrypt_module_is_block_mode", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_mcrypt_module_self_test(Value* algorithm, Value* lib_dir) asm("_ZN4HPHP25f_mcrypt_module_self_testERKNS_6StringES2_");

void fg1_mcrypt_module_self_test(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mcrypt_module_self_test(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 2
    if (!IS_STRING_TYPE((args-1)->m_type)) {
      tvCastToStringInPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_mcrypt_module_self_test(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
}

TypedValue* fg_mcrypt_module_self_test(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || IS_STRING_TYPE((args - 1)->m_type)) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_mcrypt_module_self_test(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
    } else {
      fg1_mcrypt_module_self_test(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mcrypt_module_self_test", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mcrypt_create_iv(TypedValue* _rv, int size, int source) asm("_ZN4HPHP18f_mcrypt_create_ivEii");

void fg1_mcrypt_create_iv(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mcrypt_create_iv(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
    break;
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  fh_mcrypt_create_iv(rv, (int)(args[-0].m_data.num), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mcrypt_create_iv(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfInt64) &&
        (args - 0)->m_type == KindOfInt64) {
      fh_mcrypt_create_iv(rv, (int)(args[-0].m_data.num), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mcrypt_create_iv(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mcrypt_create_iv", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mcrypt_encrypt(TypedValue* _rv, Value* cipher, Value* key, Value* data, Value* mode, Value* iv) asm("_ZN4HPHP16f_mcrypt_encryptERKNS_6StringES2_S2_S2_S2_");

void fg1_mcrypt_encrypt(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mcrypt_encrypt(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 5
    if (!IS_STRING_TYPE((args-4)->m_type)) {
      tvCastToStringInPlace(args-4);
    }
  case 4:
    break;
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
  fh_mcrypt_encrypt(rv, &args[-0].m_data, &args[-1].m_data, &args[-2].m_data, &args[-3].m_data, (count > 4) ? &args[-4].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mcrypt_encrypt(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 4 && count <= 5) {
    if ((count <= 4 || IS_STRING_TYPE((args - 4)->m_type)) &&
        IS_STRING_TYPE((args - 3)->m_type) &&
        IS_STRING_TYPE((args - 2)->m_type) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_mcrypt_encrypt(rv, &args[-0].m_data, &args[-1].m_data, &args[-2].m_data, &args[-3].m_data, (count > 4) ? &args[-4].m_data : (Value*)(&null_string));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mcrypt_encrypt(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mcrypt_encrypt", count, 4, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mcrypt_decrypt(TypedValue* _rv, Value* cipher, Value* key, Value* data, Value* mode, Value* iv) asm("_ZN4HPHP16f_mcrypt_decryptERKNS_6StringES2_S2_S2_S2_");

void fg1_mcrypt_decrypt(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mcrypt_decrypt(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 5
    if (!IS_STRING_TYPE((args-4)->m_type)) {
      tvCastToStringInPlace(args-4);
    }
  case 4:
    break;
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
  fh_mcrypt_decrypt(rv, &args[-0].m_data, &args[-1].m_data, &args[-2].m_data, &args[-3].m_data, (count > 4) ? &args[-4].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mcrypt_decrypt(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 4 && count <= 5) {
    if ((count <= 4 || IS_STRING_TYPE((args - 4)->m_type)) &&
        IS_STRING_TYPE((args - 3)->m_type) &&
        IS_STRING_TYPE((args - 2)->m_type) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_mcrypt_decrypt(rv, &args[-0].m_data, &args[-1].m_data, &args[-2].m_data, &args[-3].m_data, (count > 4) ? &args[-4].m_data : (Value*)(&null_string));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mcrypt_decrypt(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mcrypt_decrypt", count, 4, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mcrypt_cbc(TypedValue* _rv, Value* cipher, Value* key, Value* data, int mode, Value* iv) asm("_ZN4HPHP12f_mcrypt_cbcERKNS_6StringES2_S2_iS2_");

void fg1_mcrypt_cbc(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mcrypt_cbc(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 5
    if (!IS_STRING_TYPE((args-4)->m_type)) {
      tvCastToStringInPlace(args-4);
    }
  case 4:
    break;
  }
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
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
  fh_mcrypt_cbc(rv, &args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (int)(args[-3].m_data.num), (count > 4) ? &args[-4].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mcrypt_cbc(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 4 && count <= 5) {
    if ((count <= 4 || IS_STRING_TYPE((args - 4)->m_type)) &&
        (args - 3)->m_type == KindOfInt64 &&
        IS_STRING_TYPE((args - 2)->m_type) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_mcrypt_cbc(rv, &args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (int)(args[-3].m_data.num), (count > 4) ? &args[-4].m_data : (Value*)(&null_string));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mcrypt_cbc(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mcrypt_cbc", count, 4, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mcrypt_cfb(TypedValue* _rv, Value* cipher, Value* key, Value* data, int mode, Value* iv) asm("_ZN4HPHP12f_mcrypt_cfbERKNS_6StringES2_S2_iS2_");

void fg1_mcrypt_cfb(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mcrypt_cfb(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 5
    if (!IS_STRING_TYPE((args-4)->m_type)) {
      tvCastToStringInPlace(args-4);
    }
  case 4:
    break;
  }
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
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
  fh_mcrypt_cfb(rv, &args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (int)(args[-3].m_data.num), (count > 4) ? &args[-4].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mcrypt_cfb(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 4 && count <= 5) {
    if ((count <= 4 || IS_STRING_TYPE((args - 4)->m_type)) &&
        (args - 3)->m_type == KindOfInt64 &&
        IS_STRING_TYPE((args - 2)->m_type) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_mcrypt_cfb(rv, &args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (int)(args[-3].m_data.num), (count > 4) ? &args[-4].m_data : (Value*)(&null_string));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mcrypt_cfb(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mcrypt_cfb", count, 4, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mcrypt_ecb(TypedValue* _rv, Value* cipher, Value* key, Value* data, int mode, Value* iv) asm("_ZN4HPHP12f_mcrypt_ecbERKNS_6StringES2_S2_iS2_");

void fg1_mcrypt_ecb(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mcrypt_ecb(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 5
    if (!IS_STRING_TYPE((args-4)->m_type)) {
      tvCastToStringInPlace(args-4);
    }
  case 4:
    break;
  }
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
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
  fh_mcrypt_ecb(rv, &args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (int)(args[-3].m_data.num), (count > 4) ? &args[-4].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mcrypt_ecb(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 4 && count <= 5) {
    if ((count <= 4 || IS_STRING_TYPE((args - 4)->m_type)) &&
        (args - 3)->m_type == KindOfInt64 &&
        IS_STRING_TYPE((args - 2)->m_type) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_mcrypt_ecb(rv, &args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (int)(args[-3].m_data.num), (count > 4) ? &args[-4].m_data : (Value*)(&null_string));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mcrypt_ecb(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mcrypt_ecb", count, 4, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mcrypt_ofb(TypedValue* _rv, Value* cipher, Value* key, Value* data, int mode, Value* iv) asm("_ZN4HPHP12f_mcrypt_ofbERKNS_6StringES2_S2_iS2_");

void fg1_mcrypt_ofb(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mcrypt_ofb(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 5
    if (!IS_STRING_TYPE((args-4)->m_type)) {
      tvCastToStringInPlace(args-4);
    }
  case 4:
    break;
  }
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
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
  fh_mcrypt_ofb(rv, &args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (int)(args[-3].m_data.num), (count > 4) ? &args[-4].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mcrypt_ofb(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 4 && count <= 5) {
    if ((count <= 4 || IS_STRING_TYPE((args - 4)->m_type)) &&
        (args - 3)->m_type == KindOfInt64 &&
        IS_STRING_TYPE((args - 2)->m_type) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_mcrypt_ofb(rv, &args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (int)(args[-3].m_data.num), (count > 4) ? &args[-4].m_data : (Value*)(&null_string));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mcrypt_ofb(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mcrypt_ofb", count, 4, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mcrypt_get_block_size(TypedValue* _rv, Value* cipher, Value* module) asm("_ZN4HPHP23f_mcrypt_get_block_sizeERKNS_6StringES2_");

void fg1_mcrypt_get_block_size(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mcrypt_get_block_size(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 2
    if (!IS_STRING_TYPE((args-1)->m_type)) {
      tvCastToStringInPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_mcrypt_get_block_size(rv, &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mcrypt_get_block_size(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || IS_STRING_TYPE((args - 1)->m_type)) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_mcrypt_get_block_size(rv, &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mcrypt_get_block_size(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mcrypt_get_block_size", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mcrypt_get_cipher_name(TypedValue* _rv, Value* cipher) asm("_ZN4HPHP24f_mcrypt_get_cipher_nameERKNS_6StringE");

void fg1_mcrypt_get_cipher_name(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mcrypt_get_cipher_name(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_mcrypt_get_cipher_name(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mcrypt_get_cipher_name(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_mcrypt_get_cipher_name(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mcrypt_get_cipher_name(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mcrypt_get_cipher_name", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mcrypt_get_iv_size(TypedValue* _rv, Value* cipher, Value* mode) asm("_ZN4HPHP20f_mcrypt_get_iv_sizeERKNS_6StringES2_");

void fg1_mcrypt_get_iv_size(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mcrypt_get_iv_size(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_mcrypt_get_iv_size(rv, &args[-0].m_data, &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mcrypt_get_iv_size(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_mcrypt_get_iv_size(rv, &args[-0].m_data, &args[-1].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mcrypt_get_iv_size(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mcrypt_get_iv_size", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_mcrypt_get_key_size(Value* cipher, Value* module) asm("_ZN4HPHP21f_mcrypt_get_key_sizeERKNS_6StringES2_");

void fg1_mcrypt_get_key_size(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mcrypt_get_key_size(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_mcrypt_get_key_size(&args[-0].m_data, &args[-1].m_data);
}

TypedValue* fg_mcrypt_get_key_size(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_mcrypt_get_key_size(&args[-0].m_data, &args[-1].m_data);
    } else {
      fg1_mcrypt_get_key_size(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mcrypt_get_key_size", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_mcrypt_enc_get_algorithms_name(Value* _rv, Value* td) asm("_ZN4HPHP32f_mcrypt_enc_get_algorithms_nameERKNS_6ObjectE");

void fg1_mcrypt_enc_get_algorithms_name(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mcrypt_enc_get_algorithms_name(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfString;
  fh_mcrypt_enc_get_algorithms_name(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_mcrypt_enc_get_algorithms_name(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfString;
      fh_mcrypt_enc_get_algorithms_name(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_mcrypt_enc_get_algorithms_name(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mcrypt_enc_get_algorithms_name", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_mcrypt_enc_get_block_size(Value* td) asm("_ZN4HPHP27f_mcrypt_enc_get_block_sizeERKNS_6ObjectE");

void fg1_mcrypt_enc_get_block_size(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mcrypt_enc_get_block_size(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_mcrypt_enc_get_block_size(&args[-0].m_data);
}

TypedValue* fg_mcrypt_enc_get_block_size(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_mcrypt_enc_get_block_size(&args[-0].m_data);
    } else {
      fg1_mcrypt_enc_get_block_size(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mcrypt_enc_get_block_size", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_mcrypt_enc_get_iv_size(Value* td) asm("_ZN4HPHP24f_mcrypt_enc_get_iv_sizeERKNS_6ObjectE");

void fg1_mcrypt_enc_get_iv_size(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mcrypt_enc_get_iv_size(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_mcrypt_enc_get_iv_size(&args[-0].m_data);
}

TypedValue* fg_mcrypt_enc_get_iv_size(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_mcrypt_enc_get_iv_size(&args[-0].m_data);
    } else {
      fg1_mcrypt_enc_get_iv_size(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mcrypt_enc_get_iv_size", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_mcrypt_enc_get_key_size(Value* td) asm("_ZN4HPHP25f_mcrypt_enc_get_key_sizeERKNS_6ObjectE");

void fg1_mcrypt_enc_get_key_size(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mcrypt_enc_get_key_size(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_mcrypt_enc_get_key_size(&args[-0].m_data);
}

TypedValue* fg_mcrypt_enc_get_key_size(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_mcrypt_enc_get_key_size(&args[-0].m_data);
    } else {
      fg1_mcrypt_enc_get_key_size(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mcrypt_enc_get_key_size", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_mcrypt_enc_get_modes_name(Value* _rv, Value* td) asm("_ZN4HPHP27f_mcrypt_enc_get_modes_nameERKNS_6ObjectE");

void fg1_mcrypt_enc_get_modes_name(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mcrypt_enc_get_modes_name(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfString;
  fh_mcrypt_enc_get_modes_name(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_mcrypt_enc_get_modes_name(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfString;
      fh_mcrypt_enc_get_modes_name(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_mcrypt_enc_get_modes_name(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mcrypt_enc_get_modes_name", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_mcrypt_enc_get_supported_key_sizes(Value* _rv, Value* td) asm("_ZN4HPHP36f_mcrypt_enc_get_supported_key_sizesERKNS_6ObjectE");

void fg1_mcrypt_enc_get_supported_key_sizes(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mcrypt_enc_get_supported_key_sizes(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfArray;
  fh_mcrypt_enc_get_supported_key_sizes(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_mcrypt_enc_get_supported_key_sizes(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfArray;
      fh_mcrypt_enc_get_supported_key_sizes(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_mcrypt_enc_get_supported_key_sizes(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mcrypt_enc_get_supported_key_sizes", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_mcrypt_enc_is_block_algorithm_mode(Value* td) asm("_ZN4HPHP36f_mcrypt_enc_is_block_algorithm_modeERKNS_6ObjectE");

void fg1_mcrypt_enc_is_block_algorithm_mode(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mcrypt_enc_is_block_algorithm_mode(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_mcrypt_enc_is_block_algorithm_mode(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_mcrypt_enc_is_block_algorithm_mode(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_mcrypt_enc_is_block_algorithm_mode(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_mcrypt_enc_is_block_algorithm_mode(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mcrypt_enc_is_block_algorithm_mode", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_mcrypt_enc_is_block_algorithm(Value* td) asm("_ZN4HPHP31f_mcrypt_enc_is_block_algorithmERKNS_6ObjectE");

void fg1_mcrypt_enc_is_block_algorithm(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mcrypt_enc_is_block_algorithm(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_mcrypt_enc_is_block_algorithm(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_mcrypt_enc_is_block_algorithm(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_mcrypt_enc_is_block_algorithm(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_mcrypt_enc_is_block_algorithm(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mcrypt_enc_is_block_algorithm", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_mcrypt_enc_is_block_mode(Value* td) asm("_ZN4HPHP26f_mcrypt_enc_is_block_modeERKNS_6ObjectE");

void fg1_mcrypt_enc_is_block_mode(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mcrypt_enc_is_block_mode(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_mcrypt_enc_is_block_mode(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_mcrypt_enc_is_block_mode(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_mcrypt_enc_is_block_mode(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_mcrypt_enc_is_block_mode(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mcrypt_enc_is_block_mode", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_mcrypt_enc_self_test(Value* td) asm("_ZN4HPHP22f_mcrypt_enc_self_testERKNS_6ObjectE");

void fg1_mcrypt_enc_self_test(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mcrypt_enc_self_test(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_mcrypt_enc_self_test(&args[-0].m_data);
}

TypedValue* fg_mcrypt_enc_self_test(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_mcrypt_enc_self_test(&args[-0].m_data);
    } else {
      fg1_mcrypt_enc_self_test(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mcrypt_enc_self_test", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_mcrypt_generic_init(Value* td, Value* key, Value* iv) asm("_ZN4HPHP21f_mcrypt_generic_initERKNS_6ObjectERKNS_6StringES5_");

void fg1_mcrypt_generic_init(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mcrypt_generic_init(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_mcrypt_generic_init(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data);
}

TypedValue* fg_mcrypt_generic_init(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if (IS_STRING_TYPE((args - 2)->m_type) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_mcrypt_generic_init(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data);
    } else {
      fg1_mcrypt_generic_init(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mcrypt_generic_init", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mcrypt_generic(TypedValue* _rv, Value* td, Value* data) asm("_ZN4HPHP16f_mcrypt_genericERKNS_6ObjectERKNS_6StringE");

void fg1_mcrypt_generic(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mcrypt_generic(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_mcrypt_generic(rv, &args[-0].m_data, &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mcrypt_generic(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      fh_mcrypt_generic(rv, &args[-0].m_data, &args[-1].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mcrypt_generic(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mcrypt_generic", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mdecrypt_generic(TypedValue* _rv, Value* td, Value* data) asm("_ZN4HPHP18f_mdecrypt_genericERKNS_6ObjectERKNS_6StringE");

void fg1_mdecrypt_generic(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mdecrypt_generic(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_mdecrypt_generic(rv, &args[-0].m_data, &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mdecrypt_generic(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      fh_mdecrypt_generic(rv, &args[-0].m_data, &args[-1].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mdecrypt_generic(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mdecrypt_generic", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_mcrypt_generic_deinit(Value* td) asm("_ZN4HPHP23f_mcrypt_generic_deinitERKNS_6ObjectE");

void fg1_mcrypt_generic_deinit(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mcrypt_generic_deinit(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_mcrypt_generic_deinit(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_mcrypt_generic_deinit(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_mcrypt_generic_deinit(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_mcrypt_generic_deinit(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mcrypt_generic_deinit", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_mcrypt_generic_end(Value* td) asm("_ZN4HPHP20f_mcrypt_generic_endERKNS_6ObjectE");

void fg1_mcrypt_generic_end(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mcrypt_generic_end(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_mcrypt_generic_end(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_mcrypt_generic_end(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_mcrypt_generic_end(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_mcrypt_generic_end(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mcrypt_generic_end", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

} // namespace HPHP
