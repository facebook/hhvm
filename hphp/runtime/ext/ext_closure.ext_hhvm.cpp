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

HPHP::VM::Instance* new_Closure_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_Closure) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_Closure(ObjectStaticCallbacks::encodeVMClass(cls));
  return inst;
}

/*
void HPHP::c_Closure::t___construct()
_ZN4HPHP9c_Closure13t___constructEv

this_ => rdi
*/

void th_7Closure___construct(ObjectData* this_) asm("_ZN4HPHP9c_Closure13t___constructEv");

TypedValue* tg_7Closure___construct(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        th_7Closure___construct((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Closure::__construct", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Closure::__construct");
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
HPHP::Variant HPHP::c_Closure::t___invoke(int, HPHP::Array const&)
_ZN4HPHP9c_Closure10t___invokeEiRKNS_5ArrayE

(return value) => rax
_rv => rdi
this_ => rsi
_argc => rdx
_argv => rcx
*/

TypedValue* th_7Closure___invoke(TypedValue* _rv, ObjectData* this_, int64_t _argc, Value* _argv) asm("_ZN4HPHP9c_Closure10t___invokeEiRKNS_5ArrayE");

TypedValue* tg_7Closure___invoke(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      Array extraArgs;
      {
        ArrayInit ai(count-0, false);
        for (int64_t i = 0; i < count; ++i) {
          TypedValue* extraArg = ar->getExtraArg(i-0);
          if (tvIsStronglyBound(extraArg)) {
            ai.setRef(i-0, tvAsVariant(extraArg));
          } else {
            ai.set(i-0, tvAsVariant(extraArg));
          }
        }
        extraArgs = ai.create();
      }
      th_7Closure___invoke((&(rv)), (this_), (count), (Value*)(&extraArgs));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_instance_method_fatal("Closure::__invoke");
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
HPHP::Variant HPHP::c_Closure::t___clone()
_ZN4HPHP9c_Closure9t___cloneEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_7Closure___clone(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP9c_Closure9t___cloneEv");

TypedValue* tg_7Closure___clone(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_7Closure___clone((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Closure::__clone", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Closure::__clone");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

HPHP::VM::Instance* new_DummyClosure_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_DummyClosure) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_DummyClosure(ObjectStaticCallbacks::encodeVMClass(cls));
  return inst;
}

/*
void HPHP::c_DummyClosure::t___construct()
_ZN4HPHP14c_DummyClosure13t___constructEv

this_ => rdi
*/

void th_12DummyClosure___construct(ObjectData* this_) asm("_ZN4HPHP14c_DummyClosure13t___constructEv");

TypedValue* tg_12DummyClosure___construct(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        th_12DummyClosure___construct((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DummyClosure::__construct", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DummyClosure::__construct");
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

