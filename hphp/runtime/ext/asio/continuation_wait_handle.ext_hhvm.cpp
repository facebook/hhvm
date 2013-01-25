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

HPHP::VM::Instance* new_ContinuationWaitHandle_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_ContinuationWaitHandle) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_ContinuationWaitHandle(ObjectStaticCallbacks::encodeVMClass(cls));
  return inst;
}

/*
void HPHP::c_ContinuationWaitHandle::t___construct()
_ZN4HPHP24c_ContinuationWaitHandle13t___constructEv

this_ => rdi
*/

void th_22ContinuationWaitHandle___construct(ObjectData* this_) asm("_ZN4HPHP24c_ContinuationWaitHandle13t___constructEv");

TypedValue* tg_22ContinuationWaitHandle___construct(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        th_22ContinuationWaitHandle___construct((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("ContinuationWaitHandle::__construct", 0, 1);
      }
    } else {
      throw_instance_method_fatal("ContinuationWaitHandle::__construct");
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
HPHP::Object HPHP::c_ContinuationWaitHandle::ti_start(char const*, HPHP::Object const&, int)
_ZN4HPHP24c_ContinuationWaitHandle8ti_startEPKcRKNS_6ObjectEi

(return value) => rax
_rv => rdi
cls_ => rsi
continuation => rdx
prio => rcx
*/

Value* th_22ContinuationWaitHandle_start(Value* _rv, char const* cls_, Value* continuation, int prio) asm("_ZN4HPHP24c_ContinuationWaitHandle8ti_startEPKcRKNS_6ObjectEi");

TypedValue* tg1_22ContinuationWaitHandle_start(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue* tg1_22ContinuationWaitHandle_start(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfObject;
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
  th_22ContinuationWaitHandle_start((Value*)(rv), ("ContinuationWaitHandle"), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_22ContinuationWaitHandle_start(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64) && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfObject;
        th_22ContinuationWaitHandle_start((Value*)(&(rv)), ("ContinuationWaitHandle"), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        tg1_22ContinuationWaitHandle_start(&rv, ar, count );
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("ContinuationWaitHandle::start", count, 1, 2, 1);
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
void HPHP::c_ContinuationWaitHandle::ti_markcurrentassucceeded(char const*, HPHP::Variant const&)
_ZN4HPHP24c_ContinuationWaitHandle25ti_markcurrentassucceededEPKcRKNS_7VariantE

cls_ => rdi
result => rsi
*/

void th_22ContinuationWaitHandle_markCurrentAsSucceeded(char const* cls_, TypedValue* result) asm("_ZN4HPHP24c_ContinuationWaitHandle25ti_markcurrentassucceededEPKcRKNS_7VariantE");

TypedValue* tg_22ContinuationWaitHandle_markCurrentAsSucceeded(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv.m_data.num = 0LL;
      rv._count = 0;
      rv.m_type = KindOfNull;
      th_22ContinuationWaitHandle_markCurrentAsSucceeded(("ContinuationWaitHandle"), (args-0));
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("ContinuationWaitHandle::markCurrentAsSucceeded", count, 1, 1, 1);
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
void HPHP::c_ContinuationWaitHandle::ti_markcurrentastailcall(char const*)
_ZN4HPHP24c_ContinuationWaitHandle24ti_markcurrentastailcallEPKc

cls_ => rdi
*/

void th_22ContinuationWaitHandle_markCurrentAsTailCall(char const* cls_) asm("_ZN4HPHP24c_ContinuationWaitHandle24ti_markcurrentastailcallEPKc");

TypedValue* tg_22ContinuationWaitHandle_markCurrentAsTailCall(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_data.num = 0LL;
      rv._count = 0;
      rv.m_type = KindOfNull;
      th_22ContinuationWaitHandle_markCurrentAsTailCall(("ContinuationWaitHandle"));
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("ContinuationWaitHandle::markCurrentAsTailCall", 0, 1);
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
HPHP::Object HPHP::c_ContinuationWaitHandle::t_getprivdata()
_ZN4HPHP24c_ContinuationWaitHandle13t_getprivdataEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_22ContinuationWaitHandle_getPrivData(Value* _rv, ObjectData* this_) asm("_ZN4HPHP24c_ContinuationWaitHandle13t_getprivdataEv");

TypedValue* tg_22ContinuationWaitHandle_getPrivData(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfObject;
        th_22ContinuationWaitHandle_getPrivData((Value*)(&(rv)), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("ContinuationWaitHandle::getPrivData", 0, 1);
      }
    } else {
      throw_instance_method_fatal("ContinuationWaitHandle::getPrivData");
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
void HPHP::c_ContinuationWaitHandle::t_setprivdata(HPHP::Object const&)
_ZN4HPHP24c_ContinuationWaitHandle13t_setprivdataERKNS_6ObjectE

this_ => rdi
data => rsi
*/

void th_22ContinuationWaitHandle_setPrivData(ObjectData* this_, Value* data) asm("_ZN4HPHP24c_ContinuationWaitHandle13t_setprivdataERKNS_6ObjectE");

TypedValue* tg1_22ContinuationWaitHandle_setPrivData(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_22ContinuationWaitHandle_setPrivData(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  tvCastToObjectInPlace(args-0);
  th_22ContinuationWaitHandle_setPrivData((this_), (Value*)(args-0));
  return rv;
}

TypedValue* tg_22ContinuationWaitHandle_setPrivData(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if ((args-0)->m_type == KindOfObject) {
          rv.m_data.num = 0LL;
          rv._count = 0;
          rv.m_type = KindOfNull;
          th_22ContinuationWaitHandle_setPrivData((this_), (Value*)(args-0));
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_22ContinuationWaitHandle_setPrivData(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("ContinuationWaitHandle::setPrivData", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("ContinuationWaitHandle::setPrivData");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}


} // !HPHP

