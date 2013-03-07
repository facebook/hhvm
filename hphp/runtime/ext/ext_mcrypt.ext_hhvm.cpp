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
HPHP::Variant HPHP::f_mcrypt_module_open(HPHP::String const&, HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP20f_mcrypt_module_openERKNS_6StringES2_S2_S2_

(return value) => rax
_rv => rdi
algorithm => rsi
algorithm_directory => rdx
mode => rcx
mode_directory => r8
*/

TypedValue* fh_mcrypt_module_open(TypedValue* _rv, Value* algorithm, Value* algorithm_directory, Value* mode, Value* mode_directory) asm("_ZN4HPHP20f_mcrypt_module_openERKNS_6StringES2_S2_S2_");

TypedValue * fg1_mcrypt_module_open(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mcrypt_module_open(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
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
  fh_mcrypt_module_open((rv), (Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (Value*)(args-3));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mcrypt_module_open(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 4LL) {
      if (IS_STRING_TYPE((args-3)->m_type) && IS_STRING_TYPE((args-2)->m_type) && IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        fh_mcrypt_module_open((&(rv)), (Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (Value*)(args-3));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mcrypt_module_open(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mcrypt_module_open", count, 4, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_mcrypt_module_close(HPHP::Object const&)
_ZN4HPHP21f_mcrypt_module_closeERKNS_6ObjectE

(return value) => rax
td => rdi
*/

bool fh_mcrypt_module_close(Value* td) asm("_ZN4HPHP21f_mcrypt_module_closeERKNS_6ObjectE");

TypedValue * fg1_mcrypt_module_close(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mcrypt_module_close(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_mcrypt_module_close((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_mcrypt_module_close(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_mcrypt_module_close((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mcrypt_module_close(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mcrypt_module_close", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_mcrypt_list_algorithms(HPHP::String const&)
_ZN4HPHP24f_mcrypt_list_algorithmsERKNS_6StringE

(return value) => rax
_rv => rdi
lib_dir => rsi
*/

Value* fh_mcrypt_list_algorithms(Value* _rv, Value* lib_dir) asm("_ZN4HPHP24f_mcrypt_list_algorithmsERKNS_6StringE");

TypedValue * fg1_mcrypt_list_algorithms(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mcrypt_list_algorithms(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfArray;
  tvCastToStringInPlace(args-0);
  fh_mcrypt_list_algorithms((Value*)(rv), (count > 0) ? (Value*)(args-0) : (Value*)(&null_string));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mcrypt_list_algorithms(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      if ((count <= 0 || IS_STRING_TYPE((args-0)->m_type))) {
        rv.m_type = KindOfArray;
        fh_mcrypt_list_algorithms((Value*)(&(rv)), (count > 0) ? (Value*)(args-0) : (Value*)(&null_string));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mcrypt_list_algorithms(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("mcrypt_list_algorithms", 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_mcrypt_list_modes(HPHP::String const&)
_ZN4HPHP19f_mcrypt_list_modesERKNS_6StringE

(return value) => rax
_rv => rdi
lib_dir => rsi
*/

Value* fh_mcrypt_list_modes(Value* _rv, Value* lib_dir) asm("_ZN4HPHP19f_mcrypt_list_modesERKNS_6StringE");

TypedValue * fg1_mcrypt_list_modes(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mcrypt_list_modes(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfArray;
  tvCastToStringInPlace(args-0);
  fh_mcrypt_list_modes((Value*)(rv), (count > 0) ? (Value*)(args-0) : (Value*)(&null_string));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mcrypt_list_modes(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      if ((count <= 0 || IS_STRING_TYPE((args-0)->m_type))) {
        rv.m_type = KindOfArray;
        fh_mcrypt_list_modes((Value*)(&(rv)), (count > 0) ? (Value*)(args-0) : (Value*)(&null_string));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mcrypt_list_modes(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("mcrypt_list_modes", 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_mcrypt_module_get_algo_block_size(HPHP::String const&, HPHP::String const&)
_ZN4HPHP35f_mcrypt_module_get_algo_block_sizeERKNS_6StringES2_

(return value) => rax
algorithm => rdi
lib_dir => rsi
*/

long fh_mcrypt_module_get_algo_block_size(Value* algorithm, Value* lib_dir) asm("_ZN4HPHP35f_mcrypt_module_get_algo_block_sizeERKNS_6StringES2_");

TypedValue * fg1_mcrypt_module_get_algo_block_size(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mcrypt_module_get_algo_block_size(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfInt64;
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
  rv->m_data.num = (int64_t)fh_mcrypt_module_get_algo_block_size((Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_string));
  return rv;
}

TypedValue* fg_mcrypt_module_get_algo_block_size(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && IS_STRING_TYPE((args-0)->m_type)) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_mcrypt_module_get_algo_block_size((Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_string));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mcrypt_module_get_algo_block_size(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mcrypt_module_get_algo_block_size", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_mcrypt_module_get_algo_key_size(HPHP::String const&, HPHP::String const&)
_ZN4HPHP33f_mcrypt_module_get_algo_key_sizeERKNS_6StringES2_

(return value) => rax
algorithm => rdi
lib_dir => rsi
*/

long fh_mcrypt_module_get_algo_key_size(Value* algorithm, Value* lib_dir) asm("_ZN4HPHP33f_mcrypt_module_get_algo_key_sizeERKNS_6StringES2_");

TypedValue * fg1_mcrypt_module_get_algo_key_size(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mcrypt_module_get_algo_key_size(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfInt64;
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
  rv->m_data.num = (int64_t)fh_mcrypt_module_get_algo_key_size((Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_string));
  return rv;
}

TypedValue* fg_mcrypt_module_get_algo_key_size(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && IS_STRING_TYPE((args-0)->m_type)) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_mcrypt_module_get_algo_key_size((Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_string));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mcrypt_module_get_algo_key_size(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mcrypt_module_get_algo_key_size", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_mcrypt_module_get_supported_key_sizes(HPHP::String const&, HPHP::String const&)
_ZN4HPHP39f_mcrypt_module_get_supported_key_sizesERKNS_6StringES2_

(return value) => rax
_rv => rdi
algorithm => rsi
lib_dir => rdx
*/

Value* fh_mcrypt_module_get_supported_key_sizes(Value* _rv, Value* algorithm, Value* lib_dir) asm("_ZN4HPHP39f_mcrypt_module_get_supported_key_sizesERKNS_6StringES2_");

TypedValue * fg1_mcrypt_module_get_supported_key_sizes(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mcrypt_module_get_supported_key_sizes(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfArray;
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
  fh_mcrypt_module_get_supported_key_sizes((Value*)(rv), (Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_string));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mcrypt_module_get_supported_key_sizes(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && IS_STRING_TYPE((args-0)->m_type)) {
        rv.m_type = KindOfArray;
        fh_mcrypt_module_get_supported_key_sizes((Value*)(&(rv)), (Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_string));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mcrypt_module_get_supported_key_sizes(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mcrypt_module_get_supported_key_sizes", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_mcrypt_module_is_block_algorithm_mode(HPHP::String const&, HPHP::String const&)
_ZN4HPHP39f_mcrypt_module_is_block_algorithm_modeERKNS_6StringES2_

(return value) => rax
mode => rdi
lib_dir => rsi
*/

bool fh_mcrypt_module_is_block_algorithm_mode(Value* mode, Value* lib_dir) asm("_ZN4HPHP39f_mcrypt_module_is_block_algorithm_modeERKNS_6StringES2_");

TypedValue * fg1_mcrypt_module_is_block_algorithm_mode(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mcrypt_module_is_block_algorithm_mode(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
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
  rv->m_data.num = (fh_mcrypt_module_is_block_algorithm_mode((Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_string))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_mcrypt_module_is_block_algorithm_mode(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && IS_STRING_TYPE((args-0)->m_type)) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_mcrypt_module_is_block_algorithm_mode((Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_string))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mcrypt_module_is_block_algorithm_mode(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mcrypt_module_is_block_algorithm_mode", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_mcrypt_module_is_block_algorithm(HPHP::String const&, HPHP::String const&)
_ZN4HPHP34f_mcrypt_module_is_block_algorithmERKNS_6StringES2_

(return value) => rax
algorithm => rdi
lib_dir => rsi
*/

bool fh_mcrypt_module_is_block_algorithm(Value* algorithm, Value* lib_dir) asm("_ZN4HPHP34f_mcrypt_module_is_block_algorithmERKNS_6StringES2_");

TypedValue * fg1_mcrypt_module_is_block_algorithm(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mcrypt_module_is_block_algorithm(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
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
  rv->m_data.num = (fh_mcrypt_module_is_block_algorithm((Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_string))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_mcrypt_module_is_block_algorithm(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && IS_STRING_TYPE((args-0)->m_type)) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_mcrypt_module_is_block_algorithm((Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_string))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mcrypt_module_is_block_algorithm(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mcrypt_module_is_block_algorithm", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_mcrypt_module_is_block_mode(HPHP::String const&, HPHP::String const&)
_ZN4HPHP29f_mcrypt_module_is_block_modeERKNS_6StringES2_

(return value) => rax
mode => rdi
lib_dir => rsi
*/

bool fh_mcrypt_module_is_block_mode(Value* mode, Value* lib_dir) asm("_ZN4HPHP29f_mcrypt_module_is_block_modeERKNS_6StringES2_");

TypedValue * fg1_mcrypt_module_is_block_mode(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mcrypt_module_is_block_mode(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
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
  rv->m_data.num = (fh_mcrypt_module_is_block_mode((Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_string))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_mcrypt_module_is_block_mode(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && IS_STRING_TYPE((args-0)->m_type)) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_mcrypt_module_is_block_mode((Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_string))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mcrypt_module_is_block_mode(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mcrypt_module_is_block_mode", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_mcrypt_module_self_test(HPHP::String const&, HPHP::String const&)
_ZN4HPHP25f_mcrypt_module_self_testERKNS_6StringES2_

(return value) => rax
algorithm => rdi
lib_dir => rsi
*/

bool fh_mcrypt_module_self_test(Value* algorithm, Value* lib_dir) asm("_ZN4HPHP25f_mcrypt_module_self_testERKNS_6StringES2_");

TypedValue * fg1_mcrypt_module_self_test(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mcrypt_module_self_test(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
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
  rv->m_data.num = (fh_mcrypt_module_self_test((Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_string))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_mcrypt_module_self_test(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && IS_STRING_TYPE((args-0)->m_type)) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_mcrypt_module_self_test((Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_string))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mcrypt_module_self_test(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mcrypt_module_self_test", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mcrypt_create_iv(int, int)
_ZN4HPHP18f_mcrypt_create_ivEii

(return value) => rax
_rv => rdi
size => rsi
source => rdx
*/

TypedValue* fh_mcrypt_create_iv(TypedValue* _rv, int size, int source) asm("_ZN4HPHP18f_mcrypt_create_ivEii");

TypedValue * fg1_mcrypt_create_iv(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mcrypt_create_iv(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
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
  fh_mcrypt_create_iv((rv), (int)(args[-0].m_data.num), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mcrypt_create_iv(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64) && (args-0)->m_type == KindOfInt64) {
        fh_mcrypt_create_iv((&(rv)), (int)(args[-0].m_data.num), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mcrypt_create_iv(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mcrypt_create_iv", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mcrypt_encrypt(HPHP::String const&, HPHP::String const&, HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP16f_mcrypt_encryptERKNS_6StringES2_S2_S2_S2_

(return value) => rax
_rv => rdi
cipher => rsi
key => rdx
data => rcx
mode => r8
iv => r9
*/

TypedValue* fh_mcrypt_encrypt(TypedValue* _rv, Value* cipher, Value* key, Value* data, Value* mode, Value* iv) asm("_ZN4HPHP16f_mcrypt_encryptERKNS_6StringES2_S2_S2_S2_");

TypedValue * fg1_mcrypt_encrypt(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mcrypt_encrypt(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
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
  fh_mcrypt_encrypt((rv), (Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (Value*)(args-3), (count > 4) ? (Value*)(args-4) : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mcrypt_encrypt(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 4LL && count <= 5LL) {
      if ((count <= 4 || IS_STRING_TYPE((args-4)->m_type)) && IS_STRING_TYPE((args-3)->m_type) && IS_STRING_TYPE((args-2)->m_type) && IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        fh_mcrypt_encrypt((&(rv)), (Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (Value*)(args-3), (count > 4) ? (Value*)(args-4) : (Value*)(&null_string));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mcrypt_encrypt(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mcrypt_encrypt", count, 4, 5, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 5);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mcrypt_decrypt(HPHP::String const&, HPHP::String const&, HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP16f_mcrypt_decryptERKNS_6StringES2_S2_S2_S2_

(return value) => rax
_rv => rdi
cipher => rsi
key => rdx
data => rcx
mode => r8
iv => r9
*/

TypedValue* fh_mcrypt_decrypt(TypedValue* _rv, Value* cipher, Value* key, Value* data, Value* mode, Value* iv) asm("_ZN4HPHP16f_mcrypt_decryptERKNS_6StringES2_S2_S2_S2_");

TypedValue * fg1_mcrypt_decrypt(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mcrypt_decrypt(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
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
  fh_mcrypt_decrypt((rv), (Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (Value*)(args-3), (count > 4) ? (Value*)(args-4) : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mcrypt_decrypt(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 4LL && count <= 5LL) {
      if ((count <= 4 || IS_STRING_TYPE((args-4)->m_type)) && IS_STRING_TYPE((args-3)->m_type) && IS_STRING_TYPE((args-2)->m_type) && IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        fh_mcrypt_decrypt((&(rv)), (Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (Value*)(args-3), (count > 4) ? (Value*)(args-4) : (Value*)(&null_string));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mcrypt_decrypt(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mcrypt_decrypt", count, 4, 5, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 5);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mcrypt_cbc(HPHP::String const&, HPHP::String const&, HPHP::String const&, int, HPHP::String const&)
_ZN4HPHP12f_mcrypt_cbcERKNS_6StringES2_S2_iS2_

(return value) => rax
_rv => rdi
cipher => rsi
key => rdx
data => rcx
mode => r8
iv => r9
*/

TypedValue* fh_mcrypt_cbc(TypedValue* _rv, Value* cipher, Value* key, Value* data, int mode, Value* iv) asm("_ZN4HPHP12f_mcrypt_cbcERKNS_6StringES2_S2_iS2_");

TypedValue * fg1_mcrypt_cbc(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mcrypt_cbc(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
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
  fh_mcrypt_cbc((rv), (Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (int)(args[-3].m_data.num), (count > 4) ? (Value*)(args-4) : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mcrypt_cbc(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 4LL && count <= 5LL) {
      if ((count <= 4 || IS_STRING_TYPE((args-4)->m_type)) && (args-3)->m_type == KindOfInt64 && IS_STRING_TYPE((args-2)->m_type) && IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        fh_mcrypt_cbc((&(rv)), (Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (int)(args[-3].m_data.num), (count > 4) ? (Value*)(args-4) : (Value*)(&null_string));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mcrypt_cbc(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mcrypt_cbc", count, 4, 5, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 5);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mcrypt_cfb(HPHP::String const&, HPHP::String const&, HPHP::String const&, int, HPHP::String const&)
_ZN4HPHP12f_mcrypt_cfbERKNS_6StringES2_S2_iS2_

(return value) => rax
_rv => rdi
cipher => rsi
key => rdx
data => rcx
mode => r8
iv => r9
*/

TypedValue* fh_mcrypt_cfb(TypedValue* _rv, Value* cipher, Value* key, Value* data, int mode, Value* iv) asm("_ZN4HPHP12f_mcrypt_cfbERKNS_6StringES2_S2_iS2_");

TypedValue * fg1_mcrypt_cfb(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mcrypt_cfb(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
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
  fh_mcrypt_cfb((rv), (Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (int)(args[-3].m_data.num), (count > 4) ? (Value*)(args-4) : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mcrypt_cfb(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 4LL && count <= 5LL) {
      if ((count <= 4 || IS_STRING_TYPE((args-4)->m_type)) && (args-3)->m_type == KindOfInt64 && IS_STRING_TYPE((args-2)->m_type) && IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        fh_mcrypt_cfb((&(rv)), (Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (int)(args[-3].m_data.num), (count > 4) ? (Value*)(args-4) : (Value*)(&null_string));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mcrypt_cfb(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mcrypt_cfb", count, 4, 5, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 5);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mcrypt_ecb(HPHP::String const&, HPHP::String const&, HPHP::String const&, int, HPHP::String const&)
_ZN4HPHP12f_mcrypt_ecbERKNS_6StringES2_S2_iS2_

(return value) => rax
_rv => rdi
cipher => rsi
key => rdx
data => rcx
mode => r8
iv => r9
*/

TypedValue* fh_mcrypt_ecb(TypedValue* _rv, Value* cipher, Value* key, Value* data, int mode, Value* iv) asm("_ZN4HPHP12f_mcrypt_ecbERKNS_6StringES2_S2_iS2_");

TypedValue * fg1_mcrypt_ecb(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mcrypt_ecb(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
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
  fh_mcrypt_ecb((rv), (Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (int)(args[-3].m_data.num), (count > 4) ? (Value*)(args-4) : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mcrypt_ecb(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 4LL && count <= 5LL) {
      if ((count <= 4 || IS_STRING_TYPE((args-4)->m_type)) && (args-3)->m_type == KindOfInt64 && IS_STRING_TYPE((args-2)->m_type) && IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        fh_mcrypt_ecb((&(rv)), (Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (int)(args[-3].m_data.num), (count > 4) ? (Value*)(args-4) : (Value*)(&null_string));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mcrypt_ecb(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mcrypt_ecb", count, 4, 5, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 5);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mcrypt_ofb(HPHP::String const&, HPHP::String const&, HPHP::String const&, int, HPHP::String const&)
_ZN4HPHP12f_mcrypt_ofbERKNS_6StringES2_S2_iS2_

(return value) => rax
_rv => rdi
cipher => rsi
key => rdx
data => rcx
mode => r8
iv => r9
*/

TypedValue* fh_mcrypt_ofb(TypedValue* _rv, Value* cipher, Value* key, Value* data, int mode, Value* iv) asm("_ZN4HPHP12f_mcrypt_ofbERKNS_6StringES2_S2_iS2_");

TypedValue * fg1_mcrypt_ofb(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mcrypt_ofb(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
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
  fh_mcrypt_ofb((rv), (Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (int)(args[-3].m_data.num), (count > 4) ? (Value*)(args-4) : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mcrypt_ofb(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 4LL && count <= 5LL) {
      if ((count <= 4 || IS_STRING_TYPE((args-4)->m_type)) && (args-3)->m_type == KindOfInt64 && IS_STRING_TYPE((args-2)->m_type) && IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        fh_mcrypt_ofb((&(rv)), (Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (int)(args[-3].m_data.num), (count > 4) ? (Value*)(args-4) : (Value*)(&null_string));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mcrypt_ofb(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mcrypt_ofb", count, 4, 5, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 5);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mcrypt_get_block_size(HPHP::String const&, HPHP::String const&)
_ZN4HPHP23f_mcrypt_get_block_sizeERKNS_6StringES2_

(return value) => rax
_rv => rdi
cipher => rsi
module => rdx
*/

TypedValue* fh_mcrypt_get_block_size(TypedValue* _rv, Value* cipher, Value* module) asm("_ZN4HPHP23f_mcrypt_get_block_sizeERKNS_6StringES2_");

TypedValue * fg1_mcrypt_get_block_size(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mcrypt_get_block_size(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
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
  fh_mcrypt_get_block_size((rv), (Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mcrypt_get_block_size(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && IS_STRING_TYPE((args-0)->m_type)) {
        fh_mcrypt_get_block_size((&(rv)), (Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_string));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mcrypt_get_block_size(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mcrypt_get_block_size", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mcrypt_get_cipher_name(HPHP::String const&)
_ZN4HPHP24f_mcrypt_get_cipher_nameERKNS_6StringE

(return value) => rax
_rv => rdi
cipher => rsi
*/

TypedValue* fh_mcrypt_get_cipher_name(TypedValue* _rv, Value* cipher) asm("_ZN4HPHP24f_mcrypt_get_cipher_nameERKNS_6StringE");

TypedValue * fg1_mcrypt_get_cipher_name(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mcrypt_get_cipher_name(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_mcrypt_get_cipher_name((rv), (Value*)(args-0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mcrypt_get_cipher_name(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        fh_mcrypt_get_cipher_name((&(rv)), (Value*)(args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mcrypt_get_cipher_name(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mcrypt_get_cipher_name", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mcrypt_get_iv_size(HPHP::String const&, HPHP::String const&)
_ZN4HPHP20f_mcrypt_get_iv_sizeERKNS_6StringES2_

(return value) => rax
_rv => rdi
cipher => rsi
mode => rdx
*/

TypedValue* fh_mcrypt_get_iv_size(TypedValue* _rv, Value* cipher, Value* mode) asm("_ZN4HPHP20f_mcrypt_get_iv_sizeERKNS_6StringES2_");

TypedValue * fg1_mcrypt_get_iv_size(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mcrypt_get_iv_size(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_mcrypt_get_iv_size((rv), (Value*)(args-0), (Value*)(args-1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mcrypt_get_iv_size(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        fh_mcrypt_get_iv_size((&(rv)), (Value*)(args-0), (Value*)(args-1));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mcrypt_get_iv_size(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mcrypt_get_iv_size", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_mcrypt_get_key_size(HPHP::String const&, HPHP::String const&)
_ZN4HPHP21f_mcrypt_get_key_sizeERKNS_6StringES2_

(return value) => rax
cipher => rdi
module => rsi
*/

long fh_mcrypt_get_key_size(Value* cipher, Value* module) asm("_ZN4HPHP21f_mcrypt_get_key_sizeERKNS_6StringES2_");

TypedValue * fg1_mcrypt_get_key_size(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mcrypt_get_key_size(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfInt64;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (int64_t)fh_mcrypt_get_key_size((Value*)(args-0), (Value*)(args-1));
  return rv;
}

TypedValue* fg_mcrypt_get_key_size(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_mcrypt_get_key_size((Value*)(args-0), (Value*)(args-1));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mcrypt_get_key_size(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mcrypt_get_key_size", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_mcrypt_enc_get_algorithms_name(HPHP::Object const&)
_ZN4HPHP32f_mcrypt_enc_get_algorithms_nameERKNS_6ObjectE

(return value) => rax
_rv => rdi
td => rsi
*/

Value* fh_mcrypt_enc_get_algorithms_name(Value* _rv, Value* td) asm("_ZN4HPHP32f_mcrypt_enc_get_algorithms_nameERKNS_6ObjectE");

TypedValue * fg1_mcrypt_enc_get_algorithms_name(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mcrypt_enc_get_algorithms_name(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfString;
  tvCastToObjectInPlace(args-0);
  fh_mcrypt_enc_get_algorithms_name((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mcrypt_enc_get_algorithms_name(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfString;
        fh_mcrypt_enc_get_algorithms_name((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mcrypt_enc_get_algorithms_name(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mcrypt_enc_get_algorithms_name", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_mcrypt_enc_get_block_size(HPHP::Object const&)
_ZN4HPHP27f_mcrypt_enc_get_block_sizeERKNS_6ObjectE

(return value) => rax
td => rdi
*/

long fh_mcrypt_enc_get_block_size(Value* td) asm("_ZN4HPHP27f_mcrypt_enc_get_block_sizeERKNS_6ObjectE");

TypedValue * fg1_mcrypt_enc_get_block_size(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mcrypt_enc_get_block_size(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfInt64;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (int64_t)fh_mcrypt_enc_get_block_size((Value*)(args-0));
  return rv;
}

TypedValue* fg_mcrypt_enc_get_block_size(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_mcrypt_enc_get_block_size((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mcrypt_enc_get_block_size(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mcrypt_enc_get_block_size", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_mcrypt_enc_get_iv_size(HPHP::Object const&)
_ZN4HPHP24f_mcrypt_enc_get_iv_sizeERKNS_6ObjectE

(return value) => rax
td => rdi
*/

long fh_mcrypt_enc_get_iv_size(Value* td) asm("_ZN4HPHP24f_mcrypt_enc_get_iv_sizeERKNS_6ObjectE");

TypedValue * fg1_mcrypt_enc_get_iv_size(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mcrypt_enc_get_iv_size(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfInt64;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (int64_t)fh_mcrypt_enc_get_iv_size((Value*)(args-0));
  return rv;
}

TypedValue* fg_mcrypt_enc_get_iv_size(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_mcrypt_enc_get_iv_size((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mcrypt_enc_get_iv_size(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mcrypt_enc_get_iv_size", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_mcrypt_enc_get_key_size(HPHP::Object const&)
_ZN4HPHP25f_mcrypt_enc_get_key_sizeERKNS_6ObjectE

(return value) => rax
td => rdi
*/

long fh_mcrypt_enc_get_key_size(Value* td) asm("_ZN4HPHP25f_mcrypt_enc_get_key_sizeERKNS_6ObjectE");

TypedValue * fg1_mcrypt_enc_get_key_size(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mcrypt_enc_get_key_size(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfInt64;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (int64_t)fh_mcrypt_enc_get_key_size((Value*)(args-0));
  return rv;
}

TypedValue* fg_mcrypt_enc_get_key_size(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_mcrypt_enc_get_key_size((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mcrypt_enc_get_key_size(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mcrypt_enc_get_key_size", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_mcrypt_enc_get_modes_name(HPHP::Object const&)
_ZN4HPHP27f_mcrypt_enc_get_modes_nameERKNS_6ObjectE

(return value) => rax
_rv => rdi
td => rsi
*/

Value* fh_mcrypt_enc_get_modes_name(Value* _rv, Value* td) asm("_ZN4HPHP27f_mcrypt_enc_get_modes_nameERKNS_6ObjectE");

TypedValue * fg1_mcrypt_enc_get_modes_name(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mcrypt_enc_get_modes_name(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfString;
  tvCastToObjectInPlace(args-0);
  fh_mcrypt_enc_get_modes_name((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mcrypt_enc_get_modes_name(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfString;
        fh_mcrypt_enc_get_modes_name((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mcrypt_enc_get_modes_name(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mcrypt_enc_get_modes_name", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_mcrypt_enc_get_supported_key_sizes(HPHP::Object const&)
_ZN4HPHP36f_mcrypt_enc_get_supported_key_sizesERKNS_6ObjectE

(return value) => rax
_rv => rdi
td => rsi
*/

Value* fh_mcrypt_enc_get_supported_key_sizes(Value* _rv, Value* td) asm("_ZN4HPHP36f_mcrypt_enc_get_supported_key_sizesERKNS_6ObjectE");

TypedValue * fg1_mcrypt_enc_get_supported_key_sizes(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mcrypt_enc_get_supported_key_sizes(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfArray;
  tvCastToObjectInPlace(args-0);
  fh_mcrypt_enc_get_supported_key_sizes((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mcrypt_enc_get_supported_key_sizes(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfArray;
        fh_mcrypt_enc_get_supported_key_sizes((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mcrypt_enc_get_supported_key_sizes(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mcrypt_enc_get_supported_key_sizes", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_mcrypt_enc_is_block_algorithm_mode(HPHP::Object const&)
_ZN4HPHP36f_mcrypt_enc_is_block_algorithm_modeERKNS_6ObjectE

(return value) => rax
td => rdi
*/

bool fh_mcrypt_enc_is_block_algorithm_mode(Value* td) asm("_ZN4HPHP36f_mcrypt_enc_is_block_algorithm_modeERKNS_6ObjectE");

TypedValue * fg1_mcrypt_enc_is_block_algorithm_mode(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mcrypt_enc_is_block_algorithm_mode(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_mcrypt_enc_is_block_algorithm_mode((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_mcrypt_enc_is_block_algorithm_mode(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_mcrypt_enc_is_block_algorithm_mode((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mcrypt_enc_is_block_algorithm_mode(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mcrypt_enc_is_block_algorithm_mode", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_mcrypt_enc_is_block_algorithm(HPHP::Object const&)
_ZN4HPHP31f_mcrypt_enc_is_block_algorithmERKNS_6ObjectE

(return value) => rax
td => rdi
*/

bool fh_mcrypt_enc_is_block_algorithm(Value* td) asm("_ZN4HPHP31f_mcrypt_enc_is_block_algorithmERKNS_6ObjectE");

TypedValue * fg1_mcrypt_enc_is_block_algorithm(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mcrypt_enc_is_block_algorithm(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_mcrypt_enc_is_block_algorithm((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_mcrypt_enc_is_block_algorithm(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_mcrypt_enc_is_block_algorithm((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mcrypt_enc_is_block_algorithm(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mcrypt_enc_is_block_algorithm", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_mcrypt_enc_is_block_mode(HPHP::Object const&)
_ZN4HPHP26f_mcrypt_enc_is_block_modeERKNS_6ObjectE

(return value) => rax
td => rdi
*/

bool fh_mcrypt_enc_is_block_mode(Value* td) asm("_ZN4HPHP26f_mcrypt_enc_is_block_modeERKNS_6ObjectE");

TypedValue * fg1_mcrypt_enc_is_block_mode(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mcrypt_enc_is_block_mode(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_mcrypt_enc_is_block_mode((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_mcrypt_enc_is_block_mode(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_mcrypt_enc_is_block_mode((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mcrypt_enc_is_block_mode(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mcrypt_enc_is_block_mode", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_mcrypt_enc_self_test(HPHP::Object const&)
_ZN4HPHP22f_mcrypt_enc_self_testERKNS_6ObjectE

(return value) => rax
td => rdi
*/

long fh_mcrypt_enc_self_test(Value* td) asm("_ZN4HPHP22f_mcrypt_enc_self_testERKNS_6ObjectE");

TypedValue * fg1_mcrypt_enc_self_test(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mcrypt_enc_self_test(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfInt64;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (int64_t)fh_mcrypt_enc_self_test((Value*)(args-0));
  return rv;
}

TypedValue* fg_mcrypt_enc_self_test(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_mcrypt_enc_self_test((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mcrypt_enc_self_test(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mcrypt_enc_self_test", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mcrypt_generic(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP16f_mcrypt_genericERKNS_6ObjectERKNS_6StringE

(return value) => rax
_rv => rdi
td => rsi
data => rdx
*/

TypedValue* fh_mcrypt_generic(TypedValue* _rv, Value* td, Value* data) asm("_ZN4HPHP16f_mcrypt_genericERKNS_6ObjectERKNS_6StringE");

TypedValue * fg1_mcrypt_generic(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mcrypt_generic(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_mcrypt_generic((rv), (Value*)(args-0), (Value*)(args-1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mcrypt_generic(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        fh_mcrypt_generic((&(rv)), (Value*)(args-0), (Value*)(args-1));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mcrypt_generic(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mcrypt_generic", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_mcrypt_generic_init(HPHP::Object const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP21f_mcrypt_generic_initERKNS_6ObjectERKNS_6StringES5_

(return value) => rax
td => rdi
key => rsi
iv => rdx
*/

long fh_mcrypt_generic_init(Value* td, Value* key, Value* iv) asm("_ZN4HPHP21f_mcrypt_generic_initERKNS_6ObjectERKNS_6StringES5_");

TypedValue * fg1_mcrypt_generic_init(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mcrypt_generic_init(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfInt64;
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (int64_t)fh_mcrypt_generic_init((Value*)(args-0), (Value*)(args-1), (Value*)(args-2));
  return rv;
}

TypedValue* fg_mcrypt_generic_init(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if (IS_STRING_TYPE((args-2)->m_type) && IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_mcrypt_generic_init((Value*)(args-0), (Value*)(args-1), (Value*)(args-2));
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mcrypt_generic_init(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mcrypt_generic_init", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mdecrypt_generic(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP18f_mdecrypt_genericERKNS_6ObjectERKNS_6StringE

(return value) => rax
_rv => rdi
td => rsi
data => rdx
*/

TypedValue* fh_mdecrypt_generic(TypedValue* _rv, Value* td, Value* data) asm("_ZN4HPHP18f_mdecrypt_genericERKNS_6ObjectERKNS_6StringE");

TypedValue * fg1_mdecrypt_generic(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mdecrypt_generic(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_mdecrypt_generic((rv), (Value*)(args-0), (Value*)(args-1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mdecrypt_generic(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        fh_mdecrypt_generic((&(rv)), (Value*)(args-0), (Value*)(args-1));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mdecrypt_generic(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mdecrypt_generic", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_mcrypt_generic_deinit(HPHP::Object const&)
_ZN4HPHP23f_mcrypt_generic_deinitERKNS_6ObjectE

(return value) => rax
td => rdi
*/

bool fh_mcrypt_generic_deinit(Value* td) asm("_ZN4HPHP23f_mcrypt_generic_deinitERKNS_6ObjectE");

TypedValue * fg1_mcrypt_generic_deinit(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mcrypt_generic_deinit(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_mcrypt_generic_deinit((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_mcrypt_generic_deinit(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_mcrypt_generic_deinit((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mcrypt_generic_deinit(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mcrypt_generic_deinit", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_mcrypt_generic_end(HPHP::Object const&)
_ZN4HPHP20f_mcrypt_generic_endERKNS_6ObjectE

(return value) => rax
td => rdi
*/

bool fh_mcrypt_generic_end(Value* td) asm("_ZN4HPHP20f_mcrypt_generic_endERKNS_6ObjectE");

TypedValue * fg1_mcrypt_generic_end(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mcrypt_generic_end(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_mcrypt_generic_end((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_mcrypt_generic_end(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_mcrypt_generic_end((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mcrypt_generic_end(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mcrypt_generic_end", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}




} // !HPHP

