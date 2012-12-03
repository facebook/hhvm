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
HPHP::Variant HPHP::f_fb_thrift_serialize(HPHP::Variant const&)
_ZN4HPHP21f_fb_thrift_serializeERKNS_7VariantE

(return value) => rax
_rv => rdi
thing => rsi
*/

TypedValue* fh_fb_thrift_serialize(TypedValue* _rv, TypedValue* thing) asm("_ZN4HPHP21f_fb_thrift_serializeERKNS_7VariantE");

TypedValue* fg_fb_thrift_serialize(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      fh_fb_thrift_serialize((&(rv)), (args-0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("fb_thrift_serialize", count, 1, 1, 1);
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
HPHP::Variant HPHP::f_fb_thrift_unserialize(HPHP::Variant const&, HPHP::VRefParamValue const&, HPHP::VRefParamValue const&)
_ZN4HPHP23f_fb_thrift_unserializeERKNS_7VariantERKNS_14VRefParamValueES5_

(return value) => rax
_rv => rdi
thing => rsi
success => rdx
errcode => rcx
*/

TypedValue* fh_fb_thrift_unserialize(TypedValue* _rv, TypedValue* thing, TypedValue* success, TypedValue* errcode) asm("_ZN4HPHP23f_fb_thrift_unserializeERKNS_7VariantERKNS_14VRefParamValueES5_");

TypedValue* fg_fb_thrift_unserialize(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      VRefParamValue defVal2 = null_variant;
      fh_fb_thrift_unserialize((&(rv)), (args-0), (args-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 3);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("fb_thrift_unserialize", count, 2, 3, 1);
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
HPHP::Variant HPHP::f_fb_serialize(HPHP::Variant const&)
_ZN4HPHP14f_fb_serializeERKNS_7VariantE

(return value) => rax
_rv => rdi
thing => rsi
*/

TypedValue* fh_fb_serialize(TypedValue* _rv, TypedValue* thing) asm("_ZN4HPHP14f_fb_serializeERKNS_7VariantE");

TypedValue* fg_fb_serialize(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      fh_fb_serialize((&(rv)), (args-0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("fb_serialize", count, 1, 1, 1);
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
HPHP::Variant HPHP::f_fb_unserialize(HPHP::Variant const&, HPHP::VRefParamValue const&, HPHP::VRefParamValue const&)
_ZN4HPHP16f_fb_unserializeERKNS_7VariantERKNS_14VRefParamValueES5_

(return value) => rax
_rv => rdi
thing => rsi
success => rdx
errcode => rcx
*/

TypedValue* fh_fb_unserialize(TypedValue* _rv, TypedValue* thing, TypedValue* success, TypedValue* errcode) asm("_ZN4HPHP16f_fb_unserializeERKNS_7VariantERKNS_14VRefParamValueES5_");

TypedValue* fg_fb_unserialize(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      VRefParamValue defVal2 = null_variant;
      fh_fb_unserialize((&(rv)), (args-0), (args-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 3);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("fb_unserialize", count, 2, 3, 1);
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
HPHP::Variant HPHP::f_fb_compact_serialize(HPHP::Variant const&)
_ZN4HPHP22f_fb_compact_serializeERKNS_7VariantE

(return value) => rax
_rv => rdi
thing => rsi
*/

TypedValue* fh_fb_compact_serialize(TypedValue* _rv, TypedValue* thing) asm("_ZN4HPHP22f_fb_compact_serializeERKNS_7VariantE");

TypedValue* fg_fb_compact_serialize(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      fh_fb_compact_serialize((&(rv)), (args-0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("fb_compact_serialize", count, 1, 1, 1);
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
HPHP::Variant HPHP::f_fb_compact_unserialize(HPHP::Variant const&, HPHP::VRefParamValue const&, HPHP::VRefParamValue const&)
_ZN4HPHP24f_fb_compact_unserializeERKNS_7VariantERKNS_14VRefParamValueES5_

(return value) => rax
_rv => rdi
thing => rsi
success => rdx
errcode => rcx
*/

TypedValue* fh_fb_compact_unserialize(TypedValue* _rv, TypedValue* thing, TypedValue* success, TypedValue* errcode) asm("_ZN4HPHP24f_fb_compact_unserializeERKNS_7VariantERKNS_14VRefParamValueES5_");

TypedValue* fg_fb_compact_unserialize(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      VRefParamValue defVal2 = null_variant;
      fh_fb_compact_unserialize((&(rv)), (args-0), (args-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 3);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("fb_compact_unserialize", count, 2, 3, 1);
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
bool HPHP::f_fb_could_include(HPHP::String const&)
_ZN4HPHP18f_fb_could_includeERKNS_6StringE

(return value) => rax
file => rdi
*/

bool fh_fb_could_include(Value* file) asm("_ZN4HPHP18f_fb_could_includeERKNS_6StringE");

TypedValue * fg1_fb_could_include(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_fb_could_include(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToStringInPlace(args-0);
  rv->m_data.num = (fh_fb_could_include((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_fb_could_include(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_fb_could_include((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_fb_could_include(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("fb_could_include", count, 1, 1, 1);
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
bool HPHP::f_fb_intercept(HPHP::String const&, HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP14f_fb_interceptERKNS_6StringERKNS_7VariantES5_

(return value) => rax
name => rdi
handler => rsi
data => rdx
*/

bool fh_fb_intercept(Value* name, TypedValue* handler, TypedValue* data) asm("_ZN4HPHP14f_fb_interceptERKNS_6StringERKNS_7VariantES5_");

TypedValue * fg1_fb_intercept(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_fb_intercept(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToStringInPlace(args-0);
  rv->m_data.num = (fh_fb_intercept((Value*)(args-0), (args-1), (count > 2) ? (args-2) : (TypedValue*)(&null_variant))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_fb_intercept(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_fb_intercept((Value*)(args-0), (args-1), (count > 2) ? (args-2) : (TypedValue*)(&null_variant))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_fb_intercept(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("fb_intercept", count, 2, 3, 1);
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
HPHP::Variant HPHP::f_fb_stubout_intercept_handler(HPHP::String const&, HPHP::Variant const&, HPHP::Array const&, HPHP::Variant const&, HPHP::VRefParamValue const&)
_ZN4HPHP30f_fb_stubout_intercept_handlerERKNS_6StringERKNS_7VariantERKNS_5ArrayES5_RKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
name => rsi
obj => rdx
params => rcx
data => r8
done => r9
*/

TypedValue* fh_fb_stubout_intercept_handler(TypedValue* _rv, Value* name, TypedValue* obj, Value* params, TypedValue* data, TypedValue* done) asm("_ZN4HPHP30f_fb_stubout_intercept_handlerERKNS_6StringERKNS_7VariantERKNS_5ArrayES5_RKNS_14VRefParamValueE");

TypedValue * fg1_fb_stubout_intercept_handler(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_fb_stubout_intercept_handler(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-2)->m_type != KindOfArray) {
    tvCastToArrayInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_fb_stubout_intercept_handler((rv), (Value*)(args-0), (args-1), (Value*)(args-2), (args-3), (args-4));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_fb_stubout_intercept_handler(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 5LL) {
      if ((args-2)->m_type == KindOfArray && IS_STRING_TYPE((args-0)->m_type)) {
        fh_fb_stubout_intercept_handler((&(rv)), (Value*)(args-0), (args-1), (Value*)(args-2), (args-3), (args-4));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_fb_stubout_intercept_handler(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("fb_stubout_intercept_handler", count, 5, 5, 1);
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
HPHP::Variant HPHP::f_fb_rpc_intercept_handler(HPHP::String const&, HPHP::Variant const&, HPHP::Array const&, HPHP::Variant const&, HPHP::VRefParamValue const&)
_ZN4HPHP26f_fb_rpc_intercept_handlerERKNS_6StringERKNS_7VariantERKNS_5ArrayES5_RKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
name => rsi
obj => rdx
params => rcx
data => r8
done => r9
*/

TypedValue* fh_fb_rpc_intercept_handler(TypedValue* _rv, Value* name, TypedValue* obj, Value* params, TypedValue* data, TypedValue* done) asm("_ZN4HPHP26f_fb_rpc_intercept_handlerERKNS_6StringERKNS_7VariantERKNS_5ArrayES5_RKNS_14VRefParamValueE");

TypedValue * fg1_fb_rpc_intercept_handler(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_fb_rpc_intercept_handler(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-2)->m_type != KindOfArray) {
    tvCastToArrayInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_fb_rpc_intercept_handler((rv), (Value*)(args-0), (args-1), (Value*)(args-2), (args-3), (args-4));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_fb_rpc_intercept_handler(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 5LL) {
      if ((args-2)->m_type == KindOfArray && IS_STRING_TYPE((args-0)->m_type)) {
        fh_fb_rpc_intercept_handler((&(rv)), (Value*)(args-0), (args-1), (Value*)(args-2), (args-3), (args-4));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_fb_rpc_intercept_handler(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("fb_rpc_intercept_handler", count, 5, 5, 1);
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
void HPHP::f_fb_renamed_functions(HPHP::Array const&)
_ZN4HPHP22f_fb_renamed_functionsERKNS_5ArrayE

names => rdi
*/

void fh_fb_renamed_functions(Value* names) asm("_ZN4HPHP22f_fb_renamed_functionsERKNS_5ArrayE");

TypedValue * fg1_fb_renamed_functions(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_fb_renamed_functions(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  tvCastToArrayInPlace(args-0);
  fh_fb_renamed_functions((Value*)(args-0));
  return rv;
}

TypedValue* fg_fb_renamed_functions(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfArray) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        fh_fb_renamed_functions((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_fb_renamed_functions(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("fb_renamed_functions", count, 1, 1, 1);
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
bool HPHP::f_fb_rename_function(HPHP::String const&, HPHP::String const&)
_ZN4HPHP20f_fb_rename_functionERKNS_6StringES2_

(return value) => rax
orig_func_name => rdi
new_func_name => rsi
*/

bool fh_fb_rename_function(Value* orig_func_name, Value* new_func_name) asm("_ZN4HPHP20f_fb_rename_functionERKNS_6StringES2_");

TypedValue * fg1_fb_rename_function(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_fb_rename_function(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (fh_fb_rename_function((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_fb_rename_function(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_fb_rename_function((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_fb_rename_function(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("fb_rename_function", count, 2, 2, 1);
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
bool HPHP::f_fb_autoload_map(HPHP::Variant const&, HPHP::String const&)
_ZN4HPHP17f_fb_autoload_mapERKNS_7VariantERKNS_6StringE

(return value) => rax
map => rdi
root => rsi
*/

bool fh_fb_autoload_map(TypedValue* map, Value* root) asm("_ZN4HPHP17f_fb_autoload_mapERKNS_7VariantERKNS_6StringE");

TypedValue * fg1_fb_autoload_map(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_fb_autoload_map(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToStringInPlace(args-1);
  rv->m_data.num = (fh_fb_autoload_map((args-0), (Value*)(args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_fb_autoload_map(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_fb_autoload_map((args-0), (Value*)(args-1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_fb_autoload_map(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("fb_autoload_map", count, 2, 2, 1);
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
bool HPHP::f_fb_utf8ize(HPHP::VRefParamValue const&)
_ZN4HPHP12f_fb_utf8izeERKNS_14VRefParamValueE

(return value) => rax
input => rdi
*/

bool fh_fb_utf8ize(TypedValue* input) asm("_ZN4HPHP12f_fb_utf8izeERKNS_14VRefParamValueE");

TypedValue* fg_fb_utf8ize(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv._count = 0;
      rv.m_type = KindOfBoolean;
      rv.m_data.num = (fh_fb_utf8ize((args-0))) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("fb_utf8ize", count, 1, 1, 1);
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
long long HPHP::f_fb_utf8_strlen_deprecated(HPHP::String const&)
_ZN4HPHP27f_fb_utf8_strlen_deprecatedERKNS_6StringE

(return value) => rax
input => rdi
*/

long long fh_fb_utf8_strlen_deprecated(Value* input) asm("_ZN4HPHP27f_fb_utf8_strlen_deprecatedERKNS_6StringE");

TypedValue * fg1_fb_utf8_strlen_deprecated(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_fb_utf8_strlen_deprecated(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfInt64;
  tvCastToStringInPlace(args-0);
  rv->m_data.num = (long long)fh_fb_utf8_strlen_deprecated((Value*)(args-0));
  return rv;
}

TypedValue* fg_fb_utf8_strlen_deprecated(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (long long)fh_fb_utf8_strlen_deprecated((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_fb_utf8_strlen_deprecated(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("fb_utf8_strlen_deprecated", count, 1, 1, 1);
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
long long HPHP::f_fb_utf8_strlen(HPHP::String const&)
_ZN4HPHP16f_fb_utf8_strlenERKNS_6StringE

(return value) => rax
input => rdi
*/

long long fh_fb_utf8_strlen(Value* input) asm("_ZN4HPHP16f_fb_utf8_strlenERKNS_6StringE");

TypedValue * fg1_fb_utf8_strlen(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_fb_utf8_strlen(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfInt64;
  tvCastToStringInPlace(args-0);
  rv->m_data.num = (long long)fh_fb_utf8_strlen((Value*)(args-0));
  return rv;
}

TypedValue* fg_fb_utf8_strlen(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (long long)fh_fb_utf8_strlen((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_fb_utf8_strlen(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("fb_utf8_strlen", count, 1, 1, 1);
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
HPHP::Variant HPHP::f_fb_utf8_substr(HPHP::String const&, int, int)
_ZN4HPHP16f_fb_utf8_substrERKNS_6StringEii

(return value) => rax
_rv => rdi
str => rsi
start => rdx
length => rcx
*/

TypedValue* fh_fb_utf8_substr(TypedValue* _rv, Value* str, int start, int length) asm("_ZN4HPHP16f_fb_utf8_substrERKNS_6StringEii");

TypedValue * fg1_fb_utf8_substr(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_fb_utf8_substr(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
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
  fh_fb_utf8_substr((rv), (Value*)(args-0), (int)(args[-1].m_data.num), (count > 2) ? (int)(args[-2].m_data.num) : (int)(INT_MAX));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_fb_utf8_substr(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfInt64) && (args-1)->m_type == KindOfInt64 && IS_STRING_TYPE((args-0)->m_type)) {
        fh_fb_utf8_substr((&(rv)), (Value*)(args-0), (int)(args[-1].m_data.num), (count > 2) ? (int)(args[-2].m_data.num) : (int)(INT_MAX));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_fb_utf8_substr(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("fb_utf8_substr", count, 2, 3, 1);
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
HPHP::Array HPHP::f_fb_call_user_func_safe(int, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP24f_fb_call_user_func_safeEiRKNS_7VariantERKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
function => rdx
_argv => rcx
*/

Value* fh_fb_call_user_func_safe(Value* _rv, long long _argc, TypedValue* function, Value* _argv) asm("_ZN4HPHP24f_fb_call_user_func_safeEiRKNS_7VariantERKNS_5ArrayE");

TypedValue* fg_fb_call_user_func_safe(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL) {
      rv._count = 0;
      rv.m_type = KindOfArray;
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
      fh_fb_call_user_func_safe((Value*)(&(rv)), (count), (args-0), (Value*)(&extraArgs));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_missing_arguments_nr("fb_call_user_func_safe", count+1, 1);
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
HPHP::Variant HPHP::f_fb_call_user_func_safe_return(int, HPHP::Variant const&, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP31f_fb_call_user_func_safe_returnEiRKNS_7VariantES2_RKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
function => rdx
def => rcx
_argv => r8
*/

TypedValue* fh_fb_call_user_func_safe_return(TypedValue* _rv, long long _argc, TypedValue* function, TypedValue* def, Value* _argv) asm("_ZN4HPHP31f_fb_call_user_func_safe_returnEiRKNS_7VariantES2_RKNS_5ArrayE");

TypedValue* fg_fb_call_user_func_safe_return(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL) {
      Array extraArgs;
      {
        ArrayInit ai(count-2, false);
        for (long long i = 2; i < count; ++i) {
          TypedValue* extraArg = ar->getExtraArg(i-2);
          if (tvIsStronglyBound(extraArg)) {
            ai.setRef(i-2, tvAsVariant(extraArg));
          } else {
            ai.set(i-2, tvAsVariant(extraArg));
          }
        }
        extraArgs = ai.create();
      }
      fh_fb_call_user_func_safe_return((&(rv)), (count), (args-0), (args-1), (Value*)(&extraArgs));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 2);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_missing_arguments_nr("fb_call_user_func_safe_return", count+1, 1);
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
HPHP::Array HPHP::f_fb_call_user_func_array_safe(HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP30f_fb_call_user_func_array_safeERKNS_7VariantERKNS_5ArrayE

(return value) => rax
_rv => rdi
function => rsi
params => rdx
*/

Value* fh_fb_call_user_func_array_safe(Value* _rv, TypedValue* function, Value* params) asm("_ZN4HPHP30f_fb_call_user_func_array_safeERKNS_7VariantERKNS_5ArrayE");

TypedValue * fg1_fb_call_user_func_array_safe(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_fb_call_user_func_array_safe(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfArray;
  tvCastToArrayInPlace(args-1);
  fh_fb_call_user_func_array_safe((Value*)(rv), (args-0), (Value*)(args-1));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_fb_call_user_func_array_safe(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfArray) {
        rv._count = 0;
        rv.m_type = KindOfArray;
        fh_fb_call_user_func_array_safe((Value*)(&(rv)), (args-0), (Value*)(args-1));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_fb_call_user_func_array_safe(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("fb_call_user_func_array_safe", count, 2, 2, 1);
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
HPHP::Variant HPHP::f_fb_get_code_coverage(bool)
_ZN4HPHP22f_fb_get_code_coverageEb

(return value) => rax
_rv => rdi
flush => rsi
*/

TypedValue* fh_fb_get_code_coverage(TypedValue* _rv, bool flush) asm("_ZN4HPHP22f_fb_get_code_coverageEb");

TypedValue * fg1_fb_get_code_coverage(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_fb_get_code_coverage(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToBooleanInPlace(args-0);
  fh_fb_get_code_coverage((rv), (bool)(args[-0].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_fb_get_code_coverage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfBoolean) {
        fh_fb_get_code_coverage((&(rv)), (bool)(args[-0].m_data.num));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_fb_get_code_coverage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("fb_get_code_coverage", count, 1, 1, 1);
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
void HPHP::f_fb_enable_code_coverage()
_ZN4HPHP25f_fb_enable_code_coverageEv

*/

void fh_fb_enable_code_coverage() asm("_ZN4HPHP25f_fb_enable_code_coverageEv");

TypedValue* fg_fb_enable_code_coverage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_data.num = 0LL;
      rv._count = 0;
      rv.m_type = KindOfNull;
      fh_fb_enable_code_coverage();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("fb_enable_code_coverage", 0, 1);
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
HPHP::Variant HPHP::f_fb_disable_code_coverage()
_ZN4HPHP26f_fb_disable_code_coverageEv

(return value) => rax
_rv => rdi
*/

TypedValue* fh_fb_disable_code_coverage(TypedValue* _rv) asm("_ZN4HPHP26f_fb_disable_code_coverageEv");

TypedValue* fg_fb_disable_code_coverage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      fh_fb_disable_code_coverage((&(rv)));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("fb_disable_code_coverage", 0, 1);
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
void HPHP::f_fb_load_local_databases(HPHP::Array const&)
_ZN4HPHP25f_fb_load_local_databasesERKNS_5ArrayE

servers => rdi
*/

void fh_fb_load_local_databases(Value* servers) asm("_ZN4HPHP25f_fb_load_local_databasesERKNS_5ArrayE");

TypedValue * fg1_fb_load_local_databases(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_fb_load_local_databases(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  tvCastToArrayInPlace(args-0);
  fh_fb_load_local_databases((Value*)(args-0));
  return rv;
}

TypedValue* fg_fb_load_local_databases(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfArray) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        fh_fb_load_local_databases((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_fb_load_local_databases(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("fb_load_local_databases", count, 1, 1, 1);
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
HPHP::Array HPHP::f_fb_parallel_query(HPHP::Array const&, int, bool, bool, int, int, bool)
_ZN4HPHP19f_fb_parallel_queryERKNS_5ArrayEibbiib

(return value) => rax
_rv => rdi
sql_map => rsi
max_thread => rdx
combine_result => rcx
retry_query_on_fail => r8
connect_timeout => r9
read_timeout => st0
timeout_in_ms => st8
*/

Value* fh_fb_parallel_query(Value* _rv, Value* sql_map, int max_thread, bool combine_result, bool retry_query_on_fail, int connect_timeout, int read_timeout, bool timeout_in_ms) asm("_ZN4HPHP19f_fb_parallel_queryERKNS_5ArrayEibbiib");

TypedValue * fg1_fb_parallel_query(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_fb_parallel_query(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfArray;
  switch (count) {
  default: // count >= 7
    if ((args-6)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-6);
    }
  case 6:
    if ((args-5)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-5);
    }
  case 5:
    if ((args-4)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-4);
    }
  case 4:
    if ((args-3)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-3);
    }
  case 3:
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
  if ((args-0)->m_type != KindOfArray) {
    tvCastToArrayInPlace(args-0);
  }
  fh_fb_parallel_query((Value*)(rv), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(50), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(true), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(true), (count > 4) ? (int)(args[-4].m_data.num) : (int)(-1), (count > 5) ? (int)(args[-5].m_data.num) : (int)(-1), (count > 6) ? (bool)(args[-6].m_data.num) : (bool)(false));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_fb_parallel_query(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 7LL) {
      if ((count <= 6 || (args-6)->m_type == KindOfBoolean) && (count <= 5 || (args-5)->m_type == KindOfInt64) && (count <= 4 || (args-4)->m_type == KindOfInt64) && (count <= 3 || (args-3)->m_type == KindOfBoolean) && (count <= 2 || (args-2)->m_type == KindOfBoolean) && (count <= 1 || (args-1)->m_type == KindOfInt64) && (args-0)->m_type == KindOfArray) {
        rv._count = 0;
        rv.m_type = KindOfArray;
        fh_fb_parallel_query((Value*)(&(rv)), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(50), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(true), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(true), (count > 4) ? (int)(args[-4].m_data.num) : (int)(-1), (count > 5) ? (int)(args[-5].m_data.num) : (int)(-1), (count > 6) ? (bool)(args[-6].m_data.num) : (bool)(false));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 7);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_fb_parallel_query(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 7);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("fb_parallel_query", count, 1, 7, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 7);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_fb_crossall_query(HPHP::String const&, int, bool, int, int, bool)
_ZN4HPHP19f_fb_crossall_queryERKNS_6StringEibiib

(return value) => rax
_rv => rdi
sql => rsi
max_thread => rdx
retry_query_on_fail => rcx
connect_timeout => r8
read_timeout => r9
timeout_in_ms => st0
*/

Value* fh_fb_crossall_query(Value* _rv, Value* sql, int max_thread, bool retry_query_on_fail, int connect_timeout, int read_timeout, bool timeout_in_ms) asm("_ZN4HPHP19f_fb_crossall_queryERKNS_6StringEibiib");

TypedValue * fg1_fb_crossall_query(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_fb_crossall_query(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfArray;
  switch (count) {
  default: // count >= 6
    if ((args-5)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-5);
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
  fh_fb_crossall_query((Value*)(rv), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(50), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(true), (count > 3) ? (int)(args[-3].m_data.num) : (int)(-1), (count > 4) ? (int)(args[-4].m_data.num) : (int)(-1), (count > 5) ? (bool)(args[-5].m_data.num) : (bool)(false));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_fb_crossall_query(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 6LL) {
      if ((count <= 5 || (args-5)->m_type == KindOfBoolean) && (count <= 4 || (args-4)->m_type == KindOfInt64) && (count <= 3 || (args-3)->m_type == KindOfInt64) && (count <= 2 || (args-2)->m_type == KindOfBoolean) && (count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfArray;
        fh_fb_crossall_query((Value*)(&(rv)), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(50), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(true), (count > 3) ? (int)(args[-3].m_data.num) : (int)(-1), (count > 4) ? (int)(args[-4].m_data.num) : (int)(-1), (count > 5) ? (bool)(args[-5].m_data.num) : (bool)(false));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_fb_crossall_query(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("fb_crossall_query", count, 1, 6, 1);
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
void HPHP::f_fb_set_taint(HPHP::VRefParamValue const&, int)
_ZN4HPHP14f_fb_set_taintERKNS_14VRefParamValueEi

str => rdi
taint => rsi
*/

void fh_fb_set_taint(TypedValue* str, int taint) asm("_ZN4HPHP14f_fb_set_taintERKNS_14VRefParamValueEi");

TypedValue * fg1_fb_set_taint(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_fb_set_taint(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  tvCastToInt64InPlace(args-1);
  fh_fb_set_taint((args-0), (int)(args[-1].m_data.num));
  return rv;
}

TypedValue* fg_fb_set_taint(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        fh_fb_set_taint((args-0), (int)(args[-1].m_data.num));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_fb_set_taint(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("fb_set_taint", count, 2, 2, 1);
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
void HPHP::f_fb_unset_taint(HPHP::VRefParamValue const&, int)
_ZN4HPHP16f_fb_unset_taintERKNS_14VRefParamValueEi

str => rdi
taint => rsi
*/

void fh_fb_unset_taint(TypedValue* str, int taint) asm("_ZN4HPHP16f_fb_unset_taintERKNS_14VRefParamValueEi");

TypedValue * fg1_fb_unset_taint(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_fb_unset_taint(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  tvCastToInt64InPlace(args-1);
  fh_fb_unset_taint((args-0), (int)(args[-1].m_data.num));
  return rv;
}

TypedValue* fg_fb_unset_taint(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        fh_fb_unset_taint((args-0), (int)(args[-1].m_data.num));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_fb_unset_taint(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("fb_unset_taint", count, 2, 2, 1);
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
bool HPHP::f_fb_get_taint(HPHP::String const&, int)
_ZN4HPHP14f_fb_get_taintERKNS_6StringEi

(return value) => rax
str => rdi
taint => rsi
*/

bool fh_fb_get_taint(Value* str, int taint) asm("_ZN4HPHP14f_fb_get_taintERKNS_6StringEi");

TypedValue * fg1_fb_get_taint(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_fb_get_taint(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (fh_fb_get_taint((Value*)(args-0), (int)(args[-1].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_fb_get_taint(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_fb_get_taint((Value*)(args-0), (int)(args[-1].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_fb_get_taint(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("fb_get_taint", count, 2, 2, 1);
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
HPHP::Array HPHP::f_fb_get_taint_warning_counts()
_ZN4HPHP29f_fb_get_taint_warning_countsEv

(return value) => rax
_rv => rdi
*/

Value* fh_fb_get_taint_warning_counts(Value* _rv) asm("_ZN4HPHP29f_fb_get_taint_warning_countsEv");

TypedValue* fg_fb_get_taint_warning_counts(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfArray;
      fh_fb_get_taint_warning_counts((Value*)(&(rv)));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("fb_get_taint_warning_counts", 0, 1);
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
void HPHP::f_fb_enable_html_taint_trace()
_ZN4HPHP28f_fb_enable_html_taint_traceEv

*/

void fh_fb_enable_html_taint_trace() asm("_ZN4HPHP28f_fb_enable_html_taint_traceEv");

TypedValue* fg_fb_enable_html_taint_trace(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_data.num = 0LL;
      rv._count = 0;
      rv.m_type = KindOfNull;
      fh_fb_enable_html_taint_trace();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("fb_enable_html_taint_trace", 0, 1);
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
HPHP::Variant HPHP::f_fb_const_fetch(HPHP::Variant const&)
_ZN4HPHP16f_fb_const_fetchERKNS_7VariantE

(return value) => rax
_rv => rdi
key => rsi
*/

TypedValue* fh_fb_const_fetch(TypedValue* _rv, TypedValue* key) asm("_ZN4HPHP16f_fb_const_fetchERKNS_7VariantE");

TypedValue* fg_fb_const_fetch(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      fh_fb_const_fetch((&(rv)), (args-0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("fb_const_fetch", count, 1, 1, 1);
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
bool HPHP::f_fb_output_compression(bool)
_ZN4HPHP23f_fb_output_compressionEb

(return value) => rax
new_value => rdi
*/

bool fh_fb_output_compression(bool new_value) asm("_ZN4HPHP23f_fb_output_compressionEb");

TypedValue * fg1_fb_output_compression(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_fb_output_compression(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToBooleanInPlace(args-0);
  rv->m_data.num = (fh_fb_output_compression((bool)(args[-0].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_fb_output_compression(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfBoolean) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_fb_output_compression((bool)(args[-0].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_fb_output_compression(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("fb_output_compression", count, 1, 1, 1);
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
void HPHP::f_fb_set_exit_callback(HPHP::Variant const&)
_ZN4HPHP22f_fb_set_exit_callbackERKNS_7VariantE

function => rdi
*/

void fh_fb_set_exit_callback(TypedValue* function) asm("_ZN4HPHP22f_fb_set_exit_callbackERKNS_7VariantE");

TypedValue* fg_fb_set_exit_callback(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv.m_data.num = 0LL;
      rv._count = 0;
      rv.m_type = KindOfNull;
      fh_fb_set_exit_callback((args-0));
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("fb_set_exit_callback", count, 1, 1, 1);
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
HPHP::Array HPHP::f_fb_get_flush_stat()
_ZN4HPHP19f_fb_get_flush_statEv

(return value) => rax
_rv => rdi
*/

Value* fh_fb_get_flush_stat(Value* _rv) asm("_ZN4HPHP19f_fb_get_flush_statEv");

TypedValue* fg_fb_get_flush_stat(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfArray;
      fh_fb_get_flush_stat((Value*)(&(rv)));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("fb_get_flush_stat", 0, 1);
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
long long HPHP::f_fb_get_last_flush_size()
_ZN4HPHP24f_fb_get_last_flush_sizeEv

(return value) => rax
*/

long long fh_fb_get_last_flush_size() asm("_ZN4HPHP24f_fb_get_last_flush_sizeEv");

TypedValue* fg_fb_get_last_flush_size(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfInt64;
      rv.m_data.num = (long long)fh_fb_get_last_flush_size();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("fb_get_last_flush_size", 0, 1);
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
HPHP::Variant HPHP::f_fb_lazy_stat(HPHP::String const&)
_ZN4HPHP14f_fb_lazy_statERKNS_6StringE

(return value) => rax
_rv => rdi
filename => rsi
*/

TypedValue* fh_fb_lazy_stat(TypedValue* _rv, Value* filename) asm("_ZN4HPHP14f_fb_lazy_statERKNS_6StringE");

TypedValue * fg1_fb_lazy_stat(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_fb_lazy_stat(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_fb_lazy_stat((rv), (Value*)(args-0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_fb_lazy_stat(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        fh_fb_lazy_stat((&(rv)), (Value*)(args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_fb_lazy_stat(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("fb_lazy_stat", count, 1, 1, 1);
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
HPHP::Variant HPHP::f_fb_lazy_lstat(HPHP::String const&)
_ZN4HPHP15f_fb_lazy_lstatERKNS_6StringE

(return value) => rax
_rv => rdi
filename => rsi
*/

TypedValue* fh_fb_lazy_lstat(TypedValue* _rv, Value* filename) asm("_ZN4HPHP15f_fb_lazy_lstatERKNS_6StringE");

TypedValue * fg1_fb_lazy_lstat(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_fb_lazy_lstat(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_fb_lazy_lstat((rv), (Value*)(args-0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_fb_lazy_lstat(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        fh_fb_lazy_lstat((&(rv)), (Value*)(args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_fb_lazy_lstat(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("fb_lazy_lstat", count, 1, 1, 1);
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
HPHP::String HPHP::f_fb_lazy_realpath(HPHP::String const&)
_ZN4HPHP18f_fb_lazy_realpathERKNS_6StringE

(return value) => rax
_rv => rdi
filename => rsi
*/

Value* fh_fb_lazy_realpath(Value* _rv, Value* filename) asm("_ZN4HPHP18f_fb_lazy_realpathERKNS_6StringE");

TypedValue * fg1_fb_lazy_realpath(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_fb_lazy_realpath(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToStringInPlace(args-0);
  fh_fb_lazy_realpath((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_fb_lazy_realpath(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_fb_lazy_realpath((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_fb_lazy_realpath(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("fb_lazy_realpath", count, 1, 1, 1);
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
HPHP::String HPHP::f_fb_gc_collect_cycles()
_ZN4HPHP22f_fb_gc_collect_cyclesEv

(return value) => rax
_rv => rdi
*/

Value* fh_fb_gc_collect_cycles(Value* _rv) asm("_ZN4HPHP22f_fb_gc_collect_cyclesEv");

TypedValue* fg_fb_gc_collect_cycles(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfString;
      fh_fb_gc_collect_cycles((Value*)(&(rv)));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("fb_gc_collect_cycles", 0, 1);
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
void HPHP::f_fb_gc_detect_cycles(HPHP::String const&)
_ZN4HPHP21f_fb_gc_detect_cyclesERKNS_6StringE

filename => rdi
*/

void fh_fb_gc_detect_cycles(Value* filename) asm("_ZN4HPHP21f_fb_gc_detect_cyclesERKNS_6StringE");

TypedValue * fg1_fb_gc_detect_cycles(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_fb_gc_detect_cycles(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  tvCastToStringInPlace(args-0);
  fh_fb_gc_detect_cycles((Value*)(args-0));
  return rv;
}

TypedValue* fg_fb_gc_detect_cycles(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        fh_fb_gc_detect_cycles((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_fb_gc_detect_cycles(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("fb_gc_detect_cycles", count, 1, 1, 1);
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

