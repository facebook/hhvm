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

HPHP::VM::Instance* new_Vector_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_Vector) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_Vector(ObjectStaticCallbacks::encodeVMClass(cls));
  return inst;
}

/*
void HPHP::c_Vector::t___construct()
_ZN4HPHP8c_Vector13t___constructEv

this_ => rdi
*/

void th_6Vector___construct(ObjectData* this_) asm("_ZN4HPHP8c_Vector13t___constructEv");

TypedValue* tg_6Vector___construct(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        th_6Vector___construct((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Vector::__construct", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Vector::__construct");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_Vector::t_isempty()
_ZN4HPHP8c_Vector9t_isemptyEv

(return value) => rax
this_ => rdi
*/

bool th_6Vector_isEmpty(ObjectData* this_) asm("_ZN4HPHP8c_Vector9t_isemptyEv");

TypedValue* tg_6Vector_isEmpty(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_6Vector_isEmpty((this_))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Vector::isEmpty", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Vector::isEmpty");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
long long HPHP::c_Vector::t_count()
_ZN4HPHP8c_Vector7t_countEv

(return value) => rax
this_ => rdi
*/

long long th_6Vector_count(ObjectData* this_) asm("_ZN4HPHP8c_Vector7t_countEv");

TypedValue* tg_6Vector_count(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (long long)th_6Vector_count((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Vector::count", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Vector::count");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_Vector::t_at(HPHP::Variant const&)
_ZN4HPHP8c_Vector4t_atERKNS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
key => rdx
*/

TypedValue* th_6Vector_at(TypedValue* _rv, ObjectData* this_, TypedValue* key) asm("_ZN4HPHP8c_Vector4t_atERKNS_7VariantE");

TypedValue* tg_6Vector_at(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        th_6Vector_at((&(rv)), (this_), (args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("Vector::at", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("Vector::at");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_Vector::t_get(HPHP::Variant const&)
_ZN4HPHP8c_Vector5t_getERKNS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
key => rdx
*/

TypedValue* th_6Vector_get(TypedValue* _rv, ObjectData* this_, TypedValue* key) asm("_ZN4HPHP8c_Vector5t_getERKNS_7VariantE");

TypedValue* tg_6Vector_get(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        th_6Vector_get((&(rv)), (this_), (args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("Vector::get", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("Vector::get");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_Vector::t_put(HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP8c_Vector5t_putERKNS_7VariantES3_

(return value) => rax
_rv => rdi
this_ => rsi
key => rdx
value => rcx
*/

Value* th_6Vector_put(Value* _rv, ObjectData* this_, TypedValue* key, TypedValue* value) asm("_ZN4HPHP8c_Vector5t_putERKNS_7VariantES3_");

TypedValue* tg_6Vector_put(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        rv._count = 0;
        rv.m_type = KindOfObject;
        th_6Vector_put((Value*)(&(rv)), (this_), (args-0), (args-1));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("Vector::put", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("Vector::put");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_Vector::t_clear()
_ZN4HPHP8c_Vector7t_clearEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_6Vector_clear(Value* _rv, ObjectData* this_) asm("_ZN4HPHP8c_Vector7t_clearEv");

TypedValue* tg_6Vector_clear(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfObject;
        th_6Vector_clear((Value*)(&(rv)), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Vector::clear", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Vector::clear");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_Vector::t_contains(HPHP::Variant const&)
_ZN4HPHP8c_Vector10t_containsERKNS_7VariantE

(return value) => rax
this_ => rdi
key => rsi
*/

bool th_6Vector_contains(ObjectData* this_, TypedValue* key) asm("_ZN4HPHP8c_Vector10t_containsERKNS_7VariantE");

TypedValue* tg_6Vector_contains(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_6Vector_contains((this_), (args-0))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("Vector::contains", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("Vector::contains");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_Vector::t_append(HPHP::Variant const&)
_ZN4HPHP8c_Vector8t_appendERKNS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
val => rdx
*/

Value* th_6Vector_append(Value* _rv, ObjectData* this_, TypedValue* val) asm("_ZN4HPHP8c_Vector8t_appendERKNS_7VariantE");

TypedValue* tg_6Vector_append(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv._count = 0;
        rv.m_type = KindOfObject;
        th_6Vector_append((Value*)(&(rv)), (this_), (args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("Vector::append", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("Vector::append");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_Vector::t_add(HPHP::Variant const&)
_ZN4HPHP8c_Vector5t_addERKNS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
val => rdx
*/

Value* th_6Vector_add(Value* _rv, ObjectData* this_, TypedValue* val) asm("_ZN4HPHP8c_Vector5t_addERKNS_7VariantE");

TypedValue* tg_6Vector_add(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv._count = 0;
        rv.m_type = KindOfObject;
        th_6Vector_add((Value*)(&(rv)), (this_), (args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("Vector::add", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("Vector::add");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_Vector::t_pop()
_ZN4HPHP8c_Vector5t_popEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_6Vector_pop(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP8c_Vector5t_popEv");

TypedValue* tg_6Vector_pop(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_6Vector_pop((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Vector::pop", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Vector::pop");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
void HPHP::c_Vector::t_resize(HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP8c_Vector8t_resizeERKNS_7VariantES3_

this_ => rdi
sz => rsi
value => rdx
*/

void th_6Vector_resize(ObjectData* this_, TypedValue* sz, TypedValue* value) asm("_ZN4HPHP8c_Vector8t_resizeERKNS_7VariantES3_");

TypedValue* tg_6Vector_resize(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        th_6Vector_resize((this_), (args-0), (args-1));
        frame_free_locals_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("Vector::resize", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("Vector::resize");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Array HPHP::c_Vector::t_toarray()
_ZN4HPHP8c_Vector9t_toarrayEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_6Vector_toArray(Value* _rv, ObjectData* this_) asm("_ZN4HPHP8c_Vector9t_toarrayEv");

TypedValue* tg_6Vector_toArray(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfArray;
        th_6Vector_toArray((Value*)(&(rv)), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Vector::toArray", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Vector::toArray");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_Vector::t_getiterator()
_ZN4HPHP8c_Vector13t_getiteratorEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_6Vector_getIterator(Value* _rv, ObjectData* this_) asm("_ZN4HPHP8c_Vector13t_getiteratorEv");

TypedValue* tg_6Vector_getIterator(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfObject;
        th_6Vector_getIterator((Value*)(&(rv)), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Vector::getIterator", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Vector::getIterator");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
void HPHP::c_Vector::t_sort(HPHP::Variant const&)
_ZN4HPHP8c_Vector6t_sortERKNS_7VariantE

this_ => rdi
col => rsi
*/

void th_6Vector_sort(ObjectData* this_, TypedValue* col) asm("_ZN4HPHP8c_Vector6t_sortERKNS_7VariantE");

TypedValue* tg_6Vector_sort(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count <= 1LL) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        Variant defVal0;
        th_6Vector_sort((this_), (count > 0) ? (args-0) : (TypedValue*)(&defVal0));
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Vector::sort", 1, 1);
      }
    } else {
      throw_instance_method_fatal("Vector::sort");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
void HPHP::c_Vector::t_reverse()
_ZN4HPHP8c_Vector9t_reverseEv

this_ => rdi
*/

void th_6Vector_reverse(ObjectData* this_) asm("_ZN4HPHP8c_Vector9t_reverseEv");

TypedValue* tg_6Vector_reverse(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        th_6Vector_reverse((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Vector::reverse", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Vector::reverse");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
void HPHP::c_Vector::t_splice(HPHP::Variant const&, HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP8c_Vector8t_spliceERKNS_7VariantES3_S3_

this_ => rdi
offset => rsi
len => rdx
replacement => rcx
*/

void th_6Vector_splice(ObjectData* this_, TypedValue* offset, TypedValue* len, TypedValue* replacement) asm("_ZN4HPHP8c_Vector8t_spliceERKNS_7VariantES3_S3_");

TypedValue* tg_6Vector_splice(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL && count <= 3LL) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        Variant defVal1;
        Variant defVal2;
        th_6Vector_splice((this_), (args-0), (count > 1) ? (args-1) : (TypedValue*)(&defVal1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
        frame_free_locals_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("Vector::splice", count, 1, 3, 1);
      }
    } else {
      throw_instance_method_fatal("Vector::splice");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
long long HPHP::c_Vector::t_linearsearch(HPHP::Variant const&)
_ZN4HPHP8c_Vector14t_linearsearchERKNS_7VariantE

(return value) => rax
this_ => rdi
search_value => rsi
*/

long long th_6Vector_linearSearch(ObjectData* this_, TypedValue* search_value) asm("_ZN4HPHP8c_Vector14t_linearsearchERKNS_7VariantE");

TypedValue* tg_6Vector_linearSearch(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (long long)th_6Vector_linearSearch((this_), (args-0));
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("Vector::linearSearch", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("Vector::linearSearch");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
void HPHP::c_Vector::t_shuffle()
_ZN4HPHP8c_Vector9t_shuffleEv

this_ => rdi
*/

void th_6Vector_shuffle(ObjectData* this_) asm("_ZN4HPHP8c_Vector9t_shuffleEv");

TypedValue* tg_6Vector_shuffle(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        th_6Vector_shuffle((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Vector::shuffle", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Vector::shuffle");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::String HPHP::c_Vector::t___tostring()
_ZN4HPHP8c_Vector12t___tostringEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_6Vector___toString(Value* _rv, ObjectData* this_) asm("_ZN4HPHP8c_Vector12t___tostringEv");

TypedValue* tg_6Vector___toString(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfString;
        th_6Vector___toString((Value*)(&(rv)), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Vector::__toString", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Vector::__toString");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_Vector::t___get(HPHP::Variant)
_ZN4HPHP8c_Vector7t___getENS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
name => rdx
*/

TypedValue* th_6Vector___get(TypedValue* _rv, ObjectData* this_, TypedValue* name) asm("_ZN4HPHP8c_Vector7t___getENS_7VariantE");

TypedValue* tg_6Vector___get(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        th_6Vector___get((&(rv)), (this_), (args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("Vector::__get", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("Vector::__get");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_Vector::t___set(HPHP::Variant, HPHP::Variant)
_ZN4HPHP8c_Vector7t___setENS_7VariantES1_

(return value) => rax
_rv => rdi
this_ => rsi
name => rdx
value => rcx
*/

TypedValue* th_6Vector___set(TypedValue* _rv, ObjectData* this_, TypedValue* name, TypedValue* value) asm("_ZN4HPHP8c_Vector7t___setENS_7VariantES1_");

TypedValue* tg_6Vector___set(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        th_6Vector___set((&(rv)), (this_), (args-0), (args-1));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("Vector::__set", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("Vector::__set");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_Vector::t___isset(HPHP::Variant)
_ZN4HPHP8c_Vector9t___issetENS_7VariantE

(return value) => rax
this_ => rdi
name => rsi
*/

bool th_6Vector___isset(ObjectData* this_, TypedValue* name) asm("_ZN4HPHP8c_Vector9t___issetENS_7VariantE");

TypedValue* tg_6Vector___isset(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_6Vector___isset((this_), (args-0))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("Vector::__isset", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("Vector::__isset");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_Vector::t___unset(HPHP::Variant)
_ZN4HPHP8c_Vector9t___unsetENS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
name => rdx
*/

TypedValue* th_6Vector___unset(TypedValue* _rv, ObjectData* this_, TypedValue* name) asm("_ZN4HPHP8c_Vector9t___unsetENS_7VariantE");

TypedValue* tg_6Vector___unset(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        th_6Vector___unset((&(rv)), (this_), (args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("Vector::__unset", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("Vector::__unset");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_Vector::ti_fromarray(char const*, HPHP::Variant const&)
_ZN4HPHP8c_Vector12ti_fromarrayEPKcRKNS_7VariantE

(return value) => rax
_rv => rdi
cls_ => rsi
arr => rdx
*/

Value* th_6Vector_fromArray(Value* _rv, char const* cls_, TypedValue* arr) asm("_ZN4HPHP8c_Vector12ti_fromarrayEPKcRKNS_7VariantE");

TypedValue* tg_6Vector_fromArray(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv._count = 0;
      rv.m_type = KindOfObject;
      th_6Vector_fromArray((Value*)(&(rv)), ("Vector"), (args-0));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("Vector::fromArray", count, 1, 1, 1);
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
HPHP::Object HPHP::c_Vector::ti_fromvector(char const*, HPHP::Variant const&)
_ZN4HPHP8c_Vector13ti_fromvectorEPKcRKNS_7VariantE

(return value) => rax
_rv => rdi
cls_ => rsi
vec => rdx
*/

Value* th_6Vector_fromVector(Value* _rv, char const* cls_, TypedValue* vec) asm("_ZN4HPHP8c_Vector13ti_fromvectorEPKcRKNS_7VariantE");

TypedValue* tg_6Vector_fromVector(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv._count = 0;
      rv.m_type = KindOfObject;
      th_6Vector_fromVector((Value*)(&(rv)), ("Vector"), (args-0));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("Vector::fromVector", count, 1, 1, 1);
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
HPHP::Object HPHP::c_Vector::ti_slice(char const*, HPHP::Variant const&, HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP8c_Vector8ti_sliceEPKcRKNS_7VariantES5_S5_

(return value) => rax
_rv => rdi
cls_ => rsi
vec => rdx
offset => rcx
len => r8
*/

Value* th_6Vector_slice(Value* _rv, char const* cls_, TypedValue* vec, TypedValue* offset, TypedValue* len) asm("_ZN4HPHP8c_Vector8ti_sliceEPKcRKNS_7VariantES5_S5_");

TypedValue* tg_6Vector_slice(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      rv._count = 0;
      rv.m_type = KindOfObject;
      Variant defVal2;
      th_6Vector_slice((Value*)(&(rv)), ("Vector"), (args-0), (args-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 3);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("Vector::slice", count, 2, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

HPHP::VM::Instance* new_VectorIterator_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_VectorIterator) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_VectorIterator(ObjectStaticCallbacks::encodeVMClass(cls));
  return inst;
}

/*
void HPHP::c_VectorIterator::t___construct()
_ZN4HPHP16c_VectorIterator13t___constructEv

this_ => rdi
*/

void th_14VectorIterator___construct(ObjectData* this_) asm("_ZN4HPHP16c_VectorIterator13t___constructEv");

TypedValue* tg_14VectorIterator___construct(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        th_14VectorIterator___construct((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("VectorIterator::__construct", 0, 1);
      }
    } else {
      throw_instance_method_fatal("VectorIterator::__construct");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_VectorIterator::t_current()
_ZN4HPHP16c_VectorIterator9t_currentEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_14VectorIterator_current(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP16c_VectorIterator9t_currentEv");

TypedValue* tg_14VectorIterator_current(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_14VectorIterator_current((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("VectorIterator::current", 0, 1);
      }
    } else {
      throw_instance_method_fatal("VectorIterator::current");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_VectorIterator::t_key()
_ZN4HPHP16c_VectorIterator5t_keyEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_14VectorIterator_key(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP16c_VectorIterator5t_keyEv");

TypedValue* tg_14VectorIterator_key(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_14VectorIterator_key((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("VectorIterator::key", 0, 1);
      }
    } else {
      throw_instance_method_fatal("VectorIterator::key");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_VectorIterator::t_valid()
_ZN4HPHP16c_VectorIterator7t_validEv

(return value) => rax
this_ => rdi
*/

bool th_14VectorIterator_valid(ObjectData* this_) asm("_ZN4HPHP16c_VectorIterator7t_validEv");

TypedValue* tg_14VectorIterator_valid(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_14VectorIterator_valid((this_))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("VectorIterator::valid", 0, 1);
      }
    } else {
      throw_instance_method_fatal("VectorIterator::valid");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
void HPHP::c_VectorIterator::t_next()
_ZN4HPHP16c_VectorIterator6t_nextEv

this_ => rdi
*/

void th_14VectorIterator_next(ObjectData* this_) asm("_ZN4HPHP16c_VectorIterator6t_nextEv");

TypedValue* tg_14VectorIterator_next(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        th_14VectorIterator_next((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("VectorIterator::next", 0, 1);
      }
    } else {
      throw_instance_method_fatal("VectorIterator::next");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
void HPHP::c_VectorIterator::t_rewind()
_ZN4HPHP16c_VectorIterator8t_rewindEv

this_ => rdi
*/

void th_14VectorIterator_rewind(ObjectData* this_) asm("_ZN4HPHP16c_VectorIterator8t_rewindEv");

TypedValue* tg_14VectorIterator_rewind(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        th_14VectorIterator_rewind((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("VectorIterator::rewind", 0, 1);
      }
    } else {
      throw_instance_method_fatal("VectorIterator::rewind");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

HPHP::VM::Instance* new_Map_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_Map) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_Map(ObjectStaticCallbacks::encodeVMClass(cls));
  return inst;
}

/*
void HPHP::c_Map::t___construct()
_ZN4HPHP5c_Map13t___constructEv

this_ => rdi
*/

void th_3Map___construct(ObjectData* this_) asm("_ZN4HPHP5c_Map13t___constructEv");

TypedValue* tg_3Map___construct(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        th_3Map___construct((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Map::__construct", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Map::__construct");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_Map::t_isempty()
_ZN4HPHP5c_Map9t_isemptyEv

(return value) => rax
this_ => rdi
*/

bool th_3Map_isEmpty(ObjectData* this_) asm("_ZN4HPHP5c_Map9t_isemptyEv");

TypedValue* tg_3Map_isEmpty(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_3Map_isEmpty((this_))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Map::isEmpty", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Map::isEmpty");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
long long HPHP::c_Map::t_count()
_ZN4HPHP5c_Map7t_countEv

(return value) => rax
this_ => rdi
*/

long long th_3Map_count(ObjectData* this_) asm("_ZN4HPHP5c_Map7t_countEv");

TypedValue* tg_3Map_count(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (long long)th_3Map_count((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Map::count", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Map::count");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_Map::t_at(HPHP::Variant const&)
_ZN4HPHP5c_Map4t_atERKNS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
key => rdx
*/

TypedValue* th_3Map_at(TypedValue* _rv, ObjectData* this_, TypedValue* key) asm("_ZN4HPHP5c_Map4t_atERKNS_7VariantE");

TypedValue* tg_3Map_at(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        th_3Map_at((&(rv)), (this_), (args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("Map::at", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("Map::at");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_Map::t_get(HPHP::Variant const&)
_ZN4HPHP5c_Map5t_getERKNS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
key => rdx
*/

TypedValue* th_3Map_get(TypedValue* _rv, ObjectData* this_, TypedValue* key) asm("_ZN4HPHP5c_Map5t_getERKNS_7VariantE");

TypedValue* tg_3Map_get(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        th_3Map_get((&(rv)), (this_), (args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("Map::get", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("Map::get");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_Map::t_put(HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP5c_Map5t_putERKNS_7VariantES3_

(return value) => rax
_rv => rdi
this_ => rsi
key => rdx
value => rcx
*/

Value* th_3Map_put(Value* _rv, ObjectData* this_, TypedValue* key, TypedValue* value) asm("_ZN4HPHP5c_Map5t_putERKNS_7VariantES3_");

TypedValue* tg_3Map_put(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        rv._count = 0;
        rv.m_type = KindOfObject;
        th_3Map_put((Value*)(&(rv)), (this_), (args-0), (args-1));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("Map::put", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("Map::put");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_Map::t_clear()
_ZN4HPHP5c_Map7t_clearEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_3Map_clear(Value* _rv, ObjectData* this_) asm("_ZN4HPHP5c_Map7t_clearEv");

TypedValue* tg_3Map_clear(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfObject;
        th_3Map_clear((Value*)(&(rv)), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Map::clear", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Map::clear");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_Map::t_contains(HPHP::Variant const&)
_ZN4HPHP5c_Map10t_containsERKNS_7VariantE

(return value) => rax
this_ => rdi
key => rsi
*/

bool th_3Map_contains(ObjectData* this_, TypedValue* key) asm("_ZN4HPHP5c_Map10t_containsERKNS_7VariantE");

TypedValue* tg_3Map_contains(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_3Map_contains((this_), (args-0))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("Map::contains", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("Map::contains");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_Map::t_remove(HPHP::Variant const&)
_ZN4HPHP5c_Map8t_removeERKNS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
key => rdx
*/

Value* th_3Map_remove(Value* _rv, ObjectData* this_, TypedValue* key) asm("_ZN4HPHP5c_Map8t_removeERKNS_7VariantE");

TypedValue* tg_3Map_remove(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv._count = 0;
        rv.m_type = KindOfObject;
        th_3Map_remove((Value*)(&(rv)), (this_), (args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("Map::remove", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("Map::remove");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_Map::t_discard(HPHP::Variant const&)
_ZN4HPHP5c_Map9t_discardERKNS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
key => rdx
*/

Value* th_3Map_discard(Value* _rv, ObjectData* this_, TypedValue* key) asm("_ZN4HPHP5c_Map9t_discardERKNS_7VariantE");

TypedValue* tg_3Map_discard(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv._count = 0;
        rv.m_type = KindOfObject;
        th_3Map_discard((Value*)(&(rv)), (this_), (args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("Map::discard", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("Map::discard");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Array HPHP::c_Map::t_toarray()
_ZN4HPHP5c_Map9t_toarrayEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_3Map_toArray(Value* _rv, ObjectData* this_) asm("_ZN4HPHP5c_Map9t_toarrayEv");

TypedValue* tg_3Map_toArray(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfArray;
        th_3Map_toArray((Value*)(&(rv)), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Map::toArray", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Map::toArray");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Array HPHP::c_Map::t_copyasarray()
_ZN4HPHP5c_Map13t_copyasarrayEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_3Map_copyAsArray(Value* _rv, ObjectData* this_) asm("_ZN4HPHP5c_Map13t_copyasarrayEv");

TypedValue* tg_3Map_copyAsArray(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfArray;
        th_3Map_copyAsArray((Value*)(&(rv)), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Map::copyAsArray", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Map::copyAsArray");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Array HPHP::c_Map::t_tokeysarray()
_ZN4HPHP5c_Map13t_tokeysarrayEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_3Map_toKeysArray(Value* _rv, ObjectData* this_) asm("_ZN4HPHP5c_Map13t_tokeysarrayEv");

TypedValue* tg_3Map_toKeysArray(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfArray;
        th_3Map_toKeysArray((Value*)(&(rv)), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Map::toKeysArray", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Map::toKeysArray");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_Map::t_values()
_ZN4HPHP5c_Map8t_valuesEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_3Map_values(Value* _rv, ObjectData* this_) asm("_ZN4HPHP5c_Map8t_valuesEv");

TypedValue* tg_3Map_values(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfObject;
        th_3Map_values((Value*)(&(rv)), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Map::values", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Map::values");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Array HPHP::c_Map::t_tovaluesarray()
_ZN4HPHP5c_Map15t_tovaluesarrayEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_3Map_toValuesArray(Value* _rv, ObjectData* this_) asm("_ZN4HPHP5c_Map15t_tovaluesarrayEv");

TypedValue* tg_3Map_toValuesArray(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfArray;
        th_3Map_toValuesArray((Value*)(&(rv)), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Map::toValuesArray", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Map::toValuesArray");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_Map::t_updatefromarray(HPHP::Variant const&)
_ZN4HPHP5c_Map17t_updatefromarrayERKNS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
arr => rdx
*/

Value* th_3Map_updateFromArray(Value* _rv, ObjectData* this_, TypedValue* arr) asm("_ZN4HPHP5c_Map17t_updatefromarrayERKNS_7VariantE");

TypedValue* tg_3Map_updateFromArray(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv._count = 0;
        rv.m_type = KindOfObject;
        th_3Map_updateFromArray((Value*)(&(rv)), (this_), (args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("Map::updateFromArray", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("Map::updateFromArray");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_Map::t_updatefromiterable(HPHP::Variant const&)
_ZN4HPHP5c_Map20t_updatefromiterableERKNS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
it => rdx
*/

Value* th_3Map_updateFromIterable(Value* _rv, ObjectData* this_, TypedValue* it) asm("_ZN4HPHP5c_Map20t_updatefromiterableERKNS_7VariantE");

TypedValue* tg_3Map_updateFromIterable(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv._count = 0;
        rv.m_type = KindOfObject;
        th_3Map_updateFromIterable((Value*)(&(rv)), (this_), (args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("Map::updateFromIterable", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("Map::updateFromIterable");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_Map::t_differencebykey(HPHP::Variant const&)
_ZN4HPHP5c_Map17t_differencebykeyERKNS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
it => rdx
*/

Value* th_3Map_differenceByKey(Value* _rv, ObjectData* this_, TypedValue* it) asm("_ZN4HPHP5c_Map17t_differencebykeyERKNS_7VariantE");

TypedValue* tg_3Map_differenceByKey(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv._count = 0;
        rv.m_type = KindOfObject;
        th_3Map_differenceByKey((Value*)(&(rv)), (this_), (args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("Map::differenceByKey", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("Map::differenceByKey");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_Map::t_getiterator()
_ZN4HPHP5c_Map13t_getiteratorEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_3Map_getIterator(Value* _rv, ObjectData* this_) asm("_ZN4HPHP5c_Map13t_getiteratorEv");

TypedValue* tg_3Map_getIterator(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfObject;
        th_3Map_getIterator((Value*)(&(rv)), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Map::getIterator", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Map::getIterator");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::String HPHP::c_Map::t___tostring()
_ZN4HPHP5c_Map12t___tostringEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_3Map___toString(Value* _rv, ObjectData* this_) asm("_ZN4HPHP5c_Map12t___tostringEv");

TypedValue* tg_3Map___toString(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfString;
        th_3Map___toString((Value*)(&(rv)), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Map::__toString", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Map::__toString");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_Map::t___get(HPHP::Variant)
_ZN4HPHP5c_Map7t___getENS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
name => rdx
*/

TypedValue* th_3Map___get(TypedValue* _rv, ObjectData* this_, TypedValue* name) asm("_ZN4HPHP5c_Map7t___getENS_7VariantE");

TypedValue* tg_3Map___get(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        th_3Map___get((&(rv)), (this_), (args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("Map::__get", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("Map::__get");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_Map::t___set(HPHP::Variant, HPHP::Variant)
_ZN4HPHP5c_Map7t___setENS_7VariantES1_

(return value) => rax
_rv => rdi
this_ => rsi
name => rdx
value => rcx
*/

TypedValue* th_3Map___set(TypedValue* _rv, ObjectData* this_, TypedValue* name, TypedValue* value) asm("_ZN4HPHP5c_Map7t___setENS_7VariantES1_");

TypedValue* tg_3Map___set(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        th_3Map___set((&(rv)), (this_), (args-0), (args-1));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("Map::__set", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("Map::__set");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_Map::t___isset(HPHP::Variant)
_ZN4HPHP5c_Map9t___issetENS_7VariantE

(return value) => rax
this_ => rdi
name => rsi
*/

bool th_3Map___isset(ObjectData* this_, TypedValue* name) asm("_ZN4HPHP5c_Map9t___issetENS_7VariantE");

TypedValue* tg_3Map___isset(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_3Map___isset((this_), (args-0))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("Map::__isset", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("Map::__isset");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_Map::t___unset(HPHP::Variant)
_ZN4HPHP5c_Map9t___unsetENS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
name => rdx
*/

TypedValue* th_3Map___unset(TypedValue* _rv, ObjectData* this_, TypedValue* name) asm("_ZN4HPHP5c_Map9t___unsetENS_7VariantE");

TypedValue* tg_3Map___unset(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        th_3Map___unset((&(rv)), (this_), (args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("Map::__unset", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("Map::__unset");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_Map::ti_fromarray(char const*, HPHP::Variant const&)
_ZN4HPHP5c_Map12ti_fromarrayEPKcRKNS_7VariantE

(return value) => rax
_rv => rdi
cls_ => rsi
mp => rdx
*/

Value* th_3Map_fromArray(Value* _rv, char const* cls_, TypedValue* mp) asm("_ZN4HPHP5c_Map12ti_fromarrayEPKcRKNS_7VariantE");

TypedValue* tg_3Map_fromArray(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv._count = 0;
      rv.m_type = KindOfObject;
      th_3Map_fromArray((Value*)(&(rv)), ("Map"), (args-0));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("Map::fromArray", count, 1, 1, 1);
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
HPHP::Object HPHP::c_Map::ti_fromiterable(char const*, HPHP::Variant const&)
_ZN4HPHP5c_Map15ti_fromiterableEPKcRKNS_7VariantE

(return value) => rax
_rv => rdi
cls_ => rsi
mp => rdx
*/

Value* th_3Map_fromIterable(Value* _rv, char const* cls_, TypedValue* mp) asm("_ZN4HPHP5c_Map15ti_fromiterableEPKcRKNS_7VariantE");

TypedValue* tg_3Map_fromIterable(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv._count = 0;
      rv.m_type = KindOfObject;
      th_3Map_fromIterable((Value*)(&(rv)), ("Map"), (args-0));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("Map::fromIterable", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

HPHP::VM::Instance* new_MapIterator_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_MapIterator) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_MapIterator(ObjectStaticCallbacks::encodeVMClass(cls));
  return inst;
}

/*
void HPHP::c_MapIterator::t___construct()
_ZN4HPHP13c_MapIterator13t___constructEv

this_ => rdi
*/

void th_11MapIterator___construct(ObjectData* this_) asm("_ZN4HPHP13c_MapIterator13t___constructEv");

TypedValue* tg_11MapIterator___construct(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        th_11MapIterator___construct((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("MapIterator::__construct", 0, 1);
      }
    } else {
      throw_instance_method_fatal("MapIterator::__construct");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_MapIterator::t_current()
_ZN4HPHP13c_MapIterator9t_currentEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_11MapIterator_current(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP13c_MapIterator9t_currentEv");

TypedValue* tg_11MapIterator_current(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_11MapIterator_current((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("MapIterator::current", 0, 1);
      }
    } else {
      throw_instance_method_fatal("MapIterator::current");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_MapIterator::t_key()
_ZN4HPHP13c_MapIterator5t_keyEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_11MapIterator_key(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP13c_MapIterator5t_keyEv");

TypedValue* tg_11MapIterator_key(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_11MapIterator_key((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("MapIterator::key", 0, 1);
      }
    } else {
      throw_instance_method_fatal("MapIterator::key");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_MapIterator::t_valid()
_ZN4HPHP13c_MapIterator7t_validEv

(return value) => rax
this_ => rdi
*/

bool th_11MapIterator_valid(ObjectData* this_) asm("_ZN4HPHP13c_MapIterator7t_validEv");

TypedValue* tg_11MapIterator_valid(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_11MapIterator_valid((this_))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("MapIterator::valid", 0, 1);
      }
    } else {
      throw_instance_method_fatal("MapIterator::valid");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
void HPHP::c_MapIterator::t_next()
_ZN4HPHP13c_MapIterator6t_nextEv

this_ => rdi
*/

void th_11MapIterator_next(ObjectData* this_) asm("_ZN4HPHP13c_MapIterator6t_nextEv");

TypedValue* tg_11MapIterator_next(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        th_11MapIterator_next((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("MapIterator::next", 0, 1);
      }
    } else {
      throw_instance_method_fatal("MapIterator::next");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
void HPHP::c_MapIterator::t_rewind()
_ZN4HPHP13c_MapIterator8t_rewindEv

this_ => rdi
*/

void th_11MapIterator_rewind(ObjectData* this_) asm("_ZN4HPHP13c_MapIterator8t_rewindEv");

TypedValue* tg_11MapIterator_rewind(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        th_11MapIterator_rewind((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("MapIterator::rewind", 0, 1);
      }
    } else {
      throw_instance_method_fatal("MapIterator::rewind");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

HPHP::VM::Instance* new_StableMap_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_StableMap) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_StableMap(ObjectStaticCallbacks::encodeVMClass(cls));
  return inst;
}

/*
void HPHP::c_StableMap::t___construct()
_ZN4HPHP11c_StableMap13t___constructEv

this_ => rdi
*/

void th_9StableMap___construct(ObjectData* this_) asm("_ZN4HPHP11c_StableMap13t___constructEv");

TypedValue* tg_9StableMap___construct(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        th_9StableMap___construct((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("StableMap::__construct", 0, 1);
      }
    } else {
      throw_instance_method_fatal("StableMap::__construct");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_StableMap::t_isempty()
_ZN4HPHP11c_StableMap9t_isemptyEv

(return value) => rax
this_ => rdi
*/

bool th_9StableMap_isEmpty(ObjectData* this_) asm("_ZN4HPHP11c_StableMap9t_isemptyEv");

TypedValue* tg_9StableMap_isEmpty(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_9StableMap_isEmpty((this_))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("StableMap::isEmpty", 0, 1);
      }
    } else {
      throw_instance_method_fatal("StableMap::isEmpty");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
long long HPHP::c_StableMap::t_count()
_ZN4HPHP11c_StableMap7t_countEv

(return value) => rax
this_ => rdi
*/

long long th_9StableMap_count(ObjectData* this_) asm("_ZN4HPHP11c_StableMap7t_countEv");

TypedValue* tg_9StableMap_count(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (long long)th_9StableMap_count((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("StableMap::count", 0, 1);
      }
    } else {
      throw_instance_method_fatal("StableMap::count");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_StableMap::t_at(HPHP::Variant const&)
_ZN4HPHP11c_StableMap4t_atERKNS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
key => rdx
*/

TypedValue* th_9StableMap_at(TypedValue* _rv, ObjectData* this_, TypedValue* key) asm("_ZN4HPHP11c_StableMap4t_atERKNS_7VariantE");

TypedValue* tg_9StableMap_at(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        th_9StableMap_at((&(rv)), (this_), (args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("StableMap::at", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("StableMap::at");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_StableMap::t_get(HPHP::Variant const&)
_ZN4HPHP11c_StableMap5t_getERKNS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
key => rdx
*/

TypedValue* th_9StableMap_get(TypedValue* _rv, ObjectData* this_, TypedValue* key) asm("_ZN4HPHP11c_StableMap5t_getERKNS_7VariantE");

TypedValue* tg_9StableMap_get(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        th_9StableMap_get((&(rv)), (this_), (args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("StableMap::get", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("StableMap::get");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_StableMap::t_put(HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP11c_StableMap5t_putERKNS_7VariantES3_

(return value) => rax
_rv => rdi
this_ => rsi
key => rdx
value => rcx
*/

Value* th_9StableMap_put(Value* _rv, ObjectData* this_, TypedValue* key, TypedValue* value) asm("_ZN4HPHP11c_StableMap5t_putERKNS_7VariantES3_");

TypedValue* tg_9StableMap_put(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        rv._count = 0;
        rv.m_type = KindOfObject;
        th_9StableMap_put((Value*)(&(rv)), (this_), (args-0), (args-1));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("StableMap::put", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("StableMap::put");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_StableMap::t_clear()
_ZN4HPHP11c_StableMap7t_clearEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_9StableMap_clear(Value* _rv, ObjectData* this_) asm("_ZN4HPHP11c_StableMap7t_clearEv");

TypedValue* tg_9StableMap_clear(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfObject;
        th_9StableMap_clear((Value*)(&(rv)), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("StableMap::clear", 0, 1);
      }
    } else {
      throw_instance_method_fatal("StableMap::clear");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_StableMap::t_contains(HPHP::Variant const&)
_ZN4HPHP11c_StableMap10t_containsERKNS_7VariantE

(return value) => rax
this_ => rdi
key => rsi
*/

bool th_9StableMap_contains(ObjectData* this_, TypedValue* key) asm("_ZN4HPHP11c_StableMap10t_containsERKNS_7VariantE");

TypedValue* tg_9StableMap_contains(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_9StableMap_contains((this_), (args-0))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("StableMap::contains", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("StableMap::contains");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_StableMap::t_remove(HPHP::Variant const&)
_ZN4HPHP11c_StableMap8t_removeERKNS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
key => rdx
*/

Value* th_9StableMap_remove(Value* _rv, ObjectData* this_, TypedValue* key) asm("_ZN4HPHP11c_StableMap8t_removeERKNS_7VariantE");

TypedValue* tg_9StableMap_remove(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv._count = 0;
        rv.m_type = KindOfObject;
        th_9StableMap_remove((Value*)(&(rv)), (this_), (args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("StableMap::remove", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("StableMap::remove");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_StableMap::t_discard(HPHP::Variant const&)
_ZN4HPHP11c_StableMap9t_discardERKNS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
key => rdx
*/

Value* th_9StableMap_discard(Value* _rv, ObjectData* this_, TypedValue* key) asm("_ZN4HPHP11c_StableMap9t_discardERKNS_7VariantE");

TypedValue* tg_9StableMap_discard(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv._count = 0;
        rv.m_type = KindOfObject;
        th_9StableMap_discard((Value*)(&(rv)), (this_), (args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("StableMap::discard", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("StableMap::discard");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Array HPHP::c_StableMap::t_toarray()
_ZN4HPHP11c_StableMap9t_toarrayEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_9StableMap_toArray(Value* _rv, ObjectData* this_) asm("_ZN4HPHP11c_StableMap9t_toarrayEv");

TypedValue* tg_9StableMap_toArray(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfArray;
        th_9StableMap_toArray((Value*)(&(rv)), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("StableMap::toArray", 0, 1);
      }
    } else {
      throw_instance_method_fatal("StableMap::toArray");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Array HPHP::c_StableMap::t_copyasarray()
_ZN4HPHP11c_StableMap13t_copyasarrayEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_9StableMap_copyAsArray(Value* _rv, ObjectData* this_) asm("_ZN4HPHP11c_StableMap13t_copyasarrayEv");

TypedValue* tg_9StableMap_copyAsArray(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfArray;
        th_9StableMap_copyAsArray((Value*)(&(rv)), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("StableMap::copyAsArray", 0, 1);
      }
    } else {
      throw_instance_method_fatal("StableMap::copyAsArray");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Array HPHP::c_StableMap::t_tokeysarray()
_ZN4HPHP11c_StableMap13t_tokeysarrayEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_9StableMap_toKeysArray(Value* _rv, ObjectData* this_) asm("_ZN4HPHP11c_StableMap13t_tokeysarrayEv");

TypedValue* tg_9StableMap_toKeysArray(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfArray;
        th_9StableMap_toKeysArray((Value*)(&(rv)), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("StableMap::toKeysArray", 0, 1);
      }
    } else {
      throw_instance_method_fatal("StableMap::toKeysArray");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_StableMap::t_values()
_ZN4HPHP11c_StableMap8t_valuesEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_9StableMap_values(Value* _rv, ObjectData* this_) asm("_ZN4HPHP11c_StableMap8t_valuesEv");

TypedValue* tg_9StableMap_values(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfObject;
        th_9StableMap_values((Value*)(&(rv)), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("StableMap::values", 0, 1);
      }
    } else {
      throw_instance_method_fatal("StableMap::values");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Array HPHP::c_StableMap::t_tovaluesarray()
_ZN4HPHP11c_StableMap15t_tovaluesarrayEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_9StableMap_toValuesArray(Value* _rv, ObjectData* this_) asm("_ZN4HPHP11c_StableMap15t_tovaluesarrayEv");

TypedValue* tg_9StableMap_toValuesArray(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfArray;
        th_9StableMap_toValuesArray((Value*)(&(rv)), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("StableMap::toValuesArray", 0, 1);
      }
    } else {
      throw_instance_method_fatal("StableMap::toValuesArray");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_StableMap::t_updatefromarray(HPHP::Variant const&)
_ZN4HPHP11c_StableMap17t_updatefromarrayERKNS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
arr => rdx
*/

Value* th_9StableMap_updateFromArray(Value* _rv, ObjectData* this_, TypedValue* arr) asm("_ZN4HPHP11c_StableMap17t_updatefromarrayERKNS_7VariantE");

TypedValue* tg_9StableMap_updateFromArray(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv._count = 0;
        rv.m_type = KindOfObject;
        th_9StableMap_updateFromArray((Value*)(&(rv)), (this_), (args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("StableMap::updateFromArray", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("StableMap::updateFromArray");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_StableMap::t_updatefromiterable(HPHP::Variant const&)
_ZN4HPHP11c_StableMap20t_updatefromiterableERKNS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
it => rdx
*/

Value* th_9StableMap_updateFromIterable(Value* _rv, ObjectData* this_, TypedValue* it) asm("_ZN4HPHP11c_StableMap20t_updatefromiterableERKNS_7VariantE");

TypedValue* tg_9StableMap_updateFromIterable(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv._count = 0;
        rv.m_type = KindOfObject;
        th_9StableMap_updateFromIterable((Value*)(&(rv)), (this_), (args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("StableMap::updateFromIterable", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("StableMap::updateFromIterable");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_StableMap::t_differencebykey(HPHP::Variant const&)
_ZN4HPHP11c_StableMap17t_differencebykeyERKNS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
it => rdx
*/

Value* th_9StableMap_differenceByKey(Value* _rv, ObjectData* this_, TypedValue* it) asm("_ZN4HPHP11c_StableMap17t_differencebykeyERKNS_7VariantE");

TypedValue* tg_9StableMap_differenceByKey(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv._count = 0;
        rv.m_type = KindOfObject;
        th_9StableMap_differenceByKey((Value*)(&(rv)), (this_), (args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("StableMap::differenceByKey", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("StableMap::differenceByKey");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_StableMap::t_getiterator()
_ZN4HPHP11c_StableMap13t_getiteratorEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_9StableMap_getIterator(Value* _rv, ObjectData* this_) asm("_ZN4HPHP11c_StableMap13t_getiteratorEv");

TypedValue* tg_9StableMap_getIterator(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfObject;
        th_9StableMap_getIterator((Value*)(&(rv)), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("StableMap::getIterator", 0, 1);
      }
    } else {
      throw_instance_method_fatal("StableMap::getIterator");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_StableMap::t___get(HPHP::Variant)
_ZN4HPHP11c_StableMap7t___getENS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
name => rdx
*/

TypedValue* th_9StableMap___get(TypedValue* _rv, ObjectData* this_, TypedValue* name) asm("_ZN4HPHP11c_StableMap7t___getENS_7VariantE");

TypedValue* tg_9StableMap___get(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        th_9StableMap___get((&(rv)), (this_), (args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("StableMap::__get", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("StableMap::__get");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_StableMap::t___set(HPHP::Variant, HPHP::Variant)
_ZN4HPHP11c_StableMap7t___setENS_7VariantES1_

(return value) => rax
_rv => rdi
this_ => rsi
name => rdx
value => rcx
*/

TypedValue* th_9StableMap___set(TypedValue* _rv, ObjectData* this_, TypedValue* name, TypedValue* value) asm("_ZN4HPHP11c_StableMap7t___setENS_7VariantES1_");

TypedValue* tg_9StableMap___set(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        th_9StableMap___set((&(rv)), (this_), (args-0), (args-1));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("StableMap::__set", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("StableMap::__set");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_StableMap::t___isset(HPHP::Variant)
_ZN4HPHP11c_StableMap9t___issetENS_7VariantE

(return value) => rax
this_ => rdi
name => rsi
*/

bool th_9StableMap___isset(ObjectData* this_, TypedValue* name) asm("_ZN4HPHP11c_StableMap9t___issetENS_7VariantE");

TypedValue* tg_9StableMap___isset(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_9StableMap___isset((this_), (args-0))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("StableMap::__isset", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("StableMap::__isset");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_StableMap::t___unset(HPHP::Variant)
_ZN4HPHP11c_StableMap9t___unsetENS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
name => rdx
*/

TypedValue* th_9StableMap___unset(TypedValue* _rv, ObjectData* this_, TypedValue* name) asm("_ZN4HPHP11c_StableMap9t___unsetENS_7VariantE");

TypedValue* tg_9StableMap___unset(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        th_9StableMap___unset((&(rv)), (this_), (args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("StableMap::__unset", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("StableMap::__unset");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::String HPHP::c_StableMap::t___tostring()
_ZN4HPHP11c_StableMap12t___tostringEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_9StableMap___toString(Value* _rv, ObjectData* this_) asm("_ZN4HPHP11c_StableMap12t___tostringEv");

TypedValue* tg_9StableMap___toString(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfString;
        th_9StableMap___toString((Value*)(&(rv)), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("StableMap::__toString", 0, 1);
      }
    } else {
      throw_instance_method_fatal("StableMap::__toString");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_StableMap::ti_fromarray(char const*, HPHP::Variant const&)
_ZN4HPHP11c_StableMap12ti_fromarrayEPKcRKNS_7VariantE

(return value) => rax
_rv => rdi
cls_ => rsi
mp => rdx
*/

Value* th_9StableMap_fromArray(Value* _rv, char const* cls_, TypedValue* mp) asm("_ZN4HPHP11c_StableMap12ti_fromarrayEPKcRKNS_7VariantE");

TypedValue* tg_9StableMap_fromArray(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv._count = 0;
      rv.m_type = KindOfObject;
      th_9StableMap_fromArray((Value*)(&(rv)), ("StableMap"), (args-0));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("StableMap::fromArray", count, 1, 1, 1);
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
HPHP::Object HPHP::c_StableMap::ti_fromiterable(char const*, HPHP::Variant const&)
_ZN4HPHP11c_StableMap15ti_fromiterableEPKcRKNS_7VariantE

(return value) => rax
_rv => rdi
cls_ => rsi
mp => rdx
*/

Value* th_9StableMap_fromIterable(Value* _rv, char const* cls_, TypedValue* mp) asm("_ZN4HPHP11c_StableMap15ti_fromiterableEPKcRKNS_7VariantE");

TypedValue* tg_9StableMap_fromIterable(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv._count = 0;
      rv.m_type = KindOfObject;
      th_9StableMap_fromIterable((Value*)(&(rv)), ("StableMap"), (args-0));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("StableMap::fromIterable", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

HPHP::VM::Instance* new_StableMapIterator_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_StableMapIterator) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_StableMapIterator(ObjectStaticCallbacks::encodeVMClass(cls));
  return inst;
}

/*
void HPHP::c_StableMapIterator::t___construct()
_ZN4HPHP19c_StableMapIterator13t___constructEv

this_ => rdi
*/

void th_17StableMapIterator___construct(ObjectData* this_) asm("_ZN4HPHP19c_StableMapIterator13t___constructEv");

TypedValue* tg_17StableMapIterator___construct(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        th_17StableMapIterator___construct((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("StableMapIterator::__construct", 0, 1);
      }
    } else {
      throw_instance_method_fatal("StableMapIterator::__construct");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_StableMapIterator::t_current()
_ZN4HPHP19c_StableMapIterator9t_currentEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_17StableMapIterator_current(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP19c_StableMapIterator9t_currentEv");

TypedValue* tg_17StableMapIterator_current(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_17StableMapIterator_current((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("StableMapIterator::current", 0, 1);
      }
    } else {
      throw_instance_method_fatal("StableMapIterator::current");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_StableMapIterator::t_key()
_ZN4HPHP19c_StableMapIterator5t_keyEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_17StableMapIterator_key(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP19c_StableMapIterator5t_keyEv");

TypedValue* tg_17StableMapIterator_key(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_17StableMapIterator_key((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("StableMapIterator::key", 0, 1);
      }
    } else {
      throw_instance_method_fatal("StableMapIterator::key");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_StableMapIterator::t_valid()
_ZN4HPHP19c_StableMapIterator7t_validEv

(return value) => rax
this_ => rdi
*/

bool th_17StableMapIterator_valid(ObjectData* this_) asm("_ZN4HPHP19c_StableMapIterator7t_validEv");

TypedValue* tg_17StableMapIterator_valid(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_17StableMapIterator_valid((this_))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("StableMapIterator::valid", 0, 1);
      }
    } else {
      throw_instance_method_fatal("StableMapIterator::valid");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
void HPHP::c_StableMapIterator::t_next()
_ZN4HPHP19c_StableMapIterator6t_nextEv

this_ => rdi
*/

void th_17StableMapIterator_next(ObjectData* this_) asm("_ZN4HPHP19c_StableMapIterator6t_nextEv");

TypedValue* tg_17StableMapIterator_next(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        th_17StableMapIterator_next((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("StableMapIterator::next", 0, 1);
      }
    } else {
      throw_instance_method_fatal("StableMapIterator::next");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
void HPHP::c_StableMapIterator::t_rewind()
_ZN4HPHP19c_StableMapIterator8t_rewindEv

this_ => rdi
*/

void th_17StableMapIterator_rewind(ObjectData* this_) asm("_ZN4HPHP19c_StableMapIterator8t_rewindEv");

TypedValue* tg_17StableMapIterator_rewind(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        th_17StableMapIterator_rewind((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("StableMapIterator::rewind", 0, 1);
      }
    } else {
      throw_instance_method_fatal("StableMapIterator::rewind");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}


} // !HPHP

