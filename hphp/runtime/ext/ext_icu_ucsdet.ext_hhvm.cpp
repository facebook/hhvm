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


HPHP::VM::Instance* new_EncodingDetector_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_EncodingDetector) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_EncodingDetector(cls);
  return inst;
}

IMPLEMENT_CLASS(EncodingDetector);
void th_16EncodingDetector___construct(ObjectData* this_) asm("_ZN4HPHP18c_EncodingDetector13t___constructEv");

TypedValue* tg_16EncodingDetector___construct(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfNull;
      th_16EncodingDetector___construct((this_));
    } else {
      throw_toomany_arguments_nr("EncodingDetector::__construct", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("EncodingDetector::__construct");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void th_16EncodingDetector_setText(ObjectData* this_, Value* text) asm("_ZN4HPHP18c_EncodingDetector9t_settextERKNS_6StringE");

void tg1_16EncodingDetector_setText(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_16EncodingDetector_setText(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfNull;
  th_16EncodingDetector_setText((this_), &args[-0].m_data);
}

TypedValue* tg_16EncodingDetector_setText(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if (IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfNull;
        th_16EncodingDetector_setText((this_), &args[-0].m_data);
      } else {
        tg1_16EncodingDetector_setText(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("EncodingDetector::setText", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("EncodingDetector::setText");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void th_16EncodingDetector_setDeclaredEncoding(ObjectData* this_, Value* text) asm("_ZN4HPHP18c_EncodingDetector21t_setdeclaredencodingERKNS_6StringE");

void tg1_16EncodingDetector_setDeclaredEncoding(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_16EncodingDetector_setDeclaredEncoding(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfNull;
  th_16EncodingDetector_setDeclaredEncoding((this_), &args[-0].m_data);
}

TypedValue* tg_16EncodingDetector_setDeclaredEncoding(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if (IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfNull;
        th_16EncodingDetector_setDeclaredEncoding((this_), &args[-0].m_data);
      } else {
        tg1_16EncodingDetector_setDeclaredEncoding(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("EncodingDetector::setDeclaredEncoding", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("EncodingDetector::setDeclaredEncoding");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* th_16EncodingDetector_detect(Value* _rv, ObjectData* this_) asm("_ZN4HPHP18c_EncodingDetector8t_detectEv");

TypedValue* tg_16EncodingDetector_detect(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfObject;
      th_16EncodingDetector_detect(&(rv->m_data), (this_));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      throw_toomany_arguments_nr("EncodingDetector::detect", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("EncodingDetector::detect");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* th_16EncodingDetector_detectAll(Value* _rv, ObjectData* this_) asm("_ZN4HPHP18c_EncodingDetector11t_detectallEv");

TypedValue* tg_16EncodingDetector_detectAll(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfArray;
      th_16EncodingDetector_detectAll(&(rv->m_data), (this_));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      throw_toomany_arguments_nr("EncodingDetector::detectAll", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("EncodingDetector::detectAll");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}


HPHP::VM::Instance* new_EncodingMatch_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_EncodingMatch) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_EncodingMatch(cls);
  return inst;
}

IMPLEMENT_CLASS(EncodingMatch);
void th_13EncodingMatch___construct(ObjectData* this_) asm("_ZN4HPHP15c_EncodingMatch13t___constructEv");

TypedValue* tg_13EncodingMatch___construct(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfNull;
      th_13EncodingMatch___construct((this_));
    } else {
      throw_toomany_arguments_nr("EncodingMatch::__construct", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("EncodingMatch::__construct");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_13EncodingMatch_isValid(ObjectData* this_) asm("_ZN4HPHP15c_EncodingMatch9t_isvalidEv");

TypedValue* tg_13EncodingMatch_isValid(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (th_13EncodingMatch_isValid((this_))) ? 1LL : 0LL;
    } else {
      throw_toomany_arguments_nr("EncodingMatch::isValid", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("EncodingMatch::isValid");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* th_13EncodingMatch_getEncoding(Value* _rv, ObjectData* this_) asm("_ZN4HPHP15c_EncodingMatch13t_getencodingEv");

TypedValue* tg_13EncodingMatch_getEncoding(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfString;
      th_13EncodingMatch_getEncoding(&(rv->m_data), (this_));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      throw_toomany_arguments_nr("EncodingMatch::getEncoding", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("EncodingMatch::getEncoding");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long th_13EncodingMatch_getConfidence(ObjectData* this_) asm("_ZN4HPHP15c_EncodingMatch15t_getconfidenceEv");

TypedValue* tg_13EncodingMatch_getConfidence(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)th_13EncodingMatch_getConfidence((this_));
    } else {
      throw_toomany_arguments_nr("EncodingMatch::getConfidence", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("EncodingMatch::getConfidence");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* th_13EncodingMatch_getLanguage(Value* _rv, ObjectData* this_) asm("_ZN4HPHP15c_EncodingMatch13t_getlanguageEv");

TypedValue* tg_13EncodingMatch_getLanguage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfString;
      th_13EncodingMatch_getLanguage(&(rv->m_data), (this_));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      throw_toomany_arguments_nr("EncodingMatch::getLanguage", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("EncodingMatch::getLanguage");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* th_13EncodingMatch_getUTF8(Value* _rv, ObjectData* this_) asm("_ZN4HPHP15c_EncodingMatch9t_getutf8Ev");

TypedValue* tg_13EncodingMatch_getUTF8(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfString;
      th_13EncodingMatch_getUTF8(&(rv->m_data), (this_));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      throw_toomany_arguments_nr("EncodingMatch::getUTF8", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("EncodingMatch::getUTF8");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

} // namespace HPHP
