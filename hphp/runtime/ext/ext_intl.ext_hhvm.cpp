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

long fh_intl_get_error_code() asm("_ZN4HPHP21f_intl_get_error_codeEv");

TypedValue* fg_intl_get_error_code(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfInt64;
    rv->m_data.num = (int64_t)fh_intl_get_error_code();
  } else {
    throw_toomany_arguments_nr("intl_get_error_code", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_intl_get_error_message(Value* _rv) asm("_ZN4HPHP24f_intl_get_error_messageEv");

TypedValue* fg_intl_get_error_message(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfString;
    fh_intl_get_error_message(&(rv->m_data));
    if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("intl_get_error_message", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_intl_error_name(Value* _rv, long error_code) asm("_ZN4HPHP17f_intl_error_nameEl");

void fg1_intl_error_name(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_intl_error_name(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  rv->m_type = KindOfString;
  fh_intl_error_name(&(rv->m_data), (long)(args[-0].m_data.num));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_intl_error_name(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfInt64) {
      rv->m_type = KindOfString;
      fh_intl_error_name(&(rv->m_data), (long)(args[-0].m_data.num));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_intl_error_name(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("intl_error_name", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_intl_is_failure(long error_code) asm("_ZN4HPHP17f_intl_is_failureEl");

void fg1_intl_is_failure(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_intl_is_failure(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_intl_is_failure((long)(args[-0].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_intl_is_failure(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfInt64) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_intl_is_failure((long)(args[-0].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_intl_is_failure(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("intl_is_failure", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}


HPHP::VM::Instance* new_Collator_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_Collator) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_Collator(cls);
  return inst;
}

IMPLEMENT_CLASS(Collator);
void th_8Collator___construct(ObjectData* this_, Value* locale) asm("_ZN4HPHP10c_Collator13t___constructERKNS_6StringE");

void tg1_8Collator___construct(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_8Collator___construct(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfNull;
  th_8Collator___construct((this_), &args[-0].m_data);
}

TypedValue* tg_8Collator___construct(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if (IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfNull;
        th_8Collator___construct((this_), &args[-0].m_data);
      } else {
        tg1_8Collator___construct(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("Collator::__construct", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("Collator::__construct");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_8Collator_asort(ObjectData* this_, TypedValue* arr, long sort_flag) asm("_ZN4HPHP10c_Collator7t_asortERKNS_14VRefParamValueEl");

void tg1_8Collator_asort(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_8Collator_asort(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-1);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_8Collator_asort((this_), (args-0), (count > 1) ? (long)(args[-1].m_data.num) : (long)(q_Collator$$SORT_REGULAR))) ? 1LL : 0LL;
}

TypedValue* tg_8Collator_asort(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 1 && count <= 2) {
      if ((count <= 1 || (args - 1)->m_type == KindOfInt64)) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_8Collator_asort((this_), (args-0), (count > 1) ? (long)(args[-1].m_data.num) : (long)(q_Collator$$SORT_REGULAR))) ? 1LL : 0LL;
      } else {
        tg1_8Collator_asort(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("Collator::asort", count, 1, 2, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("Collator::asort");
  }
  frame_free_locals_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_8Collator_compare(TypedValue* _rv, ObjectData* this_, Value* str1, Value* str2) asm("_ZN4HPHP10c_Collator9t_compareERKNS_6StringES3_");

void tg1_8Collator_compare(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_8Collator_compare(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  th_8Collator_compare(rv, (this_), &args[-0].m_data, &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* tg_8Collator_compare(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 2) {
      if (IS_STRING_TYPE((args - 1)->m_type) &&
          IS_STRING_TYPE((args - 0)->m_type)) {
        th_8Collator_compare(rv, (this_), &args[-0].m_data, &args[-1].m_data);
        if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
      } else {
        tg1_8Collator_compare(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("Collator::compare", count, 2, 2, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("Collator::compare");
  }
  frame_free_locals_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_8Collator_create(TypedValue* _rv, char const* cls_, Value* locale) asm("_ZN4HPHP10c_Collator9ti_createEPKcRKNS_6StringE");

void tg1_8Collator_create(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void tg1_8Collator_create(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  th_8Collator_create(rv, "Collator", &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* tg_8Collator_create(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      th_8Collator_create(rv, "Collator", &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      tg1_8Collator_create(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("Collator::create", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long th_8Collator_getattribute(ObjectData* this_, long attr) asm("_ZN4HPHP10c_Collator14t_getattributeEl");

void tg1_8Collator_getattribute(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_8Collator_getattribute(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)th_8Collator_getattribute((this_), (long)(args[-0].m_data.num));
}

TypedValue* tg_8Collator_getattribute(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if ((args - 0)->m_type == KindOfInt64) {
        rv->m_type = KindOfInt64;
        rv->m_data.num = (int64_t)th_8Collator_getattribute((this_), (long)(args[-0].m_data.num));
      } else {
        tg1_8Collator_getattribute(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("Collator::getattribute", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("Collator::getattribute");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long th_8Collator_geterrorcode(ObjectData* this_) asm("_ZN4HPHP10c_Collator14t_geterrorcodeEv");

TypedValue* tg_8Collator_geterrorcode(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)th_8Collator_geterrorcode((this_));
    } else {
      throw_toomany_arguments_nr("Collator::geterrorcode", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("Collator::geterrorcode");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* th_8Collator_geterrormessage(Value* _rv, ObjectData* this_) asm("_ZN4HPHP10c_Collator17t_geterrormessageEv");

TypedValue* tg_8Collator_geterrormessage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfString;
      th_8Collator_geterrormessage(&(rv->m_data), (this_));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      throw_toomany_arguments_nr("Collator::geterrormessage", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("Collator::geterrormessage");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* th_8Collator_getlocale(Value* _rv, ObjectData* this_, long type) asm("_ZN4HPHP10c_Collator11t_getlocaleEl");

void tg1_8Collator_getlocale(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_8Collator_getlocale(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  rv->m_type = KindOfString;
  th_8Collator_getlocale(&(rv->m_data), (this_), (count > 0) ? (long)(args[-0].m_data.num) : (long)(0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* tg_8Collator_getlocale(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count <= 1) {
      if ((count <= 0 || (args - 0)->m_type == KindOfInt64)) {
        rv->m_type = KindOfString;
        th_8Collator_getlocale(&(rv->m_data), (this_), (count > 0) ? (long)(args[-0].m_data.num) : (long)(0));
        if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
      } else {
        tg1_8Collator_getlocale(rv, ar, count, this_);
      }
    } else {
      throw_toomany_arguments_nr("Collator::getlocale", 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("Collator::getlocale");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long th_8Collator_getstrength(ObjectData* this_) asm("_ZN4HPHP10c_Collator13t_getstrengthEv");

TypedValue* tg_8Collator_getstrength(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)th_8Collator_getstrength((this_));
    } else {
      throw_toomany_arguments_nr("Collator::getstrength", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("Collator::getstrength");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_8Collator_setattribute(ObjectData* this_, long attr, long val) asm("_ZN4HPHP10c_Collator14t_setattributeEll");

void tg1_8Collator_setattribute(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_8Collator_setattribute(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_8Collator_setattribute((this_), (long)(args[-0].m_data.num), (long)(args[-1].m_data.num))) ? 1LL : 0LL;
}

TypedValue* tg_8Collator_setattribute(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 2) {
      if ((args - 1)->m_type == KindOfInt64 &&
          (args - 0)->m_type == KindOfInt64) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_8Collator_setattribute((this_), (long)(args[-0].m_data.num), (long)(args[-1].m_data.num))) ? 1LL : 0LL;
      } else {
        tg1_8Collator_setattribute(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("Collator::setattribute", count, 2, 2, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("Collator::setattribute");
  }
  frame_free_locals_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_8Collator_setstrength(ObjectData* this_, long strength) asm("_ZN4HPHP10c_Collator13t_setstrengthEl");

void tg1_8Collator_setstrength(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_8Collator_setstrength(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_8Collator_setstrength((this_), (long)(args[-0].m_data.num))) ? 1LL : 0LL;
}

TypedValue* tg_8Collator_setstrength(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if ((args - 0)->m_type == KindOfInt64) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_8Collator_setstrength((this_), (long)(args[-0].m_data.num))) ? 1LL : 0LL;
      } else {
        tg1_8Collator_setstrength(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("Collator::setstrength", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("Collator::setstrength");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_8Collator_sortwithsortkeys(ObjectData* this_, TypedValue* arr) asm("_ZN4HPHP10c_Collator18t_sortwithsortkeysERKNS_14VRefParamValueE");

TypedValue* tg_8Collator_sortwithsortkeys(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (th_8Collator_sortwithsortkeys((this_), (args-0))) ? 1LL : 0LL;
    } else {
      throw_wrong_arguments_nr("Collator::sortwithsortkeys", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("Collator::sortwithsortkeys");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_8Collator_sort(ObjectData* this_, TypedValue* arr, long sort_flag) asm("_ZN4HPHP10c_Collator6t_sortERKNS_14VRefParamValueEl");

void tg1_8Collator_sort(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_8Collator_sort(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-1);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_8Collator_sort((this_), (args-0), (count > 1) ? (long)(args[-1].m_data.num) : (long)(q_Collator$$SORT_REGULAR))) ? 1LL : 0LL;
}

TypedValue* tg_8Collator_sort(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 1 && count <= 2) {
      if ((count <= 1 || (args - 1)->m_type == KindOfInt64)) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_8Collator_sort((this_), (args-0), (count > 1) ? (long)(args[-1].m_data.num) : (long)(q_Collator$$SORT_REGULAR))) ? 1LL : 0LL;
      } else {
        tg1_8Collator_sort(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("Collator::sort", count, 1, 2, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("Collator::sort");
  }
  frame_free_locals_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_collator_asort(TypedValue* _rv, TypedValue* obj, TypedValue* arr, long sort_flag) asm("_ZN4HPHP16f_collator_asortERKNS_7VariantERKNS_14VRefParamValueEl");

void fg1_collator_asort(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_collator_asort(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-2);
  fh_collator_asort(rv, (args-0), (args-1), (count > 2) ? (long)(args[-2].m_data.num) : (long)(q_Collator$$SORT_REGULAR));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_collator_asort(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfInt64)) {
      fh_collator_asort(rv, (args-0), (args-1), (count > 2) ? (long)(args[-2].m_data.num) : (long)(q_Collator$$SORT_REGULAR));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_collator_asort(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("collator_asort", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_collator_compare(TypedValue* _rv, TypedValue* obj, Value* str1, Value* str2) asm("_ZN4HPHP18f_collator_compareERKNS_7VariantERKNS_6StringES5_");

void fg1_collator_compare(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_collator_compare(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  fh_collator_compare(rv, (args-0), &args[-1].m_data, &args[-2].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_collator_compare(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if (IS_STRING_TYPE((args - 2)->m_type) &&
        IS_STRING_TYPE((args - 1)->m_type)) {
      fh_collator_compare(rv, (args-0), &args[-1].m_data, &args[-2].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_collator_compare(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("collator_compare", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_collator_create(TypedValue* _rv, Value* locale) asm("_ZN4HPHP17f_collator_createERKNS_6StringE");

void fg1_collator_create(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_collator_create(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_collator_create(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_collator_create(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_collator_create(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_collator_create(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("collator_create", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_collator_get_attribute(TypedValue* _rv, TypedValue* obj, long attr) asm("_ZN4HPHP24f_collator_get_attributeERKNS_7VariantEl");

void fg1_collator_get_attribute(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_collator_get_attribute(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-1);
  fh_collator_get_attribute(rv, (args-0), (long)(args[-1].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_collator_get_attribute(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64) {
      fh_collator_get_attribute(rv, (args-0), (long)(args[-1].m_data.num));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_collator_get_attribute(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("collator_get_attribute", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_collator_get_error_code(TypedValue* _rv, TypedValue* obj) asm("_ZN4HPHP25f_collator_get_error_codeERKNS_7VariantE");

TypedValue* fg_collator_get_error_code(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    fh_collator_get_error_code(rv, (args-0));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_wrong_arguments_nr("collator_get_error_code", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_collator_get_error_message(TypedValue* _rv, TypedValue* obj) asm("_ZN4HPHP28f_collator_get_error_messageERKNS_7VariantE");

TypedValue* fg_collator_get_error_message(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    fh_collator_get_error_message(rv, (args-0));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_wrong_arguments_nr("collator_get_error_message", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_collator_get_locale(TypedValue* _rv, TypedValue* obj, long type) asm("_ZN4HPHP21f_collator_get_localeERKNS_7VariantEl");

void fg1_collator_get_locale(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_collator_get_locale(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-1);
  fh_collator_get_locale(rv, (args-0), (count > 1) ? (long)(args[-1].m_data.num) : (long)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_collator_get_locale(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfInt64)) {
      fh_collator_get_locale(rv, (args-0), (count > 1) ? (long)(args[-1].m_data.num) : (long)(0));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_collator_get_locale(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("collator_get_locale", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_collator_get_strength(TypedValue* _rv, TypedValue* obj) asm("_ZN4HPHP23f_collator_get_strengthERKNS_7VariantE");

TypedValue* fg_collator_get_strength(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    fh_collator_get_strength(rv, (args-0));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_wrong_arguments_nr("collator_get_strength", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_collator_set_attribute(TypedValue* _rv, TypedValue* obj, long attr, long val) asm("_ZN4HPHP24f_collator_set_attributeERKNS_7VariantEll");

void fg1_collator_set_attribute(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_collator_set_attribute(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  fh_collator_set_attribute(rv, (args-0), (long)(args[-1].m_data.num), (long)(args[-2].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_collator_set_attribute(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if ((args - 2)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfInt64) {
      fh_collator_set_attribute(rv, (args-0), (long)(args[-1].m_data.num), (long)(args[-2].m_data.num));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_collator_set_attribute(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("collator_set_attribute", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_collator_set_strength(TypedValue* _rv, TypedValue* obj, long strength) asm("_ZN4HPHP23f_collator_set_strengthERKNS_7VariantEl");

void fg1_collator_set_strength(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_collator_set_strength(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-1);
  fh_collator_set_strength(rv, (args-0), (long)(args[-1].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_collator_set_strength(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64) {
      fh_collator_set_strength(rv, (args-0), (long)(args[-1].m_data.num));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_collator_set_strength(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("collator_set_strength", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_collator_sort_with_sort_keys(TypedValue* _rv, TypedValue* obj, TypedValue* arr) asm("_ZN4HPHP30f_collator_sort_with_sort_keysERKNS_7VariantERKNS_14VRefParamValueE");

TypedValue* fg_collator_sort_with_sort_keys(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    fh_collator_sort_with_sort_keys(rv, (args-0), (args-1));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_wrong_arguments_nr("collator_sort_with_sort_keys", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_collator_sort(TypedValue* _rv, TypedValue* obj, TypedValue* arr, long sort_flag) asm("_ZN4HPHP15f_collator_sortERKNS_7VariantERKNS_14VRefParamValueEl");

void fg1_collator_sort(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_collator_sort(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-2);
  fh_collator_sort(rv, (args-0), (args-1), (count > 2) ? (long)(args[-2].m_data.num) : (long)(q_Collator$$SORT_REGULAR));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_collator_sort(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfInt64)) {
      fh_collator_sort(rv, (args-0), (args-1), (count > 2) ? (long)(args[-2].m_data.num) : (long)(q_Collator$$SORT_REGULAR));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_collator_sort(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("collator_sort", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}


HPHP::VM::Instance* new_Locale_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_Locale) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_Locale(cls);
  return inst;
}

IMPLEMENT_CLASS(Locale);
void th_6Locale___construct(ObjectData* this_) asm("_ZN4HPHP8c_Locale13t___constructEv");

TypedValue* tg_6Locale___construct(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfNull;
      th_6Locale___construct((this_));
    } else {
      throw_toomany_arguments_nr("Locale::__construct", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("Locale::__construct");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}


HPHP::VM::Instance* new_Normalizer_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_Normalizer) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_Normalizer(cls);
  return inst;
}

IMPLEMENT_CLASS(Normalizer);
void th_10Normalizer___construct(ObjectData* this_) asm("_ZN4HPHP12c_Normalizer13t___constructEv");

TypedValue* tg_10Normalizer___construct(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfNull;
      th_10Normalizer___construct((this_));
    } else {
      throw_toomany_arguments_nr("Normalizer::__construct", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("Normalizer::__construct");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_10Normalizer_isnormalized(TypedValue* _rv, char const* cls_, Value* input, long form) asm("_ZN4HPHP12c_Normalizer15ti_isnormalizedEPKcRKNS_6StringEl");

void tg1_10Normalizer_isnormalized(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void tg1_10Normalizer_isnormalized(TypedValue* rv, ActRec* ar, int32_t count) {
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
  th_10Normalizer_isnormalized(rv, "Normalizer", &args[-0].m_data, (count > 1) ? (long)(args[-1].m_data.num) : (long)(q_Normalizer$$FORM_C));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* tg_10Normalizer_isnormalized(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      th_10Normalizer_isnormalized(rv, "Normalizer", &args[-0].m_data, (count > 1) ? (long)(args[-1].m_data.num) : (long)(q_Normalizer$$FORM_C));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      tg1_10Normalizer_isnormalized(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("Normalizer::isnormalized", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_10Normalizer_normalize(TypedValue* _rv, char const* cls_, Value* input, long form) asm("_ZN4HPHP12c_Normalizer12ti_normalizeEPKcRKNS_6StringEl");

void tg1_10Normalizer_normalize(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void tg1_10Normalizer_normalize(TypedValue* rv, ActRec* ar, int32_t count) {
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
  th_10Normalizer_normalize(rv, "Normalizer", &args[-0].m_data, (count > 1) ? (long)(args[-1].m_data.num) : (long)(q_Normalizer$$FORM_C));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* tg_10Normalizer_normalize(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      th_10Normalizer_normalize(rv, "Normalizer", &args[-0].m_data, (count > 1) ? (long)(args[-1].m_data.num) : (long)(q_Normalizer$$FORM_C));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      tg1_10Normalizer_normalize(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("Normalizer::normalize", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_idn_to_ascii(TypedValue* _rv, Value* domain, long options, long variant, TypedValue* idna_info) asm("_ZN4HPHP14f_idn_to_asciiERKNS_6StringEllRKNS_14VRefParamValueE");

void fg1_idn_to_ascii(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_idn_to_ascii(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
  case 3:
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  VRefParamValue defVal3 = uninit_null();
  fh_idn_to_ascii(rv, &args[-0].m_data, (count > 1) ? (long)(args[-1].m_data.num) : (long)(0), (count > 2) ? (long)(args[-2].m_data.num) : (long)(0), (count > 3) ? (args-3) : (TypedValue*)(&defVal3));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_idn_to_ascii(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 4) {
    if ((count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        (count <= 1 || (args - 1)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      VRefParamValue defVal3 = uninit_null();
      fh_idn_to_ascii(rv, &args[-0].m_data, (count > 1) ? (long)(args[-1].m_data.num) : (long)(0), (count > 2) ? (long)(args[-2].m_data.num) : (long)(0), (count > 3) ? (args-3) : (TypedValue*)(&defVal3));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_idn_to_ascii(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("idn_to_ascii", count, 1, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_idn_to_unicode(TypedValue* _rv, Value* domain, long options, long variant, TypedValue* idna_info) asm("_ZN4HPHP16f_idn_to_unicodeERKNS_6StringEllRKNS_14VRefParamValueE");

void fg1_idn_to_unicode(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_idn_to_unicode(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
  case 3:
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  VRefParamValue defVal3 = uninit_null();
  fh_idn_to_unicode(rv, &args[-0].m_data, (count > 1) ? (long)(args[-1].m_data.num) : (long)(0), (count > 2) ? (long)(args[-2].m_data.num) : (long)(0), (count > 3) ? (args-3) : (TypedValue*)(&defVal3));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_idn_to_unicode(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 4) {
    if ((count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        (count <= 1 || (args - 1)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      VRefParamValue defVal3 = uninit_null();
      fh_idn_to_unicode(rv, &args[-0].m_data, (count > 1) ? (long)(args[-1].m_data.num) : (long)(0), (count > 2) ? (long)(args[-2].m_data.num) : (long)(0), (count > 3) ? (args-3) : (TypedValue*)(&defVal3));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_idn_to_unicode(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("idn_to_unicode", count, 1, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_idn_to_utf8(TypedValue* _rv, Value* domain, long options, long variant, TypedValue* idna_info) asm("_ZN4HPHP13f_idn_to_utf8ERKNS_6StringEllRKNS_14VRefParamValueE");

void fg1_idn_to_utf8(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_idn_to_utf8(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
  case 3:
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  VRefParamValue defVal3 = uninit_null();
  fh_idn_to_utf8(rv, &args[-0].m_data, (count > 1) ? (long)(args[-1].m_data.num) : (long)(0), (count > 2) ? (long)(args[-2].m_data.num) : (long)(0), (count > 3) ? (args-3) : (TypedValue*)(&defVal3));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_idn_to_utf8(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 4) {
    if ((count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        (count <= 1 || (args - 1)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      VRefParamValue defVal3 = uninit_null();
      fh_idn_to_utf8(rv, &args[-0].m_data, (count > 1) ? (long)(args[-1].m_data.num) : (long)(0), (count > 2) ? (long)(args[-2].m_data.num) : (long)(0), (count > 3) ? (args-3) : (TypedValue*)(&defVal3));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_idn_to_utf8(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("idn_to_utf8", count, 1, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

} // namespace HPHP
