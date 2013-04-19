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

Value* fh_pdo_drivers(Value* _rv) asm("_ZN4HPHP13f_pdo_driversEv");

TypedValue* fg_pdo_drivers(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfArray;
    fh_pdo_drivers(&(rv->m_data));
    if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("pdo_drivers", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}


HPHP::VM::Instance* new_PDO_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_PDO) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_PDO(cls);
  return inst;
}

IMPLEMENT_CLASS(PDO);
void th_3PDO___construct(ObjectData* this_, Value* dsn, Value* username, Value* password, Value* options) asm("_ZN4HPHP5c_PDO13t___constructERKNS_6StringES3_S3_RKNS_5ArrayE");

void tg1_3PDO___construct(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_3PDO___construct(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfNull;
  th_3PDO___construct((this_), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? &args[-2].m_data : (Value*)(&null_string), (count > 3) ? &args[-3].m_data : (Value*)(&null_array));
}

TypedValue* tg_3PDO___construct(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 1 && count <= 4) {
      if ((count <= 3 || (args - 3)->m_type == KindOfArray) &&
          (count <= 2 || IS_STRING_TYPE((args - 2)->m_type)) &&
          (count <= 1 || IS_STRING_TYPE((args - 1)->m_type)) &&
          IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfNull;
        th_3PDO___construct((this_), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? &args[-2].m_data : (Value*)(&null_string), (count > 3) ? &args[-3].m_data : (Value*)(&null_array));
      } else {
        tg1_3PDO___construct(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("PDO::__construct", count, 1, 4, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("PDO::__construct");
  }
  frame_free_locals_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_3PDO_setattribute(ObjectData* this_, long attribute, TypedValue* value) asm("_ZN4HPHP5c_PDO14t_setattributeElRKNS_7VariantE");

void tg1_3PDO_setattribute(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_3PDO_setattribute(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_3PDO_setattribute((this_), (long)(args[-0].m_data.num), (args-1))) ? 1LL : 0LL;
}

TypedValue* tg_3PDO_setattribute(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 2) {
      if ((args - 0)->m_type == KindOfInt64) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_3PDO_setattribute((this_), (long)(args[-0].m_data.num), (args-1))) ? 1LL : 0LL;
      } else {
        tg1_3PDO_setattribute(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("PDO::setattribute", count, 2, 2, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("PDO::setattribute");
  }
  frame_free_locals_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_3PDO_prepare(TypedValue* _rv, ObjectData* this_, Value* statement, Value* options) asm("_ZN4HPHP5c_PDO9t_prepareERKNS_6StringERKNS_5ArrayE");

void tg1_3PDO_prepare(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_3PDO_prepare(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
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
  th_3PDO_prepare(rv, (this_), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_array));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* tg_3PDO_prepare(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 1 && count <= 2) {
      if ((count <= 1 || (args - 1)->m_type == KindOfArray) &&
          IS_STRING_TYPE((args - 0)->m_type)) {
        th_3PDO_prepare(rv, (this_), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_array));
        if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
      } else {
        tg1_3PDO_prepare(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("PDO::prepare", count, 1, 2, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("PDO::prepare");
  }
  frame_free_locals_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_3PDO_begintransaction(ObjectData* this_) asm("_ZN4HPHP5c_PDO18t_begintransactionEv");

TypedValue* tg_3PDO_begintransaction(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (th_3PDO_begintransaction((this_))) ? 1LL : 0LL;
    } else {
      throw_toomany_arguments_nr("PDO::begintransaction", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("PDO::begintransaction");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_3PDO_commit(ObjectData* this_) asm("_ZN4HPHP5c_PDO8t_commitEv");

TypedValue* tg_3PDO_commit(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (th_3PDO_commit((this_))) ? 1LL : 0LL;
    } else {
      throw_toomany_arguments_nr("PDO::commit", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("PDO::commit");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_3PDO_rollback(ObjectData* this_) asm("_ZN4HPHP5c_PDO10t_rollbackEv");

TypedValue* tg_3PDO_rollback(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (th_3PDO_rollback((this_))) ? 1LL : 0LL;
    } else {
      throw_toomany_arguments_nr("PDO::rollback", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("PDO::rollback");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_3PDO_getattribute(TypedValue* _rv, ObjectData* this_, long attribute) asm("_ZN4HPHP5c_PDO14t_getattributeEl");

void tg1_3PDO_getattribute(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_3PDO_getattribute(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  th_3PDO_getattribute(rv, (this_), (long)(args[-0].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* tg_3PDO_getattribute(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if ((args - 0)->m_type == KindOfInt64) {
        th_3PDO_getattribute(rv, (this_), (long)(args[-0].m_data.num));
        if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
      } else {
        tg1_3PDO_getattribute(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("PDO::getattribute", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("PDO::getattribute");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_3PDO_exec(TypedValue* _rv, ObjectData* this_, Value* query) asm("_ZN4HPHP5c_PDO6t_execERKNS_6StringE");

void tg1_3PDO_exec(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_3PDO_exec(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  th_3PDO_exec(rv, (this_), &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* tg_3PDO_exec(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if (IS_STRING_TYPE((args - 0)->m_type)) {
        th_3PDO_exec(rv, (this_), &args[-0].m_data);
        if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
      } else {
        tg1_3PDO_exec(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("PDO::exec", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("PDO::exec");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_3PDO_lastinsertid(TypedValue* _rv, ObjectData* this_, Value* seqname) asm("_ZN4HPHP5c_PDO14t_lastinsertidERKNS_6StringE");

void tg1_3PDO_lastinsertid(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_3PDO_lastinsertid(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  th_3PDO_lastinsertid(rv, (this_), (count > 0) ? &args[-0].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* tg_3PDO_lastinsertid(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count <= 1) {
      if ((count <= 0 || IS_STRING_TYPE((args - 0)->m_type))) {
        th_3PDO_lastinsertid(rv, (this_), (count > 0) ? &args[-0].m_data : (Value*)(&null_string));
        if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
      } else {
        tg1_3PDO_lastinsertid(rv, ar, count, this_);
      }
    } else {
      throw_toomany_arguments_nr("PDO::lastinsertid", 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("PDO::lastinsertid");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_3PDO_errorcode(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP5c_PDO11t_errorcodeEv");

TypedValue* tg_3PDO_errorcode(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      th_3PDO_errorcode(rv, (this_));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      throw_toomany_arguments_nr("PDO::errorcode", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("PDO::errorcode");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* th_3PDO_errorinfo(Value* _rv, ObjectData* this_) asm("_ZN4HPHP5c_PDO11t_errorinfoEv");

TypedValue* tg_3PDO_errorinfo(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfArray;
      th_3PDO_errorinfo(&(rv->m_data), (this_));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      throw_toomany_arguments_nr("PDO::errorinfo", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("PDO::errorinfo");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_3PDO_query(TypedValue* _rv, ObjectData* this_, Value* sql) asm("_ZN4HPHP5c_PDO7t_queryERKNS_6StringE");

void tg1_3PDO_query(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_3PDO_query(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  th_3PDO_query(rv, (this_), &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* tg_3PDO_query(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if (IS_STRING_TYPE((args - 0)->m_type)) {
        th_3PDO_query(rv, (this_), &args[-0].m_data);
        if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
      } else {
        tg1_3PDO_query(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("PDO::query", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("PDO::query");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_3PDO_quote(TypedValue* _rv, ObjectData* this_, Value* str, long paramtype) asm("_ZN4HPHP5c_PDO7t_quoteERKNS_6StringEl");

void tg1_3PDO_quote(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_3PDO_quote(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
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
  th_3PDO_quote(rv, (this_), &args[-0].m_data, (count > 1) ? (long)(args[-1].m_data.num) : (long)(q_PDO$$PARAM_STR));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* tg_3PDO_quote(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 1 && count <= 2) {
      if ((count <= 1 || (args - 1)->m_type == KindOfInt64) &&
          IS_STRING_TYPE((args - 0)->m_type)) {
        th_3PDO_quote(rv, (this_), &args[-0].m_data, (count > 1) ? (long)(args[-1].m_data.num) : (long)(q_PDO$$PARAM_STR));
        if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
      } else {
        tg1_3PDO_quote(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("PDO::quote", count, 1, 2, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("PDO::quote");
  }
  frame_free_locals_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_3PDO___wakeup(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP5c_PDO10t___wakeupEv");

TypedValue* tg_3PDO___wakeup(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      th_3PDO___wakeup(rv, (this_));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      throw_toomany_arguments_nr("PDO::__wakeup", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("PDO::__wakeup");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_3PDO___sleep(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP5c_PDO9t___sleepEv");

TypedValue* tg_3PDO___sleep(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      th_3PDO___sleep(rv, (this_));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      throw_toomany_arguments_nr("PDO::__sleep", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("PDO::__sleep");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* th_3PDO_getavailabledrivers(Value* _rv, char const* cls_) asm("_ZN4HPHP5c_PDO22ti_getavailabledriversEPKc");

TypedValue* tg_3PDO_getavailabledrivers(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (count == 0) {
    rv->m_type = KindOfArray;
    th_3PDO_getavailabledrivers(&(rv->m_data), "PDO");
    if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("PDO::getavailabledrivers", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}


HPHP::VM::Instance* new_PDOStatement_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_PDOStatement) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_PDOStatement(cls);
  return inst;
}

IMPLEMENT_CLASS(PDOStatement);
void th_12PDOStatement___construct(ObjectData* this_) asm("_ZN4HPHP14c_PDOStatement13t___constructEv");

TypedValue* tg_12PDOStatement___construct(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfNull;
      th_12PDOStatement___construct((this_));
    } else {
      throw_toomany_arguments_nr("PDOStatement::__construct", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("PDOStatement::__construct");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_12PDOStatement_execute(TypedValue* _rv, ObjectData* this_, Value* params) asm("_ZN4HPHP14c_PDOStatement9t_executeERKNS_5ArrayE");

void tg1_12PDOStatement_execute(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_12PDOStatement_execute(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToArrayInPlace(args-0);
  th_12PDOStatement_execute(rv, (this_), (count > 0) ? &args[-0].m_data : (Value*)(&null_array));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* tg_12PDOStatement_execute(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count <= 1) {
      if ((count <= 0 || (args - 0)->m_type == KindOfArray)) {
        th_12PDOStatement_execute(rv, (this_), (count > 0) ? &args[-0].m_data : (Value*)(&null_array));
        if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
      } else {
        tg1_12PDOStatement_execute(rv, ar, count, this_);
      }
    } else {
      throw_toomany_arguments_nr("PDOStatement::execute", 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("PDOStatement::execute");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_12PDOStatement_fetch(TypedValue* _rv, ObjectData* this_, long how, long orientation, long offset) asm("_ZN4HPHP14c_PDOStatement7t_fetchElll");

void tg1_12PDOStatement_fetch(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_12PDOStatement_fetch(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
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
  th_12PDOStatement_fetch(rv, (this_), (count > 0) ? (long)(args[-0].m_data.num) : (long)(0), (count > 1) ? (long)(args[-1].m_data.num) : (long)(q_PDO$$FETCH_ORI_NEXT), (count > 2) ? (long)(args[-2].m_data.num) : (long)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* tg_12PDOStatement_fetch(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count <= 3) {
      if ((count <= 2 || (args - 2)->m_type == KindOfInt64) &&
          (count <= 1 || (args - 1)->m_type == KindOfInt64) &&
          (count <= 0 || (args - 0)->m_type == KindOfInt64)) {
        th_12PDOStatement_fetch(rv, (this_), (count > 0) ? (long)(args[-0].m_data.num) : (long)(0), (count > 1) ? (long)(args[-1].m_data.num) : (long)(q_PDO$$FETCH_ORI_NEXT), (count > 2) ? (long)(args[-2].m_data.num) : (long)(0));
        if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
      } else {
        tg1_12PDOStatement_fetch(rv, ar, count, this_);
      }
    } else {
      throw_toomany_arguments_nr("PDOStatement::fetch", 3, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("PDOStatement::fetch");
  }
  frame_free_locals_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_12PDOStatement_fetchobject(TypedValue* _rv, ObjectData* this_, Value* class_name, TypedValue* ctor_args) asm("_ZN4HPHP14c_PDOStatement13t_fetchobjectERKNS_6StringERKNS_7VariantE");

void tg1_12PDOStatement_fetchobject(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_12PDOStatement_fetchobject(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  Variant defVal1;
  th_12PDOStatement_fetchobject(rv, (this_), (count > 0) ? &args[-0].m_data : (Value*)(&null_string), (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* tg_12PDOStatement_fetchobject(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count <= 2) {
      if ((count <= 0 || IS_STRING_TYPE((args - 0)->m_type))) {
        Variant defVal1;
        th_12PDOStatement_fetchobject(rv, (this_), (count > 0) ? &args[-0].m_data : (Value*)(&null_string), (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
        if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
      } else {
        tg1_12PDOStatement_fetchobject(rv, ar, count, this_);
      }
    } else {
      throw_toomany_arguments_nr("PDOStatement::fetchobject", 2, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("PDOStatement::fetchobject");
  }
  frame_free_locals_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_12PDOStatement_fetchcolumn(TypedValue* _rv, ObjectData* this_, long column_numner) asm("_ZN4HPHP14c_PDOStatement13t_fetchcolumnEl");

void tg1_12PDOStatement_fetchcolumn(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_12PDOStatement_fetchcolumn(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  th_12PDOStatement_fetchcolumn(rv, (this_), (count > 0) ? (long)(args[-0].m_data.num) : (long)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* tg_12PDOStatement_fetchcolumn(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count <= 1) {
      if ((count <= 0 || (args - 0)->m_type == KindOfInt64)) {
        th_12PDOStatement_fetchcolumn(rv, (this_), (count > 0) ? (long)(args[-0].m_data.num) : (long)(0));
        if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
      } else {
        tg1_12PDOStatement_fetchcolumn(rv, ar, count, this_);
      }
    } else {
      throw_toomany_arguments_nr("PDOStatement::fetchcolumn", 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("PDOStatement::fetchcolumn");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_12PDOStatement_fetchall(TypedValue* _rv, ObjectData* this_, long how, TypedValue* class_name, TypedValue* ctor_args) asm("_ZN4HPHP14c_PDOStatement10t_fetchallElRKNS_7VariantES3_");

void tg1_12PDOStatement_fetchall(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_12PDOStatement_fetchall(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  Variant defVal1;
  Variant defVal2;
  th_12PDOStatement_fetchall(rv, (this_), (count > 0) ? (long)(args[-0].m_data.num) : (long)(0), (count > 1) ? (args-1) : (TypedValue*)(&defVal1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* tg_12PDOStatement_fetchall(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count <= 3) {
      if ((count <= 0 || (args - 0)->m_type == KindOfInt64)) {
        Variant defVal1;
        Variant defVal2;
        th_12PDOStatement_fetchall(rv, (this_), (count > 0) ? (long)(args[-0].m_data.num) : (long)(0), (count > 1) ? (args-1) : (TypedValue*)(&defVal1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
        if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
      } else {
        tg1_12PDOStatement_fetchall(rv, ar, count, this_);
      }
    } else {
      throw_toomany_arguments_nr("PDOStatement::fetchall", 3, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("PDOStatement::fetchall");
  }
  frame_free_locals_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_12PDOStatement_bindvalue(ObjectData* this_, TypedValue* paramno, TypedValue* param, long type) asm("_ZN4HPHP14c_PDOStatement11t_bindvalueERKNS_7VariantES3_l");

void tg1_12PDOStatement_bindvalue(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_12PDOStatement_bindvalue(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-2);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_12PDOStatement_bindvalue((this_), (args-0), (args-1), (count > 2) ? (long)(args[-2].m_data.num) : (long)(q_PDO$$PARAM_STR))) ? 1LL : 0LL;
}

TypedValue* tg_12PDOStatement_bindvalue(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 2 && count <= 3) {
      if ((count <= 2 || (args - 2)->m_type == KindOfInt64)) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_12PDOStatement_bindvalue((this_), (args-0), (args-1), (count > 2) ? (long)(args[-2].m_data.num) : (long)(q_PDO$$PARAM_STR))) ? 1LL : 0LL;
      } else {
        tg1_12PDOStatement_bindvalue(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("PDOStatement::bindvalue", count, 2, 3, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("PDOStatement::bindvalue");
  }
  frame_free_locals_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_12PDOStatement_bindparam(ObjectData* this_, TypedValue* paramno, TypedValue* param, long type, long max_value_len, TypedValue* driver_params) asm("_ZN4HPHP14c_PDOStatement11t_bindparamERKNS_7VariantERKNS_14VRefParamValueEllS3_");

void tg1_12PDOStatement_bindparam(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_12PDOStatement_bindparam(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfBoolean;
  Variant defVal4;
  rv->m_data.num = (th_12PDOStatement_bindparam((this_), (args-0), (args-1), (count > 2) ? (long)(args[-2].m_data.num) : (long)(q_PDO$$PARAM_STR), (count > 3) ? (long)(args[-3].m_data.num) : (long)(0), (count > 4) ? (args-4) : (TypedValue*)(&defVal4))) ? 1LL : 0LL;
}

TypedValue* tg_12PDOStatement_bindparam(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 2 && count <= 5) {
      if ((count <= 3 || (args - 3)->m_type == KindOfInt64) &&
          (count <= 2 || (args - 2)->m_type == KindOfInt64)) {
        rv->m_type = KindOfBoolean;
        Variant defVal4;
        rv->m_data.num = (th_12PDOStatement_bindparam((this_), (args-0), (args-1), (count > 2) ? (long)(args[-2].m_data.num) : (long)(q_PDO$$PARAM_STR), (count > 3) ? (long)(args[-3].m_data.num) : (long)(0), (count > 4) ? (args-4) : (TypedValue*)(&defVal4))) ? 1LL : 0LL;
      } else {
        tg1_12PDOStatement_bindparam(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("PDOStatement::bindparam", count, 2, 5, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("PDOStatement::bindparam");
  }
  frame_free_locals_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_12PDOStatement_bindcolumn(ObjectData* this_, TypedValue* paramno, TypedValue* param, long type, long max_value_len, TypedValue* driver_params) asm("_ZN4HPHP14c_PDOStatement12t_bindcolumnERKNS_7VariantERKNS_14VRefParamValueEllS3_");

void tg1_12PDOStatement_bindcolumn(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_12PDOStatement_bindcolumn(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfBoolean;
  Variant defVal4;
  rv->m_data.num = (th_12PDOStatement_bindcolumn((this_), (args-0), (args-1), (count > 2) ? (long)(args[-2].m_data.num) : (long)(q_PDO$$PARAM_STR), (count > 3) ? (long)(args[-3].m_data.num) : (long)(0), (count > 4) ? (args-4) : (TypedValue*)(&defVal4))) ? 1LL : 0LL;
}

TypedValue* tg_12PDOStatement_bindcolumn(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 2 && count <= 5) {
      if ((count <= 3 || (args - 3)->m_type == KindOfInt64) &&
          (count <= 2 || (args - 2)->m_type == KindOfInt64)) {
        rv->m_type = KindOfBoolean;
        Variant defVal4;
        rv->m_data.num = (th_12PDOStatement_bindcolumn((this_), (args-0), (args-1), (count > 2) ? (long)(args[-2].m_data.num) : (long)(q_PDO$$PARAM_STR), (count > 3) ? (long)(args[-3].m_data.num) : (long)(0), (count > 4) ? (args-4) : (TypedValue*)(&defVal4))) ? 1LL : 0LL;
      } else {
        tg1_12PDOStatement_bindcolumn(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("PDOStatement::bindcolumn", count, 2, 5, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("PDOStatement::bindcolumn");
  }
  frame_free_locals_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long th_12PDOStatement_rowcount(ObjectData* this_) asm("_ZN4HPHP14c_PDOStatement10t_rowcountEv");

TypedValue* tg_12PDOStatement_rowcount(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)th_12PDOStatement_rowcount((this_));
    } else {
      throw_toomany_arguments_nr("PDOStatement::rowcount", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("PDOStatement::rowcount");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_12PDOStatement_errorcode(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP14c_PDOStatement11t_errorcodeEv");

TypedValue* tg_12PDOStatement_errorcode(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      th_12PDOStatement_errorcode(rv, (this_));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      throw_toomany_arguments_nr("PDOStatement::errorcode", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("PDOStatement::errorcode");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* th_12PDOStatement_errorinfo(Value* _rv, ObjectData* this_) asm("_ZN4HPHP14c_PDOStatement11t_errorinfoEv");

TypedValue* tg_12PDOStatement_errorinfo(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfArray;
      th_12PDOStatement_errorinfo(&(rv->m_data), (this_));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      throw_toomany_arguments_nr("PDOStatement::errorinfo", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("PDOStatement::errorinfo");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_12PDOStatement_setattribute(TypedValue* _rv, ObjectData* this_, long attribute, TypedValue* value) asm("_ZN4HPHP14c_PDOStatement14t_setattributeElRKNS_7VariantE");

void tg1_12PDOStatement_setattribute(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_12PDOStatement_setattribute(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  th_12PDOStatement_setattribute(rv, (this_), (long)(args[-0].m_data.num), (args-1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* tg_12PDOStatement_setattribute(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 2) {
      if ((args - 0)->m_type == KindOfInt64) {
        th_12PDOStatement_setattribute(rv, (this_), (long)(args[-0].m_data.num), (args-1));
        if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
      } else {
        tg1_12PDOStatement_setattribute(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("PDOStatement::setattribute", count, 2, 2, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("PDOStatement::setattribute");
  }
  frame_free_locals_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_12PDOStatement_getattribute(TypedValue* _rv, ObjectData* this_, long attribute) asm("_ZN4HPHP14c_PDOStatement14t_getattributeEl");

void tg1_12PDOStatement_getattribute(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_12PDOStatement_getattribute(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  th_12PDOStatement_getattribute(rv, (this_), (long)(args[-0].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* tg_12PDOStatement_getattribute(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if ((args - 0)->m_type == KindOfInt64) {
        th_12PDOStatement_getattribute(rv, (this_), (long)(args[-0].m_data.num));
        if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
      } else {
        tg1_12PDOStatement_getattribute(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("PDOStatement::getattribute", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("PDOStatement::getattribute");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long th_12PDOStatement_columncount(ObjectData* this_) asm("_ZN4HPHP14c_PDOStatement13t_columncountEv");

TypedValue* tg_12PDOStatement_columncount(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)th_12PDOStatement_columncount((this_));
    } else {
      throw_toomany_arguments_nr("PDOStatement::columncount", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("PDOStatement::columncount");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_12PDOStatement_getcolumnmeta(TypedValue* _rv, ObjectData* this_, long column) asm("_ZN4HPHP14c_PDOStatement15t_getcolumnmetaEl");

void tg1_12PDOStatement_getcolumnmeta(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_12PDOStatement_getcolumnmeta(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  th_12PDOStatement_getcolumnmeta(rv, (this_), (long)(args[-0].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* tg_12PDOStatement_getcolumnmeta(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if ((args - 0)->m_type == KindOfInt64) {
        th_12PDOStatement_getcolumnmeta(rv, (this_), (long)(args[-0].m_data.num));
        if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
      } else {
        tg1_12PDOStatement_getcolumnmeta(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("PDOStatement::getcolumnmeta", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("PDOStatement::getcolumnmeta");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_12PDOStatement_setfetchmode(ObjectData* this_, int64_t _argc, long mode, Value* _argv) asm("_ZN4HPHP14c_PDOStatement14t_setfetchmodeEilRKNS_5ArrayE");

void tg1_12PDOStatement_setfetchmode(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_12PDOStatement_setfetchmode(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  rv->m_type = KindOfBoolean;

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
  rv->m_data.num = (th_12PDOStatement_setfetchmode((this_), count, (long)(args[-0].m_data.num), (Value*)(&extraArgs))) ? 1LL : 0LL;
}

TypedValue* tg_12PDOStatement_setfetchmode(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 1) {
      if ((args - 0)->m_type == KindOfInt64) {
        rv->m_type = KindOfBoolean;

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
        rv->m_data.num = (th_12PDOStatement_setfetchmode((this_), count, (long)(args[-0].m_data.num), (Value*)(&extraArgs))) ? 1LL : 0LL;
      } else {
        tg1_12PDOStatement_setfetchmode(rv, ar, count, this_);
      }
    } else {
      throw_missing_arguments_nr("PDOStatement::setfetchmode", 1, count, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("PDOStatement::setfetchmode");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_12PDOStatement_nextrowset(ObjectData* this_) asm("_ZN4HPHP14c_PDOStatement12t_nextrowsetEv");

TypedValue* tg_12PDOStatement_nextrowset(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (th_12PDOStatement_nextrowset((this_))) ? 1LL : 0LL;
    } else {
      throw_toomany_arguments_nr("PDOStatement::nextrowset", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("PDOStatement::nextrowset");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_12PDOStatement_closecursor(ObjectData* this_) asm("_ZN4HPHP14c_PDOStatement13t_closecursorEv");

TypedValue* tg_12PDOStatement_closecursor(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (th_12PDOStatement_closecursor((this_))) ? 1LL : 0LL;
    } else {
      throw_toomany_arguments_nr("PDOStatement::closecursor", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("PDOStatement::closecursor");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_12PDOStatement_debugdumpparams(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP14c_PDOStatement17t_debugdumpparamsEv");

TypedValue* tg_12PDOStatement_debugdumpparams(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      th_12PDOStatement_debugdumpparams(rv, (this_));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      throw_toomany_arguments_nr("PDOStatement::debugdumpparams", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("PDOStatement::debugdumpparams");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_12PDOStatement_current(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP14c_PDOStatement9t_currentEv");

TypedValue* tg_12PDOStatement_current(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      th_12PDOStatement_current(rv, (this_));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      throw_toomany_arguments_nr("PDOStatement::current", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("PDOStatement::current");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_12PDOStatement_key(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP14c_PDOStatement5t_keyEv");

TypedValue* tg_12PDOStatement_key(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      th_12PDOStatement_key(rv, (this_));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      throw_toomany_arguments_nr("PDOStatement::key", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("PDOStatement::key");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_12PDOStatement_next(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP14c_PDOStatement6t_nextEv");

TypedValue* tg_12PDOStatement_next(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      th_12PDOStatement_next(rv, (this_));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      throw_toomany_arguments_nr("PDOStatement::next", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("PDOStatement::next");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_12PDOStatement_rewind(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP14c_PDOStatement8t_rewindEv");

TypedValue* tg_12PDOStatement_rewind(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      th_12PDOStatement_rewind(rv, (this_));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      throw_toomany_arguments_nr("PDOStatement::rewind", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("PDOStatement::rewind");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_12PDOStatement_valid(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP14c_PDOStatement7t_validEv");

TypedValue* tg_12PDOStatement_valid(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      th_12PDOStatement_valid(rv, (this_));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      throw_toomany_arguments_nr("PDOStatement::valid", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("PDOStatement::valid");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_12PDOStatement___wakeup(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP14c_PDOStatement10t___wakeupEv");

TypedValue* tg_12PDOStatement___wakeup(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      th_12PDOStatement___wakeup(rv, (this_));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      throw_toomany_arguments_nr("PDOStatement::__wakeup", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("PDOStatement::__wakeup");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_12PDOStatement___sleep(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP14c_PDOStatement9t___sleepEv");

TypedValue* tg_12PDOStatement___sleep(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      th_12PDOStatement___sleep(rv, (this_));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      throw_toomany_arguments_nr("PDOStatement::__sleep", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("PDOStatement::__sleep");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

} // namespace HPHP
