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

HPHP::VM::Instance* new_UConverter_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_UConverter) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_UConverter(ObjectStaticCallbacks::encodeVMClass(cls));
  return inst;
}

/*
void HPHP::c_UConverter::t___construct(HPHP::String const&, HPHP::String const&)
_ZN4HPHP12c_UConverter13t___constructERKNS_6StringES3_

this_ => rdi
toEncoding => rsi
fromEncoding => rdx
*/

void th_10UConverter___construct(ObjectData* this_, Value* toEncoding, Value* fromEncoding) asm("_ZN4HPHP12c_UConverter13t___constructERKNS_6StringES3_");

TypedValue* tg1_10UConverter___construct(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_10UConverter___construct(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  switch (count) {
  default: // count >= 2
    if (!IS_STRING_TYPE((args-1)->m_type)) {
      tvCastToStringInPlace(args-1);
    }
  case 1:
    if (!IS_STRING_TYPE((args-0)->m_type)) {
      tvCastToStringInPlace(args-0);
    }
  case 0:
    break;
  }
  String defVal0 = "utf-8";
  String defVal1 = "utf-8";
  th_10UConverter___construct((this_), (count > 0) ? (Value*)(args-0) : (Value*)(&defVal0), (count > 1) ? (Value*)(args-1) : (Value*)(&defVal1));
  return rv;
}

TypedValue* tg_10UConverter___construct(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count <= 2LL) {
        if ((count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && (count <= 0 || IS_STRING_TYPE((args-0)->m_type))) {
          rv.m_data.num = 0LL;
          rv._count = 0;
          rv.m_type = KindOfNull;
          String defVal0 = "utf-8";
          String defVal1 = "utf-8";
          th_10UConverter___construct((this_), (count > 0) ? (Value*)(args-0) : (Value*)(&defVal0), (count > 1) ? (Value*)(args-1) : (Value*)(&defVal1));
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_10UConverter___construct(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_toomany_arguments_nr("UConverter::__construct", 2, 1);
      }
    } else {
      throw_instance_method_fatal("UConverter::__construct");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_UConverter::t___destruct()
_ZN4HPHP12c_UConverter12t___destructEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_10UConverter___destruct(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP12c_UConverter12t___destructEv");

TypedValue* tg_10UConverter___destruct(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_10UConverter___destruct((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("UConverter::__destruct", 0, 1);
      }
    } else {
      throw_instance_method_fatal("UConverter::__destruct");
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
HPHP::String HPHP::c_UConverter::t_getsourceencoding()
_ZN4HPHP12c_UConverter19t_getsourceencodingEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_10UConverter_getSourceEncoding(Value* _rv, ObjectData* this_) asm("_ZN4HPHP12c_UConverter19t_getsourceencodingEv");

TypedValue* tg_10UConverter_getSourceEncoding(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfString;
        th_10UConverter_getSourceEncoding((Value*)(&(rv)), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("UConverter::getSourceEncoding", 0, 1);
      }
    } else {
      throw_instance_method_fatal("UConverter::getSourceEncoding");
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
void HPHP::c_UConverter::t_setsourceencoding(HPHP::String const&)
_ZN4HPHP12c_UConverter19t_setsourceencodingERKNS_6StringE

this_ => rdi
encoding => rsi
*/

void th_10UConverter_setSourceEncoding(ObjectData* this_, Value* encoding) asm("_ZN4HPHP12c_UConverter19t_setsourceencodingERKNS_6StringE");

TypedValue* tg1_10UConverter_setSourceEncoding(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_10UConverter_setSourceEncoding(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  tvCastToStringInPlace(args-0);
  th_10UConverter_setSourceEncoding((this_), (Value*)(args-0));
  return rv;
}

TypedValue* tg_10UConverter_setSourceEncoding(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_data.num = 0LL;
          rv._count = 0;
          rv.m_type = KindOfNull;
          th_10UConverter_setSourceEncoding((this_), (Value*)(args-0));
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_10UConverter_setSourceEncoding(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("UConverter::setSourceEncoding", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("UConverter::setSourceEncoding");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::String HPHP::c_UConverter::t_getdestinationencoding()
_ZN4HPHP12c_UConverter24t_getdestinationencodingEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_10UConverter_getDestinationEncoding(Value* _rv, ObjectData* this_) asm("_ZN4HPHP12c_UConverter24t_getdestinationencodingEv");

TypedValue* tg_10UConverter_getDestinationEncoding(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfString;
        th_10UConverter_getDestinationEncoding((Value*)(&(rv)), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("UConverter::getDestinationEncoding", 0, 1);
      }
    } else {
      throw_instance_method_fatal("UConverter::getDestinationEncoding");
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
void HPHP::c_UConverter::t_setdestinationencoding(HPHP::String const&)
_ZN4HPHP12c_UConverter24t_setdestinationencodingERKNS_6StringE

this_ => rdi
encoding => rsi
*/

void th_10UConverter_setDestinationEncoding(ObjectData* this_, Value* encoding) asm("_ZN4HPHP12c_UConverter24t_setdestinationencodingERKNS_6StringE");

TypedValue* tg1_10UConverter_setDestinationEncoding(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_10UConverter_setDestinationEncoding(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  tvCastToStringInPlace(args-0);
  th_10UConverter_setDestinationEncoding((this_), (Value*)(args-0));
  return rv;
}

TypedValue* tg_10UConverter_setDestinationEncoding(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_data.num = 0LL;
          rv._count = 0;
          rv.m_type = KindOfNull;
          th_10UConverter_setDestinationEncoding((this_), (Value*)(args-0));
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_10UConverter_setDestinationEncoding(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("UConverter::setDestinationEncoding", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("UConverter::setDestinationEncoding");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
long long HPHP::c_UConverter::t_getsourcetype()
_ZN4HPHP12c_UConverter15t_getsourcetypeEv

(return value) => rax
this_ => rdi
*/

long long th_10UConverter_getSourceType(ObjectData* this_) asm("_ZN4HPHP12c_UConverter15t_getsourcetypeEv");

TypedValue* tg_10UConverter_getSourceType(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (long long)th_10UConverter_getSourceType((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("UConverter::getSourceType", 0, 1);
      }
    } else {
      throw_instance_method_fatal("UConverter::getSourceType");
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
long long HPHP::c_UConverter::t_getdestinationtype()
_ZN4HPHP12c_UConverter20t_getdestinationtypeEv

(return value) => rax
this_ => rdi
*/

long long th_10UConverter_getDestinationType(ObjectData* this_) asm("_ZN4HPHP12c_UConverter20t_getdestinationtypeEv");

TypedValue* tg_10UConverter_getDestinationType(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (long long)th_10UConverter_getDestinationType((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("UConverter::getDestinationType", 0, 1);
      }
    } else {
      throw_instance_method_fatal("UConverter::getDestinationType");
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
HPHP::String HPHP::c_UConverter::t_getsubstchars()
_ZN4HPHP12c_UConverter15t_getsubstcharsEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_10UConverter_getSubstChars(Value* _rv, ObjectData* this_) asm("_ZN4HPHP12c_UConverter15t_getsubstcharsEv");

TypedValue* tg_10UConverter_getSubstChars(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfString;
        th_10UConverter_getSubstChars((Value*)(&(rv)), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("UConverter::getSubstChars", 0, 1);
      }
    } else {
      throw_instance_method_fatal("UConverter::getSubstChars");
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
bool HPHP::c_UConverter::t_setsubstchars(HPHP::String const&)
_ZN4HPHP12c_UConverter15t_setsubstcharsERKNS_6StringE

(return value) => rax
this_ => rdi
chars => rsi
*/

bool th_10UConverter_setSubstChars(ObjectData* this_, Value* chars) asm("_ZN4HPHP12c_UConverter15t_setsubstcharsERKNS_6StringE");

TypedValue* tg1_10UConverter_setSubstChars(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_10UConverter_setSubstChars(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToStringInPlace(args-0);
  rv->m_data.num = (th_10UConverter_setSubstChars((this_), (Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_10UConverter_setSubstChars(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          rv._count = 0;
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_10UConverter_setSubstChars((this_), (Value*)(args-0))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_10UConverter_setSubstChars(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("UConverter::setSubstChars", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("UConverter::setSubstChars");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_UConverter::t_fromucallback(long long, HPHP::Array const&, long long, HPHP::VRefParamValue const&)
_ZN4HPHP12c_UConverter15t_fromucallbackExRKNS_5ArrayExRKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
this_ => rsi
reason => rdx
source => rcx
codepoint => r8
error => r9
*/

TypedValue* th_10UConverter_fromUCallback(TypedValue* _rv, ObjectData* this_, long long reason, Value* source, long long codepoint, TypedValue* error) asm("_ZN4HPHP12c_UConverter15t_fromucallbackExRKNS_5ArrayExRKNS_14VRefParamValueE");

TypedValue* tg1_10UConverter_fromUCallback(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_10UConverter_fromUCallback(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfArray) {
    tvCastToArrayInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  th_10UConverter_fromUCallback((rv), (this_), (long long)(args[-0].m_data.num), (Value*)(args-1), (long long)(args[-2].m_data.num), (args-3));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_10UConverter_fromUCallback(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 4LL) {
        if ((args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfArray && (args-0)->m_type == KindOfInt64) {
          th_10UConverter_fromUCallback((&(rv)), (this_), (long long)(args[-0].m_data.num), (Value*)(args-1), (long long)(args[-2].m_data.num), (args-3));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 4);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_10UConverter_fromUCallback(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 4);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("UConverter::fromUCallback", count, 4, 4, 1);
      }
    } else {
      throw_instance_method_fatal("UConverter::fromUCallback");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_UConverter::t_toucallback(long long, HPHP::String const&, HPHP::String const&, HPHP::VRefParamValue const&)
_ZN4HPHP12c_UConverter13t_toucallbackExRKNS_6StringES3_RKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
this_ => rsi
reason => rdx
source => rcx
codeunits => r8
error => r9
*/

TypedValue* th_10UConverter_toUCallback(TypedValue* _rv, ObjectData* this_, long long reason, Value* source, Value* codeunits, TypedValue* error) asm("_ZN4HPHP12c_UConverter13t_toucallbackExRKNS_6StringES3_RKNS_14VRefParamValueE");

TypedValue* tg1_10UConverter_toUCallback(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_10UConverter_toUCallback(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  th_10UConverter_toUCallback((rv), (this_), (long long)(args[-0].m_data.num), (Value*)(args-1), (Value*)(args-2), (args-3));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_10UConverter_toUCallback(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 4LL) {
        if (IS_STRING_TYPE((args-2)->m_type) && IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfInt64) {
          th_10UConverter_toUCallback((&(rv)), (this_), (long long)(args[-0].m_data.num), (Value*)(args-1), (Value*)(args-2), (args-3));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 4);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_10UConverter_toUCallback(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 4);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("UConverter::toUCallback", count, 4, 4, 1);
      }
    } else {
      throw_instance_method_fatal("UConverter::toUCallback");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_UConverter::t_convert(HPHP::String const&, bool)
_ZN4HPHP12c_UConverter9t_convertERKNS_6StringEb

(return value) => rax
_rv => rdi
this_ => rsi
str => rdx
reverse => rcx
*/

TypedValue* th_10UConverter_convert(TypedValue* _rv, ObjectData* this_, Value* str, bool reverse) asm("_ZN4HPHP12c_UConverter9t_convertERKNS_6StringEb");

TypedValue* tg1_10UConverter_convert(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_10UConverter_convert(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  th_10UConverter_convert((rv), (this_), (Value*)(args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_10UConverter_convert(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL && count <= 2LL) {
        if ((count <= 1 || (args-1)->m_type == KindOfBoolean) && IS_STRING_TYPE((args-0)->m_type)) {
          th_10UConverter_convert((&(rv)), (this_), (Value*)(args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_10UConverter_convert(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("UConverter::convert", count, 1, 2, 1);
      }
    } else {
      throw_instance_method_fatal("UConverter::convert");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_UConverter::ti_transcode(char const*, HPHP::String const&, HPHP::String const&, HPHP::String const&, HPHP::Array const&)
_ZN4HPHP12c_UConverter12ti_transcodeEPKcRKNS_6StringES5_S5_RKNS_5ArrayE

(return value) => rax
_rv => rdi
cls_ => rsi
str => rdx
toEncoding => rcx
fromEncoding => r8
options => r9
*/

TypedValue* th_10UConverter_transcode(TypedValue* _rv, char const* cls_, Value* str, Value* toEncoding, Value* fromEncoding, Value* options) asm("_ZN4HPHP12c_UConverter12ti_transcodeEPKcRKNS_6StringES5_S5_RKNS_5ArrayE");

TypedValue* tg1_10UConverter_transcode(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue* tg1_10UConverter_transcode(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfArray) {
      tvCastToArrayInPlace(args-3);
    }
  case 3:
    break;
  }
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  Array defVal3 = null_variant;
  th_10UConverter_transcode((rv), ("UConverter"), (Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (count > 3) ? (Value*)(args-3) : (Value*)(&defVal3));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_10UConverter_transcode(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 3LL && count <= 4LL) {
      if ((count <= 3 || (args-3)->m_type == KindOfArray) && IS_STRING_TYPE((args-2)->m_type) && IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        Array defVal3 = null_variant;
        th_10UConverter_transcode((&(rv)), ("UConverter"), (Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (count > 3) ? (Value*)(args-3) : (Value*)(&defVal3));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        tg1_10UConverter_transcode(&rv, ar, count );
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("UConverter::transcode", count, 3, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
long long HPHP::c_UConverter::t_geterrorcode()
_ZN4HPHP12c_UConverter14t_geterrorcodeEv

(return value) => rax
this_ => rdi
*/

long long th_10UConverter_getErrorCode(ObjectData* this_) asm("_ZN4HPHP12c_UConverter14t_geterrorcodeEv");

TypedValue* tg_10UConverter_getErrorCode(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (long long)th_10UConverter_getErrorCode((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("UConverter::getErrorCode", 0, 1);
      }
    } else {
      throw_instance_method_fatal("UConverter::getErrorCode");
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
HPHP::String HPHP::c_UConverter::t_geterrormessage()
_ZN4HPHP12c_UConverter17t_geterrormessageEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_10UConverter_getErrorMessage(Value* _rv, ObjectData* this_) asm("_ZN4HPHP12c_UConverter17t_geterrormessageEv");

TypedValue* tg_10UConverter_getErrorMessage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfString;
        th_10UConverter_getErrorMessage((Value*)(&(rv)), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("UConverter::getErrorMessage", 0, 1);
      }
    } else {
      throw_instance_method_fatal("UConverter::getErrorMessage");
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
HPHP::String HPHP::c_UConverter::ti_reasontext(char const*, long long)
_ZN4HPHP12c_UConverter13ti_reasontextEPKcx

(return value) => rax
_rv => rdi
cls_ => rsi
reason => rdx
*/

Value* th_10UConverter_reasonText(Value* _rv, char const* cls_, long long reason) asm("_ZN4HPHP12c_UConverter13ti_reasontextEPKcx");

TypedValue* tg1_10UConverter_reasonText(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue* tg1_10UConverter_reasonText(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToInt64InPlace(args-0);
  th_10UConverter_reasonText((Value*)(rv), ("UConverter"), (long long)(args[-0].m_data.num));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_10UConverter_reasonText(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfInt64) {
        rv._count = 0;
        rv.m_type = KindOfString;
        th_10UConverter_reasonText((Value*)(&(rv)), ("UConverter"), (long long)(args[-0].m_data.num));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        tg1_10UConverter_reasonText(&rv, ar, count );
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("UConverter::reasonText", count, 1, 1, 1);
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
HPHP::Array HPHP::c_UConverter::ti_getavailable(char const*)
_ZN4HPHP12c_UConverter15ti_getavailableEPKc

(return value) => rax
_rv => rdi
cls_ => rsi
*/

Value* th_10UConverter_getAvailable(Value* _rv, char const* cls_) asm("_ZN4HPHP12c_UConverter15ti_getavailableEPKc");

TypedValue* tg_10UConverter_getAvailable(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfArray;
      th_10UConverter_getAvailable((Value*)(&(rv)), ("UConverter"));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("UConverter::getAvailable", 0, 1);
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
HPHP::Array HPHP::c_UConverter::ti_getaliases(char const*, HPHP::String const&)
_ZN4HPHP12c_UConverter13ti_getaliasesEPKcRKNS_6StringE

(return value) => rax
_rv => rdi
cls_ => rsi
encoding => rdx
*/

Value* th_10UConverter_getAliases(Value* _rv, char const* cls_, Value* encoding) asm("_ZN4HPHP12c_UConverter13ti_getaliasesEPKcRKNS_6StringE");

TypedValue* tg1_10UConverter_getAliases(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue* tg1_10UConverter_getAliases(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfArray;
  tvCastToStringInPlace(args-0);
  th_10UConverter_getAliases((Value*)(rv), ("UConverter"), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_10UConverter_getAliases(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfArray;
        th_10UConverter_getAliases((Value*)(&(rv)), ("UConverter"), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        tg1_10UConverter_getAliases(&rv, ar, count );
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("UConverter::getAliases", count, 1, 1, 1);
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
HPHP::Array HPHP::c_UConverter::ti_getstandards(char const*)
_ZN4HPHP12c_UConverter15ti_getstandardsEPKc

(return value) => rax
_rv => rdi
cls_ => rsi
*/

Value* th_10UConverter_getStandards(Value* _rv, char const* cls_) asm("_ZN4HPHP12c_UConverter15ti_getstandardsEPKc");

TypedValue* tg_10UConverter_getStandards(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfArray;
      th_10UConverter_getStandards((Value*)(&(rv)), ("UConverter"));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("UConverter::getStandards", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}


} // !HPHP

