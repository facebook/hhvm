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

HPHP::VM::Instance* new_SetResultToRefWaitHandle_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_SetResultToRefWaitHandle) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_SetResultToRefWaitHandle(cls);
  return inst;
}

IMPLEMENT_CLASS(SetResultToRefWaitHandle);
/*
void HPHP::c_SetResultToRefWaitHandle::t___construct()
_ZN4HPHP26c_SetResultToRefWaitHandle13t___constructEv

this_ => rdi
*/

void th_24SetResultToRefWaitHandle___construct(ObjectData* this_) asm("_ZN4HPHP26c_SetResultToRefWaitHandle13t___constructEv");

TypedValue* tg_24SetResultToRefWaitHandle___construct(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        th_24SetResultToRefWaitHandle___construct((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("SetResultToRefWaitHandle::__construct", 0, 1);
      }
    } else {
      throw_instance_method_fatal("SetResultToRefWaitHandle::__construct");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_SetResultToRefWaitHandle::ti_create(char const*, HPHP::Object const&, HPHP::VRefParamValue const&)
_ZN4HPHP26c_SetResultToRefWaitHandle9ti_createEPKcRKNS_6ObjectERKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
cls_ => rsi
wait_handle => rdx
ref => rcx
*/

Value* th_24SetResultToRefWaitHandle_create(Value* _rv, char const* cls_, Value* wait_handle, TypedValue* ref) asm("_ZN4HPHP26c_SetResultToRefWaitHandle9ti_createEPKcRKNS_6ObjectERKNS_14VRefParamValueE");

TypedValue* tg1_24SetResultToRefWaitHandle_create(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue* tg1_24SetResultToRefWaitHandle_create(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  tvCastToObjectInPlace(args-0);
  th_24SetResultToRefWaitHandle_create((&rv->m_data), ("SetResultToRefWaitHandle"), &args[-0].m_data, (args-1));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_24SetResultToRefWaitHandle_create(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfObject;
        th_24SetResultToRefWaitHandle_create((&rv.m_data), ("SetResultToRefWaitHandle"), &args[-0].m_data, (args-1));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        tg1_24SetResultToRefWaitHandle_create(&rv, ar, count );
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("SetResultToRefWaitHandle::create", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}


} // !HPHP

