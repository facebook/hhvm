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


HPHP::VM::Instance* new_ContinuationWaitHandle_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_ContinuationWaitHandle) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_ContinuationWaitHandle(cls);
  return inst;
}

IMPLEMENT_CLASS(ContinuationWaitHandle);
void th_22ContinuationWaitHandle___construct(ObjectData* this_) asm("_ZN4HPHP24c_ContinuationWaitHandle13t___constructEv");

TypedValue* tg_22ContinuationWaitHandle___construct(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfNull;
      th_22ContinuationWaitHandle___construct((this_));
    } else {
      throw_toomany_arguments_nr("ContinuationWaitHandle::__construct", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("ContinuationWaitHandle::__construct");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* th_22ContinuationWaitHandle_start(Value* _rv, char const* cls_, Value* continuation) asm("_ZN4HPHP24c_ContinuationWaitHandle8ti_startEPKcRKNS_6ObjectE");

void tg1_22ContinuationWaitHandle_start(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void tg1_22ContinuationWaitHandle_start(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfObject;
  th_22ContinuationWaitHandle_start(&(rv->m_data), "ContinuationWaitHandle", &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* tg_22ContinuationWaitHandle_start(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfObject;
      th_22ContinuationWaitHandle_start(&(rv->m_data), "ContinuationWaitHandle", &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      tg1_22ContinuationWaitHandle_start(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("ContinuationWaitHandle::start", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* th_22ContinuationWaitHandle_getPrivData(Value* _rv, ObjectData* this_) asm("_ZN4HPHP24c_ContinuationWaitHandle13t_getprivdataEv");

TypedValue* tg_22ContinuationWaitHandle_getPrivData(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfObject;
      th_22ContinuationWaitHandle_getPrivData(&(rv->m_data), (this_));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      throw_toomany_arguments_nr("ContinuationWaitHandle::getPrivData", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("ContinuationWaitHandle::getPrivData");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void th_22ContinuationWaitHandle_setPrivData(ObjectData* this_, Value* data) asm("_ZN4HPHP24c_ContinuationWaitHandle13t_setprivdataERKNS_6ObjectE");

void tg1_22ContinuationWaitHandle_setPrivData(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_22ContinuationWaitHandle_setPrivData(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfNull;
  th_22ContinuationWaitHandle_setPrivData((this_), &args[-0].m_data);
}

TypedValue* tg_22ContinuationWaitHandle_setPrivData(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if ((args - 0)->m_type == KindOfObject) {
        rv->m_type = KindOfNull;
        th_22ContinuationWaitHandle_setPrivData((this_), &args[-0].m_data);
      } else {
        tg1_22ContinuationWaitHandle_setPrivData(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("ContinuationWaitHandle::setPrivData", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("ContinuationWaitHandle::setPrivData");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

} // namespace HPHP
