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

#include "runtime/ext_hhvm/ext_hhvm.h"
#include "runtime/base/builtin_functions.h"
#include "runtime/base/array/array_init.h"
#include "runtime/ext/ext.h"
#include "runtime/vm/class.h"
#include "runtime/vm/runtime.h"
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
void th_24SetResultToRefWaitHandle___construct(ObjectData* this_) asm("_ZN4HPHP26c_SetResultToRefWaitHandle13t___constructEv");

TypedValue* tg_24SetResultToRefWaitHandle___construct(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfNull;
      th_24SetResultToRefWaitHandle___construct((this_));
    } else {
      throw_toomany_arguments_nr("SetResultToRefWaitHandle::__construct", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SetResultToRefWaitHandle::__construct");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* th_24SetResultToRefWaitHandle_create(Value* _rv, char const* cls_, Value* wait_handle, TypedValue* ref) asm("_ZN4HPHP26c_SetResultToRefWaitHandle9ti_createEPKcRKNS_6ObjectERKNS_14VRefParamValueE");

void tg1_24SetResultToRefWaitHandle_create(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void tg1_24SetResultToRefWaitHandle_create(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfObject;
  th_24SetResultToRefWaitHandle_create(&(rv->m_data), "SetResultToRefWaitHandle", &args[-0].m_data, (args-1));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* tg_24SetResultToRefWaitHandle_create(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (count == 2) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfObject;
      th_24SetResultToRefWaitHandle_create(&(rv->m_data), "SetResultToRefWaitHandle", &args[-0].m_data, (args-1));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      tg1_24SetResultToRefWaitHandle_create(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("SetResultToRefWaitHandle::create", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

} // namespace HPHP
