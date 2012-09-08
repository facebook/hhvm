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

HPHP::VM::Instance* new_WaitHandle_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_WaitHandle) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_WaitHandle(ObjectStaticCallbacks::encodeVMClass(cls));
  return inst;
}

/*
void HPHP::c_WaitHandle::t___construct()
_ZN4HPHP12c_WaitHandle13t___constructEv

this_ => rdi
*/

void th_10WaitHandle___construct(ObjectData* this_) asm("_ZN4HPHP12c_WaitHandle13t___constructEv");

TypedValue* tg_10WaitHandle___construct(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        th_10WaitHandle___construct((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("WaitHandle::__construct", 0, 1);
      }
    } else {
      throw_instance_method_fatal("WaitHandle::__construct");
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
void HPHP::c_WaitHandle::t_import()
_ZN4HPHP12c_WaitHandle8t_importEv

this_ => rdi
*/

void th_10WaitHandle_import(ObjectData* this_) asm("_ZN4HPHP12c_WaitHandle8t_importEv");

TypedValue* tg_10WaitHandle_import(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        th_10WaitHandle_import((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("WaitHandle::import", 0, 1);
      }
    } else {
      throw_instance_method_fatal("WaitHandle::import");
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
HPHP::Variant HPHP::c_WaitHandle::t_join()
_ZN4HPHP12c_WaitHandle6t_joinEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_10WaitHandle_join(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP12c_WaitHandle6t_joinEv");

TypedValue* tg_10WaitHandle_join(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_10WaitHandle_join((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("WaitHandle::join", 0, 1);
      }
    } else {
      throw_instance_method_fatal("WaitHandle::join");
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
long long HPHP::c_WaitHandle::t_getid()
_ZN4HPHP12c_WaitHandle7t_getidEv

(return value) => rax
this_ => rdi
*/

long long th_10WaitHandle_getID(ObjectData* this_) asm("_ZN4HPHP12c_WaitHandle7t_getidEv");

TypedValue* tg_10WaitHandle_getID(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (long long)th_10WaitHandle_getID((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("WaitHandle::getID", 0, 1);
      }
    } else {
      throw_instance_method_fatal("WaitHandle::getID");
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
HPHP::String HPHP::c_WaitHandle::t_getname()
_ZN4HPHP12c_WaitHandle9t_getnameEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_10WaitHandle_getName(Value* _rv, ObjectData* this_) asm("_ZN4HPHP12c_WaitHandle9t_getnameEv");

TypedValue* tg_10WaitHandle_getName(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfString;
        th_10WaitHandle_getName((Value*)(&(rv)), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("WaitHandle::getName", 0, 1);
      }
    } else {
      throw_instance_method_fatal("WaitHandle::getName");
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
HPHP::Object HPHP::c_WaitHandle::t_getexceptioniffailed()
_ZN4HPHP12c_WaitHandle22t_getexceptioniffailedEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_10WaitHandle_getExceptionIfFailed(Value* _rv, ObjectData* this_) asm("_ZN4HPHP12c_WaitHandle22t_getexceptioniffailedEv");

TypedValue* tg_10WaitHandle_getExceptionIfFailed(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfObject;
        th_10WaitHandle_getExceptionIfFailed((Value*)(&(rv)), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("WaitHandle::getExceptionIfFailed", 0, 1);
      }
    } else {
      throw_instance_method_fatal("WaitHandle::getExceptionIfFailed");
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

