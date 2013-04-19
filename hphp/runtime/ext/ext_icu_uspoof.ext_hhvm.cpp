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


HPHP::VM::Instance* new_SpoofChecker_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_SpoofChecker) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_SpoofChecker(cls);
  return inst;
}

IMPLEMENT_CLASS(SpoofChecker);
void th_12SpoofChecker___construct(ObjectData* this_) asm("_ZN4HPHP14c_SpoofChecker13t___constructEv");

TypedValue* tg_12SpoofChecker___construct(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfNull;
      th_12SpoofChecker___construct((this_));
    } else {
      throw_toomany_arguments_nr("SpoofChecker::__construct", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SpoofChecker::__construct");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_12SpoofChecker_isSuspicious(ObjectData* this_, Value* text, TypedValue* issuesFound) asm("_ZN4HPHP14c_SpoofChecker14t_issuspiciousERKNS_6StringERKNS_14VRefParamValueE");

void tg1_12SpoofChecker_isSuspicious(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_12SpoofChecker_isSuspicious(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfBoolean;
  VRefParamValue defVal1 = uninit_null();
  rv->m_data.num = (th_12SpoofChecker_isSuspicious((this_), &args[-0].m_data, (count > 1) ? (args-1) : (TypedValue*)(&defVal1))) ? 1LL : 0LL;
}

TypedValue* tg_12SpoofChecker_isSuspicious(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 1 && count <= 2) {
      if (IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfBoolean;
        VRefParamValue defVal1 = uninit_null();
        rv->m_data.num = (th_12SpoofChecker_isSuspicious((this_), &args[-0].m_data, (count > 1) ? (args-1) : (TypedValue*)(&defVal1))) ? 1LL : 0LL;
      } else {
        tg1_12SpoofChecker_isSuspicious(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("SpoofChecker::isSuspicious", count, 1, 2, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SpoofChecker::isSuspicious");
  }
  frame_free_locals_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_12SpoofChecker_areConfusable(ObjectData* this_, Value* s1, Value* s2, TypedValue* issuesFound) asm("_ZN4HPHP14c_SpoofChecker15t_areconfusableERKNS_6StringES3_RKNS_14VRefParamValueE");

void tg1_12SpoofChecker_areConfusable(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_12SpoofChecker_areConfusable(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  VRefParamValue defVal2 = uninit_null();
  rv->m_data.num = (th_12SpoofChecker_areConfusable((this_), &args[-0].m_data, &args[-1].m_data, (count > 2) ? (args-2) : (TypedValue*)(&defVal2))) ? 1LL : 0LL;
}

TypedValue* tg_12SpoofChecker_areConfusable(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 2 && count <= 3) {
      if (IS_STRING_TYPE((args - 1)->m_type) &&
          IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfBoolean;
        VRefParamValue defVal2 = uninit_null();
        rv->m_data.num = (th_12SpoofChecker_areConfusable((this_), &args[-0].m_data, &args[-1].m_data, (count > 2) ? (args-2) : (TypedValue*)(&defVal2))) ? 1LL : 0LL;
      } else {
        tg1_12SpoofChecker_areConfusable(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("SpoofChecker::areConfusable", count, 2, 3, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SpoofChecker::areConfusable");
  }
  frame_free_locals_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void th_12SpoofChecker_setAllowedLocales(ObjectData* this_, Value* localesList) asm("_ZN4HPHP14c_SpoofChecker19t_setallowedlocalesERKNS_6StringE");

void tg1_12SpoofChecker_setAllowedLocales(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_12SpoofChecker_setAllowedLocales(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfNull;
  th_12SpoofChecker_setAllowedLocales((this_), &args[-0].m_data);
}

TypedValue* tg_12SpoofChecker_setAllowedLocales(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if (IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfNull;
        th_12SpoofChecker_setAllowedLocales((this_), &args[-0].m_data);
      } else {
        tg1_12SpoofChecker_setAllowedLocales(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("SpoofChecker::setAllowedLocales", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SpoofChecker::setAllowedLocales");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void th_12SpoofChecker_setChecks(ObjectData* this_, int checks) asm("_ZN4HPHP14c_SpoofChecker11t_setchecksEi");

void tg1_12SpoofChecker_setChecks(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_12SpoofChecker_setChecks(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  rv->m_type = KindOfNull;
  th_12SpoofChecker_setChecks((this_), (int)(args[-0].m_data.num));
}

TypedValue* tg_12SpoofChecker_setChecks(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if ((args - 0)->m_type == KindOfInt64) {
        rv->m_type = KindOfNull;
        th_12SpoofChecker_setChecks((this_), (int)(args[-0].m_data.num));
      } else {
        tg1_12SpoofChecker_setChecks(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("SpoofChecker::setChecks", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SpoofChecker::setChecks");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

} // namespace HPHP
