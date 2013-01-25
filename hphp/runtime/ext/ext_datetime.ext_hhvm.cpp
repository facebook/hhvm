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
HPHP::Object HPHP::c_DateTime::t_add(HPHP::Object const&)
_ZN4HPHP10c_DateTime5t_addERKNS_6ObjectE

(return value) => rax
_rv => rdi
this_ => rsi
interval => rdx
*/

Value* th_8DateTime_add(Value* _rv, ObjectData* this_, Value* interval) asm("_ZN4HPHP10c_DateTime5t_addERKNS_6ObjectE");

TypedValue* tg1_8DateTime_add(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_8DateTime_add(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfObject;
  tvCastToObjectInPlace(args-0);
  th_8DateTime_add((Value*)(rv), (this_), (Value*)(args-0));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_8DateTime_add(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if ((args-0)->m_type == KindOfObject) {
          rv._count = 0;
          rv.m_type = KindOfObject;
          th_8DateTime_add((Value*)(&(rv)), (this_), (Value*)(args-0));
          if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_8DateTime_add(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DateTime::add", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DateTime::add");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

HPHP::VM::Instance* new_DateTime_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_DateTime) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_DateTime(ObjectStaticCallbacks::encodeVMClass(cls));
  return inst;
}

/*
void HPHP::c_DateTime::t___construct(HPHP::String const&, HPHP::Object const&)
_ZN4HPHP10c_DateTime13t___constructERKNS_6StringERKNS_6ObjectE

this_ => rdi
time => rsi
timezone => rdx
*/

void th_8DateTime___construct(ObjectData* this_, Value* time, Value* timezone) asm("_ZN4HPHP10c_DateTime13t___constructERKNS_6StringERKNS_6ObjectE");

TypedValue* tg1_8DateTime___construct(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_8DateTime___construct(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfObject) {
      tvCastToObjectInPlace(args-1);
    }
  case 1:
    if (!IS_STRING_TYPE((args-0)->m_type)) {
      tvCastToStringInPlace(args-0);
    }
  case 0:
    break;
  }
  String defVal0 = "now";
  th_8DateTime___construct((this_), (count > 0) ? (Value*)(args-0) : (Value*)(&defVal0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_object));
  return rv;
}

TypedValue* tg_8DateTime___construct(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count <= 2LL) {
        if ((count <= 1 || (args-1)->m_type == KindOfObject) && (count <= 0 || IS_STRING_TYPE((args-0)->m_type))) {
          rv.m_data.num = 0LL;
          rv._count = 0;
          rv.m_type = KindOfNull;
          String defVal0 = "now";
          th_8DateTime___construct((this_), (count > 0) ? (Value*)(args-0) : (Value*)(&defVal0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_object));
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_8DateTime___construct(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_toomany_arguments_nr("DateTime::__construct", 2, 1);
      }
    } else {
      throw_instance_method_fatal("DateTime::__construct");
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
HPHP::Object HPHP::c_DateTime::ti_createfromformat(char const*, HPHP::String const&, HPHP::String const&, HPHP::Object const&)
_ZN4HPHP10c_DateTime19ti_createfromformatEPKcRKNS_6StringES5_RKNS_6ObjectE

(return value) => rax
_rv => rdi
cls_ => rsi
format => rdx
time => rcx
timezone => r8
*/

Value* th_8DateTime_createFromFormat(Value* _rv, char const* cls_, Value* format, Value* time, Value* timezone) asm("_ZN4HPHP10c_DateTime19ti_createfromformatEPKcRKNS_6StringES5_RKNS_6ObjectE");

TypedValue* tg1_8DateTime_createFromFormat(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue* tg1_8DateTime_createFromFormat(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfObject;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfObject) {
      tvCastToObjectInPlace(args-2);
    }
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  th_8DateTime_createFromFormat((Value*)(rv), ("DateTime"), (Value*)(args-0), (Value*)(args-1), (count > 2) ? (Value*)(args-2) : (Value*)(&null_object));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_8DateTime_createFromFormat(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfObject) && IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfObject;
        th_8DateTime_createFromFormat((Value*)(&(rv)), ("DateTime"), (Value*)(args-0), (Value*)(args-1), (count > 2) ? (Value*)(args-2) : (Value*)(&null_object));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        tg1_8DateTime_createFromFormat(&rv, ar, count );
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("DateTime::createFromFormat", count, 2, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_DateTime::t_diff(HPHP::Object const&, bool)
_ZN4HPHP10c_DateTime6t_diffERKNS_6ObjectEb

(return value) => rax
_rv => rdi
this_ => rsi
datetime2 => rdx
absolute => rcx
*/

Value* th_8DateTime_diff(Value* _rv, ObjectData* this_, Value* datetime2, bool absolute) asm("_ZN4HPHP10c_DateTime6t_diffERKNS_6ObjectEb");

TypedValue* tg1_8DateTime_diff(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_8DateTime_diff(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfObject;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-1);
    }
  case 1:
    break;
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  th_8DateTime_diff((Value*)(rv), (this_), (Value*)(args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_8DateTime_diff(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL && count <= 2LL) {
        if ((count <= 1 || (args-1)->m_type == KindOfBoolean) && (args-0)->m_type == KindOfObject) {
          rv._count = 0;
          rv.m_type = KindOfObject;
          th_8DateTime_diff((Value*)(&(rv)), (this_), (Value*)(args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
          if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_8DateTime_diff(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DateTime::diff", count, 1, 2, 1);
      }
    } else {
      throw_instance_method_fatal("DateTime::diff");
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
HPHP::String HPHP::c_DateTime::t_format(HPHP::String const&)
_ZN4HPHP10c_DateTime8t_formatERKNS_6StringE

(return value) => rax
_rv => rdi
this_ => rsi
format => rdx
*/

Value* th_8DateTime_format(Value* _rv, ObjectData* this_, Value* format) asm("_ZN4HPHP10c_DateTime8t_formatERKNS_6StringE");

TypedValue* tg1_8DateTime_format(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_8DateTime_format(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToStringInPlace(args-0);
  th_8DateTime_format((Value*)(rv), (this_), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_8DateTime_format(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          rv._count = 0;
          rv.m_type = KindOfString;
          th_8DateTime_format((Value*)(&(rv)), (this_), (Value*)(args-0));
          if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_8DateTime_format(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DateTime::format", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DateTime::format");
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
HPHP::Array HPHP::c_DateTime::ti_getlasterrors(char const*)
_ZN4HPHP10c_DateTime16ti_getlasterrorsEPKc

(return value) => rax
_rv => rdi
cls_ => rsi
*/

Value* th_8DateTime_getLastErrors(Value* _rv, char const* cls_) asm("_ZN4HPHP10c_DateTime16ti_getlasterrorsEPKc");

TypedValue* tg_8DateTime_getLastErrors(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfArray;
      th_8DateTime_getLastErrors((Value*)(&(rv)), ("DateTime"));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("DateTime::getLastErrors", 0, 1);
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
long HPHP::c_DateTime::t_getoffset()
_ZN4HPHP10c_DateTime11t_getoffsetEv

(return value) => rax
this_ => rdi
*/

long th_8DateTime_getOffset(ObjectData* this_) asm("_ZN4HPHP10c_DateTime11t_getoffsetEv");

TypedValue* tg_8DateTime_getOffset(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)th_8DateTime_getOffset((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DateTime::getOffset", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DateTime::getOffset");
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
long HPHP::c_DateTime::t_gettimestamp()
_ZN4HPHP10c_DateTime14t_gettimestampEv

(return value) => rax
this_ => rdi
*/

long th_8DateTime_getTimestamp(ObjectData* this_) asm("_ZN4HPHP10c_DateTime14t_gettimestampEv");

TypedValue* tg_8DateTime_getTimestamp(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)th_8DateTime_getTimestamp((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DateTime::getTimestamp", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DateTime::getTimestamp");
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
HPHP::Variant HPHP::c_DateTime::t_gettimezone()
_ZN4HPHP10c_DateTime13t_gettimezoneEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_8DateTime_getTimezone(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP10c_DateTime13t_gettimezoneEv");

TypedValue* tg_8DateTime_getTimezone(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_8DateTime_getTimezone((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DateTime::getTimezone", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DateTime::getTimezone");
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
HPHP::Object HPHP::c_DateTime::t_modify(HPHP::String const&)
_ZN4HPHP10c_DateTime8t_modifyERKNS_6StringE

(return value) => rax
_rv => rdi
this_ => rsi
modify => rdx
*/

Value* th_8DateTime_modify(Value* _rv, ObjectData* this_, Value* modify) asm("_ZN4HPHP10c_DateTime8t_modifyERKNS_6StringE");

TypedValue* tg1_8DateTime_modify(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_8DateTime_modify(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfObject;
  tvCastToStringInPlace(args-0);
  th_8DateTime_modify((Value*)(rv), (this_), (Value*)(args-0));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_8DateTime_modify(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          rv._count = 0;
          rv.m_type = KindOfObject;
          th_8DateTime_modify((Value*)(&(rv)), (this_), (Value*)(args-0));
          if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_8DateTime_modify(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DateTime::modify", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DateTime::modify");
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
HPHP::Object HPHP::c_DateTime::t_setdate(long, long, long)
_ZN4HPHP10c_DateTime9t_setdateElll

(return value) => rax
_rv => rdi
this_ => rsi
year => rdx
month => rcx
day => r8
*/

Value* th_8DateTime_setDate(Value* _rv, ObjectData* this_, long year, long month, long day) asm("_ZN4HPHP10c_DateTime9t_setdateElll");

TypedValue* tg1_8DateTime_setDate(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_8DateTime_setDate(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfObject;
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  th_8DateTime_setDate((Value*)(rv), (this_), (long)(args[-0].m_data.num), (long)(args[-1].m_data.num), (long)(args[-2].m_data.num));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_8DateTime_setDate(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 3LL) {
        if ((args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfInt64) {
          rv._count = 0;
          rv.m_type = KindOfObject;
          th_8DateTime_setDate((Value*)(&(rv)), (this_), (long)(args[-0].m_data.num), (long)(args[-1].m_data.num), (long)(args[-2].m_data.num));
          if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_8DateTime_setDate(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DateTime::setDate", count, 3, 3, 1);
      }
    } else {
      throw_instance_method_fatal("DateTime::setDate");
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
HPHP::Object HPHP::c_DateTime::t_setisodate(long, long, long)
_ZN4HPHP10c_DateTime12t_setisodateElll

(return value) => rax
_rv => rdi
this_ => rsi
year => rdx
week => rcx
day => r8
*/

Value* th_8DateTime_setISODate(Value* _rv, ObjectData* this_, long year, long week, long day) asm("_ZN4HPHP10c_DateTime12t_setisodateElll");

TypedValue* tg1_8DateTime_setISODate(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_8DateTime_setISODate(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfObject;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    break;
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  th_8DateTime_setISODate((Value*)(rv), (this_), (long)(args[-0].m_data.num), (long)(args[-1].m_data.num), (count > 2) ? (long)(args[-2].m_data.num) : (long)(1));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_8DateTime_setISODate(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 2LL && count <= 3LL) {
        if ((count <= 2 || (args-2)->m_type == KindOfInt64) && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfInt64) {
          rv._count = 0;
          rv.m_type = KindOfObject;
          th_8DateTime_setISODate((Value*)(&(rv)), (this_), (long)(args[-0].m_data.num), (long)(args[-1].m_data.num), (count > 2) ? (long)(args[-2].m_data.num) : (long)(1));
          if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_8DateTime_setISODate(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DateTime::setISODate", count, 2, 3, 1);
      }
    } else {
      throw_instance_method_fatal("DateTime::setISODate");
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
HPHP::Object HPHP::c_DateTime::t_settime(long, long, long)
_ZN4HPHP10c_DateTime9t_settimeElll

(return value) => rax
_rv => rdi
this_ => rsi
hour => rdx
minute => rcx
second => r8
*/

Value* th_8DateTime_setTime(Value* _rv, ObjectData* this_, long hour, long minute, long second) asm("_ZN4HPHP10c_DateTime9t_settimeElll");

TypedValue* tg1_8DateTime_setTime(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_8DateTime_setTime(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfObject;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    break;
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  th_8DateTime_setTime((Value*)(rv), (this_), (long)(args[-0].m_data.num), (long)(args[-1].m_data.num), (count > 2) ? (long)(args[-2].m_data.num) : (long)(0));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_8DateTime_setTime(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 2LL && count <= 3LL) {
        if ((count <= 2 || (args-2)->m_type == KindOfInt64) && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfInt64) {
          rv._count = 0;
          rv.m_type = KindOfObject;
          th_8DateTime_setTime((Value*)(&(rv)), (this_), (long)(args[-0].m_data.num), (long)(args[-1].m_data.num), (count > 2) ? (long)(args[-2].m_data.num) : (long)(0));
          if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_8DateTime_setTime(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DateTime::setTime", count, 2, 3, 1);
      }
    } else {
      throw_instance_method_fatal("DateTime::setTime");
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
HPHP::Object HPHP::c_DateTime::t_settimestamp(long)
_ZN4HPHP10c_DateTime14t_settimestampEl

(return value) => rax
_rv => rdi
this_ => rsi
unixtimestamp => rdx
*/

Value* th_8DateTime_setTimestamp(Value* _rv, ObjectData* this_, long unixtimestamp) asm("_ZN4HPHP10c_DateTime14t_settimestampEl");

TypedValue* tg1_8DateTime_setTimestamp(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_8DateTime_setTimestamp(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfObject;
  tvCastToInt64InPlace(args-0);
  th_8DateTime_setTimestamp((Value*)(rv), (this_), (long)(args[-0].m_data.num));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_8DateTime_setTimestamp(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if ((args-0)->m_type == KindOfInt64) {
          rv._count = 0;
          rv.m_type = KindOfObject;
          th_8DateTime_setTimestamp((Value*)(&(rv)), (this_), (long)(args[-0].m_data.num));
          if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_8DateTime_setTimestamp(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DateTime::setTimestamp", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DateTime::setTimestamp");
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
HPHP::Object HPHP::c_DateTime::t_settimezone(HPHP::Object const&)
_ZN4HPHP10c_DateTime13t_settimezoneERKNS_6ObjectE

(return value) => rax
_rv => rdi
this_ => rsi
timezone => rdx
*/

Value* th_8DateTime_setTimezone(Value* _rv, ObjectData* this_, Value* timezone) asm("_ZN4HPHP10c_DateTime13t_settimezoneERKNS_6ObjectE");

TypedValue* tg1_8DateTime_setTimezone(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_8DateTime_setTimezone(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfObject;
  tvCastToObjectInPlace(args-0);
  th_8DateTime_setTimezone((Value*)(rv), (this_), (Value*)(args-0));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_8DateTime_setTimezone(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if ((args-0)->m_type == KindOfObject) {
          rv._count = 0;
          rv.m_type = KindOfObject;
          th_8DateTime_setTimezone((Value*)(&(rv)), (this_), (Value*)(args-0));
          if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_8DateTime_setTimezone(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DateTime::setTimezone", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DateTime::setTimezone");
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
HPHP::Object HPHP::c_DateTime::t_sub(HPHP::Object const&)
_ZN4HPHP10c_DateTime5t_subERKNS_6ObjectE

(return value) => rax
_rv => rdi
this_ => rsi
interval => rdx
*/

Value* th_8DateTime_sub(Value* _rv, ObjectData* this_, Value* interval) asm("_ZN4HPHP10c_DateTime5t_subERKNS_6ObjectE");

TypedValue* tg1_8DateTime_sub(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_8DateTime_sub(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfObject;
  tvCastToObjectInPlace(args-0);
  th_8DateTime_sub((Value*)(rv), (this_), (Value*)(args-0));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_8DateTime_sub(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if ((args-0)->m_type == KindOfObject) {
          rv._count = 0;
          rv.m_type = KindOfObject;
          th_8DateTime_sub((Value*)(&(rv)), (this_), (Value*)(args-0));
          if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_8DateTime_sub(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DateTime::sub", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DateTime::sub");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

HPHP::VM::Instance* new_DateTimeZone_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_DateTimeZone) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_DateTimeZone(ObjectStaticCallbacks::encodeVMClass(cls));
  return inst;
}

/*
void HPHP::c_DateTimeZone::t___construct(HPHP::String const&)
_ZN4HPHP14c_DateTimeZone13t___constructERKNS_6StringE

this_ => rdi
timezone => rsi
*/

void th_12DateTimeZone___construct(ObjectData* this_, Value* timezone) asm("_ZN4HPHP14c_DateTimeZone13t___constructERKNS_6StringE");

TypedValue* tg1_12DateTimeZone___construct(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_12DateTimeZone___construct(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  tvCastToStringInPlace(args-0);
  th_12DateTimeZone___construct((this_), (Value*)(args-0));
  return rv;
}

TypedValue* tg_12DateTimeZone___construct(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_data.num = 0LL;
          rv._count = 0;
          rv.m_type = KindOfNull;
          th_12DateTimeZone___construct((this_), (Value*)(args-0));
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_12DateTimeZone___construct(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DateTimeZone::__construct", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DateTimeZone::__construct");
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
HPHP::Array HPHP::c_DateTimeZone::t_getlocation()
_ZN4HPHP14c_DateTimeZone13t_getlocationEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_12DateTimeZone_getLocation(Value* _rv, ObjectData* this_) asm("_ZN4HPHP14c_DateTimeZone13t_getlocationEv");

TypedValue* tg_12DateTimeZone_getLocation(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfArray;
        th_12DateTimeZone_getLocation((Value*)(&(rv)), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DateTimeZone::getLocation", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DateTimeZone::getLocation");
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
HPHP::String HPHP::c_DateTimeZone::t_getname()
_ZN4HPHP14c_DateTimeZone9t_getnameEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_12DateTimeZone_getName(Value* _rv, ObjectData* this_) asm("_ZN4HPHP14c_DateTimeZone9t_getnameEv");

TypedValue* tg_12DateTimeZone_getName(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfString;
        th_12DateTimeZone_getName((Value*)(&(rv)), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DateTimeZone::getName", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DateTimeZone::getName");
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
long HPHP::c_DateTimeZone::t_getoffset(HPHP::Object const&)
_ZN4HPHP14c_DateTimeZone11t_getoffsetERKNS_6ObjectE

(return value) => rax
this_ => rdi
datetime => rsi
*/

long th_12DateTimeZone_getOffset(ObjectData* this_, Value* datetime) asm("_ZN4HPHP14c_DateTimeZone11t_getoffsetERKNS_6ObjectE");

TypedValue* tg1_12DateTimeZone_getOffset(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_12DateTimeZone_getOffset(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfInt64;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (int64_t)th_12DateTimeZone_getOffset((this_), (Value*)(args-0));
  return rv;
}

TypedValue* tg_12DateTimeZone_getOffset(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if ((args-0)->m_type == KindOfObject) {
          rv._count = 0;
          rv.m_type = KindOfInt64;
          rv.m_data.num = (int64_t)th_12DateTimeZone_getOffset((this_), (Value*)(args-0));
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_12DateTimeZone_getOffset(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DateTimeZone::getOffset", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DateTimeZone::getOffset");
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
HPHP::Array HPHP::c_DateTimeZone::t_gettransitions()
_ZN4HPHP14c_DateTimeZone16t_gettransitionsEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_12DateTimeZone_getTransitions(Value* _rv, ObjectData* this_) asm("_ZN4HPHP14c_DateTimeZone16t_gettransitionsEv");

TypedValue* tg_12DateTimeZone_getTransitions(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfArray;
        th_12DateTimeZone_getTransitions((Value*)(&(rv)), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DateTimeZone::getTransitions", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DateTimeZone::getTransitions");
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
HPHP::Array HPHP::c_DateTimeZone::ti_listabbreviations(char const*)
_ZN4HPHP14c_DateTimeZone20ti_listabbreviationsEPKc

(return value) => rax
_rv => rdi
cls_ => rsi
*/

Value* th_12DateTimeZone_listAbbreviations(Value* _rv, char const* cls_) asm("_ZN4HPHP14c_DateTimeZone20ti_listabbreviationsEPKc");

TypedValue* tg_12DateTimeZone_listAbbreviations(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfArray;
      th_12DateTimeZone_listAbbreviations((Value*)(&(rv)), ("DateTimeZone"));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("DateTimeZone::listAbbreviations", 0, 1);
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
HPHP::Array HPHP::c_DateTimeZone::ti_listidentifiers(char const*)
_ZN4HPHP14c_DateTimeZone18ti_listidentifiersEPKc

(return value) => rax
_rv => rdi
cls_ => rsi
*/

Value* th_12DateTimeZone_listIdentifiers(Value* _rv, char const* cls_) asm("_ZN4HPHP14c_DateTimeZone18ti_listidentifiersEPKc");

TypedValue* tg_12DateTimeZone_listIdentifiers(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfArray;
      th_12DateTimeZone_listIdentifiers((Value*)(&(rv)), ("DateTimeZone"));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("DateTimeZone::listIdentifiers", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

HPHP::VM::Instance* new_DateInterval_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_DateInterval) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_DateInterval(ObjectStaticCallbacks::encodeVMClass(cls));
  return inst;
}

/*
void HPHP::c_DateInterval::t___construct(HPHP::String const&)
_ZN4HPHP14c_DateInterval13t___constructERKNS_6StringE

this_ => rdi
interval_spec => rsi
*/

void th_12DateInterval___construct(ObjectData* this_, Value* interval_spec) asm("_ZN4HPHP14c_DateInterval13t___constructERKNS_6StringE");

TypedValue* tg1_12DateInterval___construct(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_12DateInterval___construct(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  tvCastToStringInPlace(args-0);
  th_12DateInterval___construct((this_), (Value*)(args-0));
  return rv;
}

TypedValue* tg_12DateInterval___construct(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_data.num = 0LL;
          rv._count = 0;
          rv.m_type = KindOfNull;
          th_12DateInterval___construct((this_), (Value*)(args-0));
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_12DateInterval___construct(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DateInterval::__construct", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DateInterval::__construct");
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
HPHP::Variant HPHP::c_DateInterval::t___get(HPHP::Variant)
_ZN4HPHP14c_DateInterval7t___getENS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
member => rdx
*/

TypedValue* th_12DateInterval___get(TypedValue* _rv, ObjectData* this_, TypedValue* member) asm("_ZN4HPHP14c_DateInterval7t___getENS_7VariantE");

TypedValue* tg_12DateInterval___get(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        th_12DateInterval___get((&(rv)), (this_), (args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("DateInterval::__get", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DateInterval::__get");
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
HPHP::Variant HPHP::c_DateInterval::t___set(HPHP::Variant, HPHP::Variant)
_ZN4HPHP14c_DateInterval7t___setENS_7VariantES1_

(return value) => rax
_rv => rdi
this_ => rsi
member => rdx
value => rcx
*/

TypedValue* th_12DateInterval___set(TypedValue* _rv, ObjectData* this_, TypedValue* member, TypedValue* value) asm("_ZN4HPHP14c_DateInterval7t___setENS_7VariantES1_");

TypedValue* tg_12DateInterval___set(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        th_12DateInterval___set((&(rv)), (this_), (args-0), (args-1));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("DateInterval::__set", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("DateInterval::__set");
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
HPHP::Object HPHP::c_DateInterval::ti_createfromdatestring(char const*, HPHP::String const&)
_ZN4HPHP14c_DateInterval23ti_createfromdatestringEPKcRKNS_6StringE

(return value) => rax
_rv => rdi
cls_ => rsi
time => rdx
*/

Value* th_12DateInterval_createFromDateString(Value* _rv, char const* cls_, Value* time) asm("_ZN4HPHP14c_DateInterval23ti_createfromdatestringEPKcRKNS_6StringE");

TypedValue* tg1_12DateInterval_createFromDateString(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue* tg1_12DateInterval_createFromDateString(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfObject;
  tvCastToStringInPlace(args-0);
  th_12DateInterval_createFromDateString((Value*)(rv), ("DateInterval"), (Value*)(args-0));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_12DateInterval_createFromDateString(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfObject;
        th_12DateInterval_createFromDateString((Value*)(&(rv)), ("DateInterval"), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        tg1_12DateInterval_createFromDateString(&rv, ar, count );
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("DateInterval::createFromDateString", count, 1, 1, 1);
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
HPHP::String HPHP::c_DateInterval::t_format(HPHP::String const&)
_ZN4HPHP14c_DateInterval8t_formatERKNS_6StringE

(return value) => rax
_rv => rdi
this_ => rsi
format => rdx
*/

Value* th_12DateInterval_format(Value* _rv, ObjectData* this_, Value* format) asm("_ZN4HPHP14c_DateInterval8t_formatERKNS_6StringE");

TypedValue* tg1_12DateInterval_format(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_12DateInterval_format(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToStringInPlace(args-0);
  th_12DateInterval_format((Value*)(rv), (this_), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_12DateInterval_format(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          rv._count = 0;
          rv.m_type = KindOfString;
          th_12DateInterval_format((Value*)(&(rv)), (this_), (Value*)(args-0));
          if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_12DateInterval_format(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DateInterval::format", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DateInterval::format");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}


} // !HPHP

