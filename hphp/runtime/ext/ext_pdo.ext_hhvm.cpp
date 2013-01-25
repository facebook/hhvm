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
HPHP::Array HPHP::f_pdo_drivers()
_ZN4HPHP13f_pdo_driversEv

(return value) => rax
_rv => rdi
*/

Value* fh_pdo_drivers(Value* _rv) asm("_ZN4HPHP13f_pdo_driversEv");

TypedValue* fg_pdo_drivers(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfArray;
      fh_pdo_drivers((Value*)(&(rv)));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("pdo_drivers", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



HPHP::VM::Instance* new_PDO_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_PDO) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_PDO(ObjectStaticCallbacks::encodeVMClass(cls));
  return inst;
}

/*
void HPHP::c_PDO::t___construct(HPHP::String const&, HPHP::String const&, HPHP::String const&, HPHP::Array const&)
_ZN4HPHP5c_PDO13t___constructERKNS_6StringES3_S3_RKNS_5ArrayE

this_ => rdi
dsn => rsi
username => rdx
password => rcx
options => r8
*/

void th_3PDO___construct(ObjectData* this_, Value* dsn, Value* username, Value* password, Value* options) asm("_ZN4HPHP5c_PDO13t___constructERKNS_6StringES3_S3_RKNS_5ArrayE");

