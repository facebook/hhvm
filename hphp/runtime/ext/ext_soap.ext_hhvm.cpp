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

/*
bool HPHP::f_use_soap_error_handler(bool)
_ZN4HPHP24f_use_soap_error_handlerEb

(return value) => rax
handler => rdi
*/

bool fh_use_soap_error_handler(bool handler) asm("_ZN4HPHP24f_use_soap_error_handlerEb");

TypedValue * fg1_use_soap_error_handler(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_use_soap_error_handler(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToBooleanInPlace(args-0);
  rv->m_data.num = (fh_use_soap_error_handler((count > 0) ? (bool)(args[-0].m_data.num) : (bool)(true))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_use_soap_error_handler(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      if ((count <= 0 || (args-0)->m_type == KindOfBoolean)) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_use_soap_error_handler((count > 0) ? (bool)(args[-0].m_data.num) : (bool)(true))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_use_soap_error_handler(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("use_soap_error_handler", 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_is_soap_fault(HPHP::Variant const&)
_ZN4HPHP15f_is_soap_faultERKNS_7VariantE

(return value) => rax
fault => rdi
*/

bool fh_is_soap_fault(TypedValue* fault) asm("_ZN4HPHP15f_is_soap_faultERKNS_7VariantE");

TypedValue* fg_is_soap_fault(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv.m_type = KindOfBoolean;
      rv.m_data.num = (fh_is_soap_fault((args-0))) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("is_soap_fault", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f__soap_active_version()
_ZN4HPHP22f__soap_active_versionEv

(return value) => rax
*/

long fh__soap_active_version() asm("_ZN4HPHP22f__soap_active_versionEv");

TypedValue* fg__soap_active_version(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_type = KindOfInt64;
      rv.m_data.num = (int64_t)fh__soap_active_version();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("_soap_active_version", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



HPHP::VM::Instance* new_SoapServer_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_SoapServer) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_SoapServer(cls);
  return inst;
}

IMPLEMENT_CLASS(SoapServer);
/*
void HPHP::c_SoapServer::t___construct(HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP12c_SoapServer13t___constructERKNS_7VariantERKNS_5ArrayE

this_ => rdi
wsdl => rsi
options => rdx
*/

void th_10SoapServer___construct(ObjectData* this_, TypedValue* wsdl, Value* options) asm("_ZN4HPHP12c_SoapServer13t___constructERKNS_7VariantERKNS_5ArrayE");

TypedValue* tg1_10SoapServer___construct(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_10SoapServer___construct(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  tvCastToArrayInPlace(args-1);
  th_10SoapServer___construct((this_), (args-0), (count > 1) ? &args[-1].m_data : (Value*)(&null_array));
  return rv;
}

TypedValue* tg_10SoapServer___construct(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL && count <= 2LL) {
        if ((count <= 1 || (args-1)->m_type == KindOfArray)) {
          rv.m_data.num = 0LL;
          rv.m_type = KindOfNull;
          th_10SoapServer___construct((this_), (args-0), (count > 1) ? &args[-1].m_data : (Value*)(&null_array));
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_10SoapServer___construct(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("SoapServer::__construct", count, 1, 2, 1);
      }
    } else {
      throw_instance_method_fatal("SoapServer::__construct");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
void HPHP::c_SoapServer::t_setclass(int, HPHP::String const&, HPHP::Array const&)
_ZN4HPHP12c_SoapServer10t_setclassEiRKNS_6StringERKNS_5ArrayE

this_ => rdi
_argc => rsi
name => rdx
_argv => rcx
*/

void th_10SoapServer_setclass(ObjectData* this_, int64_t _argc, Value* name, Value* _argv) asm("_ZN4HPHP12c_SoapServer10t_setclassEiRKNS_6StringERKNS_5ArrayE");

TypedValue* tg1_10SoapServer_setclass(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_10SoapServer_setclass(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  tvCastToStringInPlace(args-0);
  Array extraArgs;
  {
    ArrayInit ai(count-1);
    for (int64_t i = 1; i < count; ++i) {
      TypedValue* extraArg = ar->getExtraArg(i-1);
      if (tvIsStronglyBound(extraArg)) {
        ai.setRef(i-1, tvAsVariant(extraArg));
      } else {
        ai.set(i-1, tvAsVariant(extraArg));
      }
    }
    extraArgs = ai.create();
  }
  th_10SoapServer_setclass((this_), (count), &args[-0].m_data, (Value*)(&extraArgs));
  return rv;
}

TypedValue* tg_10SoapServer_setclass(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_data.num = 0LL;
          rv.m_type = KindOfNull;
          Array extraArgs;
          {
            ArrayInit ai(count-1);
            for (int64_t i = 1; i < count; ++i) {
              TypedValue* extraArg = ar->getExtraArg(i-1);
              if (tvIsStronglyBound(extraArg)) {
                ai.setRef(i-1, tvAsVariant(extraArg));
              } else {
                ai.set(i-1, tvAsVariant(extraArg));
              }
            }
            extraArgs = ai.create();
          }
          th_10SoapServer_setclass((this_), (count), &args[-0].m_data, (Value*)(&extraArgs));
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_10SoapServer_setclass(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_missing_arguments_nr("SoapServer::setclass", 1, count, 1);
      }
    } else {
      throw_instance_method_fatal("SoapServer::setclass");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
void HPHP::c_SoapServer::t_setobject(HPHP::Object const&)
_ZN4HPHP12c_SoapServer11t_setobjectERKNS_6ObjectE

this_ => rdi
obj => rsi
*/

void th_10SoapServer_setobject(ObjectData* this_, Value* obj) asm("_ZN4HPHP12c_SoapServer11t_setobjectERKNS_6ObjectE");

TypedValue* tg1_10SoapServer_setobject(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_10SoapServer_setobject(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  tvCastToObjectInPlace(args-0);
  th_10SoapServer_setobject((this_), &args[-0].m_data);
  return rv;
}

TypedValue* tg_10SoapServer_setobject(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if ((args-0)->m_type == KindOfObject) {
          rv.m_data.num = 0LL;
          rv.m_type = KindOfNull;
          th_10SoapServer_setobject((this_), &args[-0].m_data);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_10SoapServer_setobject(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("SoapServer::setobject", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("SoapServer::setobject");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
void HPHP::c_SoapServer::t_addfunction(HPHP::Variant const&)
_ZN4HPHP12c_SoapServer13t_addfunctionERKNS_7VariantE

this_ => rdi
func => rsi
*/

void th_10SoapServer_addfunction(ObjectData* this_, TypedValue* func) asm("_ZN4HPHP12c_SoapServer13t_addfunctionERKNS_7VariantE");

TypedValue* tg_10SoapServer_addfunction(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        th_10SoapServer_addfunction((this_), (args-0));
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("SoapServer::addfunction", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("SoapServer::addfunction");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_SoapServer::t_getfunctions()
_ZN4HPHP12c_SoapServer14t_getfunctionsEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_10SoapServer_getfunctions(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP12c_SoapServer14t_getfunctionsEv");

TypedValue* tg_10SoapServer_getfunctions(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_10SoapServer_getfunctions((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("SoapServer::getfunctions", 0, 1);
      }
    } else {
      throw_instance_method_fatal("SoapServer::getfunctions");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
void HPHP::c_SoapServer::t_handle(HPHP::String const&)
_ZN4HPHP12c_SoapServer8t_handleERKNS_6StringE

this_ => rdi
request => rsi
*/

void th_10SoapServer_handle(ObjectData* this_, Value* request) asm("_ZN4HPHP12c_SoapServer8t_handleERKNS_6StringE");

TypedValue* tg1_10SoapServer_handle(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_10SoapServer_handle(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  tvCastToStringInPlace(args-0);
  th_10SoapServer_handle((this_), (count > 0) ? &args[-0].m_data : (Value*)(&null_string));
  return rv;
}

TypedValue* tg_10SoapServer_handle(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count <= 1LL) {
        if ((count <= 0 || IS_STRING_TYPE((args-0)->m_type))) {
          rv.m_data.num = 0LL;
          rv.m_type = KindOfNull;
          th_10SoapServer_handle((this_), (count > 0) ? &args[-0].m_data : (Value*)(&null_string));
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_10SoapServer_handle(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_toomany_arguments_nr("SoapServer::handle", 1, 1);
      }
    } else {
      throw_instance_method_fatal("SoapServer::handle");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
void HPHP::c_SoapServer::t_setpersistence(long)
_ZN4HPHP12c_SoapServer16t_setpersistenceEl

this_ => rdi
mode => rsi
*/

void th_10SoapServer_setpersistence(ObjectData* this_, long mode) asm("_ZN4HPHP12c_SoapServer16t_setpersistenceEl");

TypedValue* tg1_10SoapServer_setpersistence(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_10SoapServer_setpersistence(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  tvCastToInt64InPlace(args-0);
  th_10SoapServer_setpersistence((this_), (long)(args[-0].m_data.num));
  return rv;
}

TypedValue* tg_10SoapServer_setpersistence(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if ((args-0)->m_type == KindOfInt64) {
          rv.m_data.num = 0LL;
          rv.m_type = KindOfNull;
          th_10SoapServer_setpersistence((this_), (long)(args[-0].m_data.num));
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_10SoapServer_setpersistence(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("SoapServer::setpersistence", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("SoapServer::setpersistence");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
void HPHP::c_SoapServer::t_fault(HPHP::Variant const&, HPHP::String const&, HPHP::String const&, HPHP::Variant const&, HPHP::String const&)
_ZN4HPHP12c_SoapServer7t_faultERKNS_7VariantERKNS_6StringES6_S3_S6_

this_ => rdi
code => rsi
fault => rdx
actor => rcx
detail => r8
name => r9
*/

void th_10SoapServer_fault(ObjectData* this_, TypedValue* code, Value* fault, Value* actor, TypedValue* detail, Value* name) asm("_ZN4HPHP12c_SoapServer7t_faultERKNS_7VariantERKNS_6StringES6_S3_S6_");

TypedValue* tg1_10SoapServer_fault(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_10SoapServer_fault(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  switch (count) {
  default: // count >= 5
    if (!IS_STRING_TYPE((args-4)->m_type)) {
      tvCastToStringInPlace(args-4);
    }
  case 4:
  case 3:
    if (!IS_STRING_TYPE((args-2)->m_type)) {
      tvCastToStringInPlace(args-2);
    }
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  Variant defVal3;
  th_10SoapServer_fault((this_), (args-0), &args[-1].m_data, (count > 2) ? &args[-2].m_data : (Value*)(&null_string), (count > 3) ? (args-3) : (TypedValue*)(&defVal3), (count > 4) ? &args[-4].m_data : (Value*)(&null_string));
  return rv;
}

TypedValue* tg_10SoapServer_fault(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 2LL && count <= 5LL) {
        if ((count <= 4 || IS_STRING_TYPE((args-4)->m_type)) && (count <= 2 || IS_STRING_TYPE((args-2)->m_type)) && IS_STRING_TYPE((args-1)->m_type)) {
          rv.m_data.num = 0LL;
          rv.m_type = KindOfNull;
          Variant defVal3;
          th_10SoapServer_fault((this_), (args-0), &args[-1].m_data, (count > 2) ? &args[-2].m_data : (Value*)(&null_string), (count > 3) ? (args-3) : (TypedValue*)(&defVal3), (count > 4) ? &args[-4].m_data : (Value*)(&null_string));
          frame_free_locals_inl(ar, 5);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_10SoapServer_fault(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 5);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("SoapServer::fault", count, 2, 5, 1);
      }
    } else {
      throw_instance_method_fatal("SoapServer::fault");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 5);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
void HPHP::c_SoapServer::t_addsoapheader(HPHP::Object const&)
_ZN4HPHP12c_SoapServer15t_addsoapheaderERKNS_6ObjectE

this_ => rdi
fault => rsi
*/

void th_10SoapServer_addsoapheader(ObjectData* this_, Value* fault) asm("_ZN4HPHP12c_SoapServer15t_addsoapheaderERKNS_6ObjectE");

TypedValue* tg1_10SoapServer_addsoapheader(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_10SoapServer_addsoapheader(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  tvCastToObjectInPlace(args-0);
  th_10SoapServer_addsoapheader((this_), &args[-0].m_data);
  return rv;
}

TypedValue* tg_10SoapServer_addsoapheader(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if ((args-0)->m_type == KindOfObject) {
          rv.m_data.num = 0LL;
          rv.m_type = KindOfNull;
          th_10SoapServer_addsoapheader((this_), &args[-0].m_data);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_10SoapServer_addsoapheader(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("SoapServer::addsoapheader", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("SoapServer::addsoapheader");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

HPHP::VM::Instance* new_SoapClient_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_SoapClient) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_SoapClient(cls);
  return inst;
}

IMPLEMENT_CLASS(SoapClient);
/*
void HPHP::c_SoapClient::t___construct(HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP12c_SoapClient13t___constructERKNS_7VariantERKNS_5ArrayE

this_ => rdi
wsdl => rsi
options => rdx
*/

void th_10SoapClient___construct(ObjectData* this_, TypedValue* wsdl, Value* options) asm("_ZN4HPHP12c_SoapClient13t___constructERKNS_7VariantERKNS_5ArrayE");

TypedValue* tg1_10SoapClient___construct(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_10SoapClient___construct(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  tvCastToArrayInPlace(args-1);
  th_10SoapClient___construct((this_), (args-0), (count > 1) ? &args[-1].m_data : (Value*)(&null_array));
  return rv;
}

TypedValue* tg_10SoapClient___construct(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL && count <= 2LL) {
        if ((count <= 1 || (args-1)->m_type == KindOfArray)) {
          rv.m_data.num = 0LL;
          rv.m_type = KindOfNull;
          th_10SoapClient___construct((this_), (args-0), (count > 1) ? &args[-1].m_data : (Value*)(&null_array));
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_10SoapClient___construct(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("SoapClient::__construct", count, 1, 2, 1);
      }
    } else {
      throw_instance_method_fatal("SoapClient::__construct");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_SoapClient::t___call(HPHP::Variant, HPHP::Variant)
_ZN4HPHP12c_SoapClient8t___callENS_7VariantES1_

(return value) => rax
_rv => rdi
this_ => rsi
name => rdx
args => rcx
*/

TypedValue* th_10SoapClient___call(TypedValue* _rv, ObjectData* this_, TypedValue* name, TypedValue* args) asm("_ZN4HPHP12c_SoapClient8t___callENS_7VariantES1_");

TypedValue* tg_10SoapClient___call(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        th_10SoapClient___call((&(rv)), (this_), (args-0), (args-1));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("SoapClient::__call", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("SoapClient::__call");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_SoapClient::t___soapcall(HPHP::String const&, HPHP::Array const&, HPHP::Array const&, HPHP::Variant const&, HPHP::VRefParamValue const&)
_ZN4HPHP12c_SoapClient12t___soapcallERKNS_6StringERKNS_5ArrayES6_RKNS_7VariantERKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
this_ => rsi
name => rdx
args => rcx
options => r8
input_headers => r9
output_headers => st0
*/

TypedValue* th_10SoapClient___soapcall(TypedValue* _rv, ObjectData* this_, Value* name, Value* args, Value* options, TypedValue* input_headers, TypedValue* output_headers) asm("_ZN4HPHP12c_SoapClient12t___soapcallERKNS_6StringERKNS_5ArrayES6_RKNS_7VariantERKNS_14VRefParamValueE");

TypedValue* tg1_10SoapClient___soapcall(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_10SoapClient___soapcall(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 5
  case 4:
  case 3:
    if ((args-2)->m_type != KindOfArray) {
      tvCastToArrayInPlace(args-2);
    }
  case 2:
    break;
  }
  if ((args-1)->m_type != KindOfArray) {
    tvCastToArrayInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  VRefParamValue defVal4 = uninit_null();
  th_10SoapClient___soapcall((rv), (this_), &args[-0].m_data, &args[-1].m_data, (count > 2) ? &args[-2].m_data : (Value*)(&null_array), (count > 3) ? (args-3) : (TypedValue*)(&null_variant), (count > 4) ? (args-4) : (TypedValue*)(&defVal4));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_10SoapClient___soapcall(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 2LL && count <= 5LL) {
        if ((count <= 2 || (args-2)->m_type == KindOfArray) && (args-1)->m_type == KindOfArray && IS_STRING_TYPE((args-0)->m_type)) {
          VRefParamValue defVal4 = uninit_null();
          th_10SoapClient___soapcall((&(rv)), (this_), &args[-0].m_data, &args[-1].m_data, (count > 2) ? &args[-2].m_data : (Value*)(&null_array), (count > 3) ? (args-3) : (TypedValue*)(&null_variant), (count > 4) ? (args-4) : (TypedValue*)(&defVal4));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 5);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_10SoapClient___soapcall(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 5);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("SoapClient::__soapcall", count, 2, 5, 1);
      }
    } else {
      throw_instance_method_fatal("SoapClient::__soapcall");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 5);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_SoapClient::t___getlastrequest()
_ZN4HPHP12c_SoapClient18t___getlastrequestEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_10SoapClient___getlastrequest(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP12c_SoapClient18t___getlastrequestEv");

TypedValue* tg_10SoapClient___getlastrequest(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_10SoapClient___getlastrequest((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("SoapClient::__getlastrequest", 0, 1);
      }
    } else {
      throw_instance_method_fatal("SoapClient::__getlastrequest");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_SoapClient::t___getlastresponse()
_ZN4HPHP12c_SoapClient19t___getlastresponseEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_10SoapClient___getlastresponse(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP12c_SoapClient19t___getlastresponseEv");

TypedValue* tg_10SoapClient___getlastresponse(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_10SoapClient___getlastresponse((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("SoapClient::__getlastresponse", 0, 1);
      }
    } else {
      throw_instance_method_fatal("SoapClient::__getlastresponse");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_SoapClient::t___getlastrequestheaders()
_ZN4HPHP12c_SoapClient25t___getlastrequestheadersEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_10SoapClient___getlastrequestheaders(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP12c_SoapClient25t___getlastrequestheadersEv");

TypedValue* tg_10SoapClient___getlastrequestheaders(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_10SoapClient___getlastrequestheaders((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("SoapClient::__getlastrequestheaders", 0, 1);
      }
    } else {
      throw_instance_method_fatal("SoapClient::__getlastrequestheaders");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_SoapClient::t___getlastresponseheaders()
_ZN4HPHP12c_SoapClient26t___getlastresponseheadersEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_10SoapClient___getlastresponseheaders(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP12c_SoapClient26t___getlastresponseheadersEv");

TypedValue* tg_10SoapClient___getlastresponseheaders(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_10SoapClient___getlastresponseheaders((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("SoapClient::__getlastresponseheaders", 0, 1);
      }
    } else {
      throw_instance_method_fatal("SoapClient::__getlastresponseheaders");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_SoapClient::t___getfunctions()
_ZN4HPHP12c_SoapClient16t___getfunctionsEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_10SoapClient___getfunctions(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP12c_SoapClient16t___getfunctionsEv");

TypedValue* tg_10SoapClient___getfunctions(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_10SoapClient___getfunctions((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("SoapClient::__getfunctions", 0, 1);
      }
    } else {
      throw_instance_method_fatal("SoapClient::__getfunctions");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_SoapClient::t___gettypes()
_ZN4HPHP12c_SoapClient12t___gettypesEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_10SoapClient___gettypes(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP12c_SoapClient12t___gettypesEv");

TypedValue* tg_10SoapClient___gettypes(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_10SoapClient___gettypes((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("SoapClient::__gettypes", 0, 1);
      }
    } else {
      throw_instance_method_fatal("SoapClient::__gettypes");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_SoapClient::t___dorequest(HPHP::String const&, HPHP::String const&, HPHP::String const&, long, bool)
_ZN4HPHP12c_SoapClient13t___dorequestERKNS_6StringES3_S3_lb

(return value) => rax
_rv => rdi
this_ => rsi
buf => rdx
location => rcx
action => r8
version => r9
oneway => st0
*/

TypedValue* th_10SoapClient___dorequest(TypedValue* _rv, ObjectData* this_, Value* buf, Value* location, Value* action, long version, bool oneway) asm("_ZN4HPHP12c_SoapClient13t___dorequestERKNS_6StringES3_S3_lb");

TypedValue* tg1_10SoapClient___dorequest(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_10SoapClient___dorequest(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 5
    if ((args-4)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-4);
    }
  case 4:
    break;
  }
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
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
  th_10SoapClient___dorequest((rv), (this_), &args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (long)(args[-3].m_data.num), (count > 4) ? (bool)(args[-4].m_data.num) : (bool)(false));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_10SoapClient___dorequest(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 4LL && count <= 5LL) {
        if ((count <= 4 || (args-4)->m_type == KindOfBoolean) && (args-3)->m_type == KindOfInt64 && IS_STRING_TYPE((args-2)->m_type) && IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
          th_10SoapClient___dorequest((&(rv)), (this_), &args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (long)(args[-3].m_data.num), (count > 4) ? (bool)(args[-4].m_data.num) : (bool)(false));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 5);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_10SoapClient___dorequest(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 5);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("SoapClient::__dorequest", count, 4, 5, 1);
      }
    } else {
      throw_instance_method_fatal("SoapClient::__dorequest");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 5);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_SoapClient::t___setcookie(HPHP::String const&, HPHP::String const&)
_ZN4HPHP12c_SoapClient13t___setcookieERKNS_6StringES3_

(return value) => rax
_rv => rdi
this_ => rsi
name => rdx
value => rcx
*/

TypedValue* th_10SoapClient___setcookie(TypedValue* _rv, ObjectData* this_, Value* name, Value* value) asm("_ZN4HPHP12c_SoapClient13t___setcookieERKNS_6StringES3_");

TypedValue* tg1_10SoapClient___setcookie(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_10SoapClient___setcookie(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 2
    if (!IS_STRING_TYPE((args-1)->m_type)) {
      tvCastToStringInPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  th_10SoapClient___setcookie((rv), (this_), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_10SoapClient___setcookie(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL && count <= 2LL) {
        if ((count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && IS_STRING_TYPE((args-0)->m_type)) {
          th_10SoapClient___setcookie((&(rv)), (this_), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_10SoapClient___setcookie(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("SoapClient::__setcookie", count, 1, 2, 1);
      }
    } else {
      throw_instance_method_fatal("SoapClient::__setcookie");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_SoapClient::t___setlocation(HPHP::String const&)
_ZN4HPHP12c_SoapClient15t___setlocationERKNS_6StringE

(return value) => rax
_rv => rdi
this_ => rsi
new_location => rdx
*/

TypedValue* th_10SoapClient___setlocation(TypedValue* _rv, ObjectData* this_, Value* new_location) asm("_ZN4HPHP12c_SoapClient15t___setlocationERKNS_6StringE");

TypedValue* tg1_10SoapClient___setlocation(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_10SoapClient___setlocation(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  th_10SoapClient___setlocation((rv), (this_), (count > 0) ? &args[-0].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_10SoapClient___setlocation(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count <= 1LL) {
        if ((count <= 0 || IS_STRING_TYPE((args-0)->m_type))) {
          th_10SoapClient___setlocation((&(rv)), (this_), (count > 0) ? &args[-0].m_data : (Value*)(&null_string));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_10SoapClient___setlocation(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_toomany_arguments_nr("SoapClient::__setlocation", 1, 1);
      }
    } else {
      throw_instance_method_fatal("SoapClient::__setlocation");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_SoapClient::t___setsoapheaders(HPHP::Variant const&)
_ZN4HPHP12c_SoapClient18t___setsoapheadersERKNS_7VariantE

(return value) => rax
this_ => rdi
headers => rsi
*/

bool th_10SoapClient___setsoapheaders(ObjectData* this_, TypedValue* headers) asm("_ZN4HPHP12c_SoapClient18t___setsoapheadersERKNS_7VariantE");

TypedValue* tg_10SoapClient___setsoapheaders(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count <= 1LL) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_10SoapClient___setsoapheaders((this_), (count > 0) ? (args-0) : (TypedValue*)(&null_variant))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("SoapClient::__setsoapheaders", 1, 1);
      }
    } else {
      throw_instance_method_fatal("SoapClient::__setsoapheaders");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

HPHP::VM::Instance* new_SoapVar_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_SoapVar) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_SoapVar(cls);
  return inst;
}

IMPLEMENT_CLASS(SoapVar);
/*
void HPHP::c_SoapVar::t___construct(HPHP::Variant const&, HPHP::Variant const&, HPHP::String const&, HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP9c_SoapVar13t___constructERKNS_7VariantES3_RKNS_6StringES6_S6_S6_

this_ => rdi
data => rsi
type => rdx
type_name => rcx
type_namespace => r8
node_name => r9
node_namespace => st0
*/

void th_7SoapVar___construct(ObjectData* this_, TypedValue* data, TypedValue* type, Value* type_name, Value* type_namespace, Value* node_name, Value* node_namespace) asm("_ZN4HPHP9c_SoapVar13t___constructERKNS_7VariantES3_RKNS_6StringES6_S6_S6_");

TypedValue* tg1_7SoapVar___construct(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_7SoapVar___construct(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  switch (count) {
  default: // count >= 6
    if (!IS_STRING_TYPE((args-5)->m_type)) {
      tvCastToStringInPlace(args-5);
    }
  case 5:
    if (!IS_STRING_TYPE((args-4)->m_type)) {
      tvCastToStringInPlace(args-4);
    }
  case 4:
    if (!IS_STRING_TYPE((args-3)->m_type)) {
      tvCastToStringInPlace(args-3);
    }
  case 3:
    if (!IS_STRING_TYPE((args-2)->m_type)) {
      tvCastToStringInPlace(args-2);
    }
  case 2:
    break;
  }
  th_7SoapVar___construct((this_), (args-0), (args-1), (count > 2) ? &args[-2].m_data : (Value*)(&null_string), (count > 3) ? &args[-3].m_data : (Value*)(&null_string), (count > 4) ? &args[-4].m_data : (Value*)(&null_string), (count > 5) ? &args[-5].m_data : (Value*)(&null_string));
  return rv;
}

TypedValue* tg_7SoapVar___construct(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 2LL && count <= 6LL) {
        if ((count <= 5 || IS_STRING_TYPE((args-5)->m_type)) && (count <= 4 || IS_STRING_TYPE((args-4)->m_type)) && (count <= 3 || IS_STRING_TYPE((args-3)->m_type)) && (count <= 2 || IS_STRING_TYPE((args-2)->m_type))) {
          rv.m_data.num = 0LL;
          rv.m_type = KindOfNull;
          th_7SoapVar___construct((this_), (args-0), (args-1), (count > 2) ? &args[-2].m_data : (Value*)(&null_string), (count > 3) ? &args[-3].m_data : (Value*)(&null_string), (count > 4) ? &args[-4].m_data : (Value*)(&null_string), (count > 5) ? &args[-5].m_data : (Value*)(&null_string));
          frame_free_locals_inl(ar, 6);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_7SoapVar___construct(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 6);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("SoapVar::__construct", count, 2, 6, 1);
      }
    } else {
      throw_instance_method_fatal("SoapVar::__construct");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 6);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

HPHP::VM::Instance* new_SoapParam_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_SoapParam) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_SoapParam(cls);
  return inst;
}

IMPLEMENT_CLASS(SoapParam);
/*
void HPHP::c_SoapParam::t___construct(HPHP::Variant const&, HPHP::String const&)
_ZN4HPHP11c_SoapParam13t___constructERKNS_7VariantERKNS_6StringE

this_ => rdi
data => rsi
name => rdx
*/

void th_9SoapParam___construct(ObjectData* this_, TypedValue* data, Value* name) asm("_ZN4HPHP11c_SoapParam13t___constructERKNS_7VariantERKNS_6StringE");

TypedValue* tg1_9SoapParam___construct(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9SoapParam___construct(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  tvCastToStringInPlace(args-1);
  th_9SoapParam___construct((this_), (args-0), &args[-1].m_data);
  return rv;
}

TypedValue* tg_9SoapParam___construct(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        if (IS_STRING_TYPE((args-1)->m_type)) {
          rv.m_data.num = 0LL;
          rv.m_type = KindOfNull;
          th_9SoapParam___construct((this_), (args-0), &args[-1].m_data);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9SoapParam___construct(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("SoapParam::__construct", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("SoapParam::__construct");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

HPHP::VM::Instance* new_SoapHeader_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_SoapHeader) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_SoapHeader(cls);
  return inst;
}

IMPLEMENT_CLASS(SoapHeader);
/*
void HPHP::c_SoapHeader::t___construct(HPHP::String const&, HPHP::String const&, HPHP::Variant const&, bool, HPHP::Variant const&)
_ZN4HPHP12c_SoapHeader13t___constructERKNS_6StringES3_RKNS_7VariantEbS6_

this_ => rdi
ns => rsi
name => rdx
data => rcx
mustunderstand => r8
actor => r9
*/

void th_10SoapHeader___construct(ObjectData* this_, Value* ns, Value* name, TypedValue* data, bool mustunderstand, TypedValue* actor) asm("_ZN4HPHP12c_SoapHeader13t___constructERKNS_6StringES3_RKNS_7VariantEbS6_");

TypedValue* tg1_10SoapHeader___construct(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_10SoapHeader___construct(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  switch (count) {
  default: // count >= 5
  case 4:
    if ((args-3)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-3);
    }
  case 3:
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  Variant defVal2;
  Variant defVal4;
  th_10SoapHeader___construct((this_), &args[-0].m_data, &args[-1].m_data, (count > 2) ? (args-2) : (TypedValue*)(&defVal2), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false), (count > 4) ? (args-4) : (TypedValue*)(&defVal4));
  return rv;
}

TypedValue* tg_10SoapHeader___construct(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 2LL && count <= 5LL) {
        if ((count <= 3 || (args-3)->m_type == KindOfBoolean) && IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_data.num = 0LL;
          rv.m_type = KindOfNull;
          Variant defVal2;
          Variant defVal4;
          th_10SoapHeader___construct((this_), &args[-0].m_data, &args[-1].m_data, (count > 2) ? (args-2) : (TypedValue*)(&defVal2), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false), (count > 4) ? (args-4) : (TypedValue*)(&defVal4));
          frame_free_locals_inl(ar, 5);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_10SoapHeader___construct(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 5);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("SoapHeader::__construct", count, 2, 5, 1);
      }
    } else {
      throw_instance_method_fatal("SoapHeader::__construct");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 5);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}


} // !HPHP

