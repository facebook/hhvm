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
HPHP::Array HPHP::f_spl_classes()
_ZN4HPHP13f_spl_classesEv

(return value) => rax
_rv => rdi
*/

Value* fh_spl_classes(Value* _rv) asm("_ZN4HPHP13f_spl_classesEv");

TypedValue* fg_spl_classes(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfArray;
      fh_spl_classes((Value*)(&(rv)));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("spl_classes", 0, 1);
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
HPHP::String HPHP::f_spl_object_hash(HPHP::Object const&)
_ZN4HPHP17f_spl_object_hashERKNS_6ObjectE

(return value) => rax
_rv => rdi
obj => rsi
*/

Value* fh_spl_object_hash(Value* _rv, Value* obj) asm("_ZN4HPHP17f_spl_object_hashERKNS_6ObjectE");

TypedValue * fg1_spl_object_hash(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_spl_object_hash(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToObjectInPlace(args-0);
  fh_spl_object_hash((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_spl_object_hash(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_spl_object_hash((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_spl_object_hash(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("spl_object_hash", count, 1, 1, 1);
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
HPHP::Variant HPHP::f_hphp_get_this()
_ZN4HPHP15f_hphp_get_thisEv

(return value) => rax
_rv => rdi
*/

TypedValue* fh_hphp_get_this(TypedValue* _rv) asm("_ZN4HPHP15f_hphp_get_thisEv");

TypedValue* fg_hphp_get_this(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      fh_hphp_get_this((&(rv)));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("hphp_get_this", 0, 1);
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
HPHP::Variant HPHP::f_class_implements(HPHP::Variant const&, bool)
_ZN4HPHP18f_class_implementsERKNS_7VariantEb

(return value) => rax
_rv => rdi
obj => rsi
autoload => rdx
*/

TypedValue* fh_class_implements(TypedValue* _rv, TypedValue* obj, bool autoload) asm("_ZN4HPHP18f_class_implementsERKNS_7VariantEb");

TypedValue * fg1_class_implements(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_class_implements(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToBooleanInPlace(args-1);
  fh_class_implements((rv), (args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_class_implements(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfBoolean)) {
        fh_class_implements((&(rv)), (args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_class_implements(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("class_implements", count, 1, 2, 1);
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
HPHP::Variant HPHP::f_class_parents(HPHP::Variant const&, bool)
_ZN4HPHP15f_class_parentsERKNS_7VariantEb

(return value) => rax
_rv => rdi
obj => rsi
autoload => rdx
*/

TypedValue* fh_class_parents(TypedValue* _rv, TypedValue* obj, bool autoload) asm("_ZN4HPHP15f_class_parentsERKNS_7VariantEb");

TypedValue * fg1_class_parents(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_class_parents(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToBooleanInPlace(args-1);
  fh_class_parents((rv), (args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_class_parents(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfBoolean)) {
        fh_class_parents((&(rv)), (args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_class_parents(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("class_parents", count, 1, 2, 1);
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
HPHP::Variant HPHP::f_class_uses(HPHP::Variant const&, bool)
_ZN4HPHP12f_class_usesERKNS_7VariantEb

(return value) => rax
_rv => rdi
obj => rsi
autoload => rdx
*/

TypedValue* fh_class_uses(TypedValue* _rv, TypedValue* obj, bool autoload) asm("_ZN4HPHP12f_class_usesERKNS_7VariantEb");

TypedValue * fg1_class_uses(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_class_uses(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToBooleanInPlace(args-1);
  fh_class_uses((rv), (args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_class_uses(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfBoolean)) {
        fh_class_uses((&(rv)), (args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_class_uses(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("class_uses", count, 1, 2, 1);
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
HPHP::Variant HPHP::f_iterator_apply(HPHP::Variant const&, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP16f_iterator_applyERKNS_7VariantES2_RKNS_5ArrayE

(return value) => rax
_rv => rdi
obj => rsi
func => rdx
params => rcx
*/

TypedValue* fh_iterator_apply(TypedValue* _rv, TypedValue* obj, TypedValue* func, Value* params) asm("_ZN4HPHP16f_iterator_applyERKNS_7VariantES2_RKNS_5ArrayE");

TypedValue * fg1_iterator_apply(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_iterator_apply(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToArrayInPlace(args-2);
  fh_iterator_apply((rv), (args-0), (args-1), (count > 2) ? (Value*)(args-2) : (Value*)(&null_array));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_iterator_apply(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfArray)) {
        fh_iterator_apply((&(rv)), (args-0), (args-1), (count > 2) ? (Value*)(args-2) : (Value*)(&null_array));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_iterator_apply(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("iterator_apply", count, 2, 3, 1);
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
HPHP::Variant HPHP::f_iterator_count(HPHP::Variant const&)
_ZN4HPHP16f_iterator_countERKNS_7VariantE

(return value) => rax
_rv => rdi
obj => rsi
*/

TypedValue* fh_iterator_count(TypedValue* _rv, TypedValue* obj) asm("_ZN4HPHP16f_iterator_countERKNS_7VariantE");

TypedValue* fg_iterator_count(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      fh_iterator_count((&(rv)), (args-0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("iterator_count", count, 1, 1, 1);
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
HPHP::Variant HPHP::f_iterator_to_array(HPHP::Variant const&, bool)
_ZN4HPHP19f_iterator_to_arrayERKNS_7VariantEb

(return value) => rax
_rv => rdi
obj => rsi
use_keys => rdx
*/

TypedValue* fh_iterator_to_array(TypedValue* _rv, TypedValue* obj, bool use_keys) asm("_ZN4HPHP19f_iterator_to_arrayERKNS_7VariantEb");

TypedValue * fg1_iterator_to_array(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_iterator_to_array(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToBooleanInPlace(args-1);
  fh_iterator_to_array((rv), (args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_iterator_to_array(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfBoolean)) {
        fh_iterator_to_array((&(rv)), (args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_iterator_to_array(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("iterator_to_array", count, 1, 2, 1);
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
void HPHP::f_spl_autoload_call(HPHP::String const&)
_ZN4HPHP19f_spl_autoload_callERKNS_6StringE

class_name => rdi
*/

void fh_spl_autoload_call(Value* class_name) asm("_ZN4HPHP19f_spl_autoload_callERKNS_6StringE");

TypedValue * fg1_spl_autoload_call(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_spl_autoload_call(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  tvCastToStringInPlace(args-0);
  fh_spl_autoload_call((Value*)(args-0));
  return rv;
}

TypedValue* fg_spl_autoload_call(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        fh_spl_autoload_call((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_spl_autoload_call(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("spl_autoload_call", count, 1, 1, 1);
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
HPHP::String HPHP::f_spl_autoload_extensions(HPHP::String const&)
_ZN4HPHP25f_spl_autoload_extensionsERKNS_6StringE

(return value) => rax
_rv => rdi
file_extensions => rsi
*/

Value* fh_spl_autoload_extensions(Value* _rv, Value* file_extensions) asm("_ZN4HPHP25f_spl_autoload_extensionsERKNS_6StringE");

TypedValue * fg1_spl_autoload_extensions(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_spl_autoload_extensions(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToStringInPlace(args-0);
  fh_spl_autoload_extensions((Value*)(rv), (count > 0) ? (Value*)(args-0) : (Value*)(&null_string));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_spl_autoload_extensions(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      if ((count <= 0 || IS_STRING_TYPE((args-0)->m_type))) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_spl_autoload_extensions((Value*)(&(rv)), (count > 0) ? (Value*)(args-0) : (Value*)(&null_string));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_spl_autoload_extensions(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("spl_autoload_extensions", 1, 1);
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
HPHP::Variant HPHP::f_spl_autoload_functions()
_ZN4HPHP24f_spl_autoload_functionsEv

(return value) => rax
_rv => rdi
*/

TypedValue* fh_spl_autoload_functions(TypedValue* _rv) asm("_ZN4HPHP24f_spl_autoload_functionsEv");

TypedValue* fg_spl_autoload_functions(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      fh_spl_autoload_functions((&(rv)));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("spl_autoload_functions", 0, 1);
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
bool HPHP::f_spl_autoload_register(HPHP::Variant const&, bool, bool)
_ZN4HPHP23f_spl_autoload_registerERKNS_7VariantEbb

(return value) => rax
autoload_function => rdi
throws => rsi
prepend => rdx
*/

bool fh_spl_autoload_register(TypedValue* autoload_function, bool throws, bool prepend) asm("_ZN4HPHP23f_spl_autoload_registerERKNS_7VariantEbb");

TypedValue * fg1_spl_autoload_register(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_spl_autoload_register(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-2);
    }
  case 2:
    if ((args-1)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-1);
    }
  case 1:
  case 0:
    break;
  }
  rv->m_data.num = (fh_spl_autoload_register((count > 0) ? (args-0) : (TypedValue*)(&null_variant), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_spl_autoload_register(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfBoolean) && (count <= 1 || (args-1)->m_type == KindOfBoolean)) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_spl_autoload_register((count > 0) ? (args-0) : (TypedValue*)(&null_variant), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_spl_autoload_register(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("spl_autoload_register", 3, 1);
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
bool HPHP::f_spl_autoload_unregister(HPHP::Variant const&)
_ZN4HPHP25f_spl_autoload_unregisterERKNS_7VariantE

(return value) => rax
autoload_function => rdi
*/

bool fh_spl_autoload_unregister(TypedValue* autoload_function) asm("_ZN4HPHP25f_spl_autoload_unregisterERKNS_7VariantE");

TypedValue* fg_spl_autoload_unregister(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv._count = 0;
      rv.m_type = KindOfBoolean;
      rv.m_data.num = (fh_spl_autoload_unregister((args-0))) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("spl_autoload_unregister", count, 1, 1, 1);
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
void HPHP::f_spl_autoload(HPHP::String const&, HPHP::String const&)
_ZN4HPHP14f_spl_autoloadERKNS_6StringES2_

class_name => rdi
file_extensions => rsi
*/

void fh_spl_autoload(Value* class_name, Value* file_extensions) asm("_ZN4HPHP14f_spl_autoloadERKNS_6StringES2_");

TypedValue * fg1_spl_autoload(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_spl_autoload(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
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
  fh_spl_autoload((Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_string));
  return rv;
}

TypedValue* fg_spl_autoload(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && IS_STRING_TYPE((args-0)->m_type)) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        fh_spl_autoload((Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_string));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_spl_autoload(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("spl_autoload", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}




} // !HPHP