TypedValue* tg1_3PDO___construct(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_3PDO___construct(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfArray) {
      tvCastToArrayInPlace(args-3);
    }
  case 3:
    if (!IS_STRING_TYPE((args-2)->m_type)) {
      tvCastToStringInPlace(args-2);
    }
  case 2:
    if (!IS_STRING_TYPE((args-1)->m_type)) {
      tvCastToStringInPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  th_3PDO___construct((this_), (Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_string), (count > 2) ? (Value*)(args-2) : (Value*)(&null_string), (count > 3) ? (Value*)(args-3) : (Value*)(&null_array));
  return rv;
}

TypedValue* tg_3PDO___construct(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL && count <= 4LL) {
        if ((count <= 3 || (args-3)->m_type == KindOfArray) && (count <= 2 || IS_STRING_TYPE((args-2)->m_type)) && (count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_data.num = 0LL;
          rv._count = 0;
          rv.m_type = KindOfNull;
          th_3PDO___construct((this_), (Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_string), (count > 2) ? (Value*)(args-2) : (Value*)(&null_string), (count > 3) ? (Value*)(args-3) : (Value*)(&null_array));
          frame_free_locals_inl(ar, 4);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_3PDO___construct(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 4);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("PDO::__construct", count, 1, 4, 1);
      }
    } else {
      throw_instance_method_fatal("PDO::__construct");
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
HPHP::Variant HPHP::c_PDO::t_prepare(HPHP::String const&, HPHP::Array const&)
_ZN4HPHP5c_PDO9t_prepareERKNS_6StringERKNS_5ArrayE

(return value) => rax
_rv => rdi
this_ => rsi
statement => rdx
options => rcx
*/

TypedValue* th_3PDO_prepare(TypedValue* _rv, ObjectData* this_, Value* statement, Value* options) asm("_ZN4HPHP5c_PDO9t_prepareERKNS_6StringERKNS_5ArrayE");

TypedValue* tg1_3PDO_prepare(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_3PDO_prepare(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfArray) {
      tvCastToArrayInPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  th_3PDO_prepare((rv), (this_), (Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_array));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_3PDO_prepare(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL && count <= 2LL) {
        if ((count <= 1 || (args-1)->m_type == KindOfArray) && IS_STRING_TYPE((args-0)->m_type)) {
          th_3PDO_prepare((&(rv)), (this_), (Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_array));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_3PDO_prepare(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("PDO::prepare", count, 1, 2, 1);
      }
    } else {
      throw_instance_method_fatal("PDO::prepare");
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
bool HPHP::c_PDO::t_begintransaction()
_ZN4HPHP5c_PDO18t_begintransactionEv

(return value) => rax
this_ => rdi
*/

bool th_3PDO_begintransaction(ObjectData* this_) asm("_ZN4HPHP5c_PDO18t_begintransactionEv");

TypedValue* tg_3PDO_begintransaction(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_3PDO_begintransaction((this_))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("PDO::begintransaction", 0, 1);
      }
    } else {
      throw_instance_method_fatal("PDO::begintransaction");
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
bool HPHP::c_PDO::t_commit()
_ZN4HPHP5c_PDO8t_commitEv

(return value) => rax
this_ => rdi
*/

bool th_3PDO_commit(ObjectData* this_) asm("_ZN4HPHP5c_PDO8t_commitEv");

TypedValue* tg_3PDO_commit(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_3PDO_commit((this_))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("PDO::commit", 0, 1);
      }
    } else {
      throw_instance_method_fatal("PDO::commit");
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
bool HPHP::c_PDO::t_rollback()
_ZN4HPHP5c_PDO10t_rollbackEv

(return value) => rax
this_ => rdi
*/

bool th_3PDO_rollback(ObjectData* this_) asm("_ZN4HPHP5c_PDO10t_rollbackEv");

TypedValue* tg_3PDO_rollback(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_3PDO_rollback((this_))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("PDO::rollback", 0, 1);
      }
    } else {
      throw_instance_method_fatal("PDO::rollback");
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
bool HPHP::c_PDO::t_setattribute(long, HPHP::Variant const&)
_ZN4HPHP5c_PDO14t_setattributeElRKNS_7VariantE

(return value) => rax
this_ => rdi
attribute => rsi
value => rdx
*/

bool th_3PDO_setattribute(ObjectData* this_, long attribute, TypedValue* value) asm("_ZN4HPHP5c_PDO14t_setattributeElRKNS_7VariantE");

TypedValue* tg1_3PDO_setattribute(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_3PDO_setattribute(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToInt64InPlace(args-0);
  rv->m_data.num = (th_3PDO_setattribute((this_), (long)(args[-0].m_data.num), (args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_3PDO_setattribute(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        if ((args-0)->m_type == KindOfInt64) {
          rv._count = 0;
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_3PDO_setattribute((this_), (long)(args[-0].m_data.num), (args-1))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_3PDO_setattribute(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("PDO::setattribute", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("PDO::setattribute");
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
HPHP::Variant HPHP::c_PDO::t_getattribute(long)
_ZN4HPHP5c_PDO14t_getattributeEl

(return value) => rax
_rv => rdi
this_ => rsi
attribute => rdx
*/

TypedValue* th_3PDO_getattribute(TypedValue* _rv, ObjectData* this_, long attribute) asm("_ZN4HPHP5c_PDO14t_getattributeEl");

TypedValue* tg1_3PDO_getattribute(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_3PDO_getattribute(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  th_3PDO_getattribute((rv), (this_), (long)(args[-0].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_3PDO_getattribute(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if ((args-0)->m_type == KindOfInt64) {
          th_3PDO_getattribute((&(rv)), (this_), (long)(args[-0].m_data.num));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_3PDO_getattribute(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("PDO::getattribute", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("PDO::getattribute");
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
HPHP::Variant HPHP::c_PDO::t_exec(HPHP::String const&)
_ZN4HPHP5c_PDO6t_execERKNS_6StringE

(return value) => rax
_rv => rdi
this_ => rsi
query => rdx
*/

TypedValue* th_3PDO_exec(TypedValue* _rv, ObjectData* this_, Value* query) asm("_ZN4HPHP5c_PDO6t_execERKNS_6StringE");

TypedValue* tg1_3PDO_exec(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_3PDO_exec(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  th_3PDO_exec((rv), (this_), (Value*)(args-0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_3PDO_exec(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          th_3PDO_exec((&(rv)), (this_), (Value*)(args-0));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_3PDO_exec(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("PDO::exec", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("PDO::exec");
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
HPHP::Variant HPHP::c_PDO::t_lastinsertid(HPHP::String const&)
_ZN4HPHP5c_PDO14t_lastinsertidERKNS_6StringE

(return value) => rax
_rv => rdi
this_ => rsi
seqname => rdx
*/

TypedValue* th_3PDO_lastinsertid(TypedValue* _rv, ObjectData* this_, Value* seqname) asm("_ZN4HPHP5c_PDO14t_lastinsertidERKNS_6StringE");

TypedValue* tg1_3PDO_lastinsertid(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_3PDO_lastinsertid(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  th_3PDO_lastinsertid((rv), (this_), (count > 0) ? (Value*)(args-0) : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_3PDO_lastinsertid(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count <= 1LL) {
        if ((count <= 0 || IS_STRING_TYPE((args-0)->m_type))) {
          th_3PDO_lastinsertid((&(rv)), (this_), (count > 0) ? (Value*)(args-0) : (Value*)(&null_string));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_3PDO_lastinsertid(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_toomany_arguments_nr("PDO::lastinsertid", 1, 1);
      }
    } else {
      throw_instance_method_fatal("PDO::lastinsertid");
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
HPHP::Variant HPHP::c_PDO::t_errorcode()
_ZN4HPHP5c_PDO11t_errorcodeEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_3PDO_errorcode(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP5c_PDO11t_errorcodeEv");

TypedValue* tg_3PDO_errorcode(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_3PDO_errorcode((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("PDO::errorcode", 0, 1);
      }
    } else {
      throw_instance_method_fatal("PDO::errorcode");
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
HPHP::Array HPHP::c_PDO::t_errorinfo()
_ZN4HPHP5c_PDO11t_errorinfoEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_3PDO_errorinfo(Value* _rv, ObjectData* this_) asm("_ZN4HPHP5c_PDO11t_errorinfoEv");

TypedValue* tg_3PDO_errorinfo(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfArray;
        th_3PDO_errorinfo((Value*)(&(rv)), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("PDO::errorinfo", 0, 1);
      }
    } else {
      throw_instance_method_fatal("PDO::errorinfo");
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
HPHP::Variant HPHP::c_PDO::t_query(HPHP::String const&)
_ZN4HPHP5c_PDO7t_queryERKNS_6StringE

(return value) => rax
_rv => rdi
this_ => rsi
sql => rdx
*/

TypedValue* th_3PDO_query(TypedValue* _rv, ObjectData* this_, Value* sql) asm("_ZN4HPHP5c_PDO7t_queryERKNS_6StringE");

TypedValue* tg1_3PDO_query(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_3PDO_query(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  th_3PDO_query((rv), (this_), (Value*)(args-0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_3PDO_query(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          th_3PDO_query((&(rv)), (this_), (Value*)(args-0));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_3PDO_query(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("PDO::query", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("PDO::query");
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
HPHP::Variant HPHP::c_PDO::t_quote(HPHP::String const&, long)
_ZN4HPHP5c_PDO7t_quoteERKNS_6StringEl

(return value) => rax
_rv => rdi
this_ => rsi
str => rdx
paramtype => rcx
*/

TypedValue* th_3PDO_quote(TypedValue* _rv, ObjectData* this_, Value* str, long paramtype) asm("_ZN4HPHP5c_PDO7t_quoteERKNS_6StringEl");

TypedValue* tg1_3PDO_quote(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_3PDO_quote(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  th_3PDO_quote((rv), (this_), (Value*)(args-0), (count > 1) ? (long)(args[-1].m_data.num) : (long)(q_PDO$$PARAM_STR));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_3PDO_quote(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL && count <= 2LL) {
        if ((count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
          th_3PDO_quote((&(rv)), (this_), (Value*)(args-0), (count > 1) ? (long)(args[-1].m_data.num) : (long)(q_PDO$$PARAM_STR));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_3PDO_quote(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("PDO::quote", count, 1, 2, 1);
      }
    } else {
      throw_instance_method_fatal("PDO::quote");
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
HPHP::Variant HPHP::c_PDO::t___wakeup()
_ZN4HPHP5c_PDO10t___wakeupEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_3PDO___wakeup(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP5c_PDO10t___wakeupEv");

TypedValue* tg_3PDO___wakeup(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_3PDO___wakeup((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("PDO::__wakeup", 0, 1);
      }
    } else {
      throw_instance_method_fatal("PDO::__wakeup");
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
HPHP::Variant HPHP::c_PDO::t___sleep()
_ZN4HPHP5c_PDO9t___sleepEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_3PDO___sleep(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP5c_PDO9t___sleepEv");

TypedValue* tg_3PDO___sleep(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_3PDO___sleep((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("PDO::__sleep", 0, 1);
      }
    } else {
      throw_instance_method_fatal("PDO::__sleep");
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
HPHP::Array HPHP::c_PDO::ti_getavailabledrivers(char const*)
_ZN4HPHP5c_PDO22ti_getavailabledriversEPKc

(return value) => rax
_rv => rdi
cls_ => rsi
*/

Value* th_3PDO_getavailabledrivers(Value* _rv, char const* cls_) asm("_ZN4HPHP5c_PDO22ti_getavailabledriversEPKc");

TypedValue* tg_3PDO_getavailabledrivers(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfArray;
      th_3PDO_getavailabledrivers((Value*)(&(rv)), ("PDO"));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("PDO::getavailabledrivers", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

HPHP::VM::Instance* new_PDOStatement_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_PDOStatement) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_PDOStatement(ObjectStaticCallbacks::encodeVMClass(cls));
  return inst;
}

/*
void HPHP::c_PDOStatement::t___construct()
_ZN4HPHP14c_PDOStatement13t___constructEv

this_ => rdi
*/

void th_12PDOStatement___construct(ObjectData* this_) asm("_ZN4HPHP14c_PDOStatement13t___constructEv");

TypedValue* tg_12PDOStatement___construct(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        th_12PDOStatement___construct((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("PDOStatement::__construct", 0, 1);
      }
    } else {
      throw_instance_method_fatal("PDOStatement::__construct");
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
HPHP::Variant HPHP::c_PDOStatement::t_execute(HPHP::Array const&)
_ZN4HPHP14c_PDOStatement9t_executeERKNS_5ArrayE

(return value) => rax
_rv => rdi
this_ => rsi
params => rdx
*/

TypedValue* th_12PDOStatement_execute(TypedValue* _rv, ObjectData* this_, Value* params) asm("_ZN4HPHP14c_PDOStatement9t_executeERKNS_5ArrayE");

TypedValue* tg1_12PDOStatement_execute(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_12PDOStatement_execute(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToArrayInPlace(args-0);
  th_12PDOStatement_execute((rv), (this_), (count > 0) ? (Value*)(args-0) : (Value*)(&null_array));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_12PDOStatement_execute(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count <= 1LL) {
        if ((count <= 0 || (args-0)->m_type == KindOfArray)) {
          th_12PDOStatement_execute((&(rv)), (this_), (count > 0) ? (Value*)(args-0) : (Value*)(&null_array));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_12PDOStatement_execute(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_toomany_arguments_nr("PDOStatement::execute", 1, 1);
      }
    } else {
      throw_instance_method_fatal("PDOStatement::execute");
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
HPHP::Variant HPHP::c_PDOStatement::t_fetch(long, long, long)
_ZN4HPHP14c_PDOStatement7t_fetchElll

(return value) => rax
_rv => rdi
this_ => rsi
how => rdx
orientation => rcx
offset => r8
*/

TypedValue* th_12PDOStatement_fetch(TypedValue* _rv, ObjectData* this_, long how, long orientation, long offset) asm("_ZN4HPHP14c_PDOStatement7t_fetchElll");

TypedValue* tg1_12PDOStatement_fetch(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_12PDOStatement_fetch(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
    if ((args-0)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-0);
    }
  case 0:
    break;
  }
  th_12PDOStatement_fetch((rv), (this_), (count > 0) ? (long)(args[-0].m_data.num) : (long)(0), (count > 1) ? (long)(args[-1].m_data.num) : (long)(q_PDO$$FETCH_ORI_NEXT), (count > 2) ? (long)(args[-2].m_data.num) : (long)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_12PDOStatement_fetch(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count <= 3LL) {
        if ((count <= 2 || (args-2)->m_type == KindOfInt64) && (count <= 1 || (args-1)->m_type == KindOfInt64) && (count <= 0 || (args-0)->m_type == KindOfInt64)) {
          th_12PDOStatement_fetch((&(rv)), (this_), (count > 0) ? (long)(args[-0].m_data.num) : (long)(0), (count > 1) ? (long)(args[-1].m_data.num) : (long)(q_PDO$$FETCH_ORI_NEXT), (count > 2) ? (long)(args[-2].m_data.num) : (long)(0));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_12PDOStatement_fetch(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_toomany_arguments_nr("PDOStatement::fetch", 3, 1);
      }
    } else {
      throw_instance_method_fatal("PDOStatement::fetch");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_PDOStatement::t_fetchobject(HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP14c_PDOStatement13t_fetchobjectERKNS_6StringERKNS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
class_name => rdx
ctor_args => rcx
*/

TypedValue* th_12PDOStatement_fetchobject(TypedValue* _rv, ObjectData* this_, Value* class_name, TypedValue* ctor_args) asm("_ZN4HPHP14c_PDOStatement13t_fetchobjectERKNS_6StringERKNS_7VariantE");

TypedValue* tg1_12PDOStatement_fetchobject(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_12PDOStatement_fetchobject(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  Variant defVal1;
  th_12PDOStatement_fetchobject((rv), (this_), (count > 0) ? (Value*)(args-0) : (Value*)(&null_string), (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_12PDOStatement_fetchobject(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count <= 2LL) {
        if ((count <= 0 || IS_STRING_TYPE((args-0)->m_type))) {
          Variant defVal1;
          th_12PDOStatement_fetchobject((&(rv)), (this_), (count > 0) ? (Value*)(args-0) : (Value*)(&null_string), (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_12PDOStatement_fetchobject(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_toomany_arguments_nr("PDOStatement::fetchobject", 2, 1);
      }
    } else {
      throw_instance_method_fatal("PDOStatement::fetchobject");
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
HPHP::Variant HPHP::c_PDOStatement::t_fetchcolumn(long)
_ZN4HPHP14c_PDOStatement13t_fetchcolumnEl

(return value) => rax
_rv => rdi
this_ => rsi
column_numner => rdx
*/

TypedValue* th_12PDOStatement_fetchcolumn(TypedValue* _rv, ObjectData* this_, long column_numner) asm("_ZN4HPHP14c_PDOStatement13t_fetchcolumnEl");

TypedValue* tg1_12PDOStatement_fetchcolumn(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_12PDOStatement_fetchcolumn(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  th_12PDOStatement_fetchcolumn((rv), (this_), (count > 0) ? (long)(args[-0].m_data.num) : (long)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_12PDOStatement_fetchcolumn(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count <= 1LL) {
        if ((count <= 0 || (args-0)->m_type == KindOfInt64)) {
          th_12PDOStatement_fetchcolumn((&(rv)), (this_), (count > 0) ? (long)(args[-0].m_data.num) : (long)(0));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_12PDOStatement_fetchcolumn(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_toomany_arguments_nr("PDOStatement::fetchcolumn", 1, 1);
      }
    } else {
      throw_instance_method_fatal("PDOStatement::fetchcolumn");
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
HPHP::Variant HPHP::c_PDOStatement::t_fetchall(long, HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP14c_PDOStatement10t_fetchallElRKNS_7VariantES3_

(return value) => rax
_rv => rdi
this_ => rsi
how => rdx
class_name => rcx
ctor_args => r8
*/

TypedValue* th_12PDOStatement_fetchall(TypedValue* _rv, ObjectData* this_, long how, TypedValue* class_name, TypedValue* ctor_args) asm("_ZN4HPHP14c_PDOStatement10t_fetchallElRKNS_7VariantES3_");

TypedValue* tg1_12PDOStatement_fetchall(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_12PDOStatement_fetchall(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  Variant defVal1;
  Variant defVal2;
  th_12PDOStatement_fetchall((rv), (this_), (count > 0) ? (long)(args[-0].m_data.num) : (long)(0), (count > 1) ? (args-1) : (TypedValue*)(&defVal1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_12PDOStatement_fetchall(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count <= 3LL) {
        if ((count <= 0 || (args-0)->m_type == KindOfInt64)) {
          Variant defVal1;
          Variant defVal2;
          th_12PDOStatement_fetchall((&(rv)), (this_), (count > 0) ? (long)(args[-0].m_data.num) : (long)(0), (count > 1) ? (args-1) : (TypedValue*)(&defVal1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_12PDOStatement_fetchall(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_toomany_arguments_nr("PDOStatement::fetchall", 3, 1);
      }
    } else {
      throw_instance_method_fatal("PDOStatement::fetchall");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_PDOStatement::t_bindvalue(HPHP::Variant const&, HPHP::Variant const&, long)
_ZN4HPHP14c_PDOStatement11t_bindvalueERKNS_7VariantES3_l

(return value) => rax
this_ => rdi
paramno => rsi
param => rdx
type => rcx
*/

bool th_12PDOStatement_bindvalue(ObjectData* this_, TypedValue* paramno, TypedValue* param, long type) asm("_ZN4HPHP14c_PDOStatement11t_bindvalueERKNS_7VariantES3_l");

TypedValue* tg1_12PDOStatement_bindvalue(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_12PDOStatement_bindvalue(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToInt64InPlace(args-2);
  rv->m_data.num = (th_12PDOStatement_bindvalue((this_), (args-0), (args-1), (count > 2) ? (long)(args[-2].m_data.num) : (long)(q_PDO$$PARAM_STR))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_12PDOStatement_bindvalue(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 2LL && count <= 3LL) {
        if ((count <= 2 || (args-2)->m_type == KindOfInt64)) {
          rv._count = 0;
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_12PDOStatement_bindvalue((this_), (args-0), (args-1), (count > 2) ? (long)(args[-2].m_data.num) : (long)(q_PDO$$PARAM_STR))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_12PDOStatement_bindvalue(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("PDOStatement::bindvalue", count, 2, 3, 1);
      }
    } else {
      throw_instance_method_fatal("PDOStatement::bindvalue");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_PDOStatement::t_bindparam(HPHP::Variant const&, HPHP::VRefParamValue const&, long, long, HPHP::Variant const&)
_ZN4HPHP14c_PDOStatement11t_bindparamERKNS_7VariantERKNS_14VRefParamValueEllS3_

(return value) => rax
this_ => rdi
paramno => rsi
param => rdx
type => rcx
max_value_len => r8
driver_params => r9
*/

bool th_12PDOStatement_bindparam(ObjectData* this_, TypedValue* paramno, TypedValue* param, long type, long max_value_len, TypedValue* driver_params) asm("_ZN4HPHP14c_PDOStatement11t_bindparamERKNS_7VariantERKNS_14VRefParamValueEllS3_");

TypedValue* tg1_12PDOStatement_bindparam(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_12PDOStatement_bindparam(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 5
  case 4:
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    break;
  }
  Variant defVal4;
  rv->m_data.num = (th_12PDOStatement_bindparam((this_), (args-0), (args-1), (count > 2) ? (long)(args[-2].m_data.num) : (long)(q_PDO$$PARAM_STR), (count > 3) ? (long)(args[-3].m_data.num) : (long)(0), (count > 4) ? (args-4) : (TypedValue*)(&defVal4))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_12PDOStatement_bindparam(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 2LL && count <= 5LL) {
        if ((count <= 3 || (args-3)->m_type == KindOfInt64) && (count <= 2 || (args-2)->m_type == KindOfInt64)) {
          rv._count = 0;
          rv.m_type = KindOfBoolean;
          Variant defVal4;
          rv.m_data.num = (th_12PDOStatement_bindparam((this_), (args-0), (args-1), (count > 2) ? (long)(args[-2].m_data.num) : (long)(q_PDO$$PARAM_STR), (count > 3) ? (long)(args[-3].m_data.num) : (long)(0), (count > 4) ? (args-4) : (TypedValue*)(&defVal4))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 5);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_12PDOStatement_bindparam(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 5);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("PDOStatement::bindparam", count, 2, 5, 1);
      }
    } else {
      throw_instance_method_fatal("PDOStatement::bindparam");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 5);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_PDOStatement::t_bindcolumn(HPHP::Variant const&, HPHP::VRefParamValue const&, long, long, HPHP::Variant const&)
_ZN4HPHP14c_PDOStatement12t_bindcolumnERKNS_7VariantERKNS_14VRefParamValueEllS3_

(return value) => rax
this_ => rdi
paramno => rsi
param => rdx
type => rcx
max_value_len => r8
driver_params => r9
*/

bool th_12PDOStatement_bindcolumn(ObjectData* this_, TypedValue* paramno, TypedValue* param, long type, long max_value_len, TypedValue* driver_params) asm("_ZN4HPHP14c_PDOStatement12t_bindcolumnERKNS_7VariantERKNS_14VRefParamValueEllS3_");

TypedValue* tg1_12PDOStatement_bindcolumn(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_12PDOStatement_bindcolumn(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 5
  case 4:
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    break;
  }
  Variant defVal4;
  rv->m_data.num = (th_12PDOStatement_bindcolumn((this_), (args-0), (args-1), (count > 2) ? (long)(args[-2].m_data.num) : (long)(q_PDO$$PARAM_STR), (count > 3) ? (long)(args[-3].m_data.num) : (long)(0), (count > 4) ? (args-4) : (TypedValue*)(&defVal4))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_12PDOStatement_bindcolumn(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 2LL && count <= 5LL) {
        if ((count <= 3 || (args-3)->m_type == KindOfInt64) && (count <= 2 || (args-2)->m_type == KindOfInt64)) {
          rv._count = 0;
          rv.m_type = KindOfBoolean;
          Variant defVal4;
          rv.m_data.num = (th_12PDOStatement_bindcolumn((this_), (args-0), (args-1), (count > 2) ? (long)(args[-2].m_data.num) : (long)(q_PDO$$PARAM_STR), (count > 3) ? (long)(args[-3].m_data.num) : (long)(0), (count > 4) ? (args-4) : (TypedValue*)(&defVal4))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 5);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_12PDOStatement_bindcolumn(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 5);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("PDOStatement::bindcolumn", count, 2, 5, 1);
      }
    } else {
      throw_instance_method_fatal("PDOStatement::bindcolumn");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 5);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
long HPHP::c_PDOStatement::t_rowcount()
_ZN4HPHP14c_PDOStatement10t_rowcountEv

(return value) => rax
this_ => rdi
*/

long th_12PDOStatement_rowcount(ObjectData* this_) asm("_ZN4HPHP14c_PDOStatement10t_rowcountEv");

TypedValue* tg_12PDOStatement_rowcount(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)th_12PDOStatement_rowcount((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("PDOStatement::rowcount", 0, 1);
      }
    } else {
      throw_instance_method_fatal("PDOStatement::rowcount");
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
HPHP::Variant HPHP::c_PDOStatement::t_errorcode()
_ZN4HPHP14c_PDOStatement11t_errorcodeEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_12PDOStatement_errorcode(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP14c_PDOStatement11t_errorcodeEv");

TypedValue* tg_12PDOStatement_errorcode(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_12PDOStatement_errorcode((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("PDOStatement::errorcode", 0, 1);
      }
    } else {
      throw_instance_method_fatal("PDOStatement::errorcode");
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
HPHP::Array HPHP::c_PDOStatement::t_errorinfo()
_ZN4HPHP14c_PDOStatement11t_errorinfoEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_12PDOStatement_errorinfo(Value* _rv, ObjectData* this_) asm("_ZN4HPHP14c_PDOStatement11t_errorinfoEv");

TypedValue* tg_12PDOStatement_errorinfo(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfArray;
        th_12PDOStatement_errorinfo((Value*)(&(rv)), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("PDOStatement::errorinfo", 0, 1);
      }
    } else {
      throw_instance_method_fatal("PDOStatement::errorinfo");
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
HPHP::Variant HPHP::c_PDOStatement::t_setattribute(long, HPHP::Variant const&)
_ZN4HPHP14c_PDOStatement14t_setattributeElRKNS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
attribute => rdx
value => rcx
*/

TypedValue* th_12PDOStatement_setattribute(TypedValue* _rv, ObjectData* this_, long attribute, TypedValue* value) asm("_ZN4HPHP14c_PDOStatement14t_setattributeElRKNS_7VariantE");

TypedValue* tg1_12PDOStatement_setattribute(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_12PDOStatement_setattribute(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  th_12PDOStatement_setattribute((rv), (this_), (long)(args[-0].m_data.num), (args-1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_12PDOStatement_setattribute(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        if ((args-0)->m_type == KindOfInt64) {
          th_12PDOStatement_setattribute((&(rv)), (this_), (long)(args[-0].m_data.num), (args-1));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_12PDOStatement_setattribute(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("PDOStatement::setattribute", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("PDOStatement::setattribute");
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
HPHP::Variant HPHP::c_PDOStatement::t_getattribute(long)
_ZN4HPHP14c_PDOStatement14t_getattributeEl

(return value) => rax
_rv => rdi
this_ => rsi
attribute => rdx
*/

TypedValue* th_12PDOStatement_getattribute(TypedValue* _rv, ObjectData* this_, long attribute) asm("_ZN4HPHP14c_PDOStatement14t_getattributeEl");

TypedValue* tg1_12PDOStatement_getattribute(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_12PDOStatement_getattribute(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  th_12PDOStatement_getattribute((rv), (this_), (long)(args[-0].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_12PDOStatement_getattribute(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if ((args-0)->m_type == KindOfInt64) {
          th_12PDOStatement_getattribute((&(rv)), (this_), (long)(args[-0].m_data.num));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_12PDOStatement_getattribute(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("PDOStatement::getattribute", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("PDOStatement::getattribute");
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
long HPHP::c_PDOStatement::t_columncount()
_ZN4HPHP14c_PDOStatement13t_columncountEv

(return value) => rax
this_ => rdi
*/

long th_12PDOStatement_columncount(ObjectData* this_) asm("_ZN4HPHP14c_PDOStatement13t_columncountEv");

TypedValue* tg_12PDOStatement_columncount(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)th_12PDOStatement_columncount((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("PDOStatement::columncount", 0, 1);
      }
    } else {
      throw_instance_method_fatal("PDOStatement::columncount");
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
HPHP::Variant HPHP::c_PDOStatement::t_getcolumnmeta(long)
_ZN4HPHP14c_PDOStatement15t_getcolumnmetaEl

(return value) => rax
_rv => rdi
this_ => rsi
column => rdx
*/

TypedValue* th_12PDOStatement_getcolumnmeta(TypedValue* _rv, ObjectData* this_, long column) asm("_ZN4HPHP14c_PDOStatement15t_getcolumnmetaEl");

TypedValue* tg1_12PDOStatement_getcolumnmeta(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_12PDOStatement_getcolumnmeta(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  th_12PDOStatement_getcolumnmeta((rv), (this_), (long)(args[-0].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_12PDOStatement_getcolumnmeta(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if ((args-0)->m_type == KindOfInt64) {
          th_12PDOStatement_getcolumnmeta((&(rv)), (this_), (long)(args[-0].m_data.num));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_12PDOStatement_getcolumnmeta(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("PDOStatement::getcolumnmeta", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("PDOStatement::getcolumnmeta");
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
bool HPHP::c_PDOStatement::t_setfetchmode(int, long, HPHP::Array const&)
_ZN4HPHP14c_PDOStatement14t_setfetchmodeEilRKNS_5ArrayE

(return value) => rax
this_ => rdi
_argc => rsi
mode => rdx
_argv => rcx
*/

bool th_12PDOStatement_setfetchmode(ObjectData* this_, int64_t _argc, long mode, Value* _argv) asm("_ZN4HPHP14c_PDOStatement14t_setfetchmodeEilRKNS_5ArrayE");

TypedValue* tg1_12PDOStatement_setfetchmode(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_12PDOStatement_setfetchmode(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToInt64InPlace(args-0);
  Array extraArgs;
  {
    ArrayInit ai(count-1, false);
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
  rv->m_data.num = (th_12PDOStatement_setfetchmode((this_), (count), (long)(args[-0].m_data.num), (Value*)(&extraArgs))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_12PDOStatement_setfetchmode(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL) {
        if ((args-0)->m_type == KindOfInt64) {
          rv._count = 0;
          rv.m_type = KindOfBoolean;
          Array extraArgs;
          {
            ArrayInit ai(count-1, false);
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
          rv.m_data.num = (th_12PDOStatement_setfetchmode((this_), (count), (long)(args[-0].m_data.num), (Value*)(&extraArgs))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_12PDOStatement_setfetchmode(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_missing_arguments_nr("PDOStatement::setfetchmode", count+1, 1);
      }
    } else {
      throw_instance_method_fatal("PDOStatement::setfetchmode");
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
bool HPHP::c_PDOStatement::t_nextrowset()
_ZN4HPHP14c_PDOStatement12t_nextrowsetEv

(return value) => rax
this_ => rdi
*/

bool th_12PDOStatement_nextrowset(ObjectData* this_) asm("_ZN4HPHP14c_PDOStatement12t_nextrowsetEv");

TypedValue* tg_12PDOStatement_nextrowset(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_12PDOStatement_nextrowset((this_))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("PDOStatement::nextrowset", 0, 1);
      }
    } else {
      throw_instance_method_fatal("PDOStatement::nextrowset");
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
bool HPHP::c_PDOStatement::t_closecursor()
_ZN4HPHP14c_PDOStatement13t_closecursorEv

(return value) => rax
this_ => rdi
*/

bool th_12PDOStatement_closecursor(ObjectData* this_) asm("_ZN4HPHP14c_PDOStatement13t_closecursorEv");

TypedValue* tg_12PDOStatement_closecursor(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_12PDOStatement_closecursor((this_))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("PDOStatement::closecursor", 0, 1);
      }
    } else {
      throw_instance_method_fatal("PDOStatement::closecursor");
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
HPHP::Variant HPHP::c_PDOStatement::t_debugdumpparams()
_ZN4HPHP14c_PDOStatement17t_debugdumpparamsEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_12PDOStatement_debugdumpparams(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP14c_PDOStatement17t_debugdumpparamsEv");

TypedValue* tg_12PDOStatement_debugdumpparams(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_12PDOStatement_debugdumpparams((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("PDOStatement::debugdumpparams", 0, 1);
      }
    } else {
      throw_instance_method_fatal("PDOStatement::debugdumpparams");
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
HPHP::Variant HPHP::c_PDOStatement::t_current()
_ZN4HPHP14c_PDOStatement9t_currentEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_12PDOStatement_current(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP14c_PDOStatement9t_currentEv");

TypedValue* tg_12PDOStatement_current(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_12PDOStatement_current((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("PDOStatement::current", 0, 1);
      }
    } else {
      throw_instance_method_fatal("PDOStatement::current");
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
HPHP::Variant HPHP::c_PDOStatement::t_key()
_ZN4HPHP14c_PDOStatement5t_keyEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_12PDOStatement_key(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP14c_PDOStatement5t_keyEv");

TypedValue* tg_12PDOStatement_key(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_12PDOStatement_key((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("PDOStatement::key", 0, 1);
      }
    } else {
      throw_instance_method_fatal("PDOStatement::key");
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
HPHP::Variant HPHP::c_PDOStatement::t_next()
_ZN4HPHP14c_PDOStatement6t_nextEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_12PDOStatement_next(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP14c_PDOStatement6t_nextEv");

TypedValue* tg_12PDOStatement_next(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_12PDOStatement_next((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("PDOStatement::next", 0, 1);
      }
    } else {
      throw_instance_method_fatal("PDOStatement::next");
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
HPHP::Variant HPHP::c_PDOStatement::t_rewind()
_ZN4HPHP14c_PDOStatement8t_rewindEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_12PDOStatement_rewind(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP14c_PDOStatement8t_rewindEv");

TypedValue* tg_12PDOStatement_rewind(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_12PDOStatement_rewind((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("PDOStatement::rewind", 0, 1);
      }
    } else {
      throw_instance_method_fatal("PDOStatement::rewind");
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
HPHP::Variant HPHP::c_PDOStatement::t_valid()
_ZN4HPHP14c_PDOStatement7t_validEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_12PDOStatement_valid(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP14c_PDOStatement7t_validEv");

TypedValue* tg_12PDOStatement_valid(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_12PDOStatement_valid((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("PDOStatement::valid", 0, 1);
      }
    } else {
      throw_instance_method_fatal("PDOStatement::valid");
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
HPHP::Variant HPHP::c_PDOStatement::t___wakeup()
_ZN4HPHP14c_PDOStatement10t___wakeupEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_12PDOStatement___wakeup(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP14c_PDOStatement10t___wakeupEv");

TypedValue* tg_12PDOStatement___wakeup(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_12PDOStatement___wakeup((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("PDOStatement::__wakeup", 0, 1);
      }
    } else {
      throw_instance_method_fatal("PDOStatement::__wakeup");
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
HPHP::Variant HPHP::c_PDOStatement::t___sleep()
_ZN4HPHP14c_PDOStatement9t___sleepEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_12PDOStatement___sleep(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP14c_PDOStatement9t___sleepEv");

TypedValue* tg_12PDOStatement___sleep(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_12PDOStatement___sleep((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("PDOStatement::__sleep", 0, 1);
      }
    } else {
      throw_instance_method_fatal("PDOStatement::__sleep");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}


} // !HPHP

