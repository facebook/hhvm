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

HPHP::VM::Instance* new_EncodingDetector_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_EncodingDetector) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_EncodingDetector(cls);
  return inst;
}

IMPLEMENT_CLASS(EncodingDetector);
/*
void HPHP::c_EncodingDetector::t___construct()
_ZN4HPHP18c_EncodingDetector13t___constructEv

this_ => rdi
*/

void th_16EncodingDetector___construct(ObjectData* this_) asm("_ZN4HPHP18c_EncodingDetector13t___constructEv");

TypedValue* tg_16EncodingDetector___construct(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        th_16EncodingDetector___construct((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("EncodingDetector::__construct", 0, 1);
      }
    } else {
      throw_instance_method_fatal("EncodingDetector::__construct");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
void HPHP::c_EncodingDetector::t_settext(HPHP::String const&)
_ZN4HPHP18c_EncodingDetector9t_settextERKNS_6StringE

this_ => rdi
text => rsi
*/

void th_16EncodingDetector_setText(ObjectData* this_, Value* text) asm("_ZN4HPHP18c_EncodingDetector9t_settextERKNS_6StringE");

TypedValue* tg1_16EncodingDetector_setText(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_16EncodingDetector_setText(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  tvCastToStringInPlace(args-0);
  th_16EncodingDetector_setText((this_), &args[-0].m_data);
  return rv;
}

TypedValue* tg_16EncodingDetector_setText(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_data.num = 0LL;
          rv.m_type = KindOfNull;
          th_16EncodingDetector_setText((this_), &args[-0].m_data);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_16EncodingDetector_setText(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("EncodingDetector::setText", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("EncodingDetector::setText");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
void HPHP::c_EncodingDetector::t_setdeclaredencoding(HPHP::String const&)
_ZN4HPHP18c_EncodingDetector21t_setdeclaredencodingERKNS_6StringE

this_ => rdi
text => rsi
*/

void th_16EncodingDetector_setDeclaredEncoding(ObjectData* this_, Value* text) asm("_ZN4HPHP18c_EncodingDetector21t_setdeclaredencodingERKNS_6StringE");

TypedValue* tg1_16EncodingDetector_setDeclaredEncoding(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_16EncodingDetector_setDeclaredEncoding(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  tvCastToStringInPlace(args-0);
  th_16EncodingDetector_setDeclaredEncoding((this_), &args[-0].m_data);
  return rv;
}

TypedValue* tg_16EncodingDetector_setDeclaredEncoding(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_data.num = 0LL;
          rv.m_type = KindOfNull;
          th_16EncodingDetector_setDeclaredEncoding((this_), &args[-0].m_data);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_16EncodingDetector_setDeclaredEncoding(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("EncodingDetector::setDeclaredEncoding", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("EncodingDetector::setDeclaredEncoding");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_EncodingDetector::t_detect()
_ZN4HPHP18c_EncodingDetector8t_detectEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_16EncodingDetector_detect(Value* _rv, ObjectData* this_) asm("_ZN4HPHP18c_EncodingDetector8t_detectEv");

TypedValue* tg_16EncodingDetector_detect(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfObject;
        th_16EncodingDetector_detect((&rv.m_data), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("EncodingDetector::detect", 0, 1);
      }
    } else {
      throw_instance_method_fatal("EncodingDetector::detect");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Array HPHP::c_EncodingDetector::t_detectall()
_ZN4HPHP18c_EncodingDetector11t_detectallEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_16EncodingDetector_detectAll(Value* _rv, ObjectData* this_) asm("_ZN4HPHP18c_EncodingDetector11t_detectallEv");

TypedValue* tg_16EncodingDetector_detectAll(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfArray;
        th_16EncodingDetector_detectAll((&rv.m_data), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("EncodingDetector::detectAll", 0, 1);
      }
    } else {
      throw_instance_method_fatal("EncodingDetector::detectAll");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
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
/*
void HPHP::c_EncodingMatch::t___construct()
_ZN4HPHP15c_EncodingMatch13t___constructEv

this_ => rdi
*/

void th_13EncodingMatch___construct(ObjectData* this_) asm("_ZN4HPHP15c_EncodingMatch13t___constructEv");

TypedValue* tg_13EncodingMatch___construct(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        th_13EncodingMatch___construct((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("EncodingMatch::__construct", 0, 1);
      }
    } else {
      throw_instance_method_fatal("EncodingMatch::__construct");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_EncodingMatch::t_isvalid()
_ZN4HPHP15c_EncodingMatch9t_isvalidEv

(return value) => rax
this_ => rdi
*/

bool th_13EncodingMatch_isValid(ObjectData* this_) asm("_ZN4HPHP15c_EncodingMatch9t_isvalidEv");

TypedValue* tg_13EncodingMatch_isValid(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_13EncodingMatch_isValid((this_))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("EncodingMatch::isValid", 0, 1);
      }
    } else {
      throw_instance_method_fatal("EncodingMatch::isValid");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::String HPHP::c_EncodingMatch::t_getencoding()
_ZN4HPHP15c_EncodingMatch13t_getencodingEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_13EncodingMatch_getEncoding(Value* _rv, ObjectData* this_) asm("_ZN4HPHP15c_EncodingMatch13t_getencodingEv");

TypedValue* tg_13EncodingMatch_getEncoding(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfString;
        th_13EncodingMatch_getEncoding((&rv.m_data), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("EncodingMatch::getEncoding", 0, 1);
      }
    } else {
      throw_instance_method_fatal("EncodingMatch::getEncoding");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
long HPHP::c_EncodingMatch::t_getconfidence()
_ZN4HPHP15c_EncodingMatch15t_getconfidenceEv

(return value) => rax
this_ => rdi
*/

long th_13EncodingMatch_getConfidence(ObjectData* this_) asm("_ZN4HPHP15c_EncodingMatch15t_getconfidenceEv");

TypedValue* tg_13EncodingMatch_getConfidence(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)th_13EncodingMatch_getConfidence((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("EncodingMatch::getConfidence", 0, 1);
      }
    } else {
      throw_instance_method_fatal("EncodingMatch::getConfidence");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::String HPHP::c_EncodingMatch::t_getlanguage()
_ZN4HPHP15c_EncodingMatch13t_getlanguageEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_13EncodingMatch_getLanguage(Value* _rv, ObjectData* this_) asm("_ZN4HPHP15c_EncodingMatch13t_getlanguageEv");

TypedValue* tg_13EncodingMatch_getLanguage(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfString;
        th_13EncodingMatch_getLanguage((&rv.m_data), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("EncodingMatch::getLanguage", 0, 1);
      }
    } else {
      throw_instance_method_fatal("EncodingMatch::getLanguage");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::String HPHP::c_EncodingMatch::t_getutf8()
_ZN4HPHP15c_EncodingMatch9t_getutf8Ev

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_13EncodingMatch_getUTF8(Value* _rv, ObjectData* this_) asm("_ZN4HPHP15c_EncodingMatch9t_getutf8Ev");

TypedValue* tg_13EncodingMatch_getUTF8(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfString;
        th_13EncodingMatch_getUTF8((&rv.m_data), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("EncodingMatch::getUTF8", 0, 1);
      }
    } else {
      throw_instance_method_fatal("EncodingMatch::getUTF8");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}


} // !HPHP

