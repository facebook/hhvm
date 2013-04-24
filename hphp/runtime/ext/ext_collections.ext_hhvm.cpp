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
  new ((void *)inst) c_Vector(cls);
  return inst;
}

IMPLEMENT_CLASS(Vector);
/*
void HPHP::c_Vector::t___construct(HPHP::Variant const&)
_ZN4HPHP8c_Vector13t___constructERKNS_7VariantE

this_ => rdi
iterable => rsi
*/

void th_6Vector___construct(ObjectData* this_, TypedValue* iterable) asm("_ZN4HPHP8c_Vector13t___constructERKNS_7VariantE");

TypedValue* tg_6Vector___construct(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count <= 1LL) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        Variant defVal0;
        th_6Vector___construct((this_), (count > 0) ? (args-0) : (TypedValue*)(&defVal0));
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Vector::__construct", 1, 1);
      }
    } else {
      throw_instance_method_fatal("Vector::__construct");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
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

TypedValue* tg_6Vector_isEmpty(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
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
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
long HPHP::c_Vector::t_count()
_ZN4HPHP8c_Vector7t_countEv

(return value) => rax
this_ => rdi
*/

long th_6Vector_count(ObjectData* this_) asm("_ZN4HPHP8c_Vector7t_countEv");

TypedValue* tg_6Vector_count(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)th_6Vector_count((this_));
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
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_Vector::t_items()
_ZN4HPHP8c_Vector7t_itemsEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_6Vector_items(Value* _rv, ObjectData* this_) asm("_ZN4HPHP8c_Vector7t_itemsEv");

TypedValue* tg_6Vector_items(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfObject;
        th_6Vector_items((&rv.m_data), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Vector::items", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Vector::items");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_Vector::t_keys()
_ZN4HPHP8c_Vector6t_keysEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_6Vector_keys(Value* _rv, ObjectData* this_) asm("_ZN4HPHP8c_Vector6t_keysEv");

TypedValue* tg_6Vector_keys(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfObject;
        th_6Vector_keys((&rv.m_data), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Vector::keys", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Vector::keys");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_Vector::t_view()
_ZN4HPHP8c_Vector6t_viewEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_6Vector_view(Value* _rv, ObjectData* this_) asm("_ZN4HPHP8c_Vector6t_viewEv");

TypedValue* tg_6Vector_view(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfObject;
        th_6Vector_view((&rv.m_data), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Vector::view", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Vector::view");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_Vector::t_kvzip()
_ZN4HPHP8c_Vector7t_kvzipEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_6Vector_kvzip(Value* _rv, ObjectData* this_) asm("_ZN4HPHP8c_Vector7t_kvzipEv");

TypedValue* tg_6Vector_kvzip(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfObject;
        th_6Vector_kvzip((&rv.m_data), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Vector::kvzip", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Vector::kvzip");
    }
    rv.m_data.num = 0LL;
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

TypedValue* tg_6Vector_at(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
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

TypedValue* tg_6Vector_get(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
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
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_Vector::t_set(HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP8c_Vector5t_setERKNS_7VariantES3_

(return value) => rax
_rv => rdi
this_ => rsi
key => rdx
value => rcx
*/

Value* th_6Vector_set(Value* _rv, ObjectData* this_, TypedValue* key, TypedValue* value) asm("_ZN4HPHP8c_Vector5t_setERKNS_7VariantES3_");

TypedValue* tg_6Vector_set(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        rv.m_type = KindOfObject;
        th_6Vector_set((&rv.m_data), (this_), (args-0), (args-1));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("Vector::set", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("Vector::set");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_Vector::t_setall(HPHP::Variant const&)
_ZN4HPHP8c_Vector8t_setallERKNS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
iterable => rdx
*/

Value* th_6Vector_setAll(Value* _rv, ObjectData* this_, TypedValue* iterable) asm("_ZN4HPHP8c_Vector8t_setallERKNS_7VariantE");

TypedValue* tg_6Vector_setAll(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_type = KindOfObject;
        th_6Vector_setAll((&rv.m_data), (this_), (args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("Vector::setAll", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("Vector::setAll");
    }
    rv.m_data.num = 0LL;
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

TypedValue* tg_6Vector_put(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        rv.m_type = KindOfObject;
        th_6Vector_put((&rv.m_data), (this_), (args-0), (args-1));
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

TypedValue* tg_6Vector_clear(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfObject;
        th_6Vector_clear((&rv.m_data), (this_));
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

TypedValue* tg_6Vector_contains(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
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
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_Vector::t_containskey(HPHP::Variant const&)
_ZN4HPHP8c_Vector13t_containskeyERKNS_7VariantE

(return value) => rax
this_ => rdi
key => rsi
*/

bool th_6Vector_containsKey(ObjectData* this_, TypedValue* key) asm("_ZN4HPHP8c_Vector13t_containskeyERKNS_7VariantE");

TypedValue* tg_6Vector_containsKey(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_6Vector_containsKey((this_), (args-0))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("Vector::containsKey", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("Vector::containsKey");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_Vector::t_removekey(HPHP::Variant const&)
_ZN4HPHP8c_Vector11t_removekeyERKNS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
key => rdx
*/

Value* th_6Vector_removeKey(Value* _rv, ObjectData* this_, TypedValue* key) asm("_ZN4HPHP8c_Vector11t_removekeyERKNS_7VariantE");

TypedValue* tg_6Vector_removeKey(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_type = KindOfObject;
        th_6Vector_removeKey((&rv.m_data), (this_), (args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("Vector::removeKey", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("Vector::removeKey");
    }
    rv.m_data.num = 0LL;
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

TypedValue* tg_6Vector_append(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_type = KindOfObject;
        th_6Vector_append((&rv.m_data), (this_), (args-0));
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

TypedValue* tg_6Vector_add(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_type = KindOfObject;
        th_6Vector_add((&rv.m_data), (this_), (args-0));
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
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_Vector::t_addall(HPHP::Variant const&)
_ZN4HPHP8c_Vector8t_addallERKNS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
iterable => rdx
*/

Value* th_6Vector_addAll(Value* _rv, ObjectData* this_, TypedValue* iterable) asm("_ZN4HPHP8c_Vector8t_addallERKNS_7VariantE");

TypedValue* tg_6Vector_addAll(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_type = KindOfObject;
        th_6Vector_addAll((&rv.m_data), (this_), (args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("Vector::addAll", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("Vector::addAll");
    }
    rv.m_data.num = 0LL;
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

TypedValue* tg_6Vector_pop(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
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

TypedValue* tg_6Vector_resize(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        rv.m_data.num = 0LL;
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

TypedValue* tg_6Vector_toArray(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfArray;
        th_6Vector_toArray((&rv.m_data), (this_));
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

TypedValue* tg_6Vector_getIterator(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfObject;
        th_6Vector_getIterator((&rv.m_data), (this_));
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
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_Vector::t_map(HPHP::Variant const&)
_ZN4HPHP8c_Vector5t_mapERKNS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
callback => rdx
*/

Value* th_6Vector_map(Value* _rv, ObjectData* this_, TypedValue* callback) asm("_ZN4HPHP8c_Vector5t_mapERKNS_7VariantE");

TypedValue* tg_6Vector_map(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_type = KindOfObject;
        th_6Vector_map((&rv.m_data), (this_), (args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("Vector::map", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("Vector::map");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_Vector::t_filter(HPHP::Variant const&)
_ZN4HPHP8c_Vector8t_filterERKNS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
callback => rdx
*/

Value* th_6Vector_filter(Value* _rv, ObjectData* this_, TypedValue* callback) asm("_ZN4HPHP8c_Vector8t_filterERKNS_7VariantE");

TypedValue* tg_6Vector_filter(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_type = KindOfObject;
        th_6Vector_filter((&rv.m_data), (this_), (args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("Vector::filter", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("Vector::filter");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_Vector::t_zip(HPHP::Variant const&)
_ZN4HPHP8c_Vector5t_zipERKNS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
iterable => rdx
*/

Value* th_6Vector_zip(Value* _rv, ObjectData* this_, TypedValue* iterable) asm("_ZN4HPHP8c_Vector5t_zipERKNS_7VariantE");

TypedValue* tg_6Vector_zip(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_type = KindOfObject;
        th_6Vector_zip((&rv.m_data), (this_), (args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("Vector::zip", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("Vector::zip");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
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

TypedValue* tg_6Vector_sort(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count <= 1LL) {
        rv.m_data.num = 0LL;
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

TypedValue* tg_6Vector_reverse(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
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

TypedValue* tg_6Vector_splice(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL && count <= 3LL) {
        rv.m_data.num = 0LL;
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
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
long HPHP::c_Vector::t_linearsearch(HPHP::Variant const&)
_ZN4HPHP8c_Vector14t_linearsearchERKNS_7VariantE

(return value) => rax
this_ => rdi
search_value => rsi
*/

long th_6Vector_linearSearch(ObjectData* this_, TypedValue* search_value) asm("_ZN4HPHP8c_Vector14t_linearsearchERKNS_7VariantE");

TypedValue* tg_6Vector_linearSearch(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)th_6Vector_linearSearch((this_), (args-0));
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

TypedValue* tg_6Vector_shuffle(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
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

TypedValue* tg_6Vector___toString(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfString;
        th_6Vector___toString((&rv.m_data), (this_));
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

TypedValue* tg_6Vector___get(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
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

TypedValue* tg_6Vector___set(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
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

TypedValue* tg_6Vector___isset(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
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

TypedValue* tg_6Vector___unset(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
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
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_Vector::ti_fromitems(char const*, HPHP::Variant const&)
_ZN4HPHP8c_Vector12ti_fromitemsEPKcRKNS_7VariantE

(return value) => rax
_rv => rdi
cls_ => rsi
iterable => rdx
*/

Value* th_6Vector_fromItems(Value* _rv, char const* cls_, TypedValue* iterable) asm("_ZN4HPHP8c_Vector12ti_fromitemsEPKcRKNS_7VariantE");

TypedValue* tg_6Vector_fromItems(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv.m_type = KindOfObject;
      th_6Vector_fromItems((&rv.m_data), ("Vector"), (args-0));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("Vector::fromItems", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
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

TypedValue* tg_6Vector_fromArray(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv.m_type = KindOfObject;
      th_6Vector_fromArray((&rv.m_data), ("Vector"), (args-0));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("Vector::fromArray", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
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

TypedValue* tg_6Vector_fromVector(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv.m_type = KindOfObject;
      th_6Vector_fromVector((&rv.m_data), ("Vector"), (args-0));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("Vector::fromVector", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
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

TypedValue* tg_6Vector_slice(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      rv.m_type = KindOfObject;
      Variant defVal2;
      th_6Vector_slice((&rv.m_data), ("Vector"), (args-0), (args-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 3);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("Vector::slice", count, 2, 3, 1);
    }
    rv.m_data.num = 0LL;
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
  new ((void *)inst) c_VectorIterator(cls);
  return inst;
}

IMPLEMENT_CLASS(VectorIterator);
/*
void HPHP::c_VectorIterator::t___construct()
_ZN4HPHP16c_VectorIterator13t___constructEv

this_ => rdi
*/

void th_14VectorIterator___construct(ObjectData* this_) asm("_ZN4HPHP16c_VectorIterator13t___constructEv");

TypedValue* tg_14VectorIterator___construct(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
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

TypedValue* tg_14VectorIterator_current(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
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

TypedValue* tg_14VectorIterator_key(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
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

TypedValue* tg_14VectorIterator_valid(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
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

TypedValue* tg_14VectorIterator_next(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
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

TypedValue* tg_14VectorIterator_rewind(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
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
  new ((void *)inst) c_Map(cls);
  return inst;
}

IMPLEMENT_CLASS(Map);
/*
void HPHP::c_Map::t___construct(HPHP::Variant const&)
_ZN4HPHP5c_Map13t___constructERKNS_7VariantE

this_ => rdi
iterable => rsi
*/

void th_3Map___construct(ObjectData* this_, TypedValue* iterable) asm("_ZN4HPHP5c_Map13t___constructERKNS_7VariantE");

TypedValue* tg_3Map___construct(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count <= 1LL) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        Variant defVal0;
        th_3Map___construct((this_), (count > 0) ? (args-0) : (TypedValue*)(&defVal0));
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Map::__construct", 1, 1);
      }
    } else {
      throw_instance_method_fatal("Map::__construct");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
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

TypedValue* tg_3Map_isEmpty(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
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
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
long HPHP::c_Map::t_count()
_ZN4HPHP5c_Map7t_countEv

(return value) => rax
this_ => rdi
*/

long th_3Map_count(ObjectData* this_) asm("_ZN4HPHP5c_Map7t_countEv");

TypedValue* tg_3Map_count(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)th_3Map_count((this_));
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
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_Map::t_items()
_ZN4HPHP5c_Map7t_itemsEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_3Map_items(Value* _rv, ObjectData* this_) asm("_ZN4HPHP5c_Map7t_itemsEv");

TypedValue* tg_3Map_items(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfObject;
        th_3Map_items((&rv.m_data), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Map::items", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Map::items");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_Map::t_keys()
_ZN4HPHP5c_Map6t_keysEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_3Map_keys(Value* _rv, ObjectData* this_) asm("_ZN4HPHP5c_Map6t_keysEv");

TypedValue* tg_3Map_keys(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfObject;
        th_3Map_keys((&rv.m_data), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Map::keys", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Map::keys");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_Map::t_view()
_ZN4HPHP5c_Map6t_viewEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_3Map_view(Value* _rv, ObjectData* this_) asm("_ZN4HPHP5c_Map6t_viewEv");

TypedValue* tg_3Map_view(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfObject;
        th_3Map_view((&rv.m_data), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Map::view", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Map::view");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_Map::t_kvzip()
_ZN4HPHP5c_Map7t_kvzipEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_3Map_kvzip(Value* _rv, ObjectData* this_) asm("_ZN4HPHP5c_Map7t_kvzipEv");

TypedValue* tg_3Map_kvzip(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfObject;
        th_3Map_kvzip((&rv.m_data), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Map::kvzip", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Map::kvzip");
    }
    rv.m_data.num = 0LL;
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

TypedValue* tg_3Map_at(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
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

TypedValue* tg_3Map_get(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
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
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_Map::t_set(HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP5c_Map5t_setERKNS_7VariantES3_

(return value) => rax
_rv => rdi
this_ => rsi
key => rdx
value => rcx
*/

Value* th_3Map_set(Value* _rv, ObjectData* this_, TypedValue* key, TypedValue* value) asm("_ZN4HPHP5c_Map5t_setERKNS_7VariantES3_");

TypedValue* tg_3Map_set(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        rv.m_type = KindOfObject;
        th_3Map_set((&rv.m_data), (this_), (args-0), (args-1));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("Map::set", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("Map::set");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_Map::t_setall(HPHP::Variant const&)
_ZN4HPHP5c_Map8t_setallERKNS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
iterable => rdx
*/

Value* th_3Map_setAll(Value* _rv, ObjectData* this_, TypedValue* iterable) asm("_ZN4HPHP5c_Map8t_setallERKNS_7VariantE");

TypedValue* tg_3Map_setAll(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_type = KindOfObject;
        th_3Map_setAll((&rv.m_data), (this_), (args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("Map::setAll", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("Map::setAll");
    }
    rv.m_data.num = 0LL;
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

TypedValue* tg_3Map_put(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        rv.m_type = KindOfObject;
        th_3Map_put((&rv.m_data), (this_), (args-0), (args-1));
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

TypedValue* tg_3Map_clear(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfObject;
        th_3Map_clear((&rv.m_data), (this_));
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

TypedValue* tg_3Map_contains(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
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
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_Map::t_containskey(HPHP::Variant const&)
_ZN4HPHP5c_Map13t_containskeyERKNS_7VariantE

(return value) => rax
this_ => rdi
key => rsi
*/

bool th_3Map_containsKey(ObjectData* this_, TypedValue* key) asm("_ZN4HPHP5c_Map13t_containskeyERKNS_7VariantE");

TypedValue* tg_3Map_containsKey(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_3Map_containsKey((this_), (args-0))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("Map::containsKey", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("Map::containsKey");
    }
    rv.m_data.num = 0LL;
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

TypedValue* tg_3Map_remove(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_type = KindOfObject;
        th_3Map_remove((&rv.m_data), (this_), (args-0));
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
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_Map::t_removekey(HPHP::Variant const&)
_ZN4HPHP5c_Map11t_removekeyERKNS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
key => rdx
*/

Value* th_3Map_removeKey(Value* _rv, ObjectData* this_, TypedValue* key) asm("_ZN4HPHP5c_Map11t_removekeyERKNS_7VariantE");

TypedValue* tg_3Map_removeKey(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_type = KindOfObject;
        th_3Map_removeKey((&rv.m_data), (this_), (args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("Map::removeKey", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("Map::removeKey");
    }
    rv.m_data.num = 0LL;
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

TypedValue* tg_3Map_discard(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_type = KindOfObject;
        th_3Map_discard((&rv.m_data), (this_), (args-0));
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
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_Map::t_add(HPHP::Variant const&)
_ZN4HPHP5c_Map5t_addERKNS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
val => rdx
*/

Value* th_3Map_add(Value* _rv, ObjectData* this_, TypedValue* val) asm("_ZN4HPHP5c_Map5t_addERKNS_7VariantE");

TypedValue* tg_3Map_add(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_type = KindOfObject;
        th_3Map_add((&rv.m_data), (this_), (args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("Map::add", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("Map::add");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_Map::t_addall(HPHP::Variant const&)
_ZN4HPHP5c_Map8t_addallERKNS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
iterable => rdx
*/

Value* th_3Map_addAll(Value* _rv, ObjectData* this_, TypedValue* iterable) asm("_ZN4HPHP5c_Map8t_addallERKNS_7VariantE");

TypedValue* tg_3Map_addAll(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_type = KindOfObject;
        th_3Map_addAll((&rv.m_data), (this_), (args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("Map::addAll", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("Map::addAll");
    }
    rv.m_data.num = 0LL;
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

TypedValue* tg_3Map_toArray(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfArray;
        th_3Map_toArray((&rv.m_data), (this_));
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

TypedValue* tg_3Map_copyAsArray(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfArray;
        th_3Map_copyAsArray((&rv.m_data), (this_));
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

TypedValue* tg_3Map_toKeysArray(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfArray;
        th_3Map_toKeysArray((&rv.m_data), (this_));
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

TypedValue* tg_3Map_values(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfObject;
        th_3Map_values((&rv.m_data), (this_));
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

TypedValue* tg_3Map_toValuesArray(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfArray;
        th_3Map_toValuesArray((&rv.m_data), (this_));
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

TypedValue* tg_3Map_updateFromArray(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_type = KindOfObject;
        th_3Map_updateFromArray((&rv.m_data), (this_), (args-0));
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

TypedValue* tg_3Map_updateFromIterable(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_type = KindOfObject;
        th_3Map_updateFromIterable((&rv.m_data), (this_), (args-0));
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

TypedValue* tg_3Map_differenceByKey(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_type = KindOfObject;
        th_3Map_differenceByKey((&rv.m_data), (this_), (args-0));
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

TypedValue* tg_3Map_getIterator(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfObject;
        th_3Map_getIterator((&rv.m_data), (this_));
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
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_Map::t_map(HPHP::Variant const&)
_ZN4HPHP5c_Map5t_mapERKNS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
callback => rdx
*/

Value* th_3Map_map(Value* _rv, ObjectData* this_, TypedValue* callback) asm("_ZN4HPHP5c_Map5t_mapERKNS_7VariantE");

TypedValue* tg_3Map_map(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_type = KindOfObject;
        th_3Map_map((&rv.m_data), (this_), (args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("Map::map", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("Map::map");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_Map::t_filter(HPHP::Variant const&)
_ZN4HPHP5c_Map8t_filterERKNS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
callback => rdx
*/

Value* th_3Map_filter(Value* _rv, ObjectData* this_, TypedValue* callback) asm("_ZN4HPHP5c_Map8t_filterERKNS_7VariantE");

TypedValue* tg_3Map_filter(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_type = KindOfObject;
        th_3Map_filter((&rv.m_data), (this_), (args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("Map::filter", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("Map::filter");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_Map::t_zip(HPHP::Variant const&)
_ZN4HPHP5c_Map5t_zipERKNS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
iterable => rdx
*/

Value* th_3Map_zip(Value* _rv, ObjectData* this_, TypedValue* iterable) asm("_ZN4HPHP5c_Map5t_zipERKNS_7VariantE");

TypedValue* tg_3Map_zip(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_type = KindOfObject;
        th_3Map_zip((&rv.m_data), (this_), (args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("Map::zip", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("Map::zip");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
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

TypedValue* tg_3Map___toString(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfString;
        th_3Map___toString((&rv.m_data), (this_));
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

TypedValue* tg_3Map___get(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
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

TypedValue* tg_3Map___set(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
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

TypedValue* tg_3Map___isset(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
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

TypedValue* tg_3Map___unset(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
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
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_Map::ti_fromitems(char const*, HPHP::Variant const&)
_ZN4HPHP5c_Map12ti_fromitemsEPKcRKNS_7VariantE

(return value) => rax
_rv => rdi
cls_ => rsi
iterable => rdx
*/

Value* th_3Map_fromItems(Value* _rv, char const* cls_, TypedValue* iterable) asm("_ZN4HPHP5c_Map12ti_fromitemsEPKcRKNS_7VariantE");

TypedValue* tg_3Map_fromItems(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv.m_type = KindOfObject;
      th_3Map_fromItems((&rv.m_data), ("Map"), (args-0));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("Map::fromItems", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
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

TypedValue* tg_3Map_fromArray(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv.m_type = KindOfObject;
      th_3Map_fromArray((&rv.m_data), ("Map"), (args-0));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("Map::fromArray", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
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

TypedValue* tg_3Map_fromIterable(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv.m_type = KindOfObject;
      th_3Map_fromIterable((&rv.m_data), ("Map"), (args-0));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("Map::fromIterable", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
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
  new ((void *)inst) c_MapIterator(cls);
  return inst;
}

IMPLEMENT_CLASS(MapIterator);
/*
void HPHP::c_MapIterator::t___construct()
_ZN4HPHP13c_MapIterator13t___constructEv

this_ => rdi
*/

void th_11MapIterator___construct(ObjectData* this_) asm("_ZN4HPHP13c_MapIterator13t___constructEv");

TypedValue* tg_11MapIterator___construct(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
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

TypedValue* tg_11MapIterator_current(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
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

TypedValue* tg_11MapIterator_key(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
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

TypedValue* tg_11MapIterator_valid(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
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

TypedValue* tg_11MapIterator_next(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
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

TypedValue* tg_11MapIterator_rewind(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
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
  new ((void *)inst) c_StableMap(cls);
  return inst;
}

IMPLEMENT_CLASS(StableMap);
/*
void HPHP::c_StableMap::t___construct(HPHP::Variant const&)
_ZN4HPHP11c_StableMap13t___constructERKNS_7VariantE

this_ => rdi
iterable => rsi
*/

void th_9StableMap___construct(ObjectData* this_, TypedValue* iterable) asm("_ZN4HPHP11c_StableMap13t___constructERKNS_7VariantE");

TypedValue* tg_9StableMap___construct(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count <= 1LL) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        Variant defVal0;
        th_9StableMap___construct((this_), (count > 0) ? (args-0) : (TypedValue*)(&defVal0));
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("StableMap::__construct", 1, 1);
      }
    } else {
      throw_instance_method_fatal("StableMap::__construct");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
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

TypedValue* tg_9StableMap_isEmpty(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
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
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
long HPHP::c_StableMap::t_count()
_ZN4HPHP11c_StableMap7t_countEv

(return value) => rax
this_ => rdi
*/

long th_9StableMap_count(ObjectData* this_) asm("_ZN4HPHP11c_StableMap7t_countEv");

TypedValue* tg_9StableMap_count(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)th_9StableMap_count((this_));
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
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_StableMap::t_items()
_ZN4HPHP11c_StableMap7t_itemsEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_9StableMap_items(Value* _rv, ObjectData* this_) asm("_ZN4HPHP11c_StableMap7t_itemsEv");

TypedValue* tg_9StableMap_items(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfObject;
        th_9StableMap_items((&rv.m_data), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("StableMap::items", 0, 1);
      }
    } else {
      throw_instance_method_fatal("StableMap::items");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_StableMap::t_keys()
_ZN4HPHP11c_StableMap6t_keysEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_9StableMap_keys(Value* _rv, ObjectData* this_) asm("_ZN4HPHP11c_StableMap6t_keysEv");

TypedValue* tg_9StableMap_keys(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfObject;
        th_9StableMap_keys((&rv.m_data), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("StableMap::keys", 0, 1);
      }
    } else {
      throw_instance_method_fatal("StableMap::keys");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_StableMap::t_view()
_ZN4HPHP11c_StableMap6t_viewEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_9StableMap_view(Value* _rv, ObjectData* this_) asm("_ZN4HPHP11c_StableMap6t_viewEv");

TypedValue* tg_9StableMap_view(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfObject;
        th_9StableMap_view((&rv.m_data), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("StableMap::view", 0, 1);
      }
    } else {
      throw_instance_method_fatal("StableMap::view");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_StableMap::t_kvzip()
_ZN4HPHP11c_StableMap7t_kvzipEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_9StableMap_kvzip(Value* _rv, ObjectData* this_) asm("_ZN4HPHP11c_StableMap7t_kvzipEv");

TypedValue* tg_9StableMap_kvzip(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfObject;
        th_9StableMap_kvzip((&rv.m_data), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("StableMap::kvzip", 0, 1);
      }
    } else {
      throw_instance_method_fatal("StableMap::kvzip");
    }
    rv.m_data.num = 0LL;
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

TypedValue* tg_9StableMap_at(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
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

TypedValue* tg_9StableMap_get(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
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
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_StableMap::t_set(HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP11c_StableMap5t_setERKNS_7VariantES3_

(return value) => rax
_rv => rdi
this_ => rsi
key => rdx
value => rcx
*/

Value* th_9StableMap_set(Value* _rv, ObjectData* this_, TypedValue* key, TypedValue* value) asm("_ZN4HPHP11c_StableMap5t_setERKNS_7VariantES3_");

TypedValue* tg_9StableMap_set(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        rv.m_type = KindOfObject;
        th_9StableMap_set((&rv.m_data), (this_), (args-0), (args-1));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("StableMap::set", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("StableMap::set");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_StableMap::t_setall(HPHP::Variant const&)
_ZN4HPHP11c_StableMap8t_setallERKNS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
iterable => rdx
*/

Value* th_9StableMap_setAll(Value* _rv, ObjectData* this_, TypedValue* iterable) asm("_ZN4HPHP11c_StableMap8t_setallERKNS_7VariantE");

TypedValue* tg_9StableMap_setAll(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_type = KindOfObject;
        th_9StableMap_setAll((&rv.m_data), (this_), (args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("StableMap::setAll", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("StableMap::setAll");
    }
    rv.m_data.num = 0LL;
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

TypedValue* tg_9StableMap_put(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        rv.m_type = KindOfObject;
        th_9StableMap_put((&rv.m_data), (this_), (args-0), (args-1));
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

TypedValue* tg_9StableMap_clear(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfObject;
        th_9StableMap_clear((&rv.m_data), (this_));
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

TypedValue* tg_9StableMap_contains(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
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
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_StableMap::t_containskey(HPHP::Variant const&)
_ZN4HPHP11c_StableMap13t_containskeyERKNS_7VariantE

(return value) => rax
this_ => rdi
key => rsi
*/

bool th_9StableMap_containsKey(ObjectData* this_, TypedValue* key) asm("_ZN4HPHP11c_StableMap13t_containskeyERKNS_7VariantE");

TypedValue* tg_9StableMap_containsKey(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_9StableMap_containsKey((this_), (args-0))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("StableMap::containsKey", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("StableMap::containsKey");
    }
    rv.m_data.num = 0LL;
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

TypedValue* tg_9StableMap_remove(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_type = KindOfObject;
        th_9StableMap_remove((&rv.m_data), (this_), (args-0));
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
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_StableMap::t_removekey(HPHP::Variant const&)
_ZN4HPHP11c_StableMap11t_removekeyERKNS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
key => rdx
*/

Value* th_9StableMap_removeKey(Value* _rv, ObjectData* this_, TypedValue* key) asm("_ZN4HPHP11c_StableMap11t_removekeyERKNS_7VariantE");

TypedValue* tg_9StableMap_removeKey(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_type = KindOfObject;
        th_9StableMap_removeKey((&rv.m_data), (this_), (args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("StableMap::removeKey", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("StableMap::removeKey");
    }
    rv.m_data.num = 0LL;
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

TypedValue* tg_9StableMap_discard(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_type = KindOfObject;
        th_9StableMap_discard((&rv.m_data), (this_), (args-0));
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
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_StableMap::t_add(HPHP::Variant const&)
_ZN4HPHP11c_StableMap5t_addERKNS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
val => rdx
*/

Value* th_9StableMap_add(Value* _rv, ObjectData* this_, TypedValue* val) asm("_ZN4HPHP11c_StableMap5t_addERKNS_7VariantE");

TypedValue* tg_9StableMap_add(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_type = KindOfObject;
        th_9StableMap_add((&rv.m_data), (this_), (args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("StableMap::add", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("StableMap::add");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_StableMap::t_addall(HPHP::Variant const&)
_ZN4HPHP11c_StableMap8t_addallERKNS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
iterable => rdx
*/

Value* th_9StableMap_addAll(Value* _rv, ObjectData* this_, TypedValue* iterable) asm("_ZN4HPHP11c_StableMap8t_addallERKNS_7VariantE");

TypedValue* tg_9StableMap_addAll(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_type = KindOfObject;
        th_9StableMap_addAll((&rv.m_data), (this_), (args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("StableMap::addAll", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("StableMap::addAll");
    }
    rv.m_data.num = 0LL;
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

TypedValue* tg_9StableMap_toArray(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfArray;
        th_9StableMap_toArray((&rv.m_data), (this_));
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

TypedValue* tg_9StableMap_copyAsArray(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfArray;
        th_9StableMap_copyAsArray((&rv.m_data), (this_));
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

TypedValue* tg_9StableMap_toKeysArray(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfArray;
        th_9StableMap_toKeysArray((&rv.m_data), (this_));
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

TypedValue* tg_9StableMap_values(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfObject;
        th_9StableMap_values((&rv.m_data), (this_));
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

TypedValue* tg_9StableMap_toValuesArray(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfArray;
        th_9StableMap_toValuesArray((&rv.m_data), (this_));
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

TypedValue* tg_9StableMap_updateFromArray(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_type = KindOfObject;
        th_9StableMap_updateFromArray((&rv.m_data), (this_), (args-0));
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

TypedValue* tg_9StableMap_updateFromIterable(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_type = KindOfObject;
        th_9StableMap_updateFromIterable((&rv.m_data), (this_), (args-0));
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

TypedValue* tg_9StableMap_differenceByKey(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_type = KindOfObject;
        th_9StableMap_differenceByKey((&rv.m_data), (this_), (args-0));
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

TypedValue* tg_9StableMap_getIterator(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfObject;
        th_9StableMap_getIterator((&rv.m_data), (this_));
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
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_StableMap::t_map(HPHP::Variant const&)
_ZN4HPHP11c_StableMap5t_mapERKNS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
callback => rdx
*/

Value* th_9StableMap_map(Value* _rv, ObjectData* this_, TypedValue* callback) asm("_ZN4HPHP11c_StableMap5t_mapERKNS_7VariantE");

TypedValue* tg_9StableMap_map(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_type = KindOfObject;
        th_9StableMap_map((&rv.m_data), (this_), (args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("StableMap::map", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("StableMap::map");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_StableMap::t_filter(HPHP::Variant const&)
_ZN4HPHP11c_StableMap8t_filterERKNS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
callback => rdx
*/

Value* th_9StableMap_filter(Value* _rv, ObjectData* this_, TypedValue* callback) asm("_ZN4HPHP11c_StableMap8t_filterERKNS_7VariantE");

TypedValue* tg_9StableMap_filter(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_type = KindOfObject;
        th_9StableMap_filter((&rv.m_data), (this_), (args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("StableMap::filter", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("StableMap::filter");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_StableMap::t_zip(HPHP::Variant const&)
_ZN4HPHP11c_StableMap5t_zipERKNS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
iterable => rdx
*/

Value* th_9StableMap_zip(Value* _rv, ObjectData* this_, TypedValue* iterable) asm("_ZN4HPHP11c_StableMap5t_zipERKNS_7VariantE");

TypedValue* tg_9StableMap_zip(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_type = KindOfObject;
        th_9StableMap_zip((&rv.m_data), (this_), (args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("StableMap::zip", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("StableMap::zip");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
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

TypedValue* tg_9StableMap___get(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
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

TypedValue* tg_9StableMap___set(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
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

TypedValue* tg_9StableMap___isset(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
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

TypedValue* tg_9StableMap___unset(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
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
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_StableMap::ti_fromitems(char const*, HPHP::Variant const&)
_ZN4HPHP11c_StableMap12ti_fromitemsEPKcRKNS_7VariantE

(return value) => rax
_rv => rdi
cls_ => rsi
iterable => rdx
*/

Value* th_9StableMap_fromItems(Value* _rv, char const* cls_, TypedValue* iterable) asm("_ZN4HPHP11c_StableMap12ti_fromitemsEPKcRKNS_7VariantE");

TypedValue* tg_9StableMap_fromItems(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv.m_type = KindOfObject;
      th_9StableMap_fromItems((&rv.m_data), ("StableMap"), (args-0));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("StableMap::fromItems", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
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

TypedValue* tg_9StableMap___toString(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfString;
        th_9StableMap___toString((&rv.m_data), (this_));
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

TypedValue* tg_9StableMap_fromArray(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv.m_type = KindOfObject;
      th_9StableMap_fromArray((&rv.m_data), ("StableMap"), (args-0));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("StableMap::fromArray", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
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

TypedValue* tg_9StableMap_fromIterable(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv.m_type = KindOfObject;
      th_9StableMap_fromIterable((&rv.m_data), ("StableMap"), (args-0));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("StableMap::fromIterable", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
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
  new ((void *)inst) c_StableMapIterator(cls);
  return inst;
}

IMPLEMENT_CLASS(StableMapIterator);
/*
void HPHP::c_StableMapIterator::t___construct()
_ZN4HPHP19c_StableMapIterator13t___constructEv

this_ => rdi
*/

void th_17StableMapIterator___construct(ObjectData* this_) asm("_ZN4HPHP19c_StableMapIterator13t___constructEv");

TypedValue* tg_17StableMapIterator___construct(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
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

TypedValue* tg_17StableMapIterator_current(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
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

TypedValue* tg_17StableMapIterator_key(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
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

TypedValue* tg_17StableMapIterator_valid(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
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

TypedValue* tg_17StableMapIterator_next(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
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

TypedValue* tg_17StableMapIterator_rewind(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
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
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

HPHP::VM::Instance* new_Pair_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_Pair) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_Pair(cls);
  return inst;
}

IMPLEMENT_CLASS(Pair);
/*
void HPHP::c_Pair::t___construct()
_ZN4HPHP6c_Pair13t___constructEv

this_ => rdi
*/

void th_4Pair___construct(ObjectData* this_) asm("_ZN4HPHP6c_Pair13t___constructEv");

TypedValue* tg_4Pair___construct(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        th_4Pair___construct((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Pair::__construct", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Pair::__construct");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_Pair::t_isempty()
_ZN4HPHP6c_Pair9t_isemptyEv

(return value) => rax
this_ => rdi
*/

bool th_4Pair_isEmpty(ObjectData* this_) asm("_ZN4HPHP6c_Pair9t_isemptyEv");

TypedValue* tg_4Pair_isEmpty(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_4Pair_isEmpty((this_))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Pair::isEmpty", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Pair::isEmpty");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
long HPHP::c_Pair::t_count()
_ZN4HPHP6c_Pair7t_countEv

(return value) => rax
this_ => rdi
*/

long th_4Pair_count(ObjectData* this_) asm("_ZN4HPHP6c_Pair7t_countEv");

TypedValue* tg_4Pair_count(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)th_4Pair_count((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Pair::count", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Pair::count");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_Pair::t_items()
_ZN4HPHP6c_Pair7t_itemsEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_4Pair_items(Value* _rv, ObjectData* this_) asm("_ZN4HPHP6c_Pair7t_itemsEv");

TypedValue* tg_4Pair_items(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfObject;
        th_4Pair_items((&rv.m_data), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Pair::items", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Pair::items");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_Pair::t_keys()
_ZN4HPHP6c_Pair6t_keysEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_4Pair_keys(Value* _rv, ObjectData* this_) asm("_ZN4HPHP6c_Pair6t_keysEv");

TypedValue* tg_4Pair_keys(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfObject;
        th_4Pair_keys((&rv.m_data), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Pair::keys", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Pair::keys");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_Pair::t_view()
_ZN4HPHP6c_Pair6t_viewEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_4Pair_view(Value* _rv, ObjectData* this_) asm("_ZN4HPHP6c_Pair6t_viewEv");

TypedValue* tg_4Pair_view(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfObject;
        th_4Pair_view((&rv.m_data), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Pair::view", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Pair::view");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_Pair::t_kvzip()
_ZN4HPHP6c_Pair7t_kvzipEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_4Pair_kvzip(Value* _rv, ObjectData* this_) asm("_ZN4HPHP6c_Pair7t_kvzipEv");

TypedValue* tg_4Pair_kvzip(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfObject;
        th_4Pair_kvzip((&rv.m_data), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Pair::kvzip", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Pair::kvzip");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Array HPHP::c_Pair::t_toarray()
_ZN4HPHP6c_Pair9t_toarrayEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_4Pair_toArray(Value* _rv, ObjectData* this_) asm("_ZN4HPHP6c_Pair9t_toarrayEv");

TypedValue* tg_4Pair_toArray(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfArray;
        th_4Pair_toArray((&rv.m_data), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Pair::toArray", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Pair::toArray");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_Pair::t_getiterator()
_ZN4HPHP6c_Pair13t_getiteratorEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_4Pair_getIterator(Value* _rv, ObjectData* this_) asm("_ZN4HPHP6c_Pair13t_getiteratorEv");

TypedValue* tg_4Pair_getIterator(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfObject;
        th_4Pair_getIterator((&rv.m_data), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Pair::getIterator", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Pair::getIterator");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_Pair::t_map(HPHP::Variant const&)
_ZN4HPHP6c_Pair5t_mapERKNS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
callback => rdx
*/

Value* th_4Pair_map(Value* _rv, ObjectData* this_, TypedValue* callback) asm("_ZN4HPHP6c_Pair5t_mapERKNS_7VariantE");

TypedValue* tg_4Pair_map(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_type = KindOfObject;
        th_4Pair_map((&rv.m_data), (this_), (args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("Pair::map", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("Pair::map");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_Pair::t_filter(HPHP::Variant const&)
_ZN4HPHP6c_Pair8t_filterERKNS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
callback => rdx
*/

Value* th_4Pair_filter(Value* _rv, ObjectData* this_, TypedValue* callback) asm("_ZN4HPHP6c_Pair8t_filterERKNS_7VariantE");

TypedValue* tg_4Pair_filter(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_type = KindOfObject;
        th_4Pair_filter((&rv.m_data), (this_), (args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("Pair::filter", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("Pair::filter");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_Pair::t_zip(HPHP::Variant const&)
_ZN4HPHP6c_Pair5t_zipERKNS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
iterable => rdx
*/

Value* th_4Pair_zip(Value* _rv, ObjectData* this_, TypedValue* iterable) asm("_ZN4HPHP6c_Pair5t_zipERKNS_7VariantE");

TypedValue* tg_4Pair_zip(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_type = KindOfObject;
        th_4Pair_zip((&rv.m_data), (this_), (args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("Pair::zip", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("Pair::zip");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_Pair::t_at(HPHP::Variant const&)
_ZN4HPHP6c_Pair4t_atERKNS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
key => rdx
*/

TypedValue* th_4Pair_at(TypedValue* _rv, ObjectData* this_, TypedValue* key) asm("_ZN4HPHP6c_Pair4t_atERKNS_7VariantE");

TypedValue* tg_4Pair_at(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        th_4Pair_at((&(rv)), (this_), (args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("Pair::at", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("Pair::at");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_Pair::t_get(HPHP::Variant const&)
_ZN4HPHP6c_Pair5t_getERKNS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
key => rdx
*/

TypedValue* th_4Pair_get(TypedValue* _rv, ObjectData* this_, TypedValue* key) asm("_ZN4HPHP6c_Pair5t_getERKNS_7VariantE");

TypedValue* tg_4Pair_get(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        th_4Pair_get((&(rv)), (this_), (args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("Pair::get", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("Pair::get");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_Pair::t_containskey(HPHP::Variant const&)
_ZN4HPHP6c_Pair13t_containskeyERKNS_7VariantE

(return value) => rax
this_ => rdi
key => rsi
*/

bool th_4Pair_containsKey(ObjectData* this_, TypedValue* key) asm("_ZN4HPHP6c_Pair13t_containskeyERKNS_7VariantE");

TypedValue* tg_4Pair_containsKey(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_4Pair_containsKey((this_), (args-0))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("Pair::containsKey", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("Pair::containsKey");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

HPHP::VM::Instance* new_PairIterator_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_PairIterator) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_PairIterator(cls);
  return inst;
}

IMPLEMENT_CLASS(PairIterator);
/*
void HPHP::c_PairIterator::t___construct()
_ZN4HPHP14c_PairIterator13t___constructEv

this_ => rdi
*/

void th_12PairIterator___construct(ObjectData* this_) asm("_ZN4HPHP14c_PairIterator13t___constructEv");

TypedValue* tg_12PairIterator___construct(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        th_12PairIterator___construct((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("PairIterator::__construct", 0, 1);
      }
    } else {
      throw_instance_method_fatal("PairIterator::__construct");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_PairIterator::t_current()
_ZN4HPHP14c_PairIterator9t_currentEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_12PairIterator_current(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP14c_PairIterator9t_currentEv");

TypedValue* tg_12PairIterator_current(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_12PairIterator_current((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("PairIterator::current", 0, 1);
      }
    } else {
      throw_instance_method_fatal("PairIterator::current");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_PairIterator::t_key()
_ZN4HPHP14c_PairIterator5t_keyEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_12PairIterator_key(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP14c_PairIterator5t_keyEv");

TypedValue* tg_12PairIterator_key(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_12PairIterator_key((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("PairIterator::key", 0, 1);
      }
    } else {
      throw_instance_method_fatal("PairIterator::key");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_PairIterator::t_valid()
_ZN4HPHP14c_PairIterator7t_validEv

(return value) => rax
this_ => rdi
*/

bool th_12PairIterator_valid(ObjectData* this_) asm("_ZN4HPHP14c_PairIterator7t_validEv");

TypedValue* tg_12PairIterator_valid(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_12PairIterator_valid((this_))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("PairIterator::valid", 0, 1);
      }
    } else {
      throw_instance_method_fatal("PairIterator::valid");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
void HPHP::c_PairIterator::t_next()
_ZN4HPHP14c_PairIterator6t_nextEv

this_ => rdi
*/

void th_12PairIterator_next(ObjectData* this_) asm("_ZN4HPHP14c_PairIterator6t_nextEv");

TypedValue* tg_12PairIterator_next(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        th_12PairIterator_next((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("PairIterator::next", 0, 1);
      }
    } else {
      throw_instance_method_fatal("PairIterator::next");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
void HPHP::c_PairIterator::t_rewind()
_ZN4HPHP14c_PairIterator8t_rewindEv

this_ => rdi
*/

void th_12PairIterator_rewind(ObjectData* this_) asm("_ZN4HPHP14c_PairIterator8t_rewindEv");

TypedValue* tg_12PairIterator_rewind(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        th_12PairIterator_rewind((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("PairIterator::rewind", 0, 1);
      }
    } else {
      throw_instance_method_fatal("PairIterator::rewind");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}


} // !HPHP

