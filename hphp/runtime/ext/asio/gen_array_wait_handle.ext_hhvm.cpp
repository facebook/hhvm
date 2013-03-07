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

HPHP::VM::Instance* new_GenArrayWaitHandle_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_GenArrayWaitHandle) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_GenArrayWaitHandle(cls);
  return inst;
}

IMPLEMENT_CLASS(GenArrayWaitHandle);
/*
void HPHP::c_GenArrayWaitHandle::t___construct()
_ZN4HPHP20c_GenArrayWaitHandle13t___constructEv

this_ => rdi
*/

void th_18GenArrayWaitHandle___construct(ObjectData* this_) asm("_ZN4HPHP20c_GenArrayWaitHandle13t___constructEv");

TypedValue* tg_18GenArrayWaitHandle___construct(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        th_18GenArrayWaitHandle___construct((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("GenArrayWaitHandle::__construct", 0, 1);
      }
    } else {
      throw_instance_method_fatal("GenArrayWaitHandle::__construct");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_GenArrayWaitHandle::ti_create(char const*, HPHP::Array const&)
_ZN4HPHP20c_GenArrayWaitHandle9ti_createEPKcRKNS_5ArrayE

(return value) => rax
_rv => rdi
cls_ => rsi
dependencies => rdx
*/

Value* th_18GenArrayWaitHandle_create(Value* _rv, char const* cls_, Value* dependencies) asm("_ZN4HPHP20c_GenArrayWaitHandle9ti_createEPKcRKNS_5ArrayE");

TypedValue* tg1_18GenArrayWaitHandle_create(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue* tg1_18GenArrayWaitHandle_create(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  tvCastToArrayInPlace(args-0);
  th_18GenArrayWaitHandle_create((Value*)(rv), ("GenArrayWaitHandle"), (Value*)(args-0));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_18GenArrayWaitHandle_create(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfArray) {
        rv.m_type = KindOfObject;
        th_18GenArrayWaitHandle_create((Value*)(&(rv)), ("GenArrayWaitHandle"), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        tg1_18GenArrayWaitHandle_create(&rv, ar, count );
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("GenArrayWaitHandle::create", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}


} // !HPHP

