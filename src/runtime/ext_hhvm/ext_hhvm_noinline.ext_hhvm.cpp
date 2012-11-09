/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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
bool HPHP::fni_apc_compile_file(HPHP::String const&, bool, long long)
_ZN4HPHP20fni_apc_compile_fileERKNS_6StringEbx

(return value) => rax
filename => rdi
atomic => rsi
cache_id => rdx
*/

bool fh_apc_compile_file(Value* filename, bool atomic, long long cache_id) asm("_ZN4HPHP20fni_apc_compile_fileERKNS_6StringEbx");

TypedValue * fg1_apc_compile_file(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_apc_compile_file(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    if ((args-1)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (fh_apc_compile_file((Value*)(args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true), (count > 2) ? (long long)(args[-2].m_data.num) : (long long)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_apc_compile_file(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfInt64) && (count <= 1 || (args-1)->m_type == KindOfBoolean) && IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_apc_compile_file((Value*)(args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true), (count > 2) ? (long long)(args[-2].m_data.num) : (long long)(0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_apc_compile_file(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("apc_compile_file", count, 1, 3, 1);
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
bool HPHP::fni_apc_define_constants(HPHP::String const&, HPHP::String const&, bool, long long)
_ZN4HPHP24fni_apc_define_constantsERKNS_6StringES2_bx

(return value) => rax
key => rdi
constants => rsi
case_sensitive => rdx
cache_id => rcx
*/

bool fh_apc_define_constants(Value* key, Value* constants, bool case_sensitive, long long cache_id) asm("_ZN4HPHP24fni_apc_define_constantsERKNS_6StringES2_bx");

TypedValue * fg1_apc_define_constants(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_apc_define_constants(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
    if ((args-2)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-2);
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
  rv->m_data.num = (fh_apc_define_constants((Value*)(args-0), (Value*)(args-1), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(true), (count > 3) ? (long long)(args[-3].m_data.num) : (long long)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_apc_define_constants(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 4LL) {
      if ((count <= 3 || (args-3)->m_type == KindOfInt64) && (count <= 2 || (args-2)->m_type == KindOfBoolean) && IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_apc_define_constants((Value*)(args-0), (Value*)(args-1), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(true), (count > 3) ? (long long)(args[-3].m_data.num) : (long long)(0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_apc_define_constants(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("apc_define_constants", count, 2, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::fni_apc_load_constants(HPHP::String const&, bool, long long)
_ZN4HPHP22fni_apc_load_constantsERKNS_6StringEbx

(return value) => rax
key => rdi
case_sensitive => rsi
cache_id => rdx
*/

bool fh_apc_load_constants(Value* key, bool case_sensitive, long long cache_id) asm("_ZN4HPHP22fni_apc_load_constantsERKNS_6StringEbx");

TypedValue * fg1_apc_load_constants(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_apc_load_constants(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    if ((args-1)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (fh_apc_load_constants((Value*)(args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true), (count > 2) ? (long long)(args[-2].m_data.num) : (long long)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_apc_load_constants(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfInt64) && (count <= 1 || (args-1)->m_type == KindOfBoolean) && IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_apc_load_constants((Value*)(args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true), (count > 2) ? (long long)(args[-2].m_data.num) : (long long)(0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_apc_load_constants(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("apc_load_constants", count, 1, 3, 1);
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
HPHP::Array HPHP::fni_apc_sma_info(bool)
_ZN4HPHP16fni_apc_sma_infoEb

(return value) => rax
_rv => rdi
limited => rsi
*/

Value* fh_apc_sma_info(Value* _rv, bool limited) asm("_ZN4HPHP16fni_apc_sma_infoEb");

TypedValue * fg1_apc_sma_info(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_apc_sma_info(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfArray;
  tvCastToBooleanInPlace(args-0);
  fh_apc_sma_info((Value*)(rv), (count > 0) ? (bool)(args[-0].m_data.num) : (bool)(false));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_apc_sma_info(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      if ((count <= 0 || (args-0)->m_type == KindOfBoolean)) {
        rv._count = 0;
        rv.m_type = KindOfArray;
        fh_apc_sma_info((Value*)(&(rv)), (count > 0) ? (bool)(args[-0].m_data.num) : (bool)(false));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_apc_sma_info(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("apc_sma_info", 1, 1);
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
HPHP::Array HPHP::fni_apc_filehits()
_ZN4HPHP16fni_apc_filehitsEv

(return value) => rax
_rv => rdi
*/

Value* fh_apc_filehits(Value* _rv) asm("_ZN4HPHP16fni_apc_filehitsEv");

TypedValue* fg_apc_filehits(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfArray;
      fh_apc_filehits((Value*)(&(rv)));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("apc_filehits", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::fni_apc_delete_file(HPHP::Variant const&, long long)
_ZN4HPHP19fni_apc_delete_fileERKNS_7VariantEx

(return value) => rax
_rv => rdi
keys => rsi
cache_id => rdx
*/

TypedValue* fh_apc_delete_file(TypedValue* _rv, TypedValue* keys, long long cache_id) asm("_ZN4HPHP19fni_apc_delete_fileERKNS_7VariantEx");

TypedValue * fg1_apc_delete_file(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_apc_delete_file(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-1);
  fh_apc_delete_file((rv), (args-0), (count > 1) ? (long long)(args[-1].m_data.num) : (long long)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_apc_delete_file(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64)) {
        fh_apc_delete_file((&(rv)), (args-0), (count > 1) ? (long long)(args[-1].m_data.num) : (long long)(0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_apc_delete_file(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("apc_delete_file", count, 1, 2, 1);
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
HPHP::Variant HPHP::fni_apc_bin_dump(long long, HPHP::Variant const&)
_ZN4HPHP16fni_apc_bin_dumpExRKNS_7VariantE

(return value) => rax
_rv => rdi
cache_id => rsi
filter => rdx
*/

TypedValue* fh_apc_bin_dump(TypedValue* _rv, long long cache_id, TypedValue* filter) asm("_ZN4HPHP16fni_apc_bin_dumpExRKNS_7VariantE");

TypedValue * fg1_apc_bin_dump(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_apc_bin_dump(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  fh_apc_bin_dump((rv), (count > 0) ? (long long)(args[-0].m_data.num) : (long long)(0), (count > 1) ? (args-1) : (TypedValue*)(&null_variant));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_apc_bin_dump(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 2LL) {
      if ((count <= 0 || (args-0)->m_type == KindOfInt64)) {
        fh_apc_bin_dump((&(rv)), (count > 0) ? (long long)(args[-0].m_data.num) : (long long)(0), (count > 1) ? (args-1) : (TypedValue*)(&null_variant));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_apc_bin_dump(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("apc_bin_dump", 2, 1);
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
bool HPHP::fni_apc_bin_load(HPHP::String const&, long long, long long)
_ZN4HPHP16fni_apc_bin_loadERKNS_6StringExx

(return value) => rax
data => rdi
flags => rsi
cache_id => rdx
*/

bool fh_apc_bin_load(Value* data, long long flags, long long cache_id) asm("_ZN4HPHP16fni_apc_bin_loadERKNS_6StringExx");

TypedValue * fg1_apc_bin_load(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_apc_bin_load(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
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
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (fh_apc_bin_load((Value*)(args-0), (count > 1) ? (long long)(args[-1].m_data.num) : (long long)(0), (count > 2) ? (long long)(args[-2].m_data.num) : (long long)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_apc_bin_load(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfInt64) && (count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_apc_bin_load((Value*)(args-0), (count > 1) ? (long long)(args[-1].m_data.num) : (long long)(0), (count > 2) ? (long long)(args[-2].m_data.num) : (long long)(0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_apc_bin_load(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("apc_bin_load", count, 1, 3, 1);
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
HPHP::Variant HPHP::fni_apc_bin_dumpfile(long long, HPHP::Variant const&, HPHP::String const&, long long, HPHP::Object const&)
_ZN4HPHP20fni_apc_bin_dumpfileExRKNS_7VariantERKNS_6StringExRKNS_6ObjectE

(return value) => rax
_rv => rdi
cache_id => rsi
filter => rdx
filename => rcx
flags => r8
context => r9
*/

TypedValue* fh_apc_bin_dumpfile(TypedValue* _rv, long long cache_id, TypedValue* filter, Value* filename, long long flags, Value* context) asm("_ZN4HPHP20fni_apc_bin_dumpfileExRKNS_7VariantERKNS_6StringExRKNS_6ObjectE");

TypedValue * fg1_apc_bin_dumpfile(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_apc_bin_dumpfile(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 5
    if ((args-4)->m_type != KindOfObject) {
      tvCastToObjectInPlace(args-4);
    }
  case 4:
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
    break;
  }
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  Object defVal4 = null;
  fh_apc_bin_dumpfile((rv), (long long)(args[-0].m_data.num), (args-1), (Value*)(args-2), (count > 3) ? (long long)(args[-3].m_data.num) : (long long)(0), (count > 4) ? (Value*)(args-4) : (Value*)(&defVal4));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_apc_bin_dumpfile(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 3LL && count <= 5LL) {
      if ((count <= 4 || (args-4)->m_type == KindOfObject) && (count <= 3 || (args-3)->m_type == KindOfInt64) && IS_STRING_TYPE((args-2)->m_type) && (args-0)->m_type == KindOfInt64) {
        Object defVal4 = null;
        fh_apc_bin_dumpfile((&(rv)), (long long)(args[-0].m_data.num), (args-1), (Value*)(args-2), (count > 3) ? (long long)(args[-3].m_data.num) : (long long)(0), (count > 4) ? (Value*)(args-4) : (Value*)(&defVal4));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_apc_bin_dumpfile(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("apc_bin_dumpfile", count, 3, 5, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 5);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::fni_apc_bin_loadfile(HPHP::String const&, HPHP::Object const&, long long, long long)
_ZN4HPHP20fni_apc_bin_loadfileERKNS_6StringERKNS_6ObjectExx

(return value) => rax
filename => rdi
context => rsi
flags => rdx
cache_id => rcx
*/

bool fh_apc_bin_loadfile(Value* filename, Value* context, long long flags, long long cache_id) asm("_ZN4HPHP20fni_apc_bin_loadfileERKNS_6StringERKNS_6ObjectExx");

TypedValue * fg1_apc_bin_loadfile(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_apc_bin_loadfile(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    if ((args-1)->m_type != KindOfObject) {
      tvCastToObjectInPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  Object defVal1 = null;
  rv->m_data.num = (fh_apc_bin_loadfile((Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&defVal1), (count > 2) ? (long long)(args[-2].m_data.num) : (long long)(0), (count > 3) ? (long long)(args[-3].m_data.num) : (long long)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_apc_bin_loadfile(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 4LL) {
      if ((count <= 3 || (args-3)->m_type == KindOfInt64) && (count <= 2 || (args-2)->m_type == KindOfInt64) && (count <= 1 || (args-1)->m_type == KindOfObject) && IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        Object defVal1 = null;
        rv.m_data.num = (fh_apc_bin_loadfile((Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&defVal1), (count > 2) ? (long long)(args[-2].m_data.num) : (long long)(0), (count > 3) ? (long long)(args[-3].m_data.num) : (long long)(0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_apc_bin_loadfile(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("apc_bin_loadfile", count, 1, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::fni_array_fill(int, int, HPHP::Variant const&)
_ZN4HPHP14fni_array_fillEiiRKNS_7VariantE

(return value) => rax
_rv => rdi
start_index => rsi
num => rdx
value => rcx
*/

TypedValue* fh_array_fill(TypedValue* _rv, int start_index, int num, TypedValue* value) asm("_ZN4HPHP14fni_array_fillEiiRKNS_7VariantE");

TypedValue * fg1_array_fill(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_array_fill(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
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
    long long count = ar->numArgs();
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::fni_key_exists(HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP14fni_key_existsERKNS_7VariantES2_

(return value) => rax
key => rdi
search => rsi
*/

bool fh_key_exists(TypedValue* key, TypedValue* search) asm("_ZN4HPHP14fni_key_existsERKNS_7VariantES2_");

TypedValue* fg_key_exists(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      rv._count = 0;
      rv.m_type = KindOfBoolean;
      rv.m_data.num = (fh_key_exists((args-0), (args-1))) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 2);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("key_exists", count, 2, 2, 1);
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
HPHP::Variant HPHP::fni_array_pop(HPHP::VRefParamValue const&)
_ZN4HPHP13fni_array_popERKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
array => rsi
*/

TypedValue* fh_array_pop(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP13fni_array_popERKNS_14VRefParamValueE");

TypedValue* fg_array_pop(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::fni_array_shift(HPHP::VRefParamValue const&)
_ZN4HPHP15fni_array_shiftERKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
array => rsi
*/

TypedValue* fh_array_shift(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP15fni_array_shiftERKNS_14VRefParamValueE");

TypedValue* fg_array_shift(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long long HPHP::fni_sizeof(HPHP::Variant const&, bool)
_ZN4HPHP10fni_sizeofERKNS_7VariantEb

(return value) => rax
var => rdi
recursive => rsi
*/

long long fh_sizeof(TypedValue* var, bool recursive) asm("_ZN4HPHP10fni_sizeofERKNS_7VariantEb");

TypedValue * fg1_sizeof(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_sizeof(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfInt64;
  tvCastToBooleanInPlace(args-1);
  rv->m_data.num = (long long)fh_sizeof((args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
  return rv;
}

TypedValue* fg_sizeof(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfBoolean)) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (long long)fh_sizeof((args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::fni_each(HPHP::VRefParamValue const&)
_ZN4HPHP8fni_eachERKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
array => rsi
*/

TypedValue* fh_each(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP8fni_eachERKNS_14VRefParamValueE");

TypedValue* fg_each(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::fni_current(HPHP::VRefParamValue const&)
_ZN4HPHP11fni_currentERKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
array => rsi
*/

TypedValue* fh_current(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP11fni_currentERKNS_14VRefParamValueE");

TypedValue* fg_current(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::fni_hphp_current_ref(HPHP::VRefParamValue const&)
_ZN4HPHP20fni_hphp_current_refERKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
array => rsi
*/

TypedValue* fh_hphp_current_ref(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP20fni_hphp_current_refERKNS_14VRefParamValueE");

TypedValue* fg_hphp_current_ref(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::fni_next(HPHP::VRefParamValue const&)
_ZN4HPHP8fni_nextERKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
array => rsi
*/

TypedValue* fh_next(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP8fni_nextERKNS_14VRefParamValueE");

TypedValue* fg_next(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::fni_pos(HPHP::VRefParamValue const&)
_ZN4HPHP7fni_posERKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
array => rsi
*/

TypedValue* fh_pos(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP7fni_posERKNS_14VRefParamValueE");

TypedValue* fg_pos(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::fni_prev(HPHP::VRefParamValue const&)
_ZN4HPHP8fni_prevERKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
array => rsi
*/

TypedValue* fh_prev(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP8fni_prevERKNS_14VRefParamValueE");

TypedValue* fg_prev(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::fni_reset(HPHP::VRefParamValue const&)
_ZN4HPHP9fni_resetERKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
array => rsi
*/

TypedValue* fh_reset(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP9fni_resetERKNS_14VRefParamValueE");

TypedValue* fg_reset(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::fni_end(HPHP::VRefParamValue const&)
_ZN4HPHP7fni_endERKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
array => rsi
*/

TypedValue* fh_end(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP7fni_endERKNS_14VRefParamValueE");

TypedValue* fg_end(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::fni_key(HPHP::VRefParamValue const&)
_ZN4HPHP7fni_keyERKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
array => rsi
*/

TypedValue* fh_key(TypedValue* _rv, TypedValue* array) asm("_ZN4HPHP7fni_keyERKNS_14VRefParamValueE");

TypedValue* fg_key(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::fni_bzclose(HPHP::Object const&)
_ZN4HPHP11fni_bzcloseERKNS_6ObjectE

(return value) => rax
_rv => rdi
bz => rsi
*/

TypedValue* fh_bzclose(TypedValue* _rv, Value* bz) asm("_ZN4HPHP11fni_bzcloseERKNS_6ObjectE");

TypedValue * fg1_bzclose(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_bzclose(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  fh_bzclose((rv), (Value*)(args-0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_bzclose(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        fh_bzclose((&(rv)), (Value*)(args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_bzclose(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("bzclose", count, 1, 1, 1);
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
HPHP::Variant HPHP::fni_bzread(HPHP::Object const&, int)
_ZN4HPHP10fni_bzreadERKNS_6ObjectEi

(return value) => rax
_rv => rdi
bz => rsi
length => rdx
*/

TypedValue* fh_bzread(TypedValue* _rv, Value* bz, int length) asm("_ZN4HPHP10fni_bzreadERKNS_6ObjectEi");

TypedValue * fg1_bzread(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_bzread(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
    break;
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_bzread((rv), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(1024));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_bzread(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64) && (args-0)->m_type == KindOfObject) {
        fh_bzread((&(rv)), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(1024));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_bzread(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("bzread", count, 1, 2, 1);
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
HPHP::Variant HPHP::fni_bzwrite(HPHP::Object const&, HPHP::String const&, int)
_ZN4HPHP11fni_bzwriteERKNS_6ObjectERKNS_6StringEi

(return value) => rax
_rv => rdi
bz => rsi
data => rdx
length => rcx
*/

TypedValue* fh_bzwrite(TypedValue* _rv, Value* bz, Value* data, int length) asm("_ZN4HPHP11fni_bzwriteERKNS_6ObjectERKNS_6StringEi");

TypedValue * fg1_bzwrite(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_bzwrite(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
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
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_bzwrite((rv), (Value*)(args-0), (Value*)(args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_bzwrite(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfInt64) && IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        fh_bzwrite((&(rv)), (Value*)(args-0), (Value*)(args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_bzwrite(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("bzwrite", count, 2, 3, 1);
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
bool HPHP::fni_checkdate(int, int, int)
_ZN4HPHP13fni_checkdateEiii

(return value) => rax
month => rdi
day => rsi
year => rdx
*/

bool fh_checkdate(int month, int day, int year) asm("_ZN4HPHP13fni_checkdateEiii");

TypedValue * fg1_checkdate(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_checkdate(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  rv->m_data.num = (fh_checkdate((int)(args[-0].m_data.num), (int)(args[-1].m_data.num), (int)(args[-2].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_checkdate(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfInt64) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_checkdate((int)(args[-0].m_data.num), (int)(args[-1].m_data.num), (int)(args[-2].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_checkdate(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("checkdate", count, 3, 3, 1);
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
HPHP::Object HPHP::fni_date_add(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP12fni_date_addERKNS_6ObjectES2_

(return value) => rax
_rv => rdi
datetime => rsi
interval => rdx
*/

Value* fh_date_add(Value* _rv, Value* datetime, Value* interval) asm("_ZN4HPHP12fni_date_addERKNS_6ObjectES2_");

TypedValue * fg1_date_add(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_date_add(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfObject;
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_date_add((Value*)(rv), (Value*)(args-0), (Value*)(args-1));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_date_add(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfObject;
        fh_date_add((Value*)(&(rv)), (Value*)(args-0), (Value*)(args-1));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_date_add(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("date_add", count, 2, 2, 1);
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
HPHP::Object HPHP::fni_date_create_from_format(HPHP::String const&, HPHP::String const&, HPHP::Object const&)
_ZN4HPHP27fni_date_create_from_formatERKNS_6StringES2_RKNS_6ObjectE

(return value) => rax
_rv => rdi
format => rsi
time => rdx
timezone => rcx
*/

Value* fh_date_create_from_format(Value* _rv, Value* format, Value* time, Value* timezone) asm("_ZN4HPHP27fni_date_create_from_formatERKNS_6StringES2_RKNS_6ObjectE");

TypedValue * fg1_date_create_from_format(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_date_create_from_format(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfObject;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfObject) {
      tvCastToObjectInPlace(args-2);
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
  fh_date_create_from_format((Value*)(rv), (Value*)(args-0), (Value*)(args-1), (count > 2) ? (Value*)(args-2) : (Value*)(&null_object));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_date_create_from_format(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfObject) && IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfObject;
        fh_date_create_from_format((Value*)(&(rv)), (Value*)(args-0), (Value*)(args-1), (count > 2) ? (Value*)(args-2) : (Value*)(&null_object));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_date_create_from_format(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("date_create_from_format", count, 2, 3, 1);
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
HPHP::Object HPHP::fni_date_create(HPHP::String const&, HPHP::Object const&)
_ZN4HPHP15fni_date_createERKNS_6StringERKNS_6ObjectE

(return value) => rax
_rv => rdi
time => rsi
timezone => rdx
*/

Value* fh_date_create(Value* _rv, Value* time, Value* timezone) asm("_ZN4HPHP15fni_date_createERKNS_6StringERKNS_6ObjectE");

TypedValue * fg1_date_create(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_date_create(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfObject;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfObject) {
      tvCastToObjectInPlace(args-1);
    }
  case 1:
    if (!IS_STRING_TYPE((args-0)->m_type)) {
      tvCastToStringInPlace(args-0);
    }
  case 0:
    break;
  }
  fh_date_create((Value*)(rv), (count > 0) ? (Value*)(args-0) : (Value*)(&null_string), (count > 1) ? (Value*)(args-1) : (Value*)(&null_object));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_date_create(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfObject) && (count <= 0 || IS_STRING_TYPE((args-0)->m_type))) {
        rv._count = 0;
        rv.m_type = KindOfObject;
        fh_date_create((Value*)(&(rv)), (count > 0) ? (Value*)(args-0) : (Value*)(&null_string), (count > 1) ? (Value*)(args-1) : (Value*)(&null_object));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_date_create(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("date_create", 2, 1);
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
void HPHP::fni_date_date_set(HPHP::Object const&, int, int, int)
_ZN4HPHP17fni_date_date_setERKNS_6ObjectEiii

object => rdi
year => rsi
month => rdx
day => rcx
*/

void fh_date_date_set(Value* object, int year, int month, int day) asm("_ZN4HPHP17fni_date_date_setERKNS_6ObjectEiii");

TypedValue * fg1_date_date_set(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_date_date_set(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_date_date_set((Value*)(args-0), (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num));
  return rv;
}

TypedValue* fg_date_date_set(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 4LL) {
      if ((args-3)->m_type == KindOfInt64 && (args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        fh_date_date_set((Value*)(args-0), (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num));
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_date_date_set(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("date_date_set", count, 4, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::fni_date_default_timezone_get()
_ZN4HPHP29fni_date_default_timezone_getEv

(return value) => rax
_rv => rdi
*/

Value* fh_date_default_timezone_get(Value* _rv) asm("_ZN4HPHP29fni_date_default_timezone_getEv");

TypedValue* fg_date_default_timezone_get(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfString;
      fh_date_default_timezone_get((Value*)(&(rv)));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("date_default_timezone_get", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::fni_date_default_timezone_set(HPHP::String const&)
_ZN4HPHP29fni_date_default_timezone_setERKNS_6StringE

(return value) => rax
name => rdi
*/

bool fh_date_default_timezone_set(Value* name) asm("_ZN4HPHP29fni_date_default_timezone_setERKNS_6StringE");

TypedValue * fg1_date_default_timezone_set(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_date_default_timezone_set(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToStringInPlace(args-0);
  rv->m_data.num = (fh_date_default_timezone_set((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_date_default_timezone_set(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_date_default_timezone_set((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_date_default_timezone_set(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("date_default_timezone_set", count, 1, 1, 1);
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
HPHP::Object HPHP::fni_date_diff(HPHP::Object const&, HPHP::Object const&, bool)
_ZN4HPHP13fni_date_diffERKNS_6ObjectES2_b

(return value) => rax
_rv => rdi
datetime => rsi
datetime2 => rdx
absolute => rcx
*/

Value* fh_date_diff(Value* _rv, Value* datetime, Value* datetime2, bool absolute) asm("_ZN4HPHP13fni_date_diffERKNS_6ObjectES2_b");

TypedValue * fg1_date_diff(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_date_diff(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfObject;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-2);
    }
  case 2:
    break;
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_date_diff((Value*)(rv), (Value*)(args-0), (Value*)(args-1), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_date_diff(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfBoolean) && (args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfObject;
        fh_date_diff((Value*)(&(rv)), (Value*)(args-0), (Value*)(args-1), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_date_diff(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("date_diff", count, 2, 3, 1);
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
HPHP::String HPHP::fni_date_format(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP15fni_date_formatERKNS_6ObjectERKNS_6StringE

(return value) => rax
_rv => rdi
object => rsi
format => rdx
*/

Value* fh_date_format(Value* _rv, Value* object, Value* format) asm("_ZN4HPHP15fni_date_formatERKNS_6ObjectERKNS_6StringE");

TypedValue * fg1_date_format(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_date_format(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_date_format((Value*)(rv), (Value*)(args-0), (Value*)(args-1));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_date_format(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_date_format((Value*)(&(rv)), (Value*)(args-0), (Value*)(args-1));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_date_format(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("date_format", count, 2, 2, 1);
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
HPHP::Array HPHP::fni_date_get_last_errors()
_ZN4HPHP24fni_date_get_last_errorsEv

(return value) => rax
_rv => rdi
*/

Value* fh_date_get_last_errors(Value* _rv) asm("_ZN4HPHP24fni_date_get_last_errorsEv");

TypedValue* fg_date_get_last_errors(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfArray;
      fh_date_get_last_errors((Value*)(&(rv)));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("date_get_last_errors", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Object HPHP::fni_date_interval_create_from_date_string(HPHP::String const&)
_ZN4HPHP41fni_date_interval_create_from_date_stringERKNS_6StringE

(return value) => rax
_rv => rdi
time => rsi
*/

Value* fh_date_interval_create_from_date_string(Value* _rv, Value* time) asm("_ZN4HPHP41fni_date_interval_create_from_date_stringERKNS_6StringE");

TypedValue * fg1_date_interval_create_from_date_string(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_date_interval_create_from_date_string(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfObject;
  tvCastToStringInPlace(args-0);
  fh_date_interval_create_from_date_string((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_date_interval_create_from_date_string(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfObject;
        fh_date_interval_create_from_date_string((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_date_interval_create_from_date_string(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("date_interval_create_from_date_string", count, 1, 1, 1);
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
HPHP::String HPHP::fni_date_interval_format(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP24fni_date_interval_formatERKNS_6ObjectERKNS_6StringE

(return value) => rax
_rv => rdi
interval => rsi
format_spec => rdx
*/

Value* fh_date_interval_format(Value* _rv, Value* interval, Value* format_spec) asm("_ZN4HPHP24fni_date_interval_formatERKNS_6ObjectERKNS_6StringE");

TypedValue * fg1_date_interval_format(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_date_interval_format(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_date_interval_format((Value*)(rv), (Value*)(args-0), (Value*)(args-1));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_date_interval_format(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_date_interval_format((Value*)(&(rv)), (Value*)(args-0), (Value*)(args-1));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_date_interval_format(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("date_interval_format", count, 2, 2, 1);
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
void HPHP::fni_date_isodate_set(HPHP::Object const&, int, int, int)
_ZN4HPHP20fni_date_isodate_setERKNS_6ObjectEiii

object => rdi
year => rsi
week => rdx
day => rcx
*/

void fh_date_isodate_set(Value* object, int year, int week, int day) asm("_ZN4HPHP20fni_date_isodate_setERKNS_6ObjectEiii");

TypedValue * fg1_date_isodate_set(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_date_isodate_set(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
    break;
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_date_isodate_set((Value*)(args-0), (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (count > 3) ? (int)(args[-3].m_data.num) : (int)(1));
  return rv;
}

TypedValue* fg_date_isodate_set(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 3LL && count <= 4LL) {
      if ((count <= 3 || (args-3)->m_type == KindOfInt64) && (args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        fh_date_isodate_set((Value*)(args-0), (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (count > 3) ? (int)(args[-3].m_data.num) : (int)(1));
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_date_isodate_set(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("date_isodate_set", count, 3, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::fni_date_modify(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP15fni_date_modifyERKNS_6ObjectERKNS_6StringE

object => rdi
modify => rsi
*/

void fh_date_modify(Value* object, Value* modify) asm("_ZN4HPHP15fni_date_modifyERKNS_6ObjectERKNS_6StringE");

TypedValue * fg1_date_modify(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_date_modify(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_date_modify((Value*)(args-0), (Value*)(args-1));
  return rv;
}

TypedValue* fg_date_modify(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        fh_date_modify((Value*)(args-0), (Value*)(args-1));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_date_modify(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("date_modify", count, 2, 2, 1);
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
long long HPHP::fni_date_offset_get(HPHP::Object const&)
_ZN4HPHP19fni_date_offset_getERKNS_6ObjectE

(return value) => rax
object => rdi
*/

long long fh_date_offset_get(Value* object) asm("_ZN4HPHP19fni_date_offset_getERKNS_6ObjectE");

TypedValue * fg1_date_offset_get(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_date_offset_get(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfInt64;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (long long)fh_date_offset_get((Value*)(args-0));
  return rv;
}

TypedValue* fg_date_offset_get(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (long long)fh_date_offset_get((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_date_offset_get(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("date_offset_get", count, 1, 1, 1);
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
HPHP::Variant HPHP::fni_date_parse(HPHP::String const&)
_ZN4HPHP14fni_date_parseERKNS_6StringE

(return value) => rax
_rv => rdi
date => rsi
*/

TypedValue* fh_date_parse(TypedValue* _rv, Value* date) asm("_ZN4HPHP14fni_date_parseERKNS_6StringE");

TypedValue * fg1_date_parse(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_date_parse(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_date_parse((rv), (Value*)(args-0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_date_parse(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        fh_date_parse((&(rv)), (Value*)(args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_date_parse(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("date_parse", count, 1, 1, 1);
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
HPHP::Object HPHP::fni_date_sub(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP12fni_date_subERKNS_6ObjectES2_

(return value) => rax
_rv => rdi
datetime => rsi
interval => rdx
*/

Value* fh_date_sub(Value* _rv, Value* datetime, Value* interval) asm("_ZN4HPHP12fni_date_subERKNS_6ObjectES2_");

TypedValue * fg1_date_sub(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_date_sub(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfObject;
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_date_sub((Value*)(rv), (Value*)(args-0), (Value*)(args-1));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_date_sub(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfObject;
        fh_date_sub((Value*)(&(rv)), (Value*)(args-0), (Value*)(args-1));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_date_sub(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("date_sub", count, 2, 2, 1);
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
HPHP::Array HPHP::fni_date_sun_info(long long, double, double)
_ZN4HPHP17fni_date_sun_infoExdd

(return value) => rax
_rv => rdi
ts => rsi
latitude => xmm0
longitude => xmm1
*/

Value* fh_date_sun_info(Value* _rv, long long ts, double latitude, double longitude) asm("_ZN4HPHP17fni_date_sun_infoExdd");

TypedValue * fg1_date_sun_info(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_date_sun_info(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfArray;
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  fh_date_sun_info((Value*)(rv), (long long)(args[-0].m_data.num), (args[-1].m_data.dbl), (args[-2].m_data.dbl));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_date_sun_info(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfInt64) {
        rv._count = 0;
        rv.m_type = KindOfArray;
        fh_date_sun_info((Value*)(&(rv)), (long long)(args[-0].m_data.num), (args[-1].m_data.dbl), (args[-2].m_data.dbl));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_date_sun_info(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("date_sun_info", count, 3, 3, 1);
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
HPHP::Variant HPHP::fni_date_sunrise(long long, int, double, double, double, double)
_ZN4HPHP16fni_date_sunriseExidddd

(return value) => rax
_rv => rdi
timestamp => rsi
format => rdx
latitude => xmm0
longitude => xmm1
zenith => xmm2
gmt_offset => xmm3
*/

TypedValue* fh_date_sunrise(TypedValue* _rv, long long timestamp, int format, double latitude, double longitude, double zenith, double gmt_offset) asm("_ZN4HPHP16fni_date_sunriseExidddd");

TypedValue * fg1_date_sunrise(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_date_sunrise(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 6
    if ((args-5)->m_type != KindOfDouble) {
      tvCastToDoubleInPlace(args-5);
    }
  case 5:
    if ((args-4)->m_type != KindOfDouble) {
      tvCastToDoubleInPlace(args-4);
    }
  case 4:
    if ((args-3)->m_type != KindOfDouble) {
      tvCastToDoubleInPlace(args-3);
    }
  case 3:
    if ((args-2)->m_type != KindOfDouble) {
      tvCastToDoubleInPlace(args-2);
    }
  case 2:
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
    break;
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  fh_date_sunrise((rv), (long long)(args[-0].m_data.num), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (args[-2].m_data.dbl) : (double)(0.0), (count > 3) ? (args[-3].m_data.dbl) : (double)(0.0), (count > 4) ? (args[-4].m_data.dbl) : (double)(0.0), (count > 5) ? (args[-5].m_data.dbl) : (double)(99999.0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_date_sunrise(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 6LL) {
      if ((count <= 5 || (args-5)->m_type == KindOfDouble) && (count <= 4 || (args-4)->m_type == KindOfDouble) && (count <= 3 || (args-3)->m_type == KindOfDouble) && (count <= 2 || (args-2)->m_type == KindOfDouble) && (count <= 1 || (args-1)->m_type == KindOfInt64) && (args-0)->m_type == KindOfInt64) {
        fh_date_sunrise((&(rv)), (long long)(args[-0].m_data.num), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (args[-2].m_data.dbl) : (double)(0.0), (count > 3) ? (args[-3].m_data.dbl) : (double)(0.0), (count > 4) ? (args[-4].m_data.dbl) : (double)(0.0), (count > 5) ? (args[-5].m_data.dbl) : (double)(99999.0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_date_sunrise(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("date_sunrise", count, 1, 6, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 6);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::fni_date_sunset(long long, int, double, double, double, double)
_ZN4HPHP15fni_date_sunsetExidddd

(return value) => rax
_rv => rdi
timestamp => rsi
format => rdx
latitude => xmm0
longitude => xmm1
zenith => xmm2
gmt_offset => xmm3
*/

TypedValue* fh_date_sunset(TypedValue* _rv, long long timestamp, int format, double latitude, double longitude, double zenith, double gmt_offset) asm("_ZN4HPHP15fni_date_sunsetExidddd");

TypedValue * fg1_date_sunset(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_date_sunset(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 6
    if ((args-5)->m_type != KindOfDouble) {
      tvCastToDoubleInPlace(args-5);
    }
  case 5:
    if ((args-4)->m_type != KindOfDouble) {
      tvCastToDoubleInPlace(args-4);
    }
  case 4:
    if ((args-3)->m_type != KindOfDouble) {
      tvCastToDoubleInPlace(args-3);
    }
  case 3:
    if ((args-2)->m_type != KindOfDouble) {
      tvCastToDoubleInPlace(args-2);
    }
  case 2:
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
    break;
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  fh_date_sunset((rv), (long long)(args[-0].m_data.num), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (args[-2].m_data.dbl) : (double)(0.0), (count > 3) ? (args[-3].m_data.dbl) : (double)(0.0), (count > 4) ? (args[-4].m_data.dbl) : (double)(0.0), (count > 5) ? (args[-5].m_data.dbl) : (double)(99999.0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_date_sunset(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 6LL) {
      if ((count <= 5 || (args-5)->m_type == KindOfDouble) && (count <= 4 || (args-4)->m_type == KindOfDouble) && (count <= 3 || (args-3)->m_type == KindOfDouble) && (count <= 2 || (args-2)->m_type == KindOfDouble) && (count <= 1 || (args-1)->m_type == KindOfInt64) && (args-0)->m_type == KindOfInt64) {
        fh_date_sunset((&(rv)), (long long)(args[-0].m_data.num), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (args[-2].m_data.dbl) : (double)(0.0), (count > 3) ? (args[-3].m_data.dbl) : (double)(0.0), (count > 4) ? (args[-4].m_data.dbl) : (double)(0.0), (count > 5) ? (args[-5].m_data.dbl) : (double)(99999.0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_date_sunset(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("date_sunset", count, 1, 6, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 6);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::fni_date_time_set(HPHP::Object const&, int, int, int)
_ZN4HPHP17fni_date_time_setERKNS_6ObjectEiii

object => rdi
hour => rsi
minute => rdx
second => rcx
*/

void fh_date_time_set(Value* object, int hour, int minute, int second) asm("_ZN4HPHP17fni_date_time_setERKNS_6ObjectEiii");

TypedValue * fg1_date_time_set(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_date_time_set(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
    break;
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_date_time_set((Value*)(args-0), (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0));
  return rv;
}

TypedValue* fg_date_time_set(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 3LL && count <= 4LL) {
      if ((count <= 3 || (args-3)->m_type == KindOfInt64) && (args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        fh_date_time_set((Value*)(args-0), (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0));
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_date_time_set(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("date_time_set", count, 3, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long long HPHP::fni_date_timestamp_get(HPHP::Object const&)
_ZN4HPHP22fni_date_timestamp_getERKNS_6ObjectE

(return value) => rax
datetime => rdi
*/

long long fh_date_timestamp_get(Value* datetime) asm("_ZN4HPHP22fni_date_timestamp_getERKNS_6ObjectE");

TypedValue * fg1_date_timestamp_get(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_date_timestamp_get(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfInt64;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (long long)fh_date_timestamp_get((Value*)(args-0));
  return rv;
}

TypedValue* fg_date_timestamp_get(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (long long)fh_date_timestamp_get((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_date_timestamp_get(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("date_timestamp_get", count, 1, 1, 1);
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
HPHP::Object HPHP::fni_date_timestamp_set(HPHP::Object const&, long long)
_ZN4HPHP22fni_date_timestamp_setERKNS_6ObjectEx

(return value) => rax
_rv => rdi
datetime => rsi
timestamp => rdx
*/

Value* fh_date_timestamp_set(Value* _rv, Value* datetime, long long timestamp) asm("_ZN4HPHP22fni_date_timestamp_setERKNS_6ObjectEx");

TypedValue * fg1_date_timestamp_set(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_date_timestamp_set(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfObject;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_date_timestamp_set((Value*)(rv), (Value*)(args-0), (long long)(args[-1].m_data.num));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_date_timestamp_set(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfObject;
        fh_date_timestamp_set((Value*)(&(rv)), (Value*)(args-0), (long long)(args[-1].m_data.num));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_date_timestamp_set(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("date_timestamp_set", count, 2, 2, 1);
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
HPHP::Variant HPHP::fni_date_timezone_get(HPHP::Object const&)
_ZN4HPHP21fni_date_timezone_getERKNS_6ObjectE

(return value) => rax
_rv => rdi
object => rsi
*/

TypedValue* fh_date_timezone_get(TypedValue* _rv, Value* object) asm("_ZN4HPHP21fni_date_timezone_getERKNS_6ObjectE");

TypedValue * fg1_date_timezone_get(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_date_timezone_get(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  fh_date_timezone_get((rv), (Value*)(args-0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_date_timezone_get(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        fh_date_timezone_get((&(rv)), (Value*)(args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_date_timezone_get(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("date_timezone_get", count, 1, 1, 1);
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
void HPHP::fni_date_timezone_set(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP21fni_date_timezone_setERKNS_6ObjectES2_

object => rdi
timezone => rsi
*/

void fh_date_timezone_set(Value* object, Value* timezone) asm("_ZN4HPHP21fni_date_timezone_setERKNS_6ObjectES2_");

TypedValue * fg1_date_timezone_set(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_date_timezone_set(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_date_timezone_set((Value*)(args-0), (Value*)(args-1));
  return rv;
}

TypedValue* fg_date_timezone_set(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        fh_date_timezone_set((Value*)(args-0), (Value*)(args-1));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_date_timezone_set(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("date_timezone_set", count, 2, 2, 1);
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
HPHP::Variant HPHP::fni_date(HPHP::String const&, long long)
_ZN4HPHP8fni_dateERKNS_6StringEx

(return value) => rax
_rv => rdi
format => rsi
timestamp => rdx
*/

TypedValue* fh_date(TypedValue* _rv, Value* format, long long timestamp) asm("_ZN4HPHP8fni_dateERKNS_6StringEx");

TypedValue * fg1_date(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_date(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
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
  fh_date((rv), (Value*)(args-0), (count > 1) ? (long long)(args[-1].m_data.num) : (long long)(TimeStamp::Current()));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_date(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
        fh_date((&(rv)), (Value*)(args-0), (count > 1) ? (long long)(args[-1].m_data.num) : (long long)(TimeStamp::Current()));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_date(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("date", count, 1, 2, 1);
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
HPHP::Array HPHP::fni_getdate(long long)
_ZN4HPHP11fni_getdateEx

(return value) => rax
_rv => rdi
timestamp => rsi
*/

Value* fh_getdate(Value* _rv, long long timestamp) asm("_ZN4HPHP11fni_getdateEx");

TypedValue * fg1_getdate(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_getdate(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfArray;
  tvCastToInt64InPlace(args-0);
  fh_getdate((Value*)(rv), (count > 0) ? (long long)(args[-0].m_data.num) : (long long)(TimeStamp::Current()));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_getdate(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      if ((count <= 0 || (args-0)->m_type == KindOfInt64)) {
        rv._count = 0;
        rv.m_type = KindOfArray;
        fh_getdate((Value*)(&(rv)), (count > 0) ? (long long)(args[-0].m_data.num) : (long long)(TimeStamp::Current()));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_getdate(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("getdate", 1, 1);
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
HPHP::Variant HPHP::fni_gettimeofday(bool)
_ZN4HPHP16fni_gettimeofdayEb

(return value) => rax
_rv => rdi
return_float => rsi
*/

TypedValue* fh_gettimeofday(TypedValue* _rv, bool return_float) asm("_ZN4HPHP16fni_gettimeofdayEb");

TypedValue * fg1_gettimeofday(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_gettimeofday(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToBooleanInPlace(args-0);
  fh_gettimeofday((rv), (count > 0) ? (bool)(args[-0].m_data.num) : (bool)(false));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_gettimeofday(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      if ((count <= 0 || (args-0)->m_type == KindOfBoolean)) {
        fh_gettimeofday((&(rv)), (count > 0) ? (bool)(args[-0].m_data.num) : (bool)(false));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_gettimeofday(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("gettimeofday", 1, 1);
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
HPHP::Variant HPHP::fni_gmdate(HPHP::String const&, long long)
_ZN4HPHP10fni_gmdateERKNS_6StringEx

(return value) => rax
_rv => rdi
format => rsi
timestamp => rdx
*/

TypedValue* fh_gmdate(TypedValue* _rv, Value* format, long long timestamp) asm("_ZN4HPHP10fni_gmdateERKNS_6StringEx");

TypedValue * fg1_gmdate(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_gmdate(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
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
  fh_gmdate((rv), (Value*)(args-0), (count > 1) ? (long long)(args[-1].m_data.num) : (long long)(TimeStamp::Current()));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_gmdate(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
        fh_gmdate((&(rv)), (Value*)(args-0), (count > 1) ? (long long)(args[-1].m_data.num) : (long long)(TimeStamp::Current()));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_gmdate(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("gmdate", count, 1, 2, 1);
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
HPHP::Variant HPHP::fni_gmmktime(int, int, int, int, int, int)
_ZN4HPHP12fni_gmmktimeEiiiiii

(return value) => rax
_rv => rdi
hour => rsi
minute => rdx
second => rcx
month => r8
day => r9
year => st0
*/

TypedValue* fh_gmmktime(TypedValue* _rv, int hour, int minute, int second, int month, int day, int year) asm("_ZN4HPHP12fni_gmmktimeEiiiiii");

TypedValue * fg1_gmmktime(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_gmmktime(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 6
    if ((args-5)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-5);
    }
  case 5:
    if ((args-4)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-4);
    }
  case 4:
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
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
  fh_gmmktime((rv), (count > 0) ? (int)(args[-0].m_data.num) : (int)(INT_MAX), (count > 1) ? (int)(args[-1].m_data.num) : (int)(INT_MAX), (count > 2) ? (int)(args[-2].m_data.num) : (int)(INT_MAX), (count > 3) ? (int)(args[-3].m_data.num) : (int)(INT_MAX), (count > 4) ? (int)(args[-4].m_data.num) : (int)(INT_MAX), (count > 5) ? (int)(args[-5].m_data.num) : (int)(INT_MAX));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_gmmktime(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 6LL) {
      if ((count <= 5 || (args-5)->m_type == KindOfInt64) && (count <= 4 || (args-4)->m_type == KindOfInt64) && (count <= 3 || (args-3)->m_type == KindOfInt64) && (count <= 2 || (args-2)->m_type == KindOfInt64) && (count <= 1 || (args-1)->m_type == KindOfInt64) && (count <= 0 || (args-0)->m_type == KindOfInt64)) {
        fh_gmmktime((&(rv)), (count > 0) ? (int)(args[-0].m_data.num) : (int)(INT_MAX), (count > 1) ? (int)(args[-1].m_data.num) : (int)(INT_MAX), (count > 2) ? (int)(args[-2].m_data.num) : (int)(INT_MAX), (count > 3) ? (int)(args[-3].m_data.num) : (int)(INT_MAX), (count > 4) ? (int)(args[-4].m_data.num) : (int)(INT_MAX), (count > 5) ? (int)(args[-5].m_data.num) : (int)(INT_MAX));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_gmmktime(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("gmmktime", 6, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 6);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::fni_gmstrftime(HPHP::String const&, long long)
_ZN4HPHP14fni_gmstrftimeERKNS_6StringEx

(return value) => rax
_rv => rdi
format => rsi
timestamp => rdx
*/

Value* fh_gmstrftime(Value* _rv, Value* format, long long timestamp) asm("_ZN4HPHP14fni_gmstrftimeERKNS_6StringEx");

TypedValue * fg1_gmstrftime(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_gmstrftime(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
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
  fh_gmstrftime((Value*)(rv), (Value*)(args-0), (count > 1) ? (long long)(args[-1].m_data.num) : (long long)(TimeStamp::Current()));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_gmstrftime(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_gmstrftime((Value*)(&(rv)), (Value*)(args-0), (count > 1) ? (long long)(args[-1].m_data.num) : (long long)(TimeStamp::Current()));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_gmstrftime(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("gmstrftime", count, 1, 2, 1);
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
HPHP::Variant HPHP::fni_idate(HPHP::String const&, long long)
_ZN4HPHP9fni_idateERKNS_6StringEx

(return value) => rax
_rv => rdi
format => rsi
timestamp => rdx
*/

TypedValue* fh_idate(TypedValue* _rv, Value* format, long long timestamp) asm("_ZN4HPHP9fni_idateERKNS_6StringEx");

TypedValue * fg1_idate(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_idate(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
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
  fh_idate((rv), (Value*)(args-0), (count > 1) ? (long long)(args[-1].m_data.num) : (long long)(TimeStamp::Current()));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_idate(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
        fh_idate((&(rv)), (Value*)(args-0), (count > 1) ? (long long)(args[-1].m_data.num) : (long long)(TimeStamp::Current()));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_idate(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("idate", count, 1, 2, 1);
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
HPHP::Array HPHP::fni_localtime(long long, bool)
_ZN4HPHP13fni_localtimeExb

(return value) => rax
_rv => rdi
timestamp => rsi
is_associative => rdx
*/

Value* fh_localtime(Value* _rv, long long timestamp, bool is_associative) asm("_ZN4HPHP13fni_localtimeExb");

TypedValue * fg1_localtime(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_localtime(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfArray;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-1);
    }
  case 1:
    if ((args-0)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-0);
    }
  case 0:
    break;
  }
  fh_localtime((Value*)(rv), (count > 0) ? (long long)(args[-0].m_data.num) : (long long)(TimeStamp::Current()), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_localtime(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfBoolean) && (count <= 0 || (args-0)->m_type == KindOfInt64)) {
        rv._count = 0;
        rv.m_type = KindOfArray;
        fh_localtime((Value*)(&(rv)), (count > 0) ? (long long)(args[-0].m_data.num) : (long long)(TimeStamp::Current()), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_localtime(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("localtime", 2, 1);
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
HPHP::Variant HPHP::fni_microtime(bool)
_ZN4HPHP13fni_microtimeEb

(return value) => rax
_rv => rdi
get_as_float => rsi
*/

TypedValue* fh_microtime(TypedValue* _rv, bool get_as_float) asm("_ZN4HPHP13fni_microtimeEb");

TypedValue * fg1_microtime(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_microtime(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToBooleanInPlace(args-0);
  fh_microtime((rv), (count > 0) ? (bool)(args[-0].m_data.num) : (bool)(false));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_microtime(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      if ((count <= 0 || (args-0)->m_type == KindOfBoolean)) {
        fh_microtime((&(rv)), (count > 0) ? (bool)(args[-0].m_data.num) : (bool)(false));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_microtime(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("microtime", 1, 1);
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
HPHP::Variant HPHP::fni_mktime(int, int, int, int, int, int)
_ZN4HPHP10fni_mktimeEiiiiii

(return value) => rax
_rv => rdi
hour => rsi
minute => rdx
second => rcx
month => r8
day => r9
year => st0
*/

TypedValue* fh_mktime(TypedValue* _rv, int hour, int minute, int second, int month, int day, int year) asm("_ZN4HPHP10fni_mktimeEiiiiii");

TypedValue * fg1_mktime(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_mktime(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 6
    if ((args-5)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-5);
    }
  case 5:
    if ((args-4)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-4);
    }
  case 4:
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
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
  fh_mktime((rv), (count > 0) ? (int)(args[-0].m_data.num) : (int)(INT_MAX), (count > 1) ? (int)(args[-1].m_data.num) : (int)(INT_MAX), (count > 2) ? (int)(args[-2].m_data.num) : (int)(INT_MAX), (count > 3) ? (int)(args[-3].m_data.num) : (int)(INT_MAX), (count > 4) ? (int)(args[-4].m_data.num) : (int)(INT_MAX), (count > 5) ? (int)(args[-5].m_data.num) : (int)(INT_MAX));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mktime(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 6LL) {
      if ((count <= 5 || (args-5)->m_type == KindOfInt64) && (count <= 4 || (args-4)->m_type == KindOfInt64) && (count <= 3 || (args-3)->m_type == KindOfInt64) && (count <= 2 || (args-2)->m_type == KindOfInt64) && (count <= 1 || (args-1)->m_type == KindOfInt64) && (count <= 0 || (args-0)->m_type == KindOfInt64)) {
        fh_mktime((&(rv)), (count > 0) ? (int)(args[-0].m_data.num) : (int)(INT_MAX), (count > 1) ? (int)(args[-1].m_data.num) : (int)(INT_MAX), (count > 2) ? (int)(args[-2].m_data.num) : (int)(INT_MAX), (count > 3) ? (int)(args[-3].m_data.num) : (int)(INT_MAX), (count > 4) ? (int)(args[-4].m_data.num) : (int)(INT_MAX), (count > 5) ? (int)(args[-5].m_data.num) : (int)(INT_MAX));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mktime(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("mktime", 6, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 6);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::fni_strftime(HPHP::String const&, long long)
_ZN4HPHP12fni_strftimeERKNS_6StringEx

(return value) => rax
_rv => rdi
format => rsi
timestamp => rdx
*/

TypedValue* fh_strftime(TypedValue* _rv, Value* format, long long timestamp) asm("_ZN4HPHP12fni_strftimeERKNS_6StringEx");

TypedValue * fg1_strftime(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_strftime(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
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
  fh_strftime((rv), (Value*)(args-0), (count > 1) ? (long long)(args[-1].m_data.num) : (long long)(TimeStamp::Current()));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_strftime(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
        fh_strftime((&(rv)), (Value*)(args-0), (count > 1) ? (long long)(args[-1].m_data.num) : (long long)(TimeStamp::Current()));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_strftime(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("strftime", count, 1, 2, 1);
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
HPHP::Variant HPHP::fni_strptime(HPHP::String const&, HPHP::String const&)
_ZN4HPHP12fni_strptimeERKNS_6StringES2_

(return value) => rax
_rv => rdi
date => rsi
format => rdx
*/

TypedValue* fh_strptime(TypedValue* _rv, Value* date, Value* format) asm("_ZN4HPHP12fni_strptimeERKNS_6StringES2_");

TypedValue * fg1_strptime(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_strptime(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_strptime((rv), (Value*)(args-0), (Value*)(args-1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_strptime(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        fh_strptime((&(rv)), (Value*)(args-0), (Value*)(args-1));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_strptime(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("strptime", count, 2, 2, 1);
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
HPHP::Variant HPHP::fni_strtotime(HPHP::String const&, long long)
_ZN4HPHP13fni_strtotimeERKNS_6StringEx

(return value) => rax
_rv => rdi
input => rsi
timestamp => rdx
*/

TypedValue* fh_strtotime(TypedValue* _rv, Value* input, long long timestamp) asm("_ZN4HPHP13fni_strtotimeERKNS_6StringEx");

TypedValue * fg1_strtotime(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_strtotime(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
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
  fh_strtotime((rv), (Value*)(args-0), (count > 1) ? (long long)(args[-1].m_data.num) : (long long)(TimeStamp::Current()));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_strtotime(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
        fh_strtotime((&(rv)), (Value*)(args-0), (count > 1) ? (long long)(args[-1].m_data.num) : (long long)(TimeStamp::Current()));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_strtotime(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("strtotime", count, 1, 2, 1);
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
long long HPHP::fni_time()
_ZN4HPHP8fni_timeEv

(return value) => rax
*/

long long fh_time() asm("_ZN4HPHP8fni_timeEv");

TypedValue* fg_time(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfInt64;
      rv.m_data.num = (long long)fh_time();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("time", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::fni_timezone_abbreviations_list()
_ZN4HPHP31fni_timezone_abbreviations_listEv

(return value) => rax
_rv => rdi
*/

Value* fh_timezone_abbreviations_list(Value* _rv) asm("_ZN4HPHP31fni_timezone_abbreviations_listEv");

TypedValue* fg_timezone_abbreviations_list(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfArray;
      fh_timezone_abbreviations_list((Value*)(&(rv)));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("timezone_abbreviations_list", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::fni_timezone_identifiers_list()
_ZN4HPHP29fni_timezone_identifiers_listEv

(return value) => rax
_rv => rdi
*/

Value* fh_timezone_identifiers_list(Value* _rv) asm("_ZN4HPHP29fni_timezone_identifiers_listEv");

TypedValue* fg_timezone_identifiers_list(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfArray;
      fh_timezone_identifiers_list((Value*)(&(rv)));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("timezone_identifiers_list", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::fni_timezone_location_get(HPHP::Object const&)
_ZN4HPHP25fni_timezone_location_getERKNS_6ObjectE

(return value) => rax
_rv => rdi
timezone => rsi
*/

Value* fh_timezone_location_get(Value* _rv, Value* timezone) asm("_ZN4HPHP25fni_timezone_location_getERKNS_6ObjectE");

TypedValue * fg1_timezone_location_get(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_timezone_location_get(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfArray;
  tvCastToObjectInPlace(args-0);
  fh_timezone_location_get((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_timezone_location_get(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfArray;
        fh_timezone_location_get((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_timezone_location_get(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("timezone_location_get", count, 1, 1, 1);
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
HPHP::Variant HPHP::fni_timezone_name_from_abbr(HPHP::String const&, int, bool)
_ZN4HPHP27fni_timezone_name_from_abbrERKNS_6StringEib

(return value) => rax
_rv => rdi
abbr => rsi
gmtoffset => rdx
isdst => rcx
*/

TypedValue* fh_timezone_name_from_abbr(TypedValue* _rv, Value* abbr, int gmtoffset, bool isdst) asm("_ZN4HPHP27fni_timezone_name_from_abbrERKNS_6StringEib");

TypedValue * fg1_timezone_name_from_abbr(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_timezone_name_from_abbr(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
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
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_timezone_name_from_abbr((rv), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(-1), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(true));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_timezone_name_from_abbr(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfBoolean) && (count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
        fh_timezone_name_from_abbr((&(rv)), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(-1), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(true));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_timezone_name_from_abbr(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("timezone_name_from_abbr", count, 1, 3, 1);
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
HPHP::String HPHP::fni_timezone_name_get(HPHP::Object const&)
_ZN4HPHP21fni_timezone_name_getERKNS_6ObjectE

(return value) => rax
_rv => rdi
object => rsi
*/

Value* fh_timezone_name_get(Value* _rv, Value* object) asm("_ZN4HPHP21fni_timezone_name_getERKNS_6ObjectE");

TypedValue * fg1_timezone_name_get(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_timezone_name_get(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToObjectInPlace(args-0);
  fh_timezone_name_get((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_timezone_name_get(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_timezone_name_get((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_timezone_name_get(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("timezone_name_get", count, 1, 1, 1);
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
long long HPHP::fni_timezone_offset_get(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP23fni_timezone_offset_getERKNS_6ObjectES2_

(return value) => rax
object => rdi
dt => rsi
*/

long long fh_timezone_offset_get(Value* object, Value* dt) asm("_ZN4HPHP23fni_timezone_offset_getERKNS_6ObjectES2_");

TypedValue * fg1_timezone_offset_get(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_timezone_offset_get(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfInt64;
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (long long)fh_timezone_offset_get((Value*)(args-0), (Value*)(args-1));
  return rv;
}

TypedValue* fg_timezone_offset_get(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (long long)fh_timezone_offset_get((Value*)(args-0), (Value*)(args-1));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_timezone_offset_get(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("timezone_offset_get", count, 2, 2, 1);
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
HPHP::Object HPHP::fni_timezone_open(HPHP::String const&)
_ZN4HPHP17fni_timezone_openERKNS_6StringE

(return value) => rax
_rv => rdi
timezone => rsi
*/

Value* fh_timezone_open(Value* _rv, Value* timezone) asm("_ZN4HPHP17fni_timezone_openERKNS_6StringE");

TypedValue * fg1_timezone_open(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_timezone_open(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfObject;
  tvCastToStringInPlace(args-0);
  fh_timezone_open((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_timezone_open(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfObject;
        fh_timezone_open((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_timezone_open(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("timezone_open", count, 1, 1, 1);
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
HPHP::Array HPHP::fni_timezone_transitions_get(HPHP::Object const&)
_ZN4HPHP28fni_timezone_transitions_getERKNS_6ObjectE

(return value) => rax
_rv => rdi
object => rsi
*/

Value* fh_timezone_transitions_get(Value* _rv, Value* object) asm("_ZN4HPHP28fni_timezone_transitions_getERKNS_6ObjectE");

TypedValue * fg1_timezone_transitions_get(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_timezone_transitions_get(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfArray;
  tvCastToObjectInPlace(args-0);
  fh_timezone_transitions_get((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_timezone_transitions_get(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfArray;
        fh_timezone_transitions_get((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_timezone_transitions_get(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("timezone_transitions_get", count, 1, 1, 1);
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
HPHP::String HPHP::fni_timezone_version_get()
_ZN4HPHP24fni_timezone_version_getEv

(return value) => rax
_rv => rdi
*/

Value* fh_timezone_version_get(Value* _rv) asm("_ZN4HPHP24fni_timezone_version_getEv");

TypedValue* fg_timezone_version_get(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfString;
      fh_timezone_version_get((Value*)(&(rv)));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("timezone_version_get", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::fni_call_user_func_array(HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP24fni_call_user_func_arrayERKNS_7VariantERKNS_5ArrayE

(return value) => rax
_rv => rdi
function => rsi
params => rdx
*/

TypedValue* fh_call_user_func_array(TypedValue* _rv, TypedValue* function, Value* params) asm("_ZN4HPHP24fni_call_user_func_arrayERKNS_7VariantERKNS_5ArrayE");

TypedValue * fg1_call_user_func_array(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_call_user_func_array(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToArrayInPlace(args-1);
  fh_call_user_func_array((rv), (args-0), (Value*)(args-1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_call_user_func_array(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfArray) {
        fh_call_user_func_array((&(rv)), (args-0), (Value*)(args-1));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_call_user_func_array(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("call_user_func_array", count, 2, 2, 1);
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
double HPHP::fni_pi()
_ZN4HPHP6fni_piEv

(return value) => xmm0
*/

double fh_pi() asm("_ZN4HPHP6fni_piEv");

TypedValue* fg_pi(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfDouble;
      rv.m_data.dbl = fh_pi();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("pi", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::fni_is_finite(double)
_ZN4HPHP13fni_is_finiteEd

(return value) => rax
val => xmm0
*/

bool fh_is_finite(double val) asm("_ZN4HPHP13fni_is_finiteEd");

TypedValue * fg1_is_finite(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_is_finite(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToDoubleInPlace(args-0);
  rv->m_data.num = (fh_is_finite((args[-0].m_data.dbl))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_is_finite(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfDouble) {
        rv._count = 0;
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::fni_is_infinite(double)
_ZN4HPHP15fni_is_infiniteEd

(return value) => rax
val => xmm0
*/

bool fh_is_infinite(double val) asm("_ZN4HPHP15fni_is_infiniteEd");

TypedValue * fg1_is_infinite(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_is_infinite(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToDoubleInPlace(args-0);
  rv->m_data.num = (fh_is_infinite((args[-0].m_data.dbl))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_is_infinite(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfDouble) {
        rv._count = 0;
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::fni_is_nan(double)
_ZN4HPHP10fni_is_nanEd

(return value) => rax
val => xmm0
*/

bool fh_is_nan(double val) asm("_ZN4HPHP10fni_is_nanEd");

TypedValue * fg1_is_nan(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_is_nan(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToDoubleInPlace(args-0);
  rv->m_data.num = (fh_is_nan((args[-0].m_data.dbl))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_is_nan(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfDouble) {
        rv._count = 0;
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::fni_ceil(double)
_ZN4HPHP8fni_ceilEd

(return value) => xmm0
value => xmm0
*/

double fh_ceil(double value) asm("_ZN4HPHP8fni_ceilEd");

TypedValue * fg1_ceil(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_ceil(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfDouble;
  tvCastToDoubleInPlace(args-0);
  rv->m_data.dbl = fh_ceil((args[-0].m_data.dbl));
  return rv;
}

TypedValue* fg_ceil(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfDouble) {
        rv._count = 0;
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::fni_floor(double)
_ZN4HPHP9fni_floorEd

(return value) => xmm0
value => xmm0
*/

double fh_floor(double value) asm("_ZN4HPHP9fni_floorEd");

TypedValue * fg1_floor(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_floor(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfDouble;
  tvCastToDoubleInPlace(args-0);
  rv->m_data.dbl = fh_floor((args[-0].m_data.dbl));
  return rv;
}

TypedValue* fg_floor(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfDouble) {
        rv._count = 0;
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::fni_deg2rad(double)
_ZN4HPHP11fni_deg2radEd

(return value) => xmm0
number => xmm0
*/

double fh_deg2rad(double number) asm("_ZN4HPHP11fni_deg2radEd");

TypedValue * fg1_deg2rad(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_deg2rad(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfDouble;
  tvCastToDoubleInPlace(args-0);
  rv->m_data.dbl = fh_deg2rad((args[-0].m_data.dbl));
  return rv;
}

TypedValue* fg_deg2rad(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfDouble) {
        rv._count = 0;
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::fni_rad2deg(double)
_ZN4HPHP11fni_rad2degEd

(return value) => xmm0
number => xmm0
*/

double fh_rad2deg(double number) asm("_ZN4HPHP11fni_rad2degEd");

TypedValue * fg1_rad2deg(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_rad2deg(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfDouble;
  tvCastToDoubleInPlace(args-0);
  rv->m_data.dbl = fh_rad2deg((args[-0].m_data.dbl));
  return rv;
}

TypedValue* fg_rad2deg(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfDouble) {
        rv._count = 0;
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::fni_decbin(long long)
_ZN4HPHP10fni_decbinEx

(return value) => rax
_rv => rdi
number => rsi
*/

Value* fh_decbin(Value* _rv, long long number) asm("_ZN4HPHP10fni_decbinEx");

TypedValue * fg1_decbin(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_decbin(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToInt64InPlace(args-0);
  fh_decbin((Value*)(rv), (long long)(args[-0].m_data.num));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_decbin(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfInt64) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_decbin((Value*)(&(rv)), (long long)(args[-0].m_data.num));
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::fni_dechex(long long)
_ZN4HPHP10fni_dechexEx

(return value) => rax
_rv => rdi
number => rsi
*/

Value* fh_dechex(Value* _rv, long long number) asm("_ZN4HPHP10fni_dechexEx");

TypedValue * fg1_dechex(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_dechex(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToInt64InPlace(args-0);
  fh_dechex((Value*)(rv), (long long)(args[-0].m_data.num));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dechex(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfInt64) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_dechex((Value*)(&(rv)), (long long)(args[-0].m_data.num));
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::fni_decoct(long long)
_ZN4HPHP10fni_decoctEx

(return value) => rax
_rv => rdi
number => rsi
*/

Value* fh_decoct(Value* _rv, long long number) asm("_ZN4HPHP10fni_decoctEx");

TypedValue * fg1_decoct(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_decoct(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToInt64InPlace(args-0);
  fh_decoct((Value*)(rv), (long long)(args[-0].m_data.num));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_decoct(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfInt64) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_decoct((Value*)(&(rv)), (long long)(args[-0].m_data.num));
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::fni_bindec(HPHP::String const&)
_ZN4HPHP10fni_bindecERKNS_6StringE

(return value) => rax
_rv => rdi
binary_string => rsi
*/

TypedValue* fh_bindec(TypedValue* _rv, Value* binary_string) asm("_ZN4HPHP10fni_bindecERKNS_6StringE");

TypedValue * fg1_bindec(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_bindec(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_bindec((rv), (Value*)(args-0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_bindec(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        fh_bindec((&(rv)), (Value*)(args-0));
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::fni_hexdec(HPHP::String const&)
_ZN4HPHP10fni_hexdecERKNS_6StringE

(return value) => rax
_rv => rdi
hex_string => rsi
*/

TypedValue* fh_hexdec(TypedValue* _rv, Value* hex_string) asm("_ZN4HPHP10fni_hexdecERKNS_6StringE");

TypedValue * fg1_hexdec(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_hexdec(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_hexdec((rv), (Value*)(args-0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_hexdec(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        fh_hexdec((&(rv)), (Value*)(args-0));
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::fni_octdec(HPHP::String const&)
_ZN4HPHP10fni_octdecERKNS_6StringE

(return value) => rax
_rv => rdi
octal_string => rsi
*/

TypedValue* fh_octdec(TypedValue* _rv, Value* octal_string) asm("_ZN4HPHP10fni_octdecERKNS_6StringE");

TypedValue * fg1_octdec(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_octdec(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_octdec((rv), (Value*)(args-0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_octdec(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        fh_octdec((&(rv)), (Value*)(args-0));
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::fni_exp(double)
_ZN4HPHP7fni_expEd

(return value) => xmm0
arg => xmm0
*/

double fh_exp(double arg) asm("_ZN4HPHP7fni_expEd");

TypedValue * fg1_exp(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_exp(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfDouble;
  tvCastToDoubleInPlace(args-0);
  rv->m_data.dbl = fh_exp((args[-0].m_data.dbl));
  return rv;
}

TypedValue* fg_exp(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfDouble) {
        rv._count = 0;
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::fni_expm1(double)
_ZN4HPHP9fni_expm1Ed

(return value) => xmm0
arg => xmm0
*/

double fh_expm1(double arg) asm("_ZN4HPHP9fni_expm1Ed");

TypedValue * fg1_expm1(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_expm1(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfDouble;
  tvCastToDoubleInPlace(args-0);
  rv->m_data.dbl = fh_expm1((args[-0].m_data.dbl));
  return rv;
}

TypedValue* fg_expm1(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfDouble) {
        rv._count = 0;
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::fni_log10(double)
_ZN4HPHP9fni_log10Ed

(return value) => xmm0
arg => xmm0
*/

double fh_log10(double arg) asm("_ZN4HPHP9fni_log10Ed");

TypedValue * fg1_log10(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_log10(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfDouble;
  tvCastToDoubleInPlace(args-0);
  rv->m_data.dbl = fh_log10((args[-0].m_data.dbl));
  return rv;
}

TypedValue* fg_log10(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfDouble) {
        rv._count = 0;
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::fni_log1p(double)
_ZN4HPHP9fni_log1pEd

(return value) => xmm0
number => xmm0
*/

double fh_log1p(double number) asm("_ZN4HPHP9fni_log1pEd");

TypedValue * fg1_log1p(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_log1p(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfDouble;
  tvCastToDoubleInPlace(args-0);
  rv->m_data.dbl = fh_log1p((args[-0].m_data.dbl));
  return rv;
}

TypedValue* fg_log1p(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfDouble) {
        rv._count = 0;
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::fni_log(double, double)
_ZN4HPHP7fni_logEdd

(return value) => xmm0
arg => xmm0
base => xmm1
*/

double fh_log(double arg, double base) asm("_ZN4HPHP7fni_logEdd");

TypedValue * fg1_log(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_log(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
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

TypedValue* fg_log(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfDouble) && (args-0)->m_type == KindOfDouble) {
        rv._count = 0;
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::fni_cos(double)
_ZN4HPHP7fni_cosEd

(return value) => xmm0
arg => xmm0
*/

double fh_cos(double arg) asm("_ZN4HPHP7fni_cosEd");

TypedValue * fg1_cos(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_cos(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfDouble;
  tvCastToDoubleInPlace(args-0);
  rv->m_data.dbl = fh_cos((args[-0].m_data.dbl));
  return rv;
}

TypedValue* fg_cos(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfDouble) {
        rv._count = 0;
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::fni_cosh(double)
_ZN4HPHP8fni_coshEd

(return value) => xmm0
arg => xmm0
*/

double fh_cosh(double arg) asm("_ZN4HPHP8fni_coshEd");

TypedValue * fg1_cosh(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_cosh(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfDouble;
  tvCastToDoubleInPlace(args-0);
  rv->m_data.dbl = fh_cosh((args[-0].m_data.dbl));
  return rv;
}

TypedValue* fg_cosh(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfDouble) {
        rv._count = 0;
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::fni_sin(double)
_ZN4HPHP7fni_sinEd

(return value) => xmm0
arg => xmm0
*/

double fh_sin(double arg) asm("_ZN4HPHP7fni_sinEd");

TypedValue * fg1_sin(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_sin(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfDouble;
  tvCastToDoubleInPlace(args-0);
  rv->m_data.dbl = fh_sin((args[-0].m_data.dbl));
  return rv;
}

TypedValue* fg_sin(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfDouble) {
        rv._count = 0;
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::fni_sinh(double)
_ZN4HPHP8fni_sinhEd

(return value) => xmm0
arg => xmm0
*/

double fh_sinh(double arg) asm("_ZN4HPHP8fni_sinhEd");

TypedValue * fg1_sinh(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_sinh(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfDouble;
  tvCastToDoubleInPlace(args-0);
  rv->m_data.dbl = fh_sinh((args[-0].m_data.dbl));
  return rv;
}

TypedValue* fg_sinh(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfDouble) {
        rv._count = 0;
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::fni_tan(double)
_ZN4HPHP7fni_tanEd

(return value) => xmm0
arg => xmm0
*/

double fh_tan(double arg) asm("_ZN4HPHP7fni_tanEd");

TypedValue * fg1_tan(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_tan(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfDouble;
  tvCastToDoubleInPlace(args-0);
  rv->m_data.dbl = fh_tan((args[-0].m_data.dbl));
  return rv;
}

TypedValue* fg_tan(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfDouble) {
        rv._count = 0;
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::fni_tanh(double)
_ZN4HPHP8fni_tanhEd

(return value) => xmm0
arg => xmm0
*/

double fh_tanh(double arg) asm("_ZN4HPHP8fni_tanhEd");

TypedValue * fg1_tanh(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_tanh(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfDouble;
  tvCastToDoubleInPlace(args-0);
  rv->m_data.dbl = fh_tanh((args[-0].m_data.dbl));
  return rv;
}

TypedValue* fg_tanh(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfDouble) {
        rv._count = 0;
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::fni_acos(double)
_ZN4HPHP8fni_acosEd

(return value) => xmm0
arg => xmm0
*/

double fh_acos(double arg) asm("_ZN4HPHP8fni_acosEd");

TypedValue * fg1_acos(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_acos(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfDouble;
  tvCastToDoubleInPlace(args-0);
  rv->m_data.dbl = fh_acos((args[-0].m_data.dbl));
  return rv;
}

TypedValue* fg_acos(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfDouble) {
        rv._count = 0;
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::fni_acosh(double)
_ZN4HPHP9fni_acoshEd

(return value) => xmm0
arg => xmm0
*/

double fh_acosh(double arg) asm("_ZN4HPHP9fni_acoshEd");

TypedValue * fg1_acosh(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_acosh(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfDouble;
  tvCastToDoubleInPlace(args-0);
  rv->m_data.dbl = fh_acosh((args[-0].m_data.dbl));
  return rv;
}

TypedValue* fg_acosh(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfDouble) {
        rv._count = 0;
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::fni_asin(double)
_ZN4HPHP8fni_asinEd

(return value) => xmm0
arg => xmm0
*/

double fh_asin(double arg) asm("_ZN4HPHP8fni_asinEd");

TypedValue * fg1_asin(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_asin(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfDouble;
  tvCastToDoubleInPlace(args-0);
  rv->m_data.dbl = fh_asin((args[-0].m_data.dbl));
  return rv;
}

TypedValue* fg_asin(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfDouble) {
        rv._count = 0;
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::fni_asinh(double)
_ZN4HPHP9fni_asinhEd

(return value) => xmm0
arg => xmm0
*/

double fh_asinh(double arg) asm("_ZN4HPHP9fni_asinhEd");

TypedValue * fg1_asinh(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_asinh(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfDouble;
  tvCastToDoubleInPlace(args-0);
  rv->m_data.dbl = fh_asinh((args[-0].m_data.dbl));
  return rv;
}

TypedValue* fg_asinh(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfDouble) {
        rv._count = 0;
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::fni_atan(double)
_ZN4HPHP8fni_atanEd

(return value) => xmm0
arg => xmm0
*/

double fh_atan(double arg) asm("_ZN4HPHP8fni_atanEd");

TypedValue * fg1_atan(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_atan(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfDouble;
  tvCastToDoubleInPlace(args-0);
  rv->m_data.dbl = fh_atan((args[-0].m_data.dbl));
  return rv;
}

TypedValue* fg_atan(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfDouble) {
        rv._count = 0;
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::fni_atanh(double)
_ZN4HPHP9fni_atanhEd

(return value) => xmm0
arg => xmm0
*/

double fh_atanh(double arg) asm("_ZN4HPHP9fni_atanhEd");

TypedValue * fg1_atanh(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_atanh(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfDouble;
  tvCastToDoubleInPlace(args-0);
  rv->m_data.dbl = fh_atanh((args[-0].m_data.dbl));
  return rv;
}

TypedValue* fg_atanh(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfDouble) {
        rv._count = 0;
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::fni_atan2(double, double)
_ZN4HPHP9fni_atan2Edd

(return value) => xmm0
y => xmm0
x => xmm1
*/

double fh_atan2(double y, double x) asm("_ZN4HPHP9fni_atan2Edd");

TypedValue * fg1_atan2(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_atan2(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
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

TypedValue* fg_atan2(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfDouble) {
        rv._count = 0;
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::fni_hypot(double, double)
_ZN4HPHP9fni_hypotEdd

(return value) => xmm0
x => xmm0
y => xmm1
*/

double fh_hypot(double x, double y) asm("_ZN4HPHP9fni_hypotEdd");

TypedValue * fg1_hypot(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_hypot(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
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

TypedValue* fg_hypot(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfDouble) {
        rv._count = 0;
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::fni_fmod(double, double)
_ZN4HPHP8fni_fmodEdd

(return value) => xmm0
x => xmm0
y => xmm1
*/

double fh_fmod(double x, double y) asm("_ZN4HPHP8fni_fmodEdd");

TypedValue * fg1_fmod(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_fmod(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
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

TypedValue* fg_fmod(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfDouble) {
        rv._count = 0;
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::fni_sqrt(double)
_ZN4HPHP8fni_sqrtEd

(return value) => xmm0
arg => xmm0
*/

double fh_sqrt(double arg) asm("_ZN4HPHP8fni_sqrtEd");

TypedValue * fg1_sqrt(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_sqrt(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfDouble;
  tvCastToDoubleInPlace(args-0);
  rv->m_data.dbl = fh_sqrt((args[-0].m_data.dbl));
  return rv;
}

TypedValue* fg_sqrt(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfDouble) {
        rv._count = 0;
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long long HPHP::fni_getrandmax()
_ZN4HPHP14fni_getrandmaxEv

(return value) => rax
*/

long long fh_getrandmax() asm("_ZN4HPHP14fni_getrandmaxEv");

TypedValue* fg_getrandmax(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfInt64;
      rv.m_data.num = (long long)fh_getrandmax();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("getrandmax", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long long HPHP::fni_mt_getrandmax()
_ZN4HPHP17fni_mt_getrandmaxEv

(return value) => rax
*/

long long fh_mt_getrandmax() asm("_ZN4HPHP17fni_mt_getrandmaxEv");

TypedValue* fg_mt_getrandmax(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfInt64;
      rv.m_data.num = (long long)fh_mt_getrandmax();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("mt_getrandmax", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long long HPHP::fni_mt_rand(long long, long long)
_ZN4HPHP11fni_mt_randExx

(return value) => rax
min => rdi
max => rsi
*/

long long fh_mt_rand(long long min, long long max) asm("_ZN4HPHP11fni_mt_randExx");

TypedValue * fg1_mt_rand(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_mt_rand(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
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
  rv->m_data.num = (long long)fh_mt_rand((count > 0) ? (long long)(args[-0].m_data.num) : (long long)(0), (count > 1) ? (long long)(args[-1].m_data.num) : (long long)(RAND_MAX));
  return rv;
}

TypedValue* fg_mt_rand(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64) && (count <= 0 || (args-0)->m_type == KindOfInt64)) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (long long)fh_mt_rand((count > 0) ? (long long)(args[-0].m_data.num) : (long long)(0), (count > 1) ? (long long)(args[-1].m_data.num) : (long long)(RAND_MAX));
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::fni_lcg_value()
_ZN4HPHP13fni_lcg_valueEv

(return value) => xmm0
*/

double fh_lcg_value() asm("_ZN4HPHP13fni_lcg_valueEv");

TypedValue* fg_lcg_value(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfDouble;
      rv.m_data.dbl = fh_lcg_value();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("lcg_value", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::fni_mysql_set_charset(HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP21fni_mysql_set_charsetERKNS_6StringERKNS_7VariantE

(return value) => rax
_rv => rdi
charset => rsi
link_identifier => rdx
*/

TypedValue* fh_mysql_set_charset(TypedValue* _rv, Value* charset, TypedValue* link_identifier) asm("_ZN4HPHP21fni_mysql_set_charsetERKNS_6StringERKNS_7VariantE");

TypedValue * fg1_mysql_set_charset(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_set_charset(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  Variant defVal1;
  fh_mysql_set_charset((rv), (Value*)(args-0), (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mysql_set_charset(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        Variant defVal1;
        fh_mysql_set_charset((&(rv)), (Value*)(args-0), (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mysql_set_charset(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mysql_set_charset", count, 1, 2, 1);
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
HPHP::Variant HPHP::fni_mysql_ping(HPHP::Variant const&)
_ZN4HPHP14fni_mysql_pingERKNS_7VariantE

(return value) => rax
_rv => rdi
link_identifier => rsi
*/

TypedValue* fh_mysql_ping(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP14fni_mysql_pingERKNS_7VariantE");

TypedValue* fg_mysql_ping(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      Variant defVal0;
      fh_mysql_ping((&(rv)), (count > 0) ? (args-0) : (TypedValue*)(&defVal0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("mysql_ping", 1, 1);
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
HPHP::Variant HPHP::fni_mysql_client_encoding(HPHP::Variant const&)
_ZN4HPHP25fni_mysql_client_encodingERKNS_7VariantE

(return value) => rax
_rv => rdi
link_identifier => rsi
*/

TypedValue* fh_mysql_client_encoding(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP25fni_mysql_client_encodingERKNS_7VariantE");

TypedValue* fg_mysql_client_encoding(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      Variant defVal0;
      fh_mysql_client_encoding((&(rv)), (count > 0) ? (args-0) : (TypedValue*)(&defVal0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("mysql_client_encoding", 1, 1);
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
HPHP::Variant HPHP::fni_mysql_close(HPHP::Variant const&)
_ZN4HPHP15fni_mysql_closeERKNS_7VariantE

(return value) => rax
_rv => rdi
link_identifier => rsi
*/

TypedValue* fh_mysql_close(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP15fni_mysql_closeERKNS_7VariantE");

TypedValue* fg_mysql_close(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      Variant defVal0;
      fh_mysql_close((&(rv)), (count > 0) ? (args-0) : (TypedValue*)(&defVal0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("mysql_close", 1, 1);
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
HPHP::String HPHP::fni_mysql_get_client_info()
_ZN4HPHP25fni_mysql_get_client_infoEv

(return value) => rax
_rv => rdi
*/

Value* fh_mysql_get_client_info(Value* _rv) asm("_ZN4HPHP25fni_mysql_get_client_infoEv");

TypedValue* fg_mysql_get_client_info(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfString;
      fh_mysql_get_client_info((Value*)(&(rv)));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("mysql_get_client_info", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::fni_mysql_get_host_info(HPHP::Variant const&)
_ZN4HPHP23fni_mysql_get_host_infoERKNS_7VariantE

(return value) => rax
_rv => rdi
link_identifier => rsi
*/

TypedValue* fh_mysql_get_host_info(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP23fni_mysql_get_host_infoERKNS_7VariantE");

TypedValue* fg_mysql_get_host_info(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      Variant defVal0;
      fh_mysql_get_host_info((&(rv)), (count > 0) ? (args-0) : (TypedValue*)(&defVal0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("mysql_get_host_info", 1, 1);
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
HPHP::Variant HPHP::fni_mysql_get_proto_info(HPHP::Variant const&)
_ZN4HPHP24fni_mysql_get_proto_infoERKNS_7VariantE

(return value) => rax
_rv => rdi
link_identifier => rsi
*/

TypedValue* fh_mysql_get_proto_info(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP24fni_mysql_get_proto_infoERKNS_7VariantE");

TypedValue* fg_mysql_get_proto_info(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      Variant defVal0;
      fh_mysql_get_proto_info((&(rv)), (count > 0) ? (args-0) : (TypedValue*)(&defVal0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("mysql_get_proto_info", 1, 1);
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
HPHP::Variant HPHP::fni_mysql_get_server_info(HPHP::Variant const&)
_ZN4HPHP25fni_mysql_get_server_infoERKNS_7VariantE

(return value) => rax
_rv => rdi
link_identifier => rsi
*/

TypedValue* fh_mysql_get_server_info(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP25fni_mysql_get_server_infoERKNS_7VariantE");

TypedValue* fg_mysql_get_server_info(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      Variant defVal0;
      fh_mysql_get_server_info((&(rv)), (count > 0) ? (args-0) : (TypedValue*)(&defVal0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("mysql_get_server_info", 1, 1);
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
HPHP::Variant HPHP::fni_mysql_info(HPHP::Variant const&)
_ZN4HPHP14fni_mysql_infoERKNS_7VariantE

(return value) => rax
_rv => rdi
link_identifier => rsi
*/

TypedValue* fh_mysql_info(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP14fni_mysql_infoERKNS_7VariantE");

TypedValue* fg_mysql_info(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      Variant defVal0;
      fh_mysql_info((&(rv)), (count > 0) ? (args-0) : (TypedValue*)(&defVal0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("mysql_info", 1, 1);
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
HPHP::Variant HPHP::fni_mysql_insert_id(HPHP::Variant const&)
_ZN4HPHP19fni_mysql_insert_idERKNS_7VariantE

(return value) => rax
_rv => rdi
link_identifier => rsi
*/

TypedValue* fh_mysql_insert_id(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP19fni_mysql_insert_idERKNS_7VariantE");

TypedValue* fg_mysql_insert_id(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      Variant defVal0;
      fh_mysql_insert_id((&(rv)), (count > 0) ? (args-0) : (TypedValue*)(&defVal0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("mysql_insert_id", 1, 1);
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
HPHP::Variant HPHP::fni_mysql_stat(HPHP::Variant const&)
_ZN4HPHP14fni_mysql_statERKNS_7VariantE

(return value) => rax
_rv => rdi
link_identifier => rsi
*/

TypedValue* fh_mysql_stat(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP14fni_mysql_statERKNS_7VariantE");

TypedValue* fg_mysql_stat(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      Variant defVal0;
      fh_mysql_stat((&(rv)), (count > 0) ? (args-0) : (TypedValue*)(&defVal0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("mysql_stat", 1, 1);
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
HPHP::Variant HPHP::fni_mysql_thread_id(HPHP::Variant const&)
_ZN4HPHP19fni_mysql_thread_idERKNS_7VariantE

(return value) => rax
_rv => rdi
link_identifier => rsi
*/

TypedValue* fh_mysql_thread_id(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP19fni_mysql_thread_idERKNS_7VariantE");

TypedValue* fg_mysql_thread_id(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      Variant defVal0;
      fh_mysql_thread_id((&(rv)), (count > 0) ? (args-0) : (TypedValue*)(&defVal0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("mysql_thread_id", 1, 1);
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
HPHP::Variant HPHP::fni_mysql_create_db(HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP19fni_mysql_create_dbERKNS_6StringERKNS_7VariantE

(return value) => rax
_rv => rdi
db => rsi
link_identifier => rdx
*/

TypedValue* fh_mysql_create_db(TypedValue* _rv, Value* db, TypedValue* link_identifier) asm("_ZN4HPHP19fni_mysql_create_dbERKNS_6StringERKNS_7VariantE");

TypedValue * fg1_mysql_create_db(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_create_db(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  Variant defVal1;
  fh_mysql_create_db((rv), (Value*)(args-0), (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mysql_create_db(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        Variant defVal1;
        fh_mysql_create_db((&(rv)), (Value*)(args-0), (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mysql_create_db(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mysql_create_db", count, 1, 2, 1);
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
HPHP::Variant HPHP::fni_mysql_select_db(HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP19fni_mysql_select_dbERKNS_6StringERKNS_7VariantE

(return value) => rax
_rv => rdi
db => rsi
link_identifier => rdx
*/

TypedValue* fh_mysql_select_db(TypedValue* _rv, Value* db, TypedValue* link_identifier) asm("_ZN4HPHP19fni_mysql_select_dbERKNS_6StringERKNS_7VariantE");

TypedValue * fg1_mysql_select_db(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_select_db(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  Variant defVal1;
  fh_mysql_select_db((rv), (Value*)(args-0), (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mysql_select_db(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        Variant defVal1;
        fh_mysql_select_db((&(rv)), (Value*)(args-0), (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mysql_select_db(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mysql_select_db", count, 1, 2, 1);
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
HPHP::Variant HPHP::fni_mysql_drop_db(HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP17fni_mysql_drop_dbERKNS_6StringERKNS_7VariantE

(return value) => rax
_rv => rdi
db => rsi
link_identifier => rdx
*/

TypedValue* fh_mysql_drop_db(TypedValue* _rv, Value* db, TypedValue* link_identifier) asm("_ZN4HPHP17fni_mysql_drop_dbERKNS_6StringERKNS_7VariantE");

TypedValue * fg1_mysql_drop_db(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_drop_db(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  Variant defVal1;
  fh_mysql_drop_db((rv), (Value*)(args-0), (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mysql_drop_db(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        Variant defVal1;
        fh_mysql_drop_db((&(rv)), (Value*)(args-0), (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mysql_drop_db(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mysql_drop_db", count, 1, 2, 1);
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
HPHP::Variant HPHP::fni_mysql_affected_rows(HPHP::Variant const&)
_ZN4HPHP23fni_mysql_affected_rowsERKNS_7VariantE

(return value) => rax
_rv => rdi
link_identifier => rsi
*/

TypedValue* fh_mysql_affected_rows(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP23fni_mysql_affected_rowsERKNS_7VariantE");

TypedValue* fg_mysql_affected_rows(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      Variant defVal0;
      fh_mysql_affected_rows((&(rv)), (count > 0) ? (args-0) : (TypedValue*)(&defVal0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("mysql_affected_rows", 1, 1);
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
HPHP::Variant HPHP::fni_mysql_db_query(HPHP::String const&, HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP18fni_mysql_db_queryERKNS_6StringES2_RKNS_7VariantE

(return value) => rax
_rv => rdi
database => rsi
query => rdx
link_identifier => rcx
*/

TypedValue* fh_mysql_db_query(TypedValue* _rv, Value* database, Value* query, TypedValue* link_identifier) asm("_ZN4HPHP18fni_mysql_db_queryERKNS_6StringES2_RKNS_7VariantE");

TypedValue * fg1_mysql_db_query(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_db_query(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  Variant defVal2;
  fh_mysql_db_query((rv), (Value*)(args-0), (Value*)(args-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mysql_db_query(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        Variant defVal2;
        fh_mysql_db_query((&(rv)), (Value*)(args-0), (Value*)(args-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mysql_db_query(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mysql_db_query", count, 2, 3, 1);
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
HPHP::Variant HPHP::fni_mysql_list_fields(HPHP::String const&, HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP21fni_mysql_list_fieldsERKNS_6StringES2_RKNS_7VariantE

(return value) => rax
_rv => rdi
database_name => rsi
table_name => rdx
link_identifier => rcx
*/

TypedValue* fh_mysql_list_fields(TypedValue* _rv, Value* database_name, Value* table_name, TypedValue* link_identifier) asm("_ZN4HPHP21fni_mysql_list_fieldsERKNS_6StringES2_RKNS_7VariantE");

TypedValue * fg1_mysql_list_fields(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_list_fields(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  Variant defVal2;
  fh_mysql_list_fields((rv), (Value*)(args-0), (Value*)(args-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mysql_list_fields(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        Variant defVal2;
        fh_mysql_list_fields((&(rv)), (Value*)(args-0), (Value*)(args-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mysql_list_fields(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mysql_list_fields", count, 2, 3, 1);
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
HPHP::Variant HPHP::fni_mysql_db_name(HPHP::Variant const&, int, HPHP::Variant const&)
_ZN4HPHP17fni_mysql_db_nameERKNS_7VariantEiS2_

(return value) => rax
_rv => rdi
result => rsi
row => rdx
field => rcx
*/

TypedValue* fh_mysql_db_name(TypedValue* _rv, TypedValue* result, int row, TypedValue* field) asm("_ZN4HPHP17fni_mysql_db_nameERKNS_7VariantEiS2_");

TypedValue * fg1_mysql_db_name(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_db_name(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-1);
  fh_mysql_db_name((rv), (args-0), (int)(args[-1].m_data.num), (count > 2) ? (args-2) : (TypedValue*)(&null_variant));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mysql_db_name(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((args-1)->m_type == KindOfInt64) {
        fh_mysql_db_name((&(rv)), (args-0), (int)(args[-1].m_data.num), (count > 2) ? (args-2) : (TypedValue*)(&null_variant));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mysql_db_name(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mysql_db_name", count, 2, 3, 1);
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
HPHP::Variant HPHP::fni_mysql_tablename(HPHP::Variant const&, int)
_ZN4HPHP19fni_mysql_tablenameERKNS_7VariantEi

(return value) => rax
_rv => rdi
result => rsi
i => rdx
*/

TypedValue* fh_mysql_tablename(TypedValue* _rv, TypedValue* result, int i) asm("_ZN4HPHP19fni_mysql_tablenameERKNS_7VariantEi");

TypedValue * fg1_mysql_tablename(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_tablename(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-1);
  fh_mysql_tablename((rv), (args-0), (int)(args[-1].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mysql_tablename(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64) {
        fh_mysql_tablename((&(rv)), (args-0), (int)(args[-1].m_data.num));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mysql_tablename(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mysql_tablename", count, 2, 2, 1);
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
bool HPHP::fni_checkdnsrr(HPHP::String const&, HPHP::String const&)
_ZN4HPHP14fni_checkdnsrrERKNS_6StringES2_

(return value) => rax
host => rdi
type => rsi
*/

bool fh_checkdnsrr(Value* host, Value* type) asm("_ZN4HPHP14fni_checkdnsrrERKNS_6StringES2_");

TypedValue * fg1_checkdnsrr(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_checkdnsrr(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
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
  rv->m_data.num = (fh_checkdnsrr((Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_string))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_checkdnsrr(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_checkdnsrr((Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_string))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_checkdnsrr(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("checkdnsrr", count, 1, 2, 1);
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
bool HPHP::fni_getmxrr(HPHP::String const&, HPHP::VRefParamValue const&, HPHP::VRefParamValue const&)
_ZN4HPHP11fni_getmxrrERKNS_6StringERKNS_14VRefParamValueES5_

(return value) => rax
hostname => rdi
mxhosts => rsi
weight => rdx
*/

bool fh_getmxrr(Value* hostname, TypedValue* mxhosts, TypedValue* weight) asm("_ZN4HPHP11fni_getmxrrERKNS_6StringERKNS_14VRefParamValueES5_");

TypedValue * fg1_getmxrr(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_getmxrr(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToStringInPlace(args-0);
  VRefParamValue defVal2 = null;
  rv->m_data.num = (fh_getmxrr((Value*)(args-0), (args-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_getmxrr(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        VRefParamValue defVal2 = null;
        rv.m_data.num = (fh_getmxrr((Value*)(args-0), (args-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_getmxrr(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("getmxrr", count, 2, 3, 1);
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
HPHP::Variant HPHP::fni_socket_get_status(HPHP::Object const&)
_ZN4HPHP21fni_socket_get_statusERKNS_6ObjectE

(return value) => rax
_rv => rdi
stream => rsi
*/

TypedValue* fh_socket_get_status(TypedValue* _rv, Value* stream) asm("_ZN4HPHP21fni_socket_get_statusERKNS_6ObjectE");

TypedValue * fg1_socket_get_status(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_socket_get_status(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  fh_socket_get_status((rv), (Value*)(args-0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_socket_get_status(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        fh_socket_get_status((&(rv)), (Value*)(args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_socket_get_status(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("socket_get_status", count, 1, 1, 1);
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
bool HPHP::fni_socket_set_blocking(HPHP::Object const&, int)
_ZN4HPHP23fni_socket_set_blockingERKNS_6ObjectEi

(return value) => rax
stream => rdi
mode => rsi
*/

bool fh_socket_set_blocking(Value* stream, int mode) asm("_ZN4HPHP23fni_socket_set_blockingERKNS_6ObjectEi");

TypedValue * fg1_socket_set_blocking(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_socket_set_blocking(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_socket_set_blocking((Value*)(args-0), (int)(args[-1].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_socket_set_blocking(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_socket_set_blocking((Value*)(args-0), (int)(args[-1].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_socket_set_blocking(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("socket_set_blocking", count, 2, 2, 1);
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
bool HPHP::fni_socket_set_timeout(HPHP::Object const&, int, int)
_ZN4HPHP22fni_socket_set_timeoutERKNS_6ObjectEii

(return value) => rax
stream => rdi
seconds => rsi
microseconds => rdx
*/

bool fh_socket_set_timeout(Value* stream, int seconds, int microseconds) asm("_ZN4HPHP22fni_socket_set_timeoutERKNS_6ObjectEii");

TypedValue * fg1_socket_set_timeout(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_socket_set_timeout(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    break;
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_socket_set_timeout((Value*)(args-0), (int)(args[-1].m_data.num), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_socket_set_timeout(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfInt64) && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_socket_set_timeout((Value*)(args-0), (int)(args[-1].m_data.num), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_socket_set_timeout(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("socket_set_timeout", count, 2, 3, 1);
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
void HPHP::fni_define_syslog_variables()
_ZN4HPHP27fni_define_syslog_variablesEv

*/

void fh_define_syslog_variables() asm("_ZN4HPHP27fni_define_syslog_variablesEv");

TypedValue* fg_define_syslog_variables(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_data.num = 0LL;
      rv._count = 0;
      rv.m_type = KindOfNull;
      fh_define_syslog_variables();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("define_syslog_variables", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::fni_openlog(HPHP::String const&, int, int)
_ZN4HPHP11fni_openlogERKNS_6StringEii

(return value) => rax
ident => rdi
option => rsi
facility => rdx
*/

bool fh_openlog(Value* ident, int option, int facility) asm("_ZN4HPHP11fni_openlogERKNS_6StringEii");

TypedValue * fg1_openlog(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_openlog(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (fh_openlog((Value*)(args-0), (int)(args[-1].m_data.num), (int)(args[-2].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_openlog(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64 && IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_openlog((Value*)(args-0), (int)(args[-1].m_data.num), (int)(args[-2].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_openlog(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("openlog", count, 3, 3, 1);
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
bool HPHP::fni_closelog()
_ZN4HPHP12fni_closelogEv

(return value) => rax
*/

bool fh_closelog() asm("_ZN4HPHP12fni_closelogEv");

TypedValue* fg_closelog(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfBoolean;
      rv.m_data.num = (fh_closelog()) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("closelog", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::fni_syslog(int, HPHP::String const&)
_ZN4HPHP10fni_syslogEiRKNS_6StringE

(return value) => rax
priority => rdi
message => rsi
*/

bool fh_syslog(int priority, Value* message) asm("_ZN4HPHP10fni_syslogEiRKNS_6StringE");

TypedValue * fg1_syslog(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_syslog(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  rv->m_data.num = (fh_syslog((int)(args[-0].m_data.num), (Value*)(args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_syslog(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfInt64) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_syslog((int)(args[-0].m_data.num), (Value*)(args-1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_syslog(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("syslog", count, 2, 2, 1);
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
long long HPHP::fni_cpu_get_count()
_ZN4HPHP17fni_cpu_get_countEv

(return value) => rax
*/

long long fh_cpu_get_count() asm("_ZN4HPHP17fni_cpu_get_countEv");

TypedValue* fg_cpu_get_count(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfInt64;
      rv.m_data.num = (long long)fh_cpu_get_count();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("cpu_get_count", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::fni_cpu_get_model()
_ZN4HPHP17fni_cpu_get_modelEv

(return value) => rax
_rv => rdi
*/

Value* fh_cpu_get_model(Value* _rv) asm("_ZN4HPHP17fni_cpu_get_modelEv");

TypedValue* fg_cpu_get_model(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfString;
      fh_cpu_get_model((Value*)(&(rv)));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("cpu_get_model", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::fni_ob_start(HPHP::Variant const&, int, bool)
_ZN4HPHP12fni_ob_startERKNS_7VariantEib

(return value) => rax
output_callback => rdi
chunk_size => rsi
erase => rdx
*/

bool fh_ob_start(TypedValue* output_callback, int chunk_size, bool erase) asm("_ZN4HPHP12fni_ob_startERKNS_7VariantEib");

TypedValue * fg1_ob_start(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_ob_start(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
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
  case 0:
    break;
  }
  Variant defVal0;
  rv->m_data.num = (fh_ob_start((count > 0) ? (args-0) : (TypedValue*)(&defVal0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(true))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_ob_start(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfBoolean) && (count <= 1 || (args-1)->m_type == KindOfInt64)) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        Variant defVal0;
        rv.m_data.num = (fh_ob_start((count > 0) ? (args-0) : (TypedValue*)(&defVal0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(true))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_ob_start(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("ob_start", 3, 1);
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
void HPHP::fni_ob_clean()
_ZN4HPHP12fni_ob_cleanEv

*/

void fh_ob_clean() asm("_ZN4HPHP12fni_ob_cleanEv");

TypedValue* fg_ob_clean(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_data.num = 0LL;
      rv._count = 0;
      rv.m_type = KindOfNull;
      fh_ob_clean();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("ob_clean", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::fni_ob_flush()
_ZN4HPHP12fni_ob_flushEv

*/

void fh_ob_flush() asm("_ZN4HPHP12fni_ob_flushEv");

TypedValue* fg_ob_flush(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_data.num = 0LL;
      rv._count = 0;
      rv.m_type = KindOfNull;
      fh_ob_flush();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("ob_flush", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::fni_ob_end_clean()
_ZN4HPHP16fni_ob_end_cleanEv

(return value) => rax
*/

bool fh_ob_end_clean() asm("_ZN4HPHP16fni_ob_end_cleanEv");

TypedValue* fg_ob_end_clean(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfBoolean;
      rv.m_data.num = (fh_ob_end_clean()) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("ob_end_clean", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::fni_ob_end_flush()
_ZN4HPHP16fni_ob_end_flushEv

(return value) => rax
*/

bool fh_ob_end_flush() asm("_ZN4HPHP16fni_ob_end_flushEv");

TypedValue* fg_ob_end_flush(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfBoolean;
      rv.m_data.num = (fh_ob_end_flush()) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("ob_end_flush", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::fni_flush()
_ZN4HPHP9fni_flushEv

*/

void fh_flush() asm("_ZN4HPHP9fni_flushEv");

TypedValue* fg_flush(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_data.num = 0LL;
      rv._count = 0;
      rv.m_type = KindOfNull;
      fh_flush();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("flush", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::fni_ob_get_clean()
_ZN4HPHP16fni_ob_get_cleanEv

(return value) => rax
_rv => rdi
*/

Value* fh_ob_get_clean(Value* _rv) asm("_ZN4HPHP16fni_ob_get_cleanEv");

TypedValue* fg_ob_get_clean(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfString;
      fh_ob_get_clean((Value*)(&(rv)));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("ob_get_clean", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::fni_ob_get_contents()
_ZN4HPHP19fni_ob_get_contentsEv

(return value) => rax
_rv => rdi
*/

Value* fh_ob_get_contents(Value* _rv) asm("_ZN4HPHP19fni_ob_get_contentsEv");

TypedValue* fg_ob_get_contents(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfString;
      fh_ob_get_contents((Value*)(&(rv)));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("ob_get_contents", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::fni_ob_get_flush()
_ZN4HPHP16fni_ob_get_flushEv

(return value) => rax
_rv => rdi
*/

Value* fh_ob_get_flush(Value* _rv) asm("_ZN4HPHP16fni_ob_get_flushEv");

TypedValue* fg_ob_get_flush(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfString;
      fh_ob_get_flush((Value*)(&(rv)));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("ob_get_flush", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long long HPHP::fni_ob_get_length()
_ZN4HPHP17fni_ob_get_lengthEv

(return value) => rax
*/

long long fh_ob_get_length() asm("_ZN4HPHP17fni_ob_get_lengthEv");

TypedValue* fg_ob_get_length(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfInt64;
      rv.m_data.num = (long long)fh_ob_get_length();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("ob_get_length", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long long HPHP::fni_ob_get_level()
_ZN4HPHP16fni_ob_get_levelEv

(return value) => rax
*/

long long fh_ob_get_level() asm("_ZN4HPHP16fni_ob_get_levelEv");

TypedValue* fg_ob_get_level(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfInt64;
      rv.m_data.num = (long long)fh_ob_get_level();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("ob_get_level", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::fni_ob_get_status(bool)
_ZN4HPHP17fni_ob_get_statusEb

(return value) => rax
_rv => rdi
full_status => rsi
*/

Value* fh_ob_get_status(Value* _rv, bool full_status) asm("_ZN4HPHP17fni_ob_get_statusEb");

TypedValue * fg1_ob_get_status(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_ob_get_status(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfArray;
  tvCastToBooleanInPlace(args-0);
  fh_ob_get_status((Value*)(rv), (count > 0) ? (bool)(args[-0].m_data.num) : (bool)(false));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_ob_get_status(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      if ((count <= 0 || (args-0)->m_type == KindOfBoolean)) {
        rv._count = 0;
        rv.m_type = KindOfArray;
        fh_ob_get_status((Value*)(&(rv)), (count > 0) ? (bool)(args[-0].m_data.num) : (bool)(false));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_ob_get_status(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("ob_get_status", 1, 1);
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
HPHP::String HPHP::fni_ob_gzhandler(HPHP::String const&, int)
_ZN4HPHP16fni_ob_gzhandlerERKNS_6StringEi

(return value) => rax
_rv => rdi
buffer => rsi
mode => rdx
*/

Value* fh_ob_gzhandler(Value* _rv, Value* buffer, int mode) asm("_ZN4HPHP16fni_ob_gzhandlerERKNS_6StringEi");

TypedValue * fg1_ob_gzhandler(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_ob_gzhandler(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_ob_gzhandler((Value*)(rv), (Value*)(args-0), (int)(args[-1].m_data.num));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_ob_gzhandler(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_ob_gzhandler((Value*)(&(rv)), (Value*)(args-0), (int)(args[-1].m_data.num));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_ob_gzhandler(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("ob_gzhandler", count, 2, 2, 1);
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
void HPHP::fni_ob_implicit_flush(bool)
_ZN4HPHP21fni_ob_implicit_flushEb

flag => rdi
*/

void fh_ob_implicit_flush(bool flag) asm("_ZN4HPHP21fni_ob_implicit_flushEb");

TypedValue * fg1_ob_implicit_flush(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_ob_implicit_flush(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  tvCastToBooleanInPlace(args-0);
  fh_ob_implicit_flush((count > 0) ? (bool)(args[-0].m_data.num) : (bool)(true));
  return rv;
}

TypedValue* fg_ob_implicit_flush(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      if ((count <= 0 || (args-0)->m_type == KindOfBoolean)) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        fh_ob_implicit_flush((count > 0) ? (bool)(args[-0].m_data.num) : (bool)(true));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_ob_implicit_flush(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("ob_implicit_flush", 1, 1);
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
HPHP::Array HPHP::fni_ob_list_handlers()
_ZN4HPHP20fni_ob_list_handlersEv

(return value) => rax
_rv => rdi
*/

Value* fh_ob_list_handlers(Value* _rv) asm("_ZN4HPHP20fni_ob_list_handlersEv");

TypedValue* fg_ob_list_handlers(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfArray;
      fh_ob_list_handlers((Value*)(&(rv)));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("ob_list_handlers", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::fni_output_add_rewrite_var(HPHP::String const&, HPHP::String const&)
_ZN4HPHP26fni_output_add_rewrite_varERKNS_6StringES2_

(return value) => rax
name => rdi
value => rsi
*/

bool fh_output_add_rewrite_var(Value* name, Value* value) asm("_ZN4HPHP26fni_output_add_rewrite_varERKNS_6StringES2_");

TypedValue * fg1_output_add_rewrite_var(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_output_add_rewrite_var(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (fh_output_add_rewrite_var((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_output_add_rewrite_var(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_output_add_rewrite_var((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_output_add_rewrite_var(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("output_add_rewrite_var", count, 2, 2, 1);
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
bool HPHP::fni_output_reset_rewrite_vars()
_ZN4HPHP29fni_output_reset_rewrite_varsEv

(return value) => rax
*/

bool fh_output_reset_rewrite_vars() asm("_ZN4HPHP29fni_output_reset_rewrite_varsEv");

TypedValue* fg_output_reset_rewrite_vars(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfBoolean;
      rv.m_data.num = (fh_output_reset_rewrite_vars()) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("output_reset_rewrite_vars", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::fni_hphp_stats(HPHP::String const&, long long)
_ZN4HPHP14fni_hphp_statsERKNS_6StringEx

name => rdi
value => rsi
*/

void fh_hphp_stats(Value* name, long long value) asm("_ZN4HPHP14fni_hphp_statsERKNS_6StringEx");

TypedValue * fg1_hphp_stats(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_stats(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_hphp_stats((Value*)(args-0), (long long)(args[-1].m_data.num));
  return rv;
}

TypedValue* fg_hphp_stats(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && IS_STRING_TYPE((args-0)->m_type)) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        fh_hphp_stats((Value*)(args-0), (long long)(args[-1].m_data.num));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_stats(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_stats", count, 2, 2, 1);
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
long long HPHP::fni_hphp_get_stats(HPHP::String const&)
_ZN4HPHP18fni_hphp_get_statsERKNS_6StringE

(return value) => rax
name => rdi
*/

long long fh_hphp_get_stats(Value* name) asm("_ZN4HPHP18fni_hphp_get_statsERKNS_6StringE");

TypedValue * fg1_hphp_get_stats(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_get_stats(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfInt64;
  tvCastToStringInPlace(args-0);
  rv->m_data.num = (long long)fh_hphp_get_stats((Value*)(args-0));
  return rv;
}

TypedValue* fg_hphp_get_stats(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (long long)fh_hphp_get_stats((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_get_stats(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_get_stats", count, 1, 1, 1);
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
HPHP::Array HPHP::fni_hphp_get_iostatus()
_ZN4HPHP21fni_hphp_get_iostatusEv

(return value) => rax
_rv => rdi
*/

Value* fh_hphp_get_iostatus(Value* _rv) asm("_ZN4HPHP21fni_hphp_get_iostatusEv");

TypedValue* fg_hphp_get_iostatus(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfArray;
      fh_hphp_get_iostatus((Value*)(&(rv)));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("hphp_get_iostatus", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::fni_hphp_set_iostatus_address(HPHP::String const&)
_ZN4HPHP29fni_hphp_set_iostatus_addressERKNS_6StringE

name => rdi
*/

void fh_hphp_set_iostatus_address(Value* name) asm("_ZN4HPHP29fni_hphp_set_iostatus_addressERKNS_6StringE");

TypedValue * fg1_hphp_set_iostatus_address(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_set_iostatus_address(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  tvCastToStringInPlace(args-0);
  fh_hphp_set_iostatus_address((Value*)(args-0));
  return rv;
}

TypedValue* fg_hphp_set_iostatus_address(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        fh_hphp_set_iostatus_address((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_set_iostatus_address(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_set_iostatus_address", count, 1, 1, 1);
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
HPHP::String HPHP::fni_posix_ctermid()
_ZN4HPHP17fni_posix_ctermidEv

(return value) => rax
_rv => rdi
*/

Value* fh_posix_ctermid(Value* _rv) asm("_ZN4HPHP17fni_posix_ctermidEv");

TypedValue* fg_posix_ctermid(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfString;
      fh_posix_ctermid((Value*)(&(rv)));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("posix_ctermid", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long long HPHP::fni_posix_get_last_error()
_ZN4HPHP24fni_posix_get_last_errorEv

(return value) => rax
*/

long long fh_posix_get_last_error() asm("_ZN4HPHP24fni_posix_get_last_errorEv");

TypedValue* fg_posix_get_last_error(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfInt64;
      rv.m_data.num = (long long)fh_posix_get_last_error();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("posix_get_last_error", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::fni_posix_getcwd()
_ZN4HPHP16fni_posix_getcwdEv

(return value) => rax
_rv => rdi
*/

Value* fh_posix_getcwd(Value* _rv) asm("_ZN4HPHP16fni_posix_getcwdEv");

TypedValue* fg_posix_getcwd(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfString;
      fh_posix_getcwd((Value*)(&(rv)));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("posix_getcwd", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long long HPHP::fni_posix_getegid()
_ZN4HPHP17fni_posix_getegidEv

(return value) => rax
*/

long long fh_posix_getegid() asm("_ZN4HPHP17fni_posix_getegidEv");

TypedValue* fg_posix_getegid(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfInt64;
      rv.m_data.num = (long long)fh_posix_getegid();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("posix_getegid", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long long HPHP::fni_posix_geteuid()
_ZN4HPHP17fni_posix_geteuidEv

(return value) => rax
*/

long long fh_posix_geteuid() asm("_ZN4HPHP17fni_posix_geteuidEv");

TypedValue* fg_posix_geteuid(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfInt64;
      rv.m_data.num = (long long)fh_posix_geteuid();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("posix_geteuid", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long long HPHP::fni_posix_getgid()
_ZN4HPHP16fni_posix_getgidEv

(return value) => rax
*/

long long fh_posix_getgid() asm("_ZN4HPHP16fni_posix_getgidEv");

TypedValue* fg_posix_getgid(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfInt64;
      rv.m_data.num = (long long)fh_posix_getgid();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("posix_getgid", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::fni_posix_getlogin()
_ZN4HPHP18fni_posix_getloginEv

(return value) => rax
_rv => rdi
*/

TypedValue* fh_posix_getlogin(TypedValue* _rv) asm("_ZN4HPHP18fni_posix_getloginEv");

TypedValue* fg_posix_getlogin(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      fh_posix_getlogin((&(rv)));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("posix_getlogin", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::fni_posix_getpgid(int)
_ZN4HPHP17fni_posix_getpgidEi

(return value) => rax
_rv => rdi
pid => rsi
*/

TypedValue* fh_posix_getpgid(TypedValue* _rv, int pid) asm("_ZN4HPHP17fni_posix_getpgidEi");

TypedValue * fg1_posix_getpgid(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_posix_getpgid(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  fh_posix_getpgid((rv), (int)(args[-0].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_posix_getpgid(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfInt64) {
        fh_posix_getpgid((&(rv)), (int)(args[-0].m_data.num));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_posix_getpgid(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("posix_getpgid", count, 1, 1, 1);
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
long long HPHP::fni_posix_getpgrp()
_ZN4HPHP17fni_posix_getpgrpEv

(return value) => rax
*/

long long fh_posix_getpgrp() asm("_ZN4HPHP17fni_posix_getpgrpEv");

TypedValue* fg_posix_getpgrp(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfInt64;
      rv.m_data.num = (long long)fh_posix_getpgrp();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("posix_getpgrp", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long long HPHP::fni_posix_getpid()
_ZN4HPHP16fni_posix_getpidEv

(return value) => rax
*/

long long fh_posix_getpid() asm("_ZN4HPHP16fni_posix_getpidEv");

TypedValue* fg_posix_getpid(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfInt64;
      rv.m_data.num = (long long)fh_posix_getpid();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("posix_getpid", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long long HPHP::fni_posix_getppid()
_ZN4HPHP17fni_posix_getppidEv

(return value) => rax
*/

long long fh_posix_getppid() asm("_ZN4HPHP17fni_posix_getppidEv");

TypedValue* fg_posix_getppid(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfInt64;
      rv.m_data.num = (long long)fh_posix_getppid();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("posix_getppid", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::fni_posix_getsid(int)
_ZN4HPHP16fni_posix_getsidEi

(return value) => rax
_rv => rdi
pid => rsi
*/

TypedValue* fh_posix_getsid(TypedValue* _rv, int pid) asm("_ZN4HPHP16fni_posix_getsidEi");

TypedValue * fg1_posix_getsid(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_posix_getsid(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  fh_posix_getsid((rv), (int)(args[-0].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_posix_getsid(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfInt64) {
        fh_posix_getsid((&(rv)), (int)(args[-0].m_data.num));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_posix_getsid(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("posix_getsid", count, 1, 1, 1);
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
long long HPHP::fni_posix_getuid()
_ZN4HPHP16fni_posix_getuidEv

(return value) => rax
*/

long long fh_posix_getuid() asm("_ZN4HPHP16fni_posix_getuidEv");

TypedValue* fg_posix_getuid(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfInt64;
      rv.m_data.num = (long long)fh_posix_getuid();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("posix_getuid", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::fni_posix_initgroups(HPHP::String const&, int)
_ZN4HPHP20fni_posix_initgroupsERKNS_6StringEi

(return value) => rax
name => rdi
base_group_id => rsi
*/

bool fh_posix_initgroups(Value* name, int base_group_id) asm("_ZN4HPHP20fni_posix_initgroupsERKNS_6StringEi");

TypedValue * fg1_posix_initgroups(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_posix_initgroups(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (fh_posix_initgroups((Value*)(args-0), (int)(args[-1].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_posix_initgroups(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_posix_initgroups((Value*)(args-0), (int)(args[-1].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_posix_initgroups(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("posix_initgroups", count, 2, 2, 1);
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
bool HPHP::fni_posix_kill(int, int)
_ZN4HPHP14fni_posix_killEii

(return value) => rax
pid => rdi
sig => rsi
*/

bool fh_posix_kill(int pid, int sig) asm("_ZN4HPHP14fni_posix_killEii");

TypedValue * fg1_posix_kill(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_posix_kill(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  rv->m_data.num = (fh_posix_kill((int)(args[-0].m_data.num), (int)(args[-1].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_posix_kill(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfInt64) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_posix_kill((int)(args[-0].m_data.num), (int)(args[-1].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_posix_kill(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("posix_kill", count, 2, 2, 1);
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
bool HPHP::fni_posix_mkfifo(HPHP::String const&, int)
_ZN4HPHP16fni_posix_mkfifoERKNS_6StringEi

(return value) => rax
pathname => rdi
mode => rsi
*/

bool fh_posix_mkfifo(Value* pathname, int mode) asm("_ZN4HPHP16fni_posix_mkfifoERKNS_6StringEi");

TypedValue * fg1_posix_mkfifo(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_posix_mkfifo(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (fh_posix_mkfifo((Value*)(args-0), (int)(args[-1].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_posix_mkfifo(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_posix_mkfifo((Value*)(args-0), (int)(args[-1].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_posix_mkfifo(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("posix_mkfifo", count, 2, 2, 1);
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
bool HPHP::fni_posix_setegid(int)
_ZN4HPHP17fni_posix_setegidEi

(return value) => rax
gid => rdi
*/

bool fh_posix_setegid(int gid) asm("_ZN4HPHP17fni_posix_setegidEi");

TypedValue * fg1_posix_setegid(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_posix_setegid(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToInt64InPlace(args-0);
  rv->m_data.num = (fh_posix_setegid((int)(args[-0].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_posix_setegid(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfInt64) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_posix_setegid((int)(args[-0].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_posix_setegid(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("posix_setegid", count, 1, 1, 1);
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
bool HPHP::fni_posix_seteuid(int)
_ZN4HPHP17fni_posix_seteuidEi

(return value) => rax
uid => rdi
*/

bool fh_posix_seteuid(int uid) asm("_ZN4HPHP17fni_posix_seteuidEi");

TypedValue * fg1_posix_seteuid(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_posix_seteuid(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToInt64InPlace(args-0);
  rv->m_data.num = (fh_posix_seteuid((int)(args[-0].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_posix_seteuid(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfInt64) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_posix_seteuid((int)(args[-0].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_posix_seteuid(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("posix_seteuid", count, 1, 1, 1);
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
bool HPHP::fni_posix_setgid(int)
_ZN4HPHP16fni_posix_setgidEi

(return value) => rax
gid => rdi
*/

bool fh_posix_setgid(int gid) asm("_ZN4HPHP16fni_posix_setgidEi");

TypedValue * fg1_posix_setgid(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_posix_setgid(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToInt64InPlace(args-0);
  rv->m_data.num = (fh_posix_setgid((int)(args[-0].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_posix_setgid(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfInt64) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_posix_setgid((int)(args[-0].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_posix_setgid(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("posix_setgid", count, 1, 1, 1);
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
bool HPHP::fni_posix_setpgid(int, int)
_ZN4HPHP17fni_posix_setpgidEii

(return value) => rax
pid => rdi
pgid => rsi
*/

bool fh_posix_setpgid(int pid, int pgid) asm("_ZN4HPHP17fni_posix_setpgidEii");

TypedValue * fg1_posix_setpgid(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_posix_setpgid(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  rv->m_data.num = (fh_posix_setpgid((int)(args[-0].m_data.num), (int)(args[-1].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_posix_setpgid(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfInt64) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_posix_setpgid((int)(args[-0].m_data.num), (int)(args[-1].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_posix_setpgid(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("posix_setpgid", count, 2, 2, 1);
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
long long HPHP::fni_posix_setsid()
_ZN4HPHP16fni_posix_setsidEv

(return value) => rax
*/

long long fh_posix_setsid() asm("_ZN4HPHP16fni_posix_setsidEv");

TypedValue* fg_posix_setsid(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfInt64;
      rv.m_data.num = (long long)fh_posix_setsid();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("posix_setsid", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::fni_posix_setuid(int)
_ZN4HPHP16fni_posix_setuidEi

(return value) => rax
uid => rdi
*/

bool fh_posix_setuid(int uid) asm("_ZN4HPHP16fni_posix_setuidEi");

TypedValue * fg1_posix_setuid(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_posix_setuid(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToInt64InPlace(args-0);
  rv->m_data.num = (fh_posix_setuid((int)(args[-0].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_posix_setuid(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfInt64) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_posix_setuid((int)(args[-0].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_posix_setuid(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("posix_setuid", count, 1, 1, 1);
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
HPHP::String HPHP::fni_posix_strerror(int)
_ZN4HPHP18fni_posix_strerrorEi

(return value) => rax
_rv => rdi
errnum => rsi
*/

Value* fh_posix_strerror(Value* _rv, int errnum) asm("_ZN4HPHP18fni_posix_strerrorEi");

TypedValue * fg1_posix_strerror(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_posix_strerror(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToInt64InPlace(args-0);
  fh_posix_strerror((Value*)(rv), (int)(args[-0].m_data.num));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_posix_strerror(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfInt64) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_posix_strerror((Value*)(&(rv)), (int)(args[-0].m_data.num));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_posix_strerror(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("posix_strerror", count, 1, 1, 1);
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
long long HPHP::fni_pcntl_alarm(int)
_ZN4HPHP15fni_pcntl_alarmEi

(return value) => rax
seconds => rdi
*/

long long fh_pcntl_alarm(int seconds) asm("_ZN4HPHP15fni_pcntl_alarmEi");

TypedValue * fg1_pcntl_alarm(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_pcntl_alarm(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfInt64;
  tvCastToInt64InPlace(args-0);
  rv->m_data.num = (long long)fh_pcntl_alarm((int)(args[-0].m_data.num));
  return rv;
}

TypedValue* fg_pcntl_alarm(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfInt64) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (long long)fh_pcntl_alarm((int)(args[-0].m_data.num));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pcntl_alarm(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pcntl_alarm", count, 1, 1, 1);
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
long long HPHP::fni_pcntl_wexitstatus(int)
_ZN4HPHP21fni_pcntl_wexitstatusEi

(return value) => rax
status => rdi
*/

long long fh_pcntl_wexitstatus(int status) asm("_ZN4HPHP21fni_pcntl_wexitstatusEi");

TypedValue * fg1_pcntl_wexitstatus(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_pcntl_wexitstatus(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfInt64;
  tvCastToInt64InPlace(args-0);
  rv->m_data.num = (long long)fh_pcntl_wexitstatus((int)(args[-0].m_data.num));
  return rv;
}

TypedValue* fg_pcntl_wexitstatus(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfInt64) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (long long)fh_pcntl_wexitstatus((int)(args[-0].m_data.num));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pcntl_wexitstatus(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pcntl_wexitstatus", count, 1, 1, 1);
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
bool HPHP::fni_pcntl_wifexited(int)
_ZN4HPHP19fni_pcntl_wifexitedEi

(return value) => rax
status => rdi
*/

bool fh_pcntl_wifexited(int status) asm("_ZN4HPHP19fni_pcntl_wifexitedEi");

TypedValue * fg1_pcntl_wifexited(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_pcntl_wifexited(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToInt64InPlace(args-0);
  rv->m_data.num = (fh_pcntl_wifexited((int)(args[-0].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_pcntl_wifexited(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfInt64) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_pcntl_wifexited((int)(args[-0].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pcntl_wifexited(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pcntl_wifexited", count, 1, 1, 1);
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
bool HPHP::fni_pcntl_wifsignaled(int)
_ZN4HPHP21fni_pcntl_wifsignaledEi

(return value) => rax
status => rdi
*/

bool fh_pcntl_wifsignaled(int status) asm("_ZN4HPHP21fni_pcntl_wifsignaledEi");

TypedValue * fg1_pcntl_wifsignaled(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_pcntl_wifsignaled(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToInt64InPlace(args-0);
  rv->m_data.num = (fh_pcntl_wifsignaled((int)(args[-0].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_pcntl_wifsignaled(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfInt64) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_pcntl_wifsignaled((int)(args[-0].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pcntl_wifsignaled(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pcntl_wifsignaled", count, 1, 1, 1);
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
bool HPHP::fni_pcntl_wifstopped(int)
_ZN4HPHP20fni_pcntl_wifstoppedEi

(return value) => rax
status => rdi
*/

bool fh_pcntl_wifstopped(int status) asm("_ZN4HPHP20fni_pcntl_wifstoppedEi");

TypedValue * fg1_pcntl_wifstopped(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_pcntl_wifstopped(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToInt64InPlace(args-0);
  rv->m_data.num = (fh_pcntl_wifstopped((int)(args[-0].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_pcntl_wifstopped(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfInt64) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_pcntl_wifstopped((int)(args[-0].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pcntl_wifstopped(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pcntl_wifstopped", count, 1, 1, 1);
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
long long HPHP::fni_pcntl_wstopsig(int)
_ZN4HPHP18fni_pcntl_wstopsigEi

(return value) => rax
status => rdi
*/

long long fh_pcntl_wstopsig(int status) asm("_ZN4HPHP18fni_pcntl_wstopsigEi");

TypedValue * fg1_pcntl_wstopsig(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_pcntl_wstopsig(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfInt64;
  tvCastToInt64InPlace(args-0);
  rv->m_data.num = (long long)fh_pcntl_wstopsig((int)(args[-0].m_data.num));
  return rv;
}

TypedValue* fg_pcntl_wstopsig(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfInt64) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (long long)fh_pcntl_wstopsig((int)(args[-0].m_data.num));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pcntl_wstopsig(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pcntl_wstopsig", count, 1, 1, 1);
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
long long HPHP::fni_pcntl_wtermsig(int)
_ZN4HPHP18fni_pcntl_wtermsigEi

(return value) => rax
status => rdi
*/

long long fh_pcntl_wtermsig(int status) asm("_ZN4HPHP18fni_pcntl_wtermsigEi");

TypedValue * fg1_pcntl_wtermsig(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_pcntl_wtermsig(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfInt64;
  tvCastToInt64InPlace(args-0);
  rv->m_data.num = (long long)fh_pcntl_wtermsig((int)(args[-0].m_data.num));
  return rv;
}

TypedValue* fg_pcntl_wtermsig(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfInt64) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (long long)fh_pcntl_wtermsig((int)(args[-0].m_data.num));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pcntl_wtermsig(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pcntl_wtermsig", count, 1, 1, 1);
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
long long HPHP::fni_hphp_object_pointer(HPHP::Object const&)
_ZN4HPHP23fni_hphp_object_pointerERKNS_6ObjectE

(return value) => rax
obj => rdi
*/

long long fh_hphp_object_pointer(Value* obj) asm("_ZN4HPHP23fni_hphp_object_pointerERKNS_6ObjectE");

TypedValue * fg1_hphp_object_pointer(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_object_pointer(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfInt64;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (long long)fh_hphp_object_pointer((Value*)(args-0));
  return rv;
}

TypedValue* fg_hphp_object_pointer(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (long long)fh_hphp_object_pointer((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_object_pointer(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_object_pointer", count, 1, 1, 1);
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
HPHP::Object HPHP::fni_stream_context_create(HPHP::Array const&, HPHP::Array const&)
_ZN4HPHP25fni_stream_context_createERKNS_5ArrayES2_

(return value) => rax
_rv => rdi
options => rsi
params => rdx
*/

Value* fh_stream_context_create(Value* _rv, Value* options, Value* params) asm("_ZN4HPHP25fni_stream_context_createERKNS_5ArrayES2_");

TypedValue * fg1_stream_context_create(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_stream_context_create(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfObject;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfArray) {
      tvCastToArrayInPlace(args-1);
    }
  case 1:
    if ((args-0)->m_type != KindOfArray) {
      tvCastToArrayInPlace(args-0);
    }
  case 0:
    break;
  }
  fh_stream_context_create((Value*)(rv), (count > 0) ? (Value*)(args-0) : (Value*)(&null_array), (count > 1) ? (Value*)(args-1) : (Value*)(&null_array));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_stream_context_create(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfArray) && (count <= 0 || (args-0)->m_type == KindOfArray)) {
        rv._count = 0;
        rv.m_type = KindOfObject;
        fh_stream_context_create((Value*)(&(rv)), (count > 0) ? (Value*)(args-0) : (Value*)(&null_array), (count > 1) ? (Value*)(args-1) : (Value*)(&null_array));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stream_context_create(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("stream_context_create", 2, 1);
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
HPHP::Object HPHP::fni_stream_context_get_default(HPHP::Array const&)
_ZN4HPHP30fni_stream_context_get_defaultERKNS_5ArrayE

(return value) => rax
_rv => rdi
options => rsi
*/

Value* fh_stream_context_get_default(Value* _rv, Value* options) asm("_ZN4HPHP30fni_stream_context_get_defaultERKNS_5ArrayE");

TypedValue * fg1_stream_context_get_default(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_stream_context_get_default(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfObject;
  tvCastToArrayInPlace(args-0);
  fh_stream_context_get_default((Value*)(rv), (count > 0) ? (Value*)(args-0) : (Value*)(&null_array));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_stream_context_get_default(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      if ((count <= 0 || (args-0)->m_type == KindOfArray)) {
        rv._count = 0;
        rv.m_type = KindOfObject;
        fh_stream_context_get_default((Value*)(&(rv)), (count > 0) ? (Value*)(args-0) : (Value*)(&null_array));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stream_context_get_default(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("stream_context_get_default", 1, 1);
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
HPHP::Variant HPHP::fni_stream_context_get_options(HPHP::Object const&)
_ZN4HPHP30fni_stream_context_get_optionsERKNS_6ObjectE

(return value) => rax
_rv => rdi
stream_or_context => rsi
*/

TypedValue* fh_stream_context_get_options(TypedValue* _rv, Value* stream_or_context) asm("_ZN4HPHP30fni_stream_context_get_optionsERKNS_6ObjectE");

TypedValue * fg1_stream_context_get_options(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_stream_context_get_options(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  fh_stream_context_get_options((rv), (Value*)(args-0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_stream_context_get_options(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        fh_stream_context_get_options((&(rv)), (Value*)(args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stream_context_get_options(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("stream_context_get_options", count, 1, 1, 1);
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
bool HPHP::fni_stream_context_set_option(HPHP::Object const&, HPHP::Variant const&, HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP29fni_stream_context_set_optionERKNS_6ObjectERKNS_7VariantERKNS_6StringES5_

(return value) => rax
stream_or_context => rdi
wrapper => rsi
option => rdx
value => rcx
*/

bool fh_stream_context_set_option(Value* stream_or_context, TypedValue* wrapper, Value* option, TypedValue* value) asm("_ZN4HPHP29fni_stream_context_set_optionERKNS_6ObjectERKNS_7VariantERKNS_6StringES5_");

TypedValue * fg1_stream_context_set_option(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_stream_context_set_option(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 4
  case 3:
    if (!IS_STRING_TYPE((args-2)->m_type)) {
      tvCastToStringInPlace(args-2);
    }
  case 2:
    break;
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_stream_context_set_option((Value*)(args-0), (args-1), (count > 2) ? (Value*)(args-2) : (Value*)(&null_string), (count > 3) ? (args-3) : (TypedValue*)(&null_variant))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_stream_context_set_option(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 4LL) {
      if ((count <= 2 || IS_STRING_TYPE((args-2)->m_type)) && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_stream_context_set_option((Value*)(args-0), (args-1), (count > 2) ? (Value*)(args-2) : (Value*)(&null_string), (count > 3) ? (args-3) : (TypedValue*)(&null_variant))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stream_context_set_option(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("stream_context_set_option", count, 2, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::fni_stream_context_set_param(HPHP::Object const&, HPHP::Array const&)
_ZN4HPHP28fni_stream_context_set_paramERKNS_6ObjectERKNS_5ArrayE

(return value) => rax
stream_or_context => rdi
params => rsi
*/

bool fh_stream_context_set_param(Value* stream_or_context, Value* params) asm("_ZN4HPHP28fni_stream_context_set_paramERKNS_6ObjectERKNS_5ArrayE");

TypedValue * fg1_stream_context_set_param(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_stream_context_set_param(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfArray) {
    tvCastToArrayInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_stream_context_set_param((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_stream_context_set_param(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfArray && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_stream_context_set_param((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stream_context_set_param(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("stream_context_set_param", count, 2, 2, 1);
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
bool HPHP::fni_stream_encoding(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP19fni_stream_encodingERKNS_6ObjectERKNS_6StringE

(return value) => rax
stream => rdi
encoding => rsi
*/

bool fh_stream_encoding(Value* stream, Value* encoding) asm("_ZN4HPHP19fni_stream_encodingERKNS_6ObjectERKNS_6StringE");

TypedValue * fg1_stream_encoding(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_stream_encoding(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 2
    if (!IS_STRING_TYPE((args-1)->m_type)) {
      tvCastToStringInPlace(args-1);
    }
  case 1:
    break;
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_stream_encoding((Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_string))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_stream_encoding(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_stream_encoding((Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_string))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stream_encoding(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("stream_encoding", count, 1, 2, 1);
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
void HPHP::fni_stream_bucket_append(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP24fni_stream_bucket_appendERKNS_6ObjectES2_

brigade => rdi
bucket => rsi
*/

void fh_stream_bucket_append(Value* brigade, Value* bucket) asm("_ZN4HPHP24fni_stream_bucket_appendERKNS_6ObjectES2_");

TypedValue * fg1_stream_bucket_append(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_stream_bucket_append(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_stream_bucket_append((Value*)(args-0), (Value*)(args-1));
  return rv;
}

TypedValue* fg_stream_bucket_append(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        fh_stream_bucket_append((Value*)(args-0), (Value*)(args-1));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stream_bucket_append(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("stream_bucket_append", count, 2, 2, 1);
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
void HPHP::fni_stream_bucket_prepend(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP25fni_stream_bucket_prependERKNS_6ObjectES2_

brigade => rdi
bucket => rsi
*/

void fh_stream_bucket_prepend(Value* brigade, Value* bucket) asm("_ZN4HPHP25fni_stream_bucket_prependERKNS_6ObjectES2_");

TypedValue * fg1_stream_bucket_prepend(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_stream_bucket_prepend(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_stream_bucket_prepend((Value*)(args-0), (Value*)(args-1));
  return rv;
}

TypedValue* fg_stream_bucket_prepend(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        fh_stream_bucket_prepend((Value*)(args-0), (Value*)(args-1));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stream_bucket_prepend(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("stream_bucket_prepend", count, 2, 2, 1);
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
HPHP::Object HPHP::fni_stream_bucket_make_writeable(HPHP::Object const&)
_ZN4HPHP32fni_stream_bucket_make_writeableERKNS_6ObjectE

(return value) => rax
_rv => rdi
brigade => rsi
*/

Value* fh_stream_bucket_make_writeable(Value* _rv, Value* brigade) asm("_ZN4HPHP32fni_stream_bucket_make_writeableERKNS_6ObjectE");

TypedValue * fg1_stream_bucket_make_writeable(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_stream_bucket_make_writeable(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfObject;
  tvCastToObjectInPlace(args-0);
  fh_stream_bucket_make_writeable((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_stream_bucket_make_writeable(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfObject;
        fh_stream_bucket_make_writeable((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stream_bucket_make_writeable(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("stream_bucket_make_writeable", count, 1, 1, 1);
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
HPHP::Object HPHP::fni_stream_bucket_new(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP21fni_stream_bucket_newERKNS_6ObjectERKNS_6StringE

(return value) => rax
_rv => rdi
stream => rsi
buffer => rdx
*/

Value* fh_stream_bucket_new(Value* _rv, Value* stream, Value* buffer) asm("_ZN4HPHP21fni_stream_bucket_newERKNS_6ObjectERKNS_6StringE");

TypedValue * fg1_stream_bucket_new(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_stream_bucket_new(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfObject;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_stream_bucket_new((Value*)(rv), (Value*)(args-0), (Value*)(args-1));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_stream_bucket_new(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfObject;
        fh_stream_bucket_new((Value*)(&(rv)), (Value*)(args-0), (Value*)(args-1));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stream_bucket_new(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("stream_bucket_new", count, 2, 2, 1);
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
bool HPHP::fni_stream_filter_register(HPHP::String const&, HPHP::String const&)
_ZN4HPHP26fni_stream_filter_registerERKNS_6StringES2_

(return value) => rax
filtername => rdi
classname => rsi
*/

bool fh_stream_filter_register(Value* filtername, Value* classname) asm("_ZN4HPHP26fni_stream_filter_registerERKNS_6StringES2_");

TypedValue * fg1_stream_filter_register(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_stream_filter_register(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (fh_stream_filter_register((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_stream_filter_register(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_stream_filter_register((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stream_filter_register(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("stream_filter_register", count, 2, 2, 1);
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
bool HPHP::fni_stream_filter_remove(HPHP::Object const&)
_ZN4HPHP24fni_stream_filter_removeERKNS_6ObjectE

(return value) => rax
stream_filter => rdi
*/

bool fh_stream_filter_remove(Value* stream_filter) asm("_ZN4HPHP24fni_stream_filter_removeERKNS_6ObjectE");

TypedValue * fg1_stream_filter_remove(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_stream_filter_remove(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_stream_filter_remove((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_stream_filter_remove(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_stream_filter_remove((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stream_filter_remove(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("stream_filter_remove", count, 1, 1, 1);
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
HPHP::Object HPHP::fni_stream_filter_append(HPHP::Object const&, HPHP::String const&, int, HPHP::Variant const&)
_ZN4HPHP24fni_stream_filter_appendERKNS_6ObjectERKNS_6StringEiRKNS_7VariantE

(return value) => rax
_rv => rdi
stream => rsi
filtername => rdx
read_write => rcx
params => r8
*/

Value* fh_stream_filter_append(Value* _rv, Value* stream, Value* filtername, int read_write, TypedValue* params) asm("_ZN4HPHP24fni_stream_filter_appendERKNS_6ObjectERKNS_6StringEiRKNS_7VariantE");

TypedValue * fg1_stream_filter_append(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_stream_filter_append(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfObject;
  switch (count) {
  default: // count >= 4
  case 3:
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_stream_filter_append((Value*)(rv), (Value*)(args-0), (Value*)(args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (args-3) : (TypedValue*)(&null_variant));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_stream_filter_append(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 4LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfInt64) && IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfObject;
        fh_stream_filter_append((Value*)(&(rv)), (Value*)(args-0), (Value*)(args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (args-3) : (TypedValue*)(&null_variant));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stream_filter_append(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("stream_filter_append", count, 2, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Object HPHP::fni_stream_filter_prepend(HPHP::Object const&, HPHP::String const&, int, HPHP::Variant const&)
_ZN4HPHP25fni_stream_filter_prependERKNS_6ObjectERKNS_6StringEiRKNS_7VariantE

(return value) => rax
_rv => rdi
stream => rsi
filtername => rdx
read_write => rcx
params => r8
*/

Value* fh_stream_filter_prepend(Value* _rv, Value* stream, Value* filtername, int read_write, TypedValue* params) asm("_ZN4HPHP25fni_stream_filter_prependERKNS_6ObjectERKNS_6StringEiRKNS_7VariantE");

TypedValue * fg1_stream_filter_prepend(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_stream_filter_prepend(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfObject;
  switch (count) {
  default: // count >= 4
  case 3:
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_stream_filter_prepend((Value*)(rv), (Value*)(args-0), (Value*)(args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (args-3) : (TypedValue*)(&null_variant));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_stream_filter_prepend(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 4LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfInt64) && IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfObject;
        fh_stream_filter_prepend((Value*)(&(rv)), (Value*)(args-0), (Value*)(args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (args-3) : (TypedValue*)(&null_variant));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stream_filter_prepend(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("stream_filter_prepend", count, 2, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::fni_stream_get_filters()
_ZN4HPHP22fni_stream_get_filtersEv

(return value) => rax
_rv => rdi
*/

Value* fh_stream_get_filters(Value* _rv) asm("_ZN4HPHP22fni_stream_get_filtersEv");

TypedValue* fg_stream_get_filters(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfArray;
      fh_stream_get_filters((Value*)(&(rv)));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("stream_get_filters", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::fni_stream_get_meta_data(HPHP::Object const&)
_ZN4HPHP24fni_stream_get_meta_dataERKNS_6ObjectE

(return value) => rax
_rv => rdi
stream => rsi
*/

TypedValue* fh_stream_get_meta_data(TypedValue* _rv, Value* stream) asm("_ZN4HPHP24fni_stream_get_meta_dataERKNS_6ObjectE");

TypedValue * fg1_stream_get_meta_data(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_stream_get_meta_data(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  fh_stream_get_meta_data((rv), (Value*)(args-0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_stream_get_meta_data(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        fh_stream_get_meta_data((&(rv)), (Value*)(args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stream_get_meta_data(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("stream_get_meta_data", count, 1, 1, 1);
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
HPHP::Array HPHP::fni_stream_get_transports()
_ZN4HPHP25fni_stream_get_transportsEv

(return value) => rax
_rv => rdi
*/

Value* fh_stream_get_transports(Value* _rv) asm("_ZN4HPHP25fni_stream_get_transportsEv");

TypedValue* fg_stream_get_transports(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfArray;
      fh_stream_get_transports((Value*)(&(rv)));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("stream_get_transports", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::fni_stream_get_wrappers()
_ZN4HPHP23fni_stream_get_wrappersEv

(return value) => rax
_rv => rdi
*/

Value* fh_stream_get_wrappers(Value* _rv) asm("_ZN4HPHP23fni_stream_get_wrappersEv");

TypedValue* fg_stream_get_wrappers(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfArray;
      fh_stream_get_wrappers((Value*)(&(rv)));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("stream_get_wrappers", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::fni_stream_register_wrapper(HPHP::String const&, HPHP::String const&)
_ZN4HPHP27fni_stream_register_wrapperERKNS_6StringES2_

(return value) => rax
protocol => rdi
classname => rsi
*/

bool fh_stream_register_wrapper(Value* protocol, Value* classname) asm("_ZN4HPHP27fni_stream_register_wrapperERKNS_6StringES2_");

TypedValue * fg1_stream_register_wrapper(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_stream_register_wrapper(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (fh_stream_register_wrapper((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_stream_register_wrapper(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_stream_register_wrapper((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stream_register_wrapper(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("stream_register_wrapper", count, 2, 2, 1);
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
bool HPHP::fni_stream_wrapper_register(HPHP::String const&, HPHP::String const&)
_ZN4HPHP27fni_stream_wrapper_registerERKNS_6StringES2_

(return value) => rax
protocol => rdi
classname => rsi
*/

bool fh_stream_wrapper_register(Value* protocol, Value* classname) asm("_ZN4HPHP27fni_stream_wrapper_registerERKNS_6StringES2_");

TypedValue * fg1_stream_wrapper_register(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_stream_wrapper_register(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (fh_stream_wrapper_register((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_stream_wrapper_register(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_stream_wrapper_register((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stream_wrapper_register(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("stream_wrapper_register", count, 2, 2, 1);
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
bool HPHP::fni_stream_wrapper_restore(HPHP::String const&)
_ZN4HPHP26fni_stream_wrapper_restoreERKNS_6StringE

(return value) => rax
protocol => rdi
*/

bool fh_stream_wrapper_restore(Value* protocol) asm("_ZN4HPHP26fni_stream_wrapper_restoreERKNS_6StringE");

TypedValue * fg1_stream_wrapper_restore(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_stream_wrapper_restore(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToStringInPlace(args-0);
  rv->m_data.num = (fh_stream_wrapper_restore((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_stream_wrapper_restore(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_stream_wrapper_restore((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stream_wrapper_restore(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("stream_wrapper_restore", count, 1, 1, 1);
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
bool HPHP::fni_stream_wrapper_unregister(HPHP::String const&)
_ZN4HPHP29fni_stream_wrapper_unregisterERKNS_6StringE

(return value) => rax
protocol => rdi
*/

bool fh_stream_wrapper_unregister(Value* protocol) asm("_ZN4HPHP29fni_stream_wrapper_unregisterERKNS_6StringE");

TypedValue * fg1_stream_wrapper_unregister(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_stream_wrapper_unregister(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToStringInPlace(args-0);
  rv->m_data.num = (fh_stream_wrapper_unregister((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_stream_wrapper_unregister(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_stream_wrapper_unregister((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stream_wrapper_unregister(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("stream_wrapper_unregister", count, 1, 1, 1);
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
HPHP::String HPHP::fni_stream_resolve_include_path(HPHP::String const&, HPHP::Object const&)
_ZN4HPHP31fni_stream_resolve_include_pathERKNS_6StringERKNS_6ObjectE

(return value) => rax
_rv => rdi
filename => rsi
context => rdx
*/

Value* fh_stream_resolve_include_path(Value* _rv, Value* filename, Value* context) asm("_ZN4HPHP31fni_stream_resolve_include_pathERKNS_6StringERKNS_6ObjectE");

TypedValue * fg1_stream_resolve_include_path(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_stream_resolve_include_path(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfObject) {
      tvCastToObjectInPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_stream_resolve_include_path((Value*)(rv), (Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_object));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_stream_resolve_include_path(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfObject) && IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_stream_resolve_include_path((Value*)(&(rv)), (Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_object));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stream_resolve_include_path(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("stream_resolve_include_path", count, 1, 2, 1);
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
long long HPHP::fni_set_file_buffer(HPHP::Object const&, int)
_ZN4HPHP19fni_set_file_bufferERKNS_6ObjectEi

(return value) => rax
stream => rdi
buffer => rsi
*/

long long fh_set_file_buffer(Value* stream, int buffer) asm("_ZN4HPHP19fni_set_file_bufferERKNS_6ObjectEi");

TypedValue * fg1_set_file_buffer(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_set_file_buffer(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfInt64;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (long long)fh_set_file_buffer((Value*)(args-0), (int)(args[-1].m_data.num));
  return rv;
}

TypedValue* fg_set_file_buffer(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (long long)fh_set_file_buffer((Value*)(args-0), (int)(args[-1].m_data.num));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_set_file_buffer(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("set_file_buffer", count, 2, 2, 1);
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
HPHP::Variant HPHP::fni_stream_socket_enable_crypto(HPHP::Object const&, bool, int, HPHP::Object const&)
_ZN4HPHP31fni_stream_socket_enable_cryptoERKNS_6ObjectEbiS2_

(return value) => rax
_rv => rdi
stream => rsi
enable => rdx
crypto_type => rcx
session_stream => r8
*/

TypedValue* fh_stream_socket_enable_crypto(TypedValue* _rv, Value* stream, bool enable, int crypto_type, Value* session_stream) asm("_ZN4HPHP31fni_stream_socket_enable_cryptoERKNS_6ObjectEbiS2_");

TypedValue * fg1_stream_socket_enable_crypto(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_stream_socket_enable_crypto(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfObject) {
      tvCastToObjectInPlace(args-3);
    }
  case 3:
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    break;
  }
  if ((args-1)->m_type != KindOfBoolean) {
    tvCastToBooleanInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_stream_socket_enable_crypto((rv), (Value*)(args-0), (bool)(args[-1].m_data.num), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (Value*)(args-3) : (Value*)(&null_object));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_stream_socket_enable_crypto(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 4LL) {
      if ((count <= 3 || (args-3)->m_type == KindOfObject) && (count <= 2 || (args-2)->m_type == KindOfInt64) && (args-1)->m_type == KindOfBoolean && (args-0)->m_type == KindOfObject) {
        fh_stream_socket_enable_crypto((&(rv)), (Value*)(args-0), (bool)(args[-1].m_data.num), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (Value*)(args-3) : (Value*)(&null_object));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stream_socket_enable_crypto(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("stream_socket_enable_crypto", count, 2, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::fni_addcslashes(HPHP::String const&, HPHP::String const&)
_ZN4HPHP15fni_addcslashesERKNS_6StringES2_

(return value) => rax
_rv => rdi
str => rsi
charlist => rdx
*/

Value* fh_addcslashes(Value* _rv, Value* str, Value* charlist) asm("_ZN4HPHP15fni_addcslashesERKNS_6StringES2_");

TypedValue * fg1_addcslashes(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_addcslashes(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_addcslashes((Value*)(rv), (Value*)(args-0), (Value*)(args-1));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_addcslashes(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_addcslashes((Value*)(&(rv)), (Value*)(args-0), (Value*)(args-1));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_addcslashes(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("addcslashes", count, 2, 2, 1);
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
HPHP::String HPHP::fni_stripcslashes(HPHP::String const&)
_ZN4HPHP17fni_stripcslashesERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

Value* fh_stripcslashes(Value* _rv, Value* str) asm("_ZN4HPHP17fni_stripcslashesERKNS_6StringE");

TypedValue * fg1_stripcslashes(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_stripcslashes(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToStringInPlace(args-0);
  fh_stripcslashes((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_stripcslashes(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_stripcslashes((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stripcslashes(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("stripcslashes", count, 1, 1, 1);
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
HPHP::String HPHP::fni_addslashes(HPHP::String const&)
_ZN4HPHP14fni_addslashesERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

Value* fh_addslashes(Value* _rv, Value* str) asm("_ZN4HPHP14fni_addslashesERKNS_6StringE");

TypedValue * fg1_addslashes(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_addslashes(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToStringInPlace(args-0);
  fh_addslashes((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_addslashes(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_addslashes((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_addslashes(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("addslashes", count, 1, 1, 1);
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
HPHP::String HPHP::fni_stripslashes(HPHP::String const&)
_ZN4HPHP16fni_stripslashesERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

Value* fh_stripslashes(Value* _rv, Value* str) asm("_ZN4HPHP16fni_stripslashesERKNS_6StringE");

TypedValue * fg1_stripslashes(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_stripslashes(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToStringInPlace(args-0);
  fh_stripslashes((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_stripslashes(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_stripslashes((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stripslashes(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("stripslashes", count, 1, 1, 1);
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
HPHP::String HPHP::fni_bin2hex(HPHP::String const&)
_ZN4HPHP11fni_bin2hexERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

Value* fh_bin2hex(Value* _rv, Value* str) asm("_ZN4HPHP11fni_bin2hexERKNS_6StringE");

TypedValue * fg1_bin2hex(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_bin2hex(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToStringInPlace(args-0);
  fh_bin2hex((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_bin2hex(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_bin2hex((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_bin2hex(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("bin2hex", count, 1, 1, 1);
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
HPHP::Variant HPHP::fni_hex2bin(HPHP::String const&)
_ZN4HPHP11fni_hex2binERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

TypedValue* fh_hex2bin(TypedValue* _rv, Value* str) asm("_ZN4HPHP11fni_hex2binERKNS_6StringE");

TypedValue * fg1_hex2bin(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_hex2bin(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_hex2bin((rv), (Value*)(args-0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_hex2bin(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        fh_hex2bin((&(rv)), (Value*)(args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hex2bin(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hex2bin", count, 1, 1, 1);
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
HPHP::String HPHP::fni_nl2br(HPHP::String const&)
_ZN4HPHP9fni_nl2brERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

Value* fh_nl2br(Value* _rv, Value* str) asm("_ZN4HPHP9fni_nl2brERKNS_6StringE");

TypedValue * fg1_nl2br(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_nl2br(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToStringInPlace(args-0);
  fh_nl2br((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_nl2br(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_nl2br((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_nl2br(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("nl2br", count, 1, 1, 1);
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
HPHP::String HPHP::fni_quotemeta(HPHP::String const&)
_ZN4HPHP13fni_quotemetaERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

Value* fh_quotemeta(Value* _rv, Value* str) asm("_ZN4HPHP13fni_quotemetaERKNS_6StringE");

TypedValue * fg1_quotemeta(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_quotemeta(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToStringInPlace(args-0);
  fh_quotemeta((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_quotemeta(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_quotemeta((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_quotemeta(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("quotemeta", count, 1, 1, 1);
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
HPHP::String HPHP::fni_str_shuffle(HPHP::String const&)
_ZN4HPHP15fni_str_shuffleERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

Value* fh_str_shuffle(Value* _rv, Value* str) asm("_ZN4HPHP15fni_str_shuffleERKNS_6StringE");

TypedValue * fg1_str_shuffle(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_str_shuffle(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToStringInPlace(args-0);
  fh_str_shuffle((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_str_shuffle(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_str_shuffle((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_str_shuffle(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("str_shuffle", count, 1, 1, 1);
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
HPHP::String HPHP::fni_strrev(HPHP::String const&)
_ZN4HPHP10fni_strrevERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

Value* fh_strrev(Value* _rv, Value* str) asm("_ZN4HPHP10fni_strrevERKNS_6StringE");

TypedValue * fg1_strrev(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_strrev(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToStringInPlace(args-0);
  fh_strrev((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_strrev(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_strrev((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_strrev(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("strrev", count, 1, 1, 1);
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
HPHP::String HPHP::fni_strtolower(HPHP::String const&)
_ZN4HPHP14fni_strtolowerERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

Value* fh_strtolower(Value* _rv, Value* str) asm("_ZN4HPHP14fni_strtolowerERKNS_6StringE");

TypedValue * fg1_strtolower(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_strtolower(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToStringInPlace(args-0);
  fh_strtolower((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_strtolower(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_strtolower((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_strtolower(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("strtolower", count, 1, 1, 1);
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
HPHP::String HPHP::fni_strtoupper(HPHP::String const&)
_ZN4HPHP14fni_strtoupperERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

Value* fh_strtoupper(Value* _rv, Value* str) asm("_ZN4HPHP14fni_strtoupperERKNS_6StringE");

TypedValue * fg1_strtoupper(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_strtoupper(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToStringInPlace(args-0);
  fh_strtoupper((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_strtoupper(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_strtoupper((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_strtoupper(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("strtoupper", count, 1, 1, 1);
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
HPHP::String HPHP::fni_ucfirst(HPHP::String const&)
_ZN4HPHP11fni_ucfirstERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

Value* fh_ucfirst(Value* _rv, Value* str) asm("_ZN4HPHP11fni_ucfirstERKNS_6StringE");

TypedValue * fg1_ucfirst(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_ucfirst(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToStringInPlace(args-0);
  fh_ucfirst((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_ucfirst(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_ucfirst((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_ucfirst(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("ucfirst", count, 1, 1, 1);
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
HPHP::String HPHP::fni_ucwords(HPHP::String const&)
_ZN4HPHP11fni_ucwordsERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

Value* fh_ucwords(Value* _rv, Value* str) asm("_ZN4HPHP11fni_ucwordsERKNS_6StringE");

TypedValue * fg1_ucwords(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_ucwords(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToStringInPlace(args-0);
  fh_ucwords((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_ucwords(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_ucwords((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_ucwords(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("ucwords", count, 1, 1, 1);
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
HPHP::String HPHP::fni_strip_tags(HPHP::String const&, HPHP::String const&)
_ZN4HPHP14fni_strip_tagsERKNS_6StringES2_

(return value) => rax
_rv => rdi
str => rsi
allowable_tags => rdx
*/

Value* fh_strip_tags(Value* _rv, Value* str, Value* allowable_tags) asm("_ZN4HPHP14fni_strip_tagsERKNS_6StringES2_");

TypedValue * fg1_strip_tags(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_strip_tags(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
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
  fh_strip_tags((Value*)(rv), (Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&empty_string));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_strip_tags(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_strip_tags((Value*)(&(rv)), (Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&empty_string));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_strip_tags(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("strip_tags", count, 1, 2, 1);
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
HPHP::String HPHP::fni_trim(HPHP::String const&, HPHP::String const&)
_ZN4HPHP8fni_trimERKNS_6StringES2_

(return value) => rax
_rv => rdi
str => rsi
charlist => rdx
*/

Value* fh_trim(Value* _rv, Value* str, Value* charlist) asm("_ZN4HPHP8fni_trimERKNS_6StringES2_");

TypedValue * fg1_trim(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_trim(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
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
  fh_trim((Value*)(rv), (Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&k_HPHP_TRIM_CHARLIST));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_trim(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_trim((Value*)(&(rv)), (Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&k_HPHP_TRIM_CHARLIST));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_trim(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("trim", count, 1, 2, 1);
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
HPHP::String HPHP::fni_ltrim(HPHP::String const&, HPHP::String const&)
_ZN4HPHP9fni_ltrimERKNS_6StringES2_

(return value) => rax
_rv => rdi
str => rsi
charlist => rdx
*/

Value* fh_ltrim(Value* _rv, Value* str, Value* charlist) asm("_ZN4HPHP9fni_ltrimERKNS_6StringES2_");

TypedValue * fg1_ltrim(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_ltrim(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
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
  fh_ltrim((Value*)(rv), (Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&k_HPHP_TRIM_CHARLIST));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_ltrim(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_ltrim((Value*)(&(rv)), (Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&k_HPHP_TRIM_CHARLIST));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_ltrim(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("ltrim", count, 1, 2, 1);
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
HPHP::String HPHP::fni_rtrim(HPHP::String const&, HPHP::String const&)
_ZN4HPHP9fni_rtrimERKNS_6StringES2_

(return value) => rax
_rv => rdi
str => rsi
charlist => rdx
*/

Value* fh_rtrim(Value* _rv, Value* str, Value* charlist) asm("_ZN4HPHP9fni_rtrimERKNS_6StringES2_");

TypedValue * fg1_rtrim(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_rtrim(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
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
  fh_rtrim((Value*)(rv), (Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&k_HPHP_TRIM_CHARLIST));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_rtrim(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_rtrim((Value*)(&(rv)), (Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&k_HPHP_TRIM_CHARLIST));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_rtrim(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("rtrim", count, 1, 2, 1);
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
HPHP::String HPHP::fni_chop(HPHP::String const&, HPHP::String const&)
_ZN4HPHP8fni_chopERKNS_6StringES2_

(return value) => rax
_rv => rdi
str => rsi
charlist => rdx
*/

Value* fh_chop(Value* _rv, Value* str, Value* charlist) asm("_ZN4HPHP8fni_chopERKNS_6StringES2_");

TypedValue * fg1_chop(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_chop(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
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
  fh_chop((Value*)(rv), (Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&k_HPHP_TRIM_CHARLIST));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_chop(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_chop((Value*)(&(rv)), (Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&k_HPHP_TRIM_CHARLIST));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_chop(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("chop", count, 1, 2, 1);
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
HPHP::Variant HPHP::fni_explode(HPHP::String const&, HPHP::String const&, int)
_ZN4HPHP11fni_explodeERKNS_6StringES2_i

(return value) => rax
_rv => rdi
delimiter => rsi
str => rdx
limit => rcx
*/

TypedValue* fh_explode(TypedValue* _rv, Value* delimiter, Value* str, int limit) asm("_ZN4HPHP11fni_explodeERKNS_6StringES2_i");

TypedValue * fg1_explode(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_explode(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
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
  fh_explode((rv), (Value*)(args-0), (Value*)(args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0x7FFFFFFF));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_explode(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfInt64) && IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        fh_explode((&(rv)), (Value*)(args-0), (Value*)(args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0x7FFFFFFF));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_explode(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("explode", count, 2, 3, 1);
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
HPHP::String HPHP::fni_join(HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP8fni_joinERKNS_7VariantES2_

(return value) => rax
_rv => rdi
glue => rsi
pieces => rdx
*/

Value* fh_join(Value* _rv, TypedValue* glue, TypedValue* pieces) asm("_ZN4HPHP8fni_joinERKNS_7VariantES2_");

TypedValue* fg_join(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      rv._count = 0;
      rv.m_type = KindOfString;
      fh_join((Value*)(&(rv)), (args-0), (count > 1) ? (args-1) : (TypedValue*)(&null_variant));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 2);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("join", count, 1, 2, 1);
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
HPHP::Variant HPHP::fni_str_split(HPHP::String const&, int)
_ZN4HPHP13fni_str_splitERKNS_6StringEi

(return value) => rax
_rv => rdi
str => rsi
split_length => rdx
*/

TypedValue* fh_str_split(TypedValue* _rv, Value* str, int split_length) asm("_ZN4HPHP13fni_str_splitERKNS_6StringEi");

TypedValue * fg1_str_split(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_str_split(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
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
  fh_str_split((rv), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_str_split(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
        fh_str_split((&(rv)), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(1));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_str_split(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("str_split", count, 1, 2, 1);
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
HPHP::Variant HPHP::fni_chunk_split(HPHP::String const&, int, HPHP::String const&)
_ZN4HPHP15fni_chunk_splitERKNS_6StringEiS2_

(return value) => rax
_rv => rdi
body => rsi
chunklen => rdx
end => rcx
*/

TypedValue* fh_chunk_split(TypedValue* _rv, Value* body, int chunklen, Value* end) asm("_ZN4HPHP15fni_chunk_splitERKNS_6StringEiS2_");

TypedValue * fg1_chunk_split(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_chunk_split(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
    if (!IS_STRING_TYPE((args-2)->m_type)) {
      tvCastToStringInPlace(args-2);
    }
  case 2:
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  String defVal2 = "\r\n";
  fh_chunk_split((rv), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(76), (count > 2) ? (Value*)(args-2) : (Value*)(&defVal2));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_chunk_split(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 3LL) {
      if ((count <= 2 || IS_STRING_TYPE((args-2)->m_type)) && (count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
        String defVal2 = "\r\n";
        fh_chunk_split((&(rv)), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(76), (count > 2) ? (Value*)(args-2) : (Value*)(&defVal2));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_chunk_split(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("chunk_split", count, 1, 3, 1);
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
HPHP::Variant HPHP::fni_substr(HPHP::String const&, int, int)
_ZN4HPHP10fni_substrERKNS_6StringEii

(return value) => rax
_rv => rdi
str => rsi
start => rdx
length => rcx
*/

TypedValue* fh_substr(TypedValue* _rv, Value* str, int start, int length) asm("_ZN4HPHP10fni_substrERKNS_6StringEii");

TypedValue * fg1_substr(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_substr(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    break;
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_substr((rv), (Value*)(args-0), (int)(args[-1].m_data.num), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0x7FFFFFFF));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_substr(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfInt64) && (args-1)->m_type == KindOfInt64 && IS_STRING_TYPE((args-0)->m_type)) {
        fh_substr((&(rv)), (Value*)(args-0), (int)(args[-1].m_data.num), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0x7FFFFFFF));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_substr(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("substr", count, 2, 3, 1);
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
HPHP::String HPHP::fni_str_pad(HPHP::String const&, int, HPHP::String const&, int)
_ZN4HPHP11fni_str_padERKNS_6StringEiS2_i

(return value) => rax
_rv => rdi
input => rsi
pad_length => rdx
pad_string => rcx
pad_type => r8
*/

Value* fh_str_pad(Value* _rv, Value* input, int pad_length, Value* pad_string, int pad_type) asm("_ZN4HPHP11fni_str_padERKNS_6StringEiS2_i");

TypedValue * fg1_str_pad(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_str_pad(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
    if (!IS_STRING_TYPE((args-2)->m_type)) {
      tvCastToStringInPlace(args-2);
    }
  case 2:
    break;
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  String defVal2 = " ";
  fh_str_pad((Value*)(rv), (Value*)(args-0), (int)(args[-1].m_data.num), (count > 2) ? (Value*)(args-2) : (Value*)(&defVal2), (count > 3) ? (int)(args[-3].m_data.num) : (int)(k_STR_PAD_RIGHT));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_str_pad(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 4LL) {
      if ((count <= 3 || (args-3)->m_type == KindOfInt64) && (count <= 2 || IS_STRING_TYPE((args-2)->m_type)) && (args-1)->m_type == KindOfInt64 && IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfString;
        String defVal2 = " ";
        fh_str_pad((Value*)(&(rv)), (Value*)(args-0), (int)(args[-1].m_data.num), (count > 2) ? (Value*)(args-2) : (Value*)(&defVal2), (count > 3) ? (int)(args[-3].m_data.num) : (int)(k_STR_PAD_RIGHT));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_str_pad(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("str_pad", count, 2, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::fni_str_repeat(HPHP::String const&, int)
_ZN4HPHP14fni_str_repeatERKNS_6StringEi

(return value) => rax
_rv => rdi
input => rsi
multiplier => rdx
*/

Value* fh_str_repeat(Value* _rv, Value* input, int multiplier) asm("_ZN4HPHP14fni_str_repeatERKNS_6StringEi");

TypedValue * fg1_str_repeat(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_str_repeat(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_str_repeat((Value*)(rv), (Value*)(args-0), (int)(args[-1].m_data.num));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_str_repeat(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_str_repeat((Value*)(&(rv)), (Value*)(args-0), (int)(args[-1].m_data.num));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_str_repeat(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("str_repeat", count, 2, 2, 1);
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
HPHP::Variant HPHP::fni_wordwrap(HPHP::String const&, int, HPHP::String const&, bool)
_ZN4HPHP12fni_wordwrapERKNS_6StringEiS2_b

(return value) => rax
_rv => rdi
str => rsi
width => rdx
wordbreak => rcx
cut => r8
*/

TypedValue* fh_wordwrap(TypedValue* _rv, Value* str, int width, Value* wordbreak, bool cut) asm("_ZN4HPHP12fni_wordwrapERKNS_6StringEiS2_b");

TypedValue * fg1_wordwrap(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_wordwrap(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-3);
    }
  case 3:
    if (!IS_STRING_TYPE((args-2)->m_type)) {
      tvCastToStringInPlace(args-2);
    }
  case 2:
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  String defVal2 = "\n";
  fh_wordwrap((rv), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(75), (count > 2) ? (Value*)(args-2) : (Value*)(&defVal2), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_wordwrap(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 4LL) {
      if ((count <= 3 || (args-3)->m_type == KindOfBoolean) && (count <= 2 || IS_STRING_TYPE((args-2)->m_type)) && (count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
        String defVal2 = "\n";
        fh_wordwrap((&(rv)), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(75), (count > 2) ? (Value*)(args-2) : (Value*)(&defVal2), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_wordwrap(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("wordwrap", count, 1, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::fni_html_entity_decode(HPHP::String const&, int, HPHP::String const&)
_ZN4HPHP22fni_html_entity_decodeERKNS_6StringEiS2_

(return value) => rax
_rv => rdi
str => rsi
quote_style => rdx
charset => rcx
*/

Value* fh_html_entity_decode(Value* _rv, Value* str, int quote_style, Value* charset) asm("_ZN4HPHP22fni_html_entity_decodeERKNS_6StringEiS2_");

TypedValue * fg1_html_entity_decode(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_html_entity_decode(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  switch (count) {
  default: // count >= 3
    if (!IS_STRING_TYPE((args-2)->m_type)) {
      tvCastToStringInPlace(args-2);
    }
  case 2:
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  String defVal2 = "ISO-8859-1";
  fh_html_entity_decode((Value*)(rv), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(k_ENT_COMPAT), (count > 2) ? (Value*)(args-2) : (Value*)(&defVal2));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_html_entity_decode(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 3LL) {
      if ((count <= 2 || IS_STRING_TYPE((args-2)->m_type)) && (count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfString;
        String defVal2 = "ISO-8859-1";
        fh_html_entity_decode((Value*)(&(rv)), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(k_ENT_COMPAT), (count > 2) ? (Value*)(args-2) : (Value*)(&defVal2));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_html_entity_decode(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("html_entity_decode", count, 1, 3, 1);
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
HPHP::String HPHP::fni_htmlentities(HPHP::String const&, int, HPHP::String const&, bool)
_ZN4HPHP16fni_htmlentitiesERKNS_6StringEiS2_b

(return value) => rax
_rv => rdi
str => rsi
quote_style => rdx
charset => rcx
double_encode => r8
*/

Value* fh_htmlentities(Value* _rv, Value* str, int quote_style, Value* charset, bool double_encode) asm("_ZN4HPHP16fni_htmlentitiesERKNS_6StringEiS2_b");

TypedValue * fg1_htmlentities(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_htmlentities(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-3);
    }
  case 3:
    if (!IS_STRING_TYPE((args-2)->m_type)) {
      tvCastToStringInPlace(args-2);
    }
  case 2:
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  String defVal2 = "ISO-8859-1";
  fh_htmlentities((Value*)(rv), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(k_ENT_COMPAT), (count > 2) ? (Value*)(args-2) : (Value*)(&defVal2), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(true));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_htmlentities(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 4LL) {
      if ((count <= 3 || (args-3)->m_type == KindOfBoolean) && (count <= 2 || IS_STRING_TYPE((args-2)->m_type)) && (count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfString;
        String defVal2 = "ISO-8859-1";
        fh_htmlentities((Value*)(&(rv)), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(k_ENT_COMPAT), (count > 2) ? (Value*)(args-2) : (Value*)(&defVal2), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(true));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_htmlentities(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("htmlentities", count, 1, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::fni_htmlspecialchars_decode(HPHP::String const&, int)
_ZN4HPHP27fni_htmlspecialchars_decodeERKNS_6StringEi

(return value) => rax
_rv => rdi
str => rsi
quote_style => rdx
*/

Value* fh_htmlspecialchars_decode(Value* _rv, Value* str, int quote_style) asm("_ZN4HPHP27fni_htmlspecialchars_decodeERKNS_6StringEi");

TypedValue * fg1_htmlspecialchars_decode(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_htmlspecialchars_decode(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
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
  fh_htmlspecialchars_decode((Value*)(rv), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(k_ENT_COMPAT));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_htmlspecialchars_decode(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_htmlspecialchars_decode((Value*)(&(rv)), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(k_ENT_COMPAT));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_htmlspecialchars_decode(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("htmlspecialchars_decode", count, 1, 2, 1);
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
HPHP::String HPHP::fni_htmlspecialchars(HPHP::String const&, int, HPHP::String const&, bool)
_ZN4HPHP20fni_htmlspecialcharsERKNS_6StringEiS2_b

(return value) => rax
_rv => rdi
str => rsi
quote_style => rdx
charset => rcx
double_encode => r8
*/

Value* fh_htmlspecialchars(Value* _rv, Value* str, int quote_style, Value* charset, bool double_encode) asm("_ZN4HPHP20fni_htmlspecialcharsERKNS_6StringEiS2_b");

TypedValue * fg1_htmlspecialchars(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_htmlspecialchars(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-3);
    }
  case 3:
    if (!IS_STRING_TYPE((args-2)->m_type)) {
      tvCastToStringInPlace(args-2);
    }
  case 2:
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  String defVal2 = "ISO-8859-1";
  fh_htmlspecialchars((Value*)(rv), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(k_ENT_COMPAT), (count > 2) ? (Value*)(args-2) : (Value*)(&defVal2), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(true));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_htmlspecialchars(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 4LL) {
      if ((count <= 3 || (args-3)->m_type == KindOfBoolean) && (count <= 2 || IS_STRING_TYPE((args-2)->m_type)) && (count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfString;
        String defVal2 = "ISO-8859-1";
        fh_htmlspecialchars((Value*)(&(rv)), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(k_ENT_COMPAT), (count > 2) ? (Value*)(args-2) : (Value*)(&defVal2), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(true));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_htmlspecialchars(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("htmlspecialchars", count, 1, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::fni_fb_htmlspecialchars(HPHP::String const&, int, HPHP::String const&, HPHP::Array const&)
_ZN4HPHP23fni_fb_htmlspecialcharsERKNS_6StringEiS2_RKNS_5ArrayE

(return value) => rax
_rv => rdi
str => rsi
quote_style => rdx
charset => rcx
extra => r8
*/

Value* fh_fb_htmlspecialchars(Value* _rv, Value* str, int quote_style, Value* charset, Value* extra) asm("_ZN4HPHP23fni_fb_htmlspecialcharsERKNS_6StringEiS2_RKNS_5ArrayE");

TypedValue * fg1_fb_htmlspecialchars(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_fb_htmlspecialchars(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfArray) {
      tvCastToArrayInPlace(args-3);
    }
  case 3:
    if (!IS_STRING_TYPE((args-2)->m_type)) {
      tvCastToStringInPlace(args-2);
    }
  case 2:
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  String defVal2 = "ISO-8859-1";
  Array defVal3 = Array();
  fh_fb_htmlspecialchars((Value*)(rv), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(k_ENT_COMPAT), (count > 2) ? (Value*)(args-2) : (Value*)(&defVal2), (count > 3) ? (Value*)(args-3) : (Value*)(&defVal3));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_fb_htmlspecialchars(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 4LL) {
      if ((count <= 3 || (args-3)->m_type == KindOfArray) && (count <= 2 || IS_STRING_TYPE((args-2)->m_type)) && (count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfString;
        String defVal2 = "ISO-8859-1";
        Array defVal3 = Array();
        fh_fb_htmlspecialchars((Value*)(&(rv)), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(k_ENT_COMPAT), (count > 2) ? (Value*)(args-2) : (Value*)(&defVal2), (count > 3) ? (Value*)(args-3) : (Value*)(&defVal3));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_fb_htmlspecialchars(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("fb_htmlspecialchars", count, 1, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::fni_quoted_printable_encode(HPHP::String const&)
_ZN4HPHP27fni_quoted_printable_encodeERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

Value* fh_quoted_printable_encode(Value* _rv, Value* str) asm("_ZN4HPHP27fni_quoted_printable_encodeERKNS_6StringE");

TypedValue * fg1_quoted_printable_encode(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_quoted_printable_encode(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToStringInPlace(args-0);
  fh_quoted_printable_encode((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_quoted_printable_encode(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_quoted_printable_encode((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_quoted_printable_encode(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("quoted_printable_encode", count, 1, 1, 1);
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
HPHP::String HPHP::fni_quoted_printable_decode(HPHP::String const&)
_ZN4HPHP27fni_quoted_printable_decodeERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

Value* fh_quoted_printable_decode(Value* _rv, Value* str) asm("_ZN4HPHP27fni_quoted_printable_decodeERKNS_6StringE");

TypedValue * fg1_quoted_printable_decode(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_quoted_printable_decode(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToStringInPlace(args-0);
  fh_quoted_printable_decode((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_quoted_printable_decode(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_quoted_printable_decode((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_quoted_printable_decode(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("quoted_printable_decode", count, 1, 1, 1);
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
HPHP::Variant HPHP::fni_convert_uudecode(HPHP::String const&)
_ZN4HPHP20fni_convert_uudecodeERKNS_6StringE

(return value) => rax
_rv => rdi
data => rsi
*/

TypedValue* fh_convert_uudecode(TypedValue* _rv, Value* data) asm("_ZN4HPHP20fni_convert_uudecodeERKNS_6StringE");

TypedValue * fg1_convert_uudecode(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_convert_uudecode(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_convert_uudecode((rv), (Value*)(args-0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_convert_uudecode(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        fh_convert_uudecode((&(rv)), (Value*)(args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_convert_uudecode(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("convert_uudecode", count, 1, 1, 1);
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
HPHP::Variant HPHP::fni_convert_uuencode(HPHP::String const&)
_ZN4HPHP20fni_convert_uuencodeERKNS_6StringE

(return value) => rax
_rv => rdi
data => rsi
*/

TypedValue* fh_convert_uuencode(TypedValue* _rv, Value* data) asm("_ZN4HPHP20fni_convert_uuencodeERKNS_6StringE");

TypedValue * fg1_convert_uuencode(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_convert_uuencode(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_convert_uuencode((rv), (Value*)(args-0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_convert_uuencode(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        fh_convert_uuencode((&(rv)), (Value*)(args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_convert_uuencode(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("convert_uuencode", count, 1, 1, 1);
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
HPHP::String HPHP::fni_str_rot13(HPHP::String const&)
_ZN4HPHP13fni_str_rot13ERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

Value* fh_str_rot13(Value* _rv, Value* str) asm("_ZN4HPHP13fni_str_rot13ERKNS_6StringE");

TypedValue * fg1_str_rot13(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_str_rot13(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToStringInPlace(args-0);
  fh_str_rot13((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_str_rot13(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_str_rot13((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_str_rot13(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("str_rot13", count, 1, 1, 1);
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
long long HPHP::fni_crc32(HPHP::String const&)
_ZN4HPHP9fni_crc32ERKNS_6StringE

(return value) => rax
str => rdi
*/

long long fh_crc32(Value* str) asm("_ZN4HPHP9fni_crc32ERKNS_6StringE");

TypedValue * fg1_crc32(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_crc32(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfInt64;
  tvCastToStringInPlace(args-0);
  rv->m_data.num = (long long)fh_crc32((Value*)(args-0));
  return rv;
}

TypedValue* fg_crc32(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (long long)fh_crc32((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_crc32(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("crc32", count, 1, 1, 1);
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
HPHP::String HPHP::fni_crypt(HPHP::String const&, HPHP::String const&)
_ZN4HPHP9fni_cryptERKNS_6StringES2_

(return value) => rax
_rv => rdi
str => rsi
salt => rdx
*/

Value* fh_crypt(Value* _rv, Value* str, Value* salt) asm("_ZN4HPHP9fni_cryptERKNS_6StringES2_");

TypedValue * fg1_crypt(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_crypt(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
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
  fh_crypt((Value*)(rv), (Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&empty_string));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_crypt(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_crypt((Value*)(&(rv)), (Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&empty_string));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_crypt(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("crypt", count, 1, 2, 1);
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
HPHP::String HPHP::fni_md5(HPHP::String const&, bool)
_ZN4HPHP7fni_md5ERKNS_6StringEb

(return value) => rax
_rv => rdi
str => rsi
raw_output => rdx
*/

Value* fh_md5(Value* _rv, Value* str, bool raw_output) asm("_ZN4HPHP7fni_md5ERKNS_6StringEb");

TypedValue * fg1_md5(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_md5(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
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
  fh_md5((Value*)(rv), (Value*)(args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_md5(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfBoolean) && IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_md5((Value*)(&(rv)), (Value*)(args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_md5(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("md5", count, 1, 2, 1);
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
HPHP::String HPHP::fni_sha1(HPHP::String const&, bool)
_ZN4HPHP8fni_sha1ERKNS_6StringEb

(return value) => rax
_rv => rdi
str => rsi
raw_output => rdx
*/

Value* fh_sha1(Value* _rv, Value* str, bool raw_output) asm("_ZN4HPHP8fni_sha1ERKNS_6StringEb");

TypedValue * fg1_sha1(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_sha1(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
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
  fh_sha1((Value*)(rv), (Value*)(args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_sha1(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfBoolean) && IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_sha1((Value*)(&(rv)), (Value*)(args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_sha1(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("sha1", count, 1, 2, 1);
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
HPHP::Array HPHP::fni_get_html_translation_table(int, int)
_ZN4HPHP30fni_get_html_translation_tableEii

(return value) => rax
_rv => rdi
table => rsi
quote_style => rdx
*/

Value* fh_get_html_translation_table(Value* _rv, int table, int quote_style) asm("_ZN4HPHP30fni_get_html_translation_tableEii");

TypedValue * fg1_get_html_translation_table(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_get_html_translation_table(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfArray;
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
  fh_get_html_translation_table((Value*)(rv), (count > 0) ? (int)(args[-0].m_data.num) : (int)(0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(k_ENT_COMPAT));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_get_html_translation_table(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64) && (count <= 0 || (args-0)->m_type == KindOfInt64)) {
        rv._count = 0;
        rv.m_type = KindOfArray;
        fh_get_html_translation_table((Value*)(&(rv)), (count > 0) ? (int)(args[-0].m_data.num) : (int)(0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(k_ENT_COMPAT));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_get_html_translation_table(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("get_html_translation_table", 2, 1);
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
HPHP::String HPHP::fni_nl_langinfo(int)
_ZN4HPHP15fni_nl_langinfoEi

(return value) => rax
_rv => rdi
item => rsi
*/

Value* fh_nl_langinfo(Value* _rv, int item) asm("_ZN4HPHP15fni_nl_langinfoEi");

TypedValue * fg1_nl_langinfo(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_nl_langinfo(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToInt64InPlace(args-0);
  fh_nl_langinfo((Value*)(rv), (int)(args[-0].m_data.num));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_nl_langinfo(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfInt64) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_nl_langinfo((Value*)(&(rv)), (int)(args[-0].m_data.num));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_nl_langinfo(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("nl_langinfo", count, 1, 1, 1);
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
HPHP::Variant HPHP::fni_printf(int, HPHP::String const&, HPHP::Array const&)
_ZN4HPHP10fni_printfEiRKNS_6StringERKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
format => rdx
_argv => rcx
*/

TypedValue* fh_printf(TypedValue* _rv, long long _argc, Value* format, Value* _argv) asm("_ZN4HPHP10fni_printfEiRKNS_6StringERKNS_5ArrayE");

TypedValue * fg1_printf(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_printf(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  Array extraArgs;
  {
    ArrayInit ai(count-1, false);
    for (long long i = 1; i < count; ++i) {
      TypedValue* extraArg = ar->getExtraArg(i-1);
      if (tvIsStronglyBound(extraArg)) {
        ai.setRef(i-1, tvAsVariant(extraArg));
      } else {
        ai.set(i-1, tvAsVariant(extraArg));
      }
    }
    extraArgs = ai.create();
  }
  fh_printf((rv), (count), (Value*)(args-0), (Value*)(&extraArgs));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_printf(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        Array extraArgs;
        {
          ArrayInit ai(count-1, false);
          for (long long i = 1; i < count; ++i) {
            TypedValue* extraArg = ar->getExtraArg(i-1);
            if (tvIsStronglyBound(extraArg)) {
              ai.setRef(i-1, tvAsVariant(extraArg));
            } else {
              ai.set(i-1, tvAsVariant(extraArg));
            }
          }
          extraArgs = ai.create();
        }
        fh_printf((&(rv)), (count), (Value*)(args-0), (Value*)(&extraArgs));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_printf(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_missing_arguments_nr("printf", count+1, 1);
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
HPHP::Variant HPHP::fni_vprintf(HPHP::String const&, HPHP::Array const&)
_ZN4HPHP11fni_vprintfERKNS_6StringERKNS_5ArrayE

(return value) => rax
_rv => rdi
format => rsi
args => rdx
*/

TypedValue* fh_vprintf(TypedValue* _rv, Value* format, Value* args) asm("_ZN4HPHP11fni_vprintfERKNS_6StringERKNS_5ArrayE");

TypedValue * fg1_vprintf(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_vprintf(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfArray) {
    tvCastToArrayInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_vprintf((rv), (Value*)(args-0), (Value*)(args-1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_vprintf(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfArray && IS_STRING_TYPE((args-0)->m_type)) {
        fh_vprintf((&(rv)), (Value*)(args-0), (Value*)(args-1));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_vprintf(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("vprintf", count, 2, 2, 1);
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
HPHP::Variant HPHP::fni_sprintf(int, HPHP::String const&, HPHP::Array const&)
_ZN4HPHP11fni_sprintfEiRKNS_6StringERKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
format => rdx
_argv => rcx
*/

TypedValue* fh_sprintf(TypedValue* _rv, long long _argc, Value* format, Value* _argv) asm("_ZN4HPHP11fni_sprintfEiRKNS_6StringERKNS_5ArrayE");

TypedValue * fg1_sprintf(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_sprintf(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  Array extraArgs;
  {
    ArrayInit ai(count-1, false);
    for (long long i = 1; i < count; ++i) {
      TypedValue* extraArg = ar->getExtraArg(i-1);
      if (tvIsStronglyBound(extraArg)) {
        ai.setRef(i-1, tvAsVariant(extraArg));
      } else {
        ai.set(i-1, tvAsVariant(extraArg));
      }
    }
    extraArgs = ai.create();
  }
  fh_sprintf((rv), (count), (Value*)(args-0), (Value*)(&extraArgs));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_sprintf(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        Array extraArgs;
        {
          ArrayInit ai(count-1, false);
          for (long long i = 1; i < count; ++i) {
            TypedValue* extraArg = ar->getExtraArg(i-1);
            if (tvIsStronglyBound(extraArg)) {
              ai.setRef(i-1, tvAsVariant(extraArg));
            } else {
              ai.set(i-1, tvAsVariant(extraArg));
            }
          }
          extraArgs = ai.create();
        }
        fh_sprintf((&(rv)), (count), (Value*)(args-0), (Value*)(&extraArgs));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_sprintf(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_missing_arguments_nr("sprintf", count+1, 1);
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
HPHP::Variant HPHP::fni_vsprintf(HPHP::String const&, HPHP::Array const&)
_ZN4HPHP12fni_vsprintfERKNS_6StringERKNS_5ArrayE

(return value) => rax
_rv => rdi
format => rsi
args => rdx
*/

TypedValue* fh_vsprintf(TypedValue* _rv, Value* format, Value* args) asm("_ZN4HPHP12fni_vsprintfERKNS_6StringERKNS_5ArrayE");

TypedValue * fg1_vsprintf(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_vsprintf(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfArray) {
    tvCastToArrayInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_vsprintf((rv), (Value*)(args-0), (Value*)(args-1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_vsprintf(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfArray && IS_STRING_TYPE((args-0)->m_type)) {
        fh_vsprintf((&(rv)), (Value*)(args-0), (Value*)(args-1));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_vsprintf(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("vsprintf", count, 2, 2, 1);
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
HPHP::String HPHP::fni_chr(long long)
_ZN4HPHP7fni_chrEx

(return value) => rax
_rv => rdi
ascii => rsi
*/

Value* fh_chr(Value* _rv, long long ascii) asm("_ZN4HPHP7fni_chrEx");

TypedValue * fg1_chr(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_chr(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToInt64InPlace(args-0);
  fh_chr((Value*)(rv), (long long)(args[-0].m_data.num));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_chr(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfInt64) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_chr((Value*)(&(rv)), (long long)(args[-0].m_data.num));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_chr(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("chr", count, 1, 1, 1);
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
long long HPHP::fni_ord(HPHP::String const&)
_ZN4HPHP7fni_ordERKNS_6StringE

(return value) => rax
str => rdi
*/

long long fh_ord(Value* str) asm("_ZN4HPHP7fni_ordERKNS_6StringE");

TypedValue * fg1_ord(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_ord(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfInt64;
  tvCastToStringInPlace(args-0);
  rv->m_data.num = (long long)fh_ord((Value*)(args-0));
  return rv;
}

TypedValue* fg_ord(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (long long)fh_ord((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_ord(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("ord", count, 1, 1, 1);
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
HPHP::Variant HPHP::fni_money_format(HPHP::String const&, double)
_ZN4HPHP16fni_money_formatERKNS_6StringEd

(return value) => rax
_rv => rdi
format => rsi
number => xmm0
*/

TypedValue* fh_money_format(TypedValue* _rv, Value* format, double number) asm("_ZN4HPHP16fni_money_formatERKNS_6StringEd");

TypedValue * fg1_money_format(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_money_format(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_money_format((rv), (Value*)(args-0), (args[-1].m_data.dbl));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_money_format(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && IS_STRING_TYPE((args-0)->m_type)) {
        fh_money_format((&(rv)), (Value*)(args-0), (args[-1].m_data.dbl));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_money_format(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("money_format", count, 2, 2, 1);
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
long long HPHP::fni_strcmp(HPHP::String const&, HPHP::String const&)
_ZN4HPHP10fni_strcmpERKNS_6StringES2_

(return value) => rax
str1 => rdi
str2 => rsi
*/

long long fh_strcmp(Value* str1, Value* str2) asm("_ZN4HPHP10fni_strcmpERKNS_6StringES2_");

TypedValue * fg1_strcmp(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_strcmp(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfInt64;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (long long)fh_strcmp((Value*)(args-0), (Value*)(args-1));
  return rv;
}

TypedValue* fg_strcmp(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (long long)fh_strcmp((Value*)(args-0), (Value*)(args-1));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_strcmp(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("strcmp", count, 2, 2, 1);
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
long long HPHP::fni_strncmp(HPHP::String const&, HPHP::String const&, int)
_ZN4HPHP11fni_strncmpERKNS_6StringES2_i

(return value) => rax
str1 => rdi
str2 => rsi
len => rdx
*/

long long fh_strncmp(Value* str1, Value* str2, int len) asm("_ZN4HPHP11fni_strncmpERKNS_6StringES2_i");

TypedValue * fg1_strncmp(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_strncmp(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfInt64;
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (long long)fh_strncmp((Value*)(args-0), (Value*)(args-1), (int)(args[-2].m_data.num));
  return rv;
}

TypedValue* fg_strncmp(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-2)->m_type == KindOfInt64 && IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (long long)fh_strncmp((Value*)(args-0), (Value*)(args-1), (int)(args[-2].m_data.num));
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_strncmp(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("strncmp", count, 3, 3, 1);
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
long long HPHP::fni_strnatcmp(HPHP::String const&, HPHP::String const&)
_ZN4HPHP13fni_strnatcmpERKNS_6StringES2_

(return value) => rax
str1 => rdi
str2 => rsi
*/

long long fh_strnatcmp(Value* str1, Value* str2) asm("_ZN4HPHP13fni_strnatcmpERKNS_6StringES2_");

TypedValue * fg1_strnatcmp(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_strnatcmp(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfInt64;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (long long)fh_strnatcmp((Value*)(args-0), (Value*)(args-1));
  return rv;
}

TypedValue* fg_strnatcmp(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (long long)fh_strnatcmp((Value*)(args-0), (Value*)(args-1));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_strnatcmp(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("strnatcmp", count, 2, 2, 1);
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
long long HPHP::fni_strcasecmp(HPHP::String const&, HPHP::String const&)
_ZN4HPHP14fni_strcasecmpERKNS_6StringES2_

(return value) => rax
str1 => rdi
str2 => rsi
*/

long long fh_strcasecmp(Value* str1, Value* str2) asm("_ZN4HPHP14fni_strcasecmpERKNS_6StringES2_");

TypedValue * fg1_strcasecmp(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_strcasecmp(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfInt64;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (long long)fh_strcasecmp((Value*)(args-0), (Value*)(args-1));
  return rv;
}

TypedValue* fg_strcasecmp(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (long long)fh_strcasecmp((Value*)(args-0), (Value*)(args-1));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_strcasecmp(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("strcasecmp", count, 2, 2, 1);
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
long long HPHP::fni_strncasecmp(HPHP::String const&, HPHP::String const&, int)
_ZN4HPHP15fni_strncasecmpERKNS_6StringES2_i

(return value) => rax
str1 => rdi
str2 => rsi
len => rdx
*/

long long fh_strncasecmp(Value* str1, Value* str2, int len) asm("_ZN4HPHP15fni_strncasecmpERKNS_6StringES2_i");

TypedValue * fg1_strncasecmp(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_strncasecmp(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfInt64;
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (long long)fh_strncasecmp((Value*)(args-0), (Value*)(args-1), (int)(args[-2].m_data.num));
  return rv;
}

TypedValue* fg_strncasecmp(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-2)->m_type == KindOfInt64 && IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (long long)fh_strncasecmp((Value*)(args-0), (Value*)(args-1), (int)(args[-2].m_data.num));
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_strncasecmp(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("strncasecmp", count, 3, 3, 1);
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
long long HPHP::fni_strnatcasecmp(HPHP::String const&, HPHP::String const&)
_ZN4HPHP17fni_strnatcasecmpERKNS_6StringES2_

(return value) => rax
str1 => rdi
str2 => rsi
*/

long long fh_strnatcasecmp(Value* str1, Value* str2) asm("_ZN4HPHP17fni_strnatcasecmpERKNS_6StringES2_");

TypedValue * fg1_strnatcasecmp(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_strnatcasecmp(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfInt64;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (long long)fh_strnatcasecmp((Value*)(args-0), (Value*)(args-1));
  return rv;
}

TypedValue* fg_strnatcasecmp(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (long long)fh_strnatcasecmp((Value*)(args-0), (Value*)(args-1));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_strnatcasecmp(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("strnatcasecmp", count, 2, 2, 1);
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
long long HPHP::fni_strcoll(HPHP::String const&, HPHP::String const&)
_ZN4HPHP11fni_strcollERKNS_6StringES2_

(return value) => rax
str1 => rdi
str2 => rsi
*/

long long fh_strcoll(Value* str1, Value* str2) asm("_ZN4HPHP11fni_strcollERKNS_6StringES2_");

TypedValue * fg1_strcoll(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_strcoll(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfInt64;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (long long)fh_strcoll((Value*)(args-0), (Value*)(args-1));
  return rv;
}

TypedValue* fg_strcoll(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (long long)fh_strcoll((Value*)(args-0), (Value*)(args-1));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_strcoll(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("strcoll", count, 2, 2, 1);
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
HPHP::Variant HPHP::fni_strchr(HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP10fni_strchrERKNS_6StringERKNS_7VariantE

(return value) => rax
_rv => rdi
haystack => rsi
needle => rdx
*/

TypedValue* fh_strchr(TypedValue* _rv, Value* haystack, TypedValue* needle) asm("_ZN4HPHP10fni_strchrERKNS_6StringERKNS_7VariantE");

TypedValue * fg1_strchr(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_strchr(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_strchr((rv), (Value*)(args-0), (args-1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_strchr(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        fh_strchr((&(rv)), (Value*)(args-0), (args-1));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_strchr(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("strchr", count, 2, 2, 1);
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
HPHP::Variant HPHP::fni_strlen(HPHP::Variant const&)
_ZN4HPHP10fni_strlenERKNS_7VariantE

(return value) => rax
_rv => rdi
str => rsi
*/

TypedValue* fh_strlen(TypedValue* _rv, TypedValue* str) asm("_ZN4HPHP10fni_strlenERKNS_7VariantE");

TypedValue* fg_strlen(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      fh_strlen((&(rv)), (args-0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("strlen", count, 1, 1, 1);
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
long long HPHP::fni_levenshtein(HPHP::String const&, HPHP::String const&, int, int, int)
_ZN4HPHP15fni_levenshteinERKNS_6StringES2_iii

(return value) => rax
str1 => rdi
str2 => rsi
cost_ins => rdx
cost_rep => rcx
cost_del => r8
*/

long long fh_levenshtein(Value* str1, Value* str2, int cost_ins, int cost_rep, int cost_del) asm("_ZN4HPHP15fni_levenshteinERKNS_6StringES2_iii");

TypedValue * fg1_levenshtein(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_levenshtein(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfInt64;
  switch (count) {
  default: // count >= 5
    if ((args-4)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-4);
    }
  case 4:
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
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
  rv->m_data.num = (long long)fh_levenshtein((Value*)(args-0), (Value*)(args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(1), (count > 3) ? (int)(args[-3].m_data.num) : (int)(1), (count > 4) ? (int)(args[-4].m_data.num) : (int)(1));
  return rv;
}

TypedValue* fg_levenshtein(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 5LL) {
      if ((count <= 4 || (args-4)->m_type == KindOfInt64) && (count <= 3 || (args-3)->m_type == KindOfInt64) && (count <= 2 || (args-2)->m_type == KindOfInt64) && IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (long long)fh_levenshtein((Value*)(args-0), (Value*)(args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(1), (count > 3) ? (int)(args[-3].m_data.num) : (int)(1), (count > 4) ? (int)(args[-4].m_data.num) : (int)(1));
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_levenshtein(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("levenshtein", count, 2, 5, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 5);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long long HPHP::fni_similar_text(HPHP::String const&, HPHP::String const&, HPHP::VRefParamValue const&)
_ZN4HPHP16fni_similar_textERKNS_6StringES2_RKNS_14VRefParamValueE

(return value) => rax
first => rdi
second => rsi
percent => rdx
*/

long long fh_similar_text(Value* first, Value* second, TypedValue* percent) asm("_ZN4HPHP16fni_similar_textERKNS_6StringES2_RKNS_14VRefParamValueE");

TypedValue * fg1_similar_text(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_similar_text(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfInt64;
  switch (count) {
  default: // count >= 3
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  VRefParamValue defVal2 = null;
  rv->m_data.num = (long long)fh_similar_text((Value*)(args-0), (Value*)(args-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
  return rv;
}

TypedValue* fg_similar_text(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        VRefParamValue defVal2 = null;
        rv.m_data.num = (long long)fh_similar_text((Value*)(args-0), (Value*)(args-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_similar_text(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("similar_text", count, 2, 3, 1);
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
HPHP::Variant HPHP::fni_soundex(HPHP::String const&)
_ZN4HPHP11fni_soundexERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

TypedValue* fh_soundex(TypedValue* _rv, Value* str) asm("_ZN4HPHP11fni_soundexERKNS_6StringE");

TypedValue * fg1_soundex(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_soundex(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_soundex((rv), (Value*)(args-0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_soundex(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        fh_soundex((&(rv)), (Value*)(args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_soundex(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("soundex", count, 1, 1, 1);
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
HPHP::Variant HPHP::fni_metaphone(HPHP::String const&, int)
_ZN4HPHP13fni_metaphoneERKNS_6StringEi

(return value) => rax
_rv => rdi
str => rsi
phones => rdx
*/

TypedValue* fh_metaphone(TypedValue* _rv, Value* str, int phones) asm("_ZN4HPHP13fni_metaphoneERKNS_6StringEi");

TypedValue * fg1_metaphone(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_metaphone(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
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
  fh_metaphone((rv), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_metaphone(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
        fh_metaphone((&(rv)), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_metaphone(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("metaphone", count, 1, 2, 1);
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
HPHP::String HPHP::fni_rawurldecode(HPHP::String const&)
_ZN4HPHP16fni_rawurldecodeERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

Value* fh_rawurldecode(Value* _rv, Value* str) asm("_ZN4HPHP16fni_rawurldecodeERKNS_6StringE");

TypedValue * fg1_rawurldecode(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_rawurldecode(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToStringInPlace(args-0);
  fh_rawurldecode((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_rawurldecode(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_rawurldecode((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_rawurldecode(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("rawurldecode", count, 1, 1, 1);
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
HPHP::String HPHP::fni_rawurlencode(HPHP::String const&)
_ZN4HPHP16fni_rawurlencodeERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

Value* fh_rawurlencode(Value* _rv, Value* str) asm("_ZN4HPHP16fni_rawurlencodeERKNS_6StringE");

TypedValue * fg1_rawurlencode(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_rawurlencode(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToStringInPlace(args-0);
  fh_rawurlencode((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_rawurlencode(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_rawurlencode((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_rawurlencode(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("rawurlencode", count, 1, 1, 1);
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
HPHP::String HPHP::fni_urldecode(HPHP::String const&)
_ZN4HPHP13fni_urldecodeERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

Value* fh_urldecode(Value* _rv, Value* str) asm("_ZN4HPHP13fni_urldecodeERKNS_6StringE");

TypedValue * fg1_urldecode(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_urldecode(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToStringInPlace(args-0);
  fh_urldecode((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_urldecode(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_urldecode((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_urldecode(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("urldecode", count, 1, 1, 1);
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
HPHP::String HPHP::fni_urlencode(HPHP::String const&)
_ZN4HPHP13fni_urlencodeERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

Value* fh_urlencode(Value* _rv, Value* str) asm("_ZN4HPHP13fni_urlencodeERKNS_6StringE");

TypedValue * fg1_urlencode(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_urlencode(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToStringInPlace(args-0);
  fh_urlencode((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_urlencode(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_urlencode((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_urlencode(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("urlencode", count, 1, 1, 1);
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
bool HPHP::fni_is_bool(HPHP::Variant const&)
_ZN4HPHP11fni_is_boolERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_bool(TypedValue* var) asm("_ZN4HPHP11fni_is_boolERKNS_7VariantE");

TypedValue* fg_is_bool(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv._count = 0;
      rv.m_type = KindOfBoolean;
      rv.m_data.num = (fh_is_bool((args-0))) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("is_bool", count, 1, 1, 1);
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
bool HPHP::fni_is_int(HPHP::Variant const&)
_ZN4HPHP10fni_is_intERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_int(TypedValue* var) asm("_ZN4HPHP10fni_is_intERKNS_7VariantE");

TypedValue* fg_is_int(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv._count = 0;
      rv.m_type = KindOfBoolean;
      rv.m_data.num = (fh_is_int((args-0))) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("is_int", count, 1, 1, 1);
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
bool HPHP::fni_is_integer(HPHP::Variant const&)
_ZN4HPHP14fni_is_integerERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_integer(TypedValue* var) asm("_ZN4HPHP14fni_is_integerERKNS_7VariantE");

TypedValue* fg_is_integer(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv._count = 0;
      rv.m_type = KindOfBoolean;
      rv.m_data.num = (fh_is_integer((args-0))) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("is_integer", count, 1, 1, 1);
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
bool HPHP::fni_is_long(HPHP::Variant const&)
_ZN4HPHP11fni_is_longERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_long(TypedValue* var) asm("_ZN4HPHP11fni_is_longERKNS_7VariantE");

TypedValue* fg_is_long(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv._count = 0;
      rv.m_type = KindOfBoolean;
      rv.m_data.num = (fh_is_long((args-0))) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("is_long", count, 1, 1, 1);
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
bool HPHP::fni_is_double(HPHP::Variant const&)
_ZN4HPHP13fni_is_doubleERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_double(TypedValue* var) asm("_ZN4HPHP13fni_is_doubleERKNS_7VariantE");

TypedValue* fg_is_double(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv._count = 0;
      rv.m_type = KindOfBoolean;
      rv.m_data.num = (fh_is_double((args-0))) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("is_double", count, 1, 1, 1);
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
bool HPHP::fni_is_float(HPHP::Variant const&)
_ZN4HPHP12fni_is_floatERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_float(TypedValue* var) asm("_ZN4HPHP12fni_is_floatERKNS_7VariantE");

TypedValue* fg_is_float(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv._count = 0;
      rv.m_type = KindOfBoolean;
      rv.m_data.num = (fh_is_float((args-0))) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("is_float", count, 1, 1, 1);
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
bool HPHP::fni_is_numeric(HPHP::Variant const&)
_ZN4HPHP14fni_is_numericERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_numeric(TypedValue* var) asm("_ZN4HPHP14fni_is_numericERKNS_7VariantE");

TypedValue* fg_is_numeric(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv._count = 0;
      rv.m_type = KindOfBoolean;
      rv.m_data.num = (fh_is_numeric((args-0))) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("is_numeric", count, 1, 1, 1);
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
bool HPHP::fni_is_real(HPHP::Variant const&)
_ZN4HPHP11fni_is_realERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_real(TypedValue* var) asm("_ZN4HPHP11fni_is_realERKNS_7VariantE");

TypedValue* fg_is_real(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv._count = 0;
      rv.m_type = KindOfBoolean;
      rv.m_data.num = (fh_is_real((args-0))) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("is_real", count, 1, 1, 1);
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
bool HPHP::fni_is_string(HPHP::Variant const&)
_ZN4HPHP13fni_is_stringERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_string(TypedValue* var) asm("_ZN4HPHP13fni_is_stringERKNS_7VariantE");

TypedValue* fg_is_string(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv._count = 0;
      rv.m_type = KindOfBoolean;
      rv.m_data.num = (fh_is_string((args-0))) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("is_string", count, 1, 1, 1);
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
bool HPHP::fni_is_scalar(HPHP::Variant const&)
_ZN4HPHP13fni_is_scalarERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_scalar(TypedValue* var) asm("_ZN4HPHP13fni_is_scalarERKNS_7VariantE");

TypedValue* fg_is_scalar(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv._count = 0;
      rv.m_type = KindOfBoolean;
      rv.m_data.num = (fh_is_scalar((args-0))) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("is_scalar", count, 1, 1, 1);
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
bool HPHP::fni_is_array(HPHP::Variant const&)
_ZN4HPHP12fni_is_arrayERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_array(TypedValue* var) asm("_ZN4HPHP12fni_is_arrayERKNS_7VariantE");

TypedValue* fg_is_array(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv._count = 0;
      rv.m_type = KindOfBoolean;
      rv.m_data.num = (fh_is_array((args-0))) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("is_array", count, 1, 1, 1);
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
bool HPHP::fni_is_resource(HPHP::Variant const&)
_ZN4HPHP15fni_is_resourceERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_resource(TypedValue* var) asm("_ZN4HPHP15fni_is_resourceERKNS_7VariantE");

TypedValue* fg_is_resource(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv._count = 0;
      rv.m_type = KindOfBoolean;
      rv.m_data.num = (fh_is_resource((args-0))) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("is_resource", count, 1, 1, 1);
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
bool HPHP::fni_is_null(HPHP::Variant const&)
_ZN4HPHP11fni_is_nullERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_null(TypedValue* var) asm("_ZN4HPHP11fni_is_nullERKNS_7VariantE");

TypedValue* fg_is_null(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv._count = 0;
      rv.m_type = KindOfBoolean;
      rv.m_data.num = (fh_is_null((args-0))) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("is_null", count, 1, 1, 1);
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
long long HPHP::fni_intval(HPHP::Variant const&, long long)
_ZN4HPHP10fni_intvalERKNS_7VariantEx

(return value) => rax
v => rdi
base => rsi
*/

long long fh_intval(TypedValue* v, long long base) asm("_ZN4HPHP10fni_intvalERKNS_7VariantEx");

TypedValue * fg1_intval(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_intval(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfInt64;
  tvCastToInt64InPlace(args-1);
  rv->m_data.num = (long long)fh_intval((args-0), (count > 1) ? (long long)(args[-1].m_data.num) : (long long)(10));
  return rv;
}

TypedValue* fg_intval(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64)) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (long long)fh_intval((args-0), (count > 1) ? (long long)(args[-1].m_data.num) : (long long)(10));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_intval(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("intval", count, 1, 2, 1);
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
double HPHP::fni_doubleval(HPHP::Variant const&)
_ZN4HPHP13fni_doublevalERKNS_7VariantE

(return value) => xmm0
v => rdi
*/

double fh_doubleval(TypedValue* v) asm("_ZN4HPHP13fni_doublevalERKNS_7VariantE");

TypedValue* fg_doubleval(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv._count = 0;
      rv.m_type = KindOfDouble;
      rv.m_data.dbl = fh_doubleval((args-0));
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("doubleval", count, 1, 1, 1);
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
double HPHP::fni_floatval(HPHP::Variant const&)
_ZN4HPHP12fni_floatvalERKNS_7VariantE

(return value) => xmm0
v => rdi
*/

double fh_floatval(TypedValue* v) asm("_ZN4HPHP12fni_floatvalERKNS_7VariantE");

TypedValue* fg_floatval(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv._count = 0;
      rv.m_type = KindOfDouble;
      rv.m_data.dbl = fh_floatval((args-0));
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("floatval", count, 1, 1, 1);
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
HPHP::String HPHP::fni_strval(HPHP::Variant const&)
_ZN4HPHP10fni_strvalERKNS_7VariantE

(return value) => rax
_rv => rdi
v => rsi
*/

Value* fh_strval(Value* _rv, TypedValue* v) asm("_ZN4HPHP10fni_strvalERKNS_7VariantE");

TypedValue* fg_strval(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv._count = 0;
      rv.m_type = KindOfString;
      fh_strval((Value*)(&(rv)), (args-0));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("strval", count, 1, 1, 1);
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
HPHP::Variant HPHP::fni_unserialize(HPHP::String const&)
_ZN4HPHP15fni_unserializeERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

TypedValue* fh_unserialize(TypedValue* _rv, Value* str) asm("_ZN4HPHP15fni_unserializeERKNS_6StringE");

TypedValue * fg1_unserialize(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_unserialize(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_unserialize((rv), (Value*)(args-0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_unserialize(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        fh_unserialize((&(rv)), (Value*)(args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_unserialize(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("unserialize", count, 1, 1, 1);
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
HPHP::String HPHP::fni_zlib_get_coding_type()
_ZN4HPHP24fni_zlib_get_coding_typeEv

(return value) => rax
_rv => rdi
*/

Value* fh_zlib_get_coding_type(Value* _rv) asm("_ZN4HPHP24fni_zlib_get_coding_typeEv");

TypedValue* fg_zlib_get_coding_type(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfString;
      fh_zlib_get_coding_type((Value*)(&(rv)));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("zlib_get_coding_type", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::fni_gzclose(HPHP::Object const&)
_ZN4HPHP11fni_gzcloseERKNS_6ObjectE

(return value) => rax
zp => rdi
*/

bool fh_gzclose(Value* zp) asm("_ZN4HPHP11fni_gzcloseERKNS_6ObjectE");

TypedValue * fg1_gzclose(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_gzclose(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_gzclose((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_gzclose(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_gzclose((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_gzclose(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("gzclose", count, 1, 1, 1);
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
bool HPHP::fni_gzrewind(HPHP::Object const&)
_ZN4HPHP12fni_gzrewindERKNS_6ObjectE

(return value) => rax
zp => rdi
*/

bool fh_gzrewind(Value* zp) asm("_ZN4HPHP12fni_gzrewindERKNS_6ObjectE");

TypedValue * fg1_gzrewind(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_gzrewind(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_gzrewind((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_gzrewind(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_gzrewind((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_gzrewind(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("gzrewind", count, 1, 1, 1);
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
bool HPHP::fni_gzeof(HPHP::Object const&)
_ZN4HPHP9fni_gzeofERKNS_6ObjectE

(return value) => rax
zp => rdi
*/

bool fh_gzeof(Value* zp) asm("_ZN4HPHP9fni_gzeofERKNS_6ObjectE");

TypedValue * fg1_gzeof(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_gzeof(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_gzeof((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_gzeof(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_gzeof((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_gzeof(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("gzeof", count, 1, 1, 1);
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
HPHP::Variant HPHP::fni_gzgetc(HPHP::Object const&)
_ZN4HPHP10fni_gzgetcERKNS_6ObjectE

(return value) => rax
_rv => rdi
zp => rsi
*/

TypedValue* fh_gzgetc(TypedValue* _rv, Value* zp) asm("_ZN4HPHP10fni_gzgetcERKNS_6ObjectE");

TypedValue * fg1_gzgetc(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_gzgetc(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  fh_gzgetc((rv), (Value*)(args-0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_gzgetc(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        fh_gzgetc((&(rv)), (Value*)(args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_gzgetc(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("gzgetc", count, 1, 1, 1);
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
HPHP::Variant HPHP::fni_gzgets(HPHP::Object const&, long long)
_ZN4HPHP10fni_gzgetsERKNS_6ObjectEx

(return value) => rax
_rv => rdi
zp => rsi
length => rdx
*/

TypedValue* fh_gzgets(TypedValue* _rv, Value* zp, long long length) asm("_ZN4HPHP10fni_gzgetsERKNS_6ObjectEx");

TypedValue * fg1_gzgets(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_gzgets(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
    break;
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_gzgets((rv), (Value*)(args-0), (count > 1) ? (long long)(args[-1].m_data.num) : (long long)(1024));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_gzgets(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64) && (args-0)->m_type == KindOfObject) {
        fh_gzgets((&(rv)), (Value*)(args-0), (count > 1) ? (long long)(args[-1].m_data.num) : (long long)(1024));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_gzgets(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("gzgets", count, 1, 2, 1);
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
HPHP::Variant HPHP::fni_gzgetss(HPHP::Object const&, long long, HPHP::String const&)
_ZN4HPHP11fni_gzgetssERKNS_6ObjectExRKNS_6StringE

(return value) => rax
_rv => rdi
zp => rsi
length => rdx
allowable_tags => rcx
*/

TypedValue* fh_gzgetss(TypedValue* _rv, Value* zp, long long length, Value* allowable_tags) asm("_ZN4HPHP11fni_gzgetssERKNS_6ObjectExRKNS_6StringE");

TypedValue * fg1_gzgetss(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_gzgetss(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
    if (!IS_STRING_TYPE((args-2)->m_type)) {
      tvCastToStringInPlace(args-2);
    }
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
  fh_gzgetss((rv), (Value*)(args-0), (count > 1) ? (long long)(args[-1].m_data.num) : (long long)(0), (count > 2) ? (Value*)(args-2) : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_gzgetss(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 3LL) {
      if ((count <= 2 || IS_STRING_TYPE((args-2)->m_type)) && (count <= 1 || (args-1)->m_type == KindOfInt64) && (args-0)->m_type == KindOfObject) {
        fh_gzgetss((&(rv)), (Value*)(args-0), (count > 1) ? (long long)(args[-1].m_data.num) : (long long)(0), (count > 2) ? (Value*)(args-2) : (Value*)(&null_string));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_gzgetss(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("gzgetss", count, 1, 3, 1);
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
HPHP::Variant HPHP::fni_gzread(HPHP::Object const&, long long)
_ZN4HPHP10fni_gzreadERKNS_6ObjectEx

(return value) => rax
_rv => rdi
zp => rsi
length => rdx
*/

TypedValue* fh_gzread(TypedValue* _rv, Value* zp, long long length) asm("_ZN4HPHP10fni_gzreadERKNS_6ObjectEx");

TypedValue * fg1_gzread(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_gzread(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
    break;
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_gzread((rv), (Value*)(args-0), (count > 1) ? (long long)(args[-1].m_data.num) : (long long)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_gzread(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64) && (args-0)->m_type == KindOfObject) {
        fh_gzread((&(rv)), (Value*)(args-0), (count > 1) ? (long long)(args[-1].m_data.num) : (long long)(0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_gzread(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("gzread", count, 1, 2, 1);
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
HPHP::Variant HPHP::fni_gzpassthru(HPHP::Object const&)
_ZN4HPHP14fni_gzpassthruERKNS_6ObjectE

(return value) => rax
_rv => rdi
zp => rsi
*/

TypedValue* fh_gzpassthru(TypedValue* _rv, Value* zp) asm("_ZN4HPHP14fni_gzpassthruERKNS_6ObjectE");

TypedValue * fg1_gzpassthru(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_gzpassthru(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  fh_gzpassthru((rv), (Value*)(args-0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_gzpassthru(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        fh_gzpassthru((&(rv)), (Value*)(args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_gzpassthru(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("gzpassthru", count, 1, 1, 1);
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
HPHP::Variant HPHP::fni_gzseek(HPHP::Object const&, long long, long long)
_ZN4HPHP10fni_gzseekERKNS_6ObjectExx

(return value) => rax
_rv => rdi
zp => rsi
offset => rdx
whence => rcx
*/

TypedValue* fh_gzseek(TypedValue* _rv, Value* zp, long long offset, long long whence) asm("_ZN4HPHP10fni_gzseekERKNS_6ObjectExx");

TypedValue * fg1_gzseek(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_gzseek(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    break;
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_gzseek((rv), (Value*)(args-0), (long long)(args[-1].m_data.num), (count > 2) ? (long long)(args[-2].m_data.num) : (long long)(k_SEEK_SET));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_gzseek(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfInt64) && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        fh_gzseek((&(rv)), (Value*)(args-0), (long long)(args[-1].m_data.num), (count > 2) ? (long long)(args[-2].m_data.num) : (long long)(k_SEEK_SET));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_gzseek(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("gzseek", count, 2, 3, 1);
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
HPHP::Variant HPHP::fni_gztell(HPHP::Object const&)
_ZN4HPHP10fni_gztellERKNS_6ObjectE

(return value) => rax
_rv => rdi
zp => rsi
*/

TypedValue* fh_gztell(TypedValue* _rv, Value* zp) asm("_ZN4HPHP10fni_gztellERKNS_6ObjectE");

TypedValue * fg1_gztell(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_gztell(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  fh_gztell((rv), (Value*)(args-0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_gztell(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        fh_gztell((&(rv)), (Value*)(args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_gztell(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("gztell", count, 1, 1, 1);
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
HPHP::Variant HPHP::fni_gzwrite(HPHP::Object const&, HPHP::String const&, long long)
_ZN4HPHP11fni_gzwriteERKNS_6ObjectERKNS_6StringEx

(return value) => rax
_rv => rdi
zp => rsi
str => rdx
length => rcx
*/

TypedValue* fh_gzwrite(TypedValue* _rv, Value* zp, Value* str, long long length) asm("_ZN4HPHP11fni_gzwriteERKNS_6ObjectERKNS_6StringEx");

TypedValue * fg1_gzwrite(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_gzwrite(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
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
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_gzwrite((rv), (Value*)(args-0), (Value*)(args-1), (count > 2) ? (long long)(args[-2].m_data.num) : (long long)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_gzwrite(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfInt64) && IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        fh_gzwrite((&(rv)), (Value*)(args-0), (Value*)(args-1), (count > 2) ? (long long)(args[-2].m_data.num) : (long long)(0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_gzwrite(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("gzwrite", count, 2, 3, 1);
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
HPHP::Variant HPHP::fni_gzputs(HPHP::Object const&, HPHP::String const&, long long)
_ZN4HPHP10fni_gzputsERKNS_6ObjectERKNS_6StringEx

(return value) => rax
_rv => rdi
zp => rsi
str => rdx
length => rcx
*/

TypedValue* fh_gzputs(TypedValue* _rv, Value* zp, Value* str, long long length) asm("_ZN4HPHP10fni_gzputsERKNS_6ObjectERKNS_6StringEx");

TypedValue * fg1_gzputs(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_gzputs(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
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
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_gzputs((rv), (Value*)(args-0), (Value*)(args-1), (count > 2) ? (long long)(args[-2].m_data.num) : (long long)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_gzputs(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfInt64) && IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        fh_gzputs((&(rv)), (Value*)(args-0), (Value*)(args-1), (count > 2) ? (long long)(args[-2].m_data.num) : (long long)(0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_gzputs(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("gzputs", count, 2, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}




} // !HPHP

