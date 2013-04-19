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


HPHP::VM::Instance* new_GenArrayWaitHandle_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_GenArrayWaitHandle) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_GenArrayWaitHandle(cls);
  return inst;
}

IMPLEMENT_CLASS(GenArrayWaitHandle);
void th_18GenArrayWaitHandle___construct(ObjectData* this_) asm("_ZN4HPHP20c_GenArrayWaitHandle13t___constructEv");

TypedValue* tg_18GenArrayWaitHandle___construct(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfNull;
      th_18GenArrayWaitHandle___construct((this_));
    } else {
      throw_toomany_arguments_nr("GenArrayWaitHandle::__construct", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("GenArrayWaitHandle::__construct");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* th_18GenArrayWaitHandle_create(Value* _rv, char const* cls_, Value* dependencies) asm("_ZN4HPHP20c_GenArrayWaitHandle9ti_createEPKcRKNS_5ArrayE");

void tg1_18GenArrayWaitHandle_create(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void tg1_18GenArrayWaitHandle_create(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToArrayInPlace(args-0);
  rv->m_type = KindOfObject;
  th_18GenArrayWaitHandle_create(&(rv->m_data), "GenArrayWaitHandle", &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* tg_18GenArrayWaitHandle_create(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (count == 1) {
    if ((args - 0)->m_type == KindOfArray) {
      rv->m_type = KindOfObject;
      th_18GenArrayWaitHandle_create(&(rv->m_data), "GenArrayWaitHandle", &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      tg1_18GenArrayWaitHandle_create(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("GenArrayWaitHandle::create", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

} // namespace HPHP
