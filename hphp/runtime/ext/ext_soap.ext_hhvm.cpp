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

bool fh_use_soap_error_handler(bool handler) asm("_ZN4HPHP24f_use_soap_error_handlerEb");

void fg1_use_soap_error_handler(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_use_soap_error_handler(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToBooleanInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_use_soap_error_handler((count > 0) ? (bool)(args[-0].m_data.num) : (bool)(true))) ? 1LL : 0LL;
}

TypedValue* fg_use_soap_error_handler(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 1) {
    if ((count <= 0 || (args - 0)->m_type == KindOfBoolean)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_use_soap_error_handler((count > 0) ? (bool)(args[-0].m_data.num) : (bool)(true))) ? 1LL : 0LL;
    } else {
      fg1_use_soap_error_handler(rv, ar, count);
    }
  } else {
    throw_toomany_arguments_nr("use_soap_error_handler", 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_is_soap_fault(TypedValue* fault) asm("_ZN4HPHP15f_is_soap_faultERKNS_7VariantE");

TypedValue* fg_is_soap_fault(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    rv->m_type = KindOfBoolean;
    rv->m_data.num = (fh_is_soap_fault((args-0))) ? 1LL : 0LL;
  } else {
    throw_wrong_arguments_nr("is_soap_fault", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh__soap_active_version() asm("_ZN4HPHP22f__soap_active_versionEv");

TypedValue* fg__soap_active_version(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfInt64;
    rv->m_data.num = (int64_t)fh__soap_active_version();
  } else {
    throw_toomany_arguments_nr("_soap_active_version", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
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
void th_10SoapServer___construct(ObjectData* this_, TypedValue* wsdl, Value* options) asm("_ZN4HPHP12c_SoapServer13t___constructERKNS_7VariantERKNS_5ArrayE");

void tg1_10SoapServer___construct(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_10SoapServer___construct(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToArrayInPlace(args-1);
  rv->m_type = KindOfNull;
  th_10SoapServer___construct((this_), (args-0), (count > 1) ? &args[-1].m_data : (Value*)(&null_array));
}

TypedValue* tg_10SoapServer___construct(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 1 && count <= 2) {
      if ((count <= 1 || (args - 1)->m_type == KindOfArray)) {
        rv->m_type = KindOfNull;
        th_10SoapServer___construct((this_), (args-0), (count > 1) ? &args[-1].m_data : (Value*)(&null_array));
      } else {
        tg1_10SoapServer___construct(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("SoapServer::__construct", count, 1, 2, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SoapServer::__construct");
  }
  frame_free_locals_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void th_10SoapServer_setclass(ObjectData* this_, int64_t _argc, Value* name, Value* _argv) asm("_ZN4HPHP12c_SoapServer10t_setclassEiRKNS_6StringERKNS_5ArrayE");

void tg1_10SoapServer_setclass(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_10SoapServer_setclass(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfNull;

  Array extraArgs;
  {
    ArrayInit ai(count-1);
    for (int32_t i = 1; i < count; ++i) {
      TypedValue* extraArg = ar->getExtraArg(i-1);
      if (tvIsStronglyBound(extraArg)) {
        ai.setRef(i-1, tvAsVariant(extraArg));
      } else {
        ai.set(i-1, tvAsVariant(extraArg));
      }
    }
    extraArgs = ai.create();
  }
  th_10SoapServer_setclass((this_), count, &args[-0].m_data, (Value*)(&extraArgs));
}

TypedValue* tg_10SoapServer_setclass(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 1) {
      if (IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfNull;

        Array extraArgs;
        {
          ArrayInit ai(count-1);
          for (int32_t i = 1; i < count; ++i) {
            TypedValue* extraArg = ar->getExtraArg(i-1);
            if (tvIsStronglyBound(extraArg)) {
              ai.setRef(i-1, tvAsVariant(extraArg));
            } else {
              ai.set(i-1, tvAsVariant(extraArg));
            }
          }
          extraArgs = ai.create();
        }
        th_10SoapServer_setclass((this_), count, &args[-0].m_data, (Value*)(&extraArgs));
      } else {
        tg1_10SoapServer_setclass(rv, ar, count, this_);
      }
    } else {
      throw_missing_arguments_nr("SoapServer::setclass", 1, count, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SoapServer::setclass");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void th_10SoapServer_setobject(ObjectData* this_, Value* obj) asm("_ZN4HPHP12c_SoapServer11t_setobjectERKNS_6ObjectE");

void tg1_10SoapServer_setobject(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_10SoapServer_setobject(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfNull;
  th_10SoapServer_setobject((this_), &args[-0].m_data);
}

TypedValue* tg_10SoapServer_setobject(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if ((args - 0)->m_type == KindOfObject) {
        rv->m_type = KindOfNull;
        th_10SoapServer_setobject((this_), &args[-0].m_data);
      } else {
        tg1_10SoapServer_setobject(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("SoapServer::setobject", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SoapServer::setobject");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void th_10SoapServer_addfunction(ObjectData* this_, TypedValue* func) asm("_ZN4HPHP12c_SoapServer13t_addfunctionERKNS_7VariantE");

TypedValue* tg_10SoapServer_addfunction(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      rv->m_type = KindOfNull;
      th_10SoapServer_addfunction((this_), (args-0));
    } else {
      throw_wrong_arguments_nr("SoapServer::addfunction", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SoapServer::addfunction");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_10SoapServer_getfunctions(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP12c_SoapServer14t_getfunctionsEv");

TypedValue* tg_10SoapServer_getfunctions(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      th_10SoapServer_getfunctions(rv, (this_));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      throw_toomany_arguments_nr("SoapServer::getfunctions", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SoapServer::getfunctions");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void th_10SoapServer_handle(ObjectData* this_, Value* request) asm("_ZN4HPHP12c_SoapServer8t_handleERKNS_6StringE");

void tg1_10SoapServer_handle(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_10SoapServer_handle(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfNull;
  th_10SoapServer_handle((this_), (count > 0) ? &args[-0].m_data : (Value*)(&null_string));
}

TypedValue* tg_10SoapServer_handle(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count <= 1) {
      if ((count <= 0 || IS_STRING_TYPE((args - 0)->m_type))) {
        rv->m_type = KindOfNull;
        th_10SoapServer_handle((this_), (count > 0) ? &args[-0].m_data : (Value*)(&null_string));
      } else {
        tg1_10SoapServer_handle(rv, ar, count, this_);
      }
    } else {
      throw_toomany_arguments_nr("SoapServer::handle", 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SoapServer::handle");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void th_10SoapServer_setpersistence(ObjectData* this_, long mode) asm("_ZN4HPHP12c_SoapServer16t_setpersistenceEl");

void tg1_10SoapServer_setpersistence(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_10SoapServer_setpersistence(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  rv->m_type = KindOfNull;
  th_10SoapServer_setpersistence((this_), (long)(args[-0].m_data.num));
}

TypedValue* tg_10SoapServer_setpersistence(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if ((args - 0)->m_type == KindOfInt64) {
        rv->m_type = KindOfNull;
        th_10SoapServer_setpersistence((this_), (long)(args[-0].m_data.num));
      } else {
        tg1_10SoapServer_setpersistence(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("SoapServer::setpersistence", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SoapServer::setpersistence");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void th_10SoapServer_fault(ObjectData* this_, TypedValue* code, Value* fault, Value* actor, TypedValue* detail, Value* name) asm("_ZN4HPHP12c_SoapServer7t_faultERKNS_7VariantERKNS_6StringES6_S3_S6_");

void tg1_10SoapServer_fault(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_10SoapServer_fault(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfNull;
  Variant defVal3;
  th_10SoapServer_fault((this_), (args-0), &args[-1].m_data, (count > 2) ? &args[-2].m_data : (Value*)(&null_string), (count > 3) ? (args-3) : (TypedValue*)(&defVal3), (count > 4) ? &args[-4].m_data : (Value*)(&null_string));
}

TypedValue* tg_10SoapServer_fault(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 2 && count <= 5) {
      if ((count <= 4 || IS_STRING_TYPE((args - 4)->m_type)) &&
          (count <= 2 || IS_STRING_TYPE((args - 2)->m_type)) &&
          IS_STRING_TYPE((args - 1)->m_type)) {
        rv->m_type = KindOfNull;
        Variant defVal3;
        th_10SoapServer_fault((this_), (args-0), &args[-1].m_data, (count > 2) ? &args[-2].m_data : (Value*)(&null_string), (count > 3) ? (args-3) : (TypedValue*)(&defVal3), (count > 4) ? &args[-4].m_data : (Value*)(&null_string));
      } else {
        tg1_10SoapServer_fault(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("SoapServer::fault", count, 2, 5, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SoapServer::fault");
  }
  frame_free_locals_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void th_10SoapServer_addsoapheader(ObjectData* this_, Value* fault) asm("_ZN4HPHP12c_SoapServer15t_addsoapheaderERKNS_6ObjectE");

void tg1_10SoapServer_addsoapheader(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_10SoapServer_addsoapheader(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfNull;
  th_10SoapServer_addsoapheader((this_), &args[-0].m_data);
}

TypedValue* tg_10SoapServer_addsoapheader(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if ((args - 0)->m_type == KindOfObject) {
        rv->m_type = KindOfNull;
        th_10SoapServer_addsoapheader((this_), &args[-0].m_data);
      } else {
        tg1_10SoapServer_addsoapheader(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("SoapServer::addsoapheader", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SoapServer::addsoapheader");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
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
void th_10SoapClient___construct(ObjectData* this_, TypedValue* wsdl, Value* options) asm("_ZN4HPHP12c_SoapClient13t___constructERKNS_7VariantERKNS_5ArrayE");

void tg1_10SoapClient___construct(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_10SoapClient___construct(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToArrayInPlace(args-1);
  rv->m_type = KindOfNull;
  th_10SoapClient___construct((this_), (args-0), (count > 1) ? &args[-1].m_data : (Value*)(&null_array));
}

TypedValue* tg_10SoapClient___construct(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 1 && count <= 2) {
      if ((count <= 1 || (args - 1)->m_type == KindOfArray)) {
        rv->m_type = KindOfNull;
        th_10SoapClient___construct((this_), (args-0), (count > 1) ? &args[-1].m_data : (Value*)(&null_array));
      } else {
        tg1_10SoapClient___construct(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("SoapClient::__construct", count, 1, 2, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SoapClient::__construct");
  }
  frame_free_locals_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_10SoapClient___call(TypedValue* _rv, ObjectData* this_, TypedValue* name, TypedValue* args) asm("_ZN4HPHP12c_SoapClient8t___callENS_7VariantES1_");

TypedValue* tg_10SoapClient___call(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 2) {
      th_10SoapClient___call(rv, (this_), (args-0), (args-1));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      throw_wrong_arguments_nr("SoapClient::__call", count, 2, 2, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SoapClient::__call");
  }
  frame_free_locals_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_10SoapClient___soapcall(TypedValue* _rv, ObjectData* this_, Value* name, Value* args, Value* options, TypedValue* input_headers, TypedValue* output_headers) asm("_ZN4HPHP12c_SoapClient12t___soapcallERKNS_6StringERKNS_5ArrayES6_RKNS_7VariantERKNS_14VRefParamValueE");

void tg1_10SoapClient___soapcall(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_10SoapClient___soapcall(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
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
  th_10SoapClient___soapcall(rv, (this_), &args[-0].m_data, &args[-1].m_data, (count > 2) ? &args[-2].m_data : (Value*)(&null_array), (count > 3) ? (args-3) : (TypedValue*)(&null_variant), (count > 4) ? (args-4) : (TypedValue*)(&defVal4));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* tg_10SoapClient___soapcall(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 2 && count <= 5) {
      if ((count <= 2 || (args - 2)->m_type == KindOfArray) &&
          (args - 1)->m_type == KindOfArray &&
          IS_STRING_TYPE((args - 0)->m_type)) {
        VRefParamValue defVal4 = uninit_null();
        th_10SoapClient___soapcall(rv, (this_), &args[-0].m_data, &args[-1].m_data, (count > 2) ? &args[-2].m_data : (Value*)(&null_array), (count > 3) ? (args-3) : (TypedValue*)(&null_variant), (count > 4) ? (args-4) : (TypedValue*)(&defVal4));
        if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
      } else {
        tg1_10SoapClient___soapcall(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("SoapClient::__soapcall", count, 2, 5, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SoapClient::__soapcall");
  }
  frame_free_locals_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_10SoapClient___getlastrequest(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP12c_SoapClient18t___getlastrequestEv");

TypedValue* tg_10SoapClient___getlastrequest(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      th_10SoapClient___getlastrequest(rv, (this_));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      throw_toomany_arguments_nr("SoapClient::__getlastrequest", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SoapClient::__getlastrequest");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_10SoapClient___getlastresponse(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP12c_SoapClient19t___getlastresponseEv");

TypedValue* tg_10SoapClient___getlastresponse(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      th_10SoapClient___getlastresponse(rv, (this_));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      throw_toomany_arguments_nr("SoapClient::__getlastresponse", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SoapClient::__getlastresponse");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_10SoapClient___getlastrequestheaders(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP12c_SoapClient25t___getlastrequestheadersEv");

TypedValue* tg_10SoapClient___getlastrequestheaders(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      th_10SoapClient___getlastrequestheaders(rv, (this_));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      throw_toomany_arguments_nr("SoapClient::__getlastrequestheaders", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SoapClient::__getlastrequestheaders");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_10SoapClient___getlastresponseheaders(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP12c_SoapClient26t___getlastresponseheadersEv");

TypedValue* tg_10SoapClient___getlastresponseheaders(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      th_10SoapClient___getlastresponseheaders(rv, (this_));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      throw_toomany_arguments_nr("SoapClient::__getlastresponseheaders", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SoapClient::__getlastresponseheaders");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_10SoapClient___getfunctions(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP12c_SoapClient16t___getfunctionsEv");

TypedValue* tg_10SoapClient___getfunctions(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      th_10SoapClient___getfunctions(rv, (this_));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      throw_toomany_arguments_nr("SoapClient::__getfunctions", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SoapClient::__getfunctions");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_10SoapClient___gettypes(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP12c_SoapClient12t___gettypesEv");

TypedValue* tg_10SoapClient___gettypes(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      th_10SoapClient___gettypes(rv, (this_));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      throw_toomany_arguments_nr("SoapClient::__gettypes", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SoapClient::__gettypes");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_10SoapClient___dorequest(TypedValue* _rv, ObjectData* this_, Value* buf, Value* location, Value* action, long version, bool oneway) asm("_ZN4HPHP12c_SoapClient13t___dorequestERKNS_6StringES3_S3_lb");

void tg1_10SoapClient___dorequest(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_10SoapClient___dorequest(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
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
  th_10SoapClient___dorequest(rv, (this_), &args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (long)(args[-3].m_data.num), (count > 4) ? (bool)(args[-4].m_data.num) : (bool)(false));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* tg_10SoapClient___dorequest(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 4 && count <= 5) {
      if ((count <= 4 || (args - 4)->m_type == KindOfBoolean) &&
          (args - 3)->m_type == KindOfInt64 &&
          IS_STRING_TYPE((args - 2)->m_type) &&
          IS_STRING_TYPE((args - 1)->m_type) &&
          IS_STRING_TYPE((args - 0)->m_type)) {
        th_10SoapClient___dorequest(rv, (this_), &args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (long)(args[-3].m_data.num), (count > 4) ? (bool)(args[-4].m_data.num) : (bool)(false));
        if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
      } else {
        tg1_10SoapClient___dorequest(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("SoapClient::__dorequest", count, 4, 5, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SoapClient::__dorequest");
  }
  frame_free_locals_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_10SoapClient___setcookie(TypedValue* _rv, ObjectData* this_, Value* name, Value* value) asm("_ZN4HPHP12c_SoapClient13t___setcookieERKNS_6StringES3_");

void tg1_10SoapClient___setcookie(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_10SoapClient___setcookie(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
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
  th_10SoapClient___setcookie(rv, (this_), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* tg_10SoapClient___setcookie(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 1 && count <= 2) {
      if ((count <= 1 || IS_STRING_TYPE((args - 1)->m_type)) &&
          IS_STRING_TYPE((args - 0)->m_type)) {
        th_10SoapClient___setcookie(rv, (this_), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string));
        if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
      } else {
        tg1_10SoapClient___setcookie(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("SoapClient::__setcookie", count, 1, 2, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SoapClient::__setcookie");
  }
  frame_free_locals_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_10SoapClient___setlocation(TypedValue* _rv, ObjectData* this_, Value* new_location) asm("_ZN4HPHP12c_SoapClient15t___setlocationERKNS_6StringE");

void tg1_10SoapClient___setlocation(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_10SoapClient___setlocation(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  th_10SoapClient___setlocation(rv, (this_), (count > 0) ? &args[-0].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* tg_10SoapClient___setlocation(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count <= 1) {
      if ((count <= 0 || IS_STRING_TYPE((args - 0)->m_type))) {
        th_10SoapClient___setlocation(rv, (this_), (count > 0) ? &args[-0].m_data : (Value*)(&null_string));
        if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
      } else {
        tg1_10SoapClient___setlocation(rv, ar, count, this_);
      }
    } else {
      throw_toomany_arguments_nr("SoapClient::__setlocation", 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SoapClient::__setlocation");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_10SoapClient___setsoapheaders(ObjectData* this_, TypedValue* headers) asm("_ZN4HPHP12c_SoapClient18t___setsoapheadersERKNS_7VariantE");

TypedValue* tg_10SoapClient___setsoapheaders(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count <= 1) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (th_10SoapClient___setsoapheaders((this_), (count > 0) ? (args-0) : (TypedValue*)(&null_variant))) ? 1LL : 0LL;
    } else {
      throw_toomany_arguments_nr("SoapClient::__setsoapheaders", 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SoapClient::__setsoapheaders");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
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
void th_7SoapVar___construct(ObjectData* this_, TypedValue* data, TypedValue* type, Value* type_name, Value* type_namespace, Value* node_name, Value* node_namespace) asm("_ZN4HPHP9c_SoapVar13t___constructERKNS_7VariantES3_RKNS_6StringES6_S6_S6_");

void tg1_7SoapVar___construct(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_7SoapVar___construct(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfNull;
  th_7SoapVar___construct((this_), (args-0), (args-1), (count > 2) ? &args[-2].m_data : (Value*)(&null_string), (count > 3) ? &args[-3].m_data : (Value*)(&null_string), (count > 4) ? &args[-4].m_data : (Value*)(&null_string), (count > 5) ? &args[-5].m_data : (Value*)(&null_string));
}

TypedValue* tg_7SoapVar___construct(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 2 && count <= 6) {
      if ((count <= 5 || IS_STRING_TYPE((args - 5)->m_type)) &&
          (count <= 4 || IS_STRING_TYPE((args - 4)->m_type)) &&
          (count <= 3 || IS_STRING_TYPE((args - 3)->m_type)) &&
          (count <= 2 || IS_STRING_TYPE((args - 2)->m_type))) {
        rv->m_type = KindOfNull;
        th_7SoapVar___construct((this_), (args-0), (args-1), (count > 2) ? &args[-2].m_data : (Value*)(&null_string), (count > 3) ? &args[-3].m_data : (Value*)(&null_string), (count > 4) ? &args[-4].m_data : (Value*)(&null_string), (count > 5) ? &args[-5].m_data : (Value*)(&null_string));
      } else {
        tg1_7SoapVar___construct(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("SoapVar::__construct", count, 2, 6, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SoapVar::__construct");
  }
  frame_free_locals_inl(ar, 6);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
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
void th_9SoapParam___construct(ObjectData* this_, TypedValue* data, Value* name) asm("_ZN4HPHP11c_SoapParam13t___constructERKNS_7VariantERKNS_6StringE");

void tg1_9SoapParam___construct(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_9SoapParam___construct(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-1);
  rv->m_type = KindOfNull;
  th_9SoapParam___construct((this_), (args-0), &args[-1].m_data);
}

TypedValue* tg_9SoapParam___construct(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 2) {
      if (IS_STRING_TYPE((args - 1)->m_type)) {
        rv->m_type = KindOfNull;
        th_9SoapParam___construct((this_), (args-0), &args[-1].m_data);
      } else {
        tg1_9SoapParam___construct(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("SoapParam::__construct", count, 2, 2, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SoapParam::__construct");
  }
  frame_free_locals_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
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
void th_10SoapHeader___construct(ObjectData* this_, Value* ns, Value* name, TypedValue* data, bool mustunderstand, TypedValue* actor) asm("_ZN4HPHP12c_SoapHeader13t___constructERKNS_6StringES3_RKNS_7VariantEbS6_");

void tg1_10SoapHeader___construct(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_10SoapHeader___construct(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfNull;
  Variant defVal2;
  Variant defVal4;
  th_10SoapHeader___construct((this_), &args[-0].m_data, &args[-1].m_data, (count > 2) ? (args-2) : (TypedValue*)(&defVal2), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false), (count > 4) ? (args-4) : (TypedValue*)(&defVal4));
}

TypedValue* tg_10SoapHeader___construct(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 2 && count <= 5) {
      if ((count <= 3 || (args - 3)->m_type == KindOfBoolean) &&
          IS_STRING_TYPE((args - 1)->m_type) &&
          IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfNull;
        Variant defVal2;
        Variant defVal4;
        th_10SoapHeader___construct((this_), &args[-0].m_data, &args[-1].m_data, (count > 2) ? (args-2) : (TypedValue*)(&defVal2), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false), (count > 4) ? (args-4) : (TypedValue*)(&defVal4));
      } else {
        tg1_10SoapHeader___construct(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("SoapHeader::__construct", count, 2, 5, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SoapHeader::__construct");
  }
  frame_free_locals_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

} // namespace HPHP
