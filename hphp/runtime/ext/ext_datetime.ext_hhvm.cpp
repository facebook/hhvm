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
bool HPHP::f_checkdate(int, int, int)
_ZN4HPHP11f_checkdateEiii

(return value) => rax
month => rdi
day => rsi
year => rdx
*/

bool fh_checkdate(int month, int day, int year) asm("_ZN4HPHP11f_checkdateEiii");

TypedValue * fg1_checkdate(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_checkdate(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  rv->m_data.num = (fh_checkdate((int)(args[-0].m_data.num), (int)(args[-1].m_data.num), (int)(args[-2].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_checkdate(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfInt64) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_checkdate((int)(args[-0].m_data.num), (int)(args[-1].m_data.num), (int)(args[-2].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_checkdate(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("checkdate", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Object HPHP::f_date_add(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP10f_date_addERKNS_6ObjectES2_

(return value) => rax
_rv => rdi
datetime => rsi
interval => rdx
*/

Value* fh_date_add(Value* _rv, Value* datetime, Value* interval) asm("_ZN4HPHP10f_date_addERKNS_6ObjectES2_");

TypedValue * fg1_date_add(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_date_add(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_date_add((&rv->m_data), &args[-0].m_data, &args[-1].m_data);
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_date_add(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfObject;
        fh_date_add((&rv.m_data), &args[-0].m_data, &args[-1].m_data);
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_date_add(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("date_add", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Object HPHP::f_date_create_from_format(HPHP::String const&, HPHP::String const&, HPHP::Object const&)
_ZN4HPHP25f_date_create_from_formatERKNS_6StringES2_RKNS_6ObjectE

(return value) => rax
_rv => rdi
format => rsi
time => rdx
timezone => rcx
*/

Value* fh_date_create_from_format(Value* _rv, Value* format, Value* time, Value* timezone) asm("_ZN4HPHP25f_date_create_from_formatERKNS_6StringES2_RKNS_6ObjectE");

TypedValue * fg1_date_create_from_format(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_date_create_from_format(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  fh_date_create_from_format((&rv->m_data), &args[-0].m_data, &args[-1].m_data, (count > 2) ? &args[-2].m_data : (Value*)(&null_object));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_date_create_from_format(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfObject) && IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        rv.m_type = KindOfObject;
        fh_date_create_from_format((&rv.m_data), &args[-0].m_data, &args[-1].m_data, (count > 2) ? &args[-2].m_data : (Value*)(&null_object));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_date_create_from_format(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("date_create_from_format", count, 2, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Object HPHP::f_date_create(HPHP::String const&, HPHP::Object const&)
_ZN4HPHP13f_date_createERKNS_6StringERKNS_6ObjectE

(return value) => rax
_rv => rdi
time => rsi
timezone => rdx
*/

Value* fh_date_create(Value* _rv, Value* time, Value* timezone) asm("_ZN4HPHP13f_date_createERKNS_6StringERKNS_6ObjectE");

TypedValue * fg1_date_create(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_date_create(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
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
  fh_date_create((&rv->m_data), (count > 0) ? &args[-0].m_data : (Value*)(&null_string), (count > 1) ? &args[-1].m_data : (Value*)(&null_object));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_date_create(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfObject) && (count <= 0 || IS_STRING_TYPE((args-0)->m_type))) {
        rv.m_type = KindOfObject;
        fh_date_create((&rv.m_data), (count > 0) ? &args[-0].m_data : (Value*)(&null_string), (count > 1) ? &args[-1].m_data : (Value*)(&null_object));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_date_create(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("date_create", 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_date_date_set(HPHP::Object const&, int, int, int)
_ZN4HPHP15f_date_date_setERKNS_6ObjectEiii

object => rdi
year => rsi
month => rdx
day => rcx
*/

void fh_date_date_set(Value* object, int year, int month, int day) asm("_ZN4HPHP15f_date_date_setERKNS_6ObjectEiii");

TypedValue * fg1_date_date_set(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_date_date_set(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_date_date_set(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num));
  return rv;
}

TypedValue* fg_date_date_set(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 4LL) {
      if ((args-3)->m_type == KindOfInt64 && (args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_date_date_set(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num));
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_date_date_set(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("date_date_set", count, 4, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_date_default_timezone_get()
_ZN4HPHP27f_date_default_timezone_getEv

(return value) => rax
_rv => rdi
*/

Value* fh_date_default_timezone_get(Value* _rv) asm("_ZN4HPHP27f_date_default_timezone_getEv");

TypedValue* fg_date_default_timezone_get(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_type = KindOfString;
      fh_date_default_timezone_get((&rv.m_data));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("date_default_timezone_get", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_date_default_timezone_set(HPHP::String const&)
_ZN4HPHP27f_date_default_timezone_setERKNS_6StringE

(return value) => rax
name => rdi
*/

bool fh_date_default_timezone_set(Value* name) asm("_ZN4HPHP27f_date_default_timezone_setERKNS_6StringE");

TypedValue * fg1_date_default_timezone_set(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_date_default_timezone_set(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToStringInPlace(args-0);
  rv->m_data.num = (fh_date_default_timezone_set(&args[-0].m_data)) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_date_default_timezone_set(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_date_default_timezone_set(&args[-0].m_data)) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_date_default_timezone_set(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("date_default_timezone_set", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Object HPHP::f_date_diff(HPHP::Object const&, HPHP::Object const&, bool)
_ZN4HPHP11f_date_diffERKNS_6ObjectES2_b

(return value) => rax
_rv => rdi
datetime => rsi
datetime2 => rdx
absolute => rcx
*/

Value* fh_date_diff(Value* _rv, Value* datetime, Value* datetime2, bool absolute) asm("_ZN4HPHP11f_date_diffERKNS_6ObjectES2_b");

TypedValue * fg1_date_diff(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_date_diff(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-2);
    }
  case 2:
    break;
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_date_diff((&rv->m_data), &args[-0].m_data, &args[-1].m_data, (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_date_diff(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfBoolean) && (args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfObject;
        fh_date_diff((&rv.m_data), &args[-0].m_data, &args[-1].m_data, (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_date_diff(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("date_diff", count, 2, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_date_format(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP13f_date_formatERKNS_6ObjectERKNS_6StringE

(return value) => rax
_rv => rdi
object => rsi
format => rdx
*/

Value* fh_date_format(Value* _rv, Value* object, Value* format) asm("_ZN4HPHP13f_date_formatERKNS_6ObjectERKNS_6StringE");

TypedValue * fg1_date_format(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_date_format(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfString;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_date_format((&rv->m_data), &args[-0].m_data, &args[-1].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_date_format(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfString;
        fh_date_format((&rv.m_data), &args[-0].m_data, &args[-1].m_data);
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_date_format(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("date_format", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_date_get_last_errors()
_ZN4HPHP22f_date_get_last_errorsEv

(return value) => rax
_rv => rdi
*/

Value* fh_date_get_last_errors(Value* _rv) asm("_ZN4HPHP22f_date_get_last_errorsEv");

TypedValue* fg_date_get_last_errors(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_type = KindOfArray;
      fh_date_get_last_errors((&rv.m_data));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("date_get_last_errors", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Object HPHP::f_date_interval_create_from_date_string(HPHP::String const&)
_ZN4HPHP39f_date_interval_create_from_date_stringERKNS_6StringE

(return value) => rax
_rv => rdi
time => rsi
*/

Value* fh_date_interval_create_from_date_string(Value* _rv, Value* time) asm("_ZN4HPHP39f_date_interval_create_from_date_stringERKNS_6StringE");

TypedValue * fg1_date_interval_create_from_date_string(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_date_interval_create_from_date_string(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  tvCastToStringInPlace(args-0);
  fh_date_interval_create_from_date_string((&rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_date_interval_create_from_date_string(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv.m_type = KindOfObject;
        fh_date_interval_create_from_date_string((&rv.m_data), &args[-0].m_data);
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_date_interval_create_from_date_string(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("date_interval_create_from_date_string", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_date_interval_format(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP22f_date_interval_formatERKNS_6ObjectERKNS_6StringE

(return value) => rax
_rv => rdi
interval => rsi
format_spec => rdx
*/

Value* fh_date_interval_format(Value* _rv, Value* interval, Value* format_spec) asm("_ZN4HPHP22f_date_interval_formatERKNS_6ObjectERKNS_6StringE");

TypedValue * fg1_date_interval_format(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_date_interval_format(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfString;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_date_interval_format((&rv->m_data), &args[-0].m_data, &args[-1].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_date_interval_format(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfString;
        fh_date_interval_format((&rv.m_data), &args[-0].m_data, &args[-1].m_data);
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_date_interval_format(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("date_interval_format", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_date_isodate_set(HPHP::Object const&, int, int, int)
_ZN4HPHP18f_date_isodate_setERKNS_6ObjectEiii

object => rdi
year => rsi
week => rdx
day => rcx
*/

void fh_date_isodate_set(Value* object, int year, int week, int day) asm("_ZN4HPHP18f_date_isodate_setERKNS_6ObjectEiii");

TypedValue * fg1_date_isodate_set(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_date_isodate_set(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
    break;
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_date_isodate_set(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (count > 3) ? (int)(args[-3].m_data.num) : (int)(1));
  return rv;
}

TypedValue* fg_date_isodate_set(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 3LL && count <= 4LL) {
      if ((count <= 3 || (args-3)->m_type == KindOfInt64) && (args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_date_isodate_set(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (count > 3) ? (int)(args[-3].m_data.num) : (int)(1));
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_date_isodate_set(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("date_isodate_set", count, 3, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_date_modify(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP13f_date_modifyERKNS_6ObjectERKNS_6StringE

object => rdi
modify => rsi
*/

void fh_date_modify(Value* object, Value* modify) asm("_ZN4HPHP13f_date_modifyERKNS_6ObjectERKNS_6StringE");

TypedValue * fg1_date_modify(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_date_modify(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_date_modify(&args[-0].m_data, &args[-1].m_data);
  return rv;
}

TypedValue* fg_date_modify(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_date_modify(&args[-0].m_data, &args[-1].m_data);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_date_modify(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("date_modify", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_date_offset_get(HPHP::Object const&)
_ZN4HPHP17f_date_offset_getERKNS_6ObjectE

(return value) => rax
object => rdi
*/

long fh_date_offset_get(Value* object) asm("_ZN4HPHP17f_date_offset_getERKNS_6ObjectE");

TypedValue * fg1_date_offset_get(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_date_offset_get(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfInt64;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (int64_t)fh_date_offset_get(&args[-0].m_data);
  return rv;
}

TypedValue* fg_date_offset_get(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_date_offset_get(&args[-0].m_data);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_date_offset_get(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("date_offset_get", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_date_parse(HPHP::String const&)
_ZN4HPHP12f_date_parseERKNS_6StringE

(return value) => rax
_rv => rdi
date => rsi
*/

TypedValue* fh_date_parse(TypedValue* _rv, Value* date) asm("_ZN4HPHP12f_date_parseERKNS_6StringE");

TypedValue * fg1_date_parse(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_date_parse(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_date_parse((rv), &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_date_parse(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        fh_date_parse((&(rv)), &args[-0].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_date_parse(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("date_parse", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Object HPHP::f_date_sub(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP10f_date_subERKNS_6ObjectES2_

(return value) => rax
_rv => rdi
datetime => rsi
interval => rdx
*/

Value* fh_date_sub(Value* _rv, Value* datetime, Value* interval) asm("_ZN4HPHP10f_date_subERKNS_6ObjectES2_");

TypedValue * fg1_date_sub(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_date_sub(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_date_sub((&rv->m_data), &args[-0].m_data, &args[-1].m_data);
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_date_sub(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfObject;
        fh_date_sub((&rv.m_data), &args[-0].m_data, &args[-1].m_data);
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_date_sub(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("date_sub", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_date_sun_info(long, double, double)
_ZN4HPHP15f_date_sun_infoEldd

(return value) => rax
_rv => rdi
ts => rsi
latitude => xmm0
longitude => xmm1
*/

Value* fh_date_sun_info(Value* _rv, long ts, double latitude, double longitude) asm("_ZN4HPHP15f_date_sun_infoEldd");

TypedValue * fg1_date_sun_info(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_date_sun_info(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfArray;
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  fh_date_sun_info((&rv->m_data), (long)(args[-0].m_data.num), (args[-1].m_data.dbl), (args[-2].m_data.dbl));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_date_sun_info(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfInt64) {
        rv.m_type = KindOfArray;
        fh_date_sun_info((&rv.m_data), (long)(args[-0].m_data.num), (args[-1].m_data.dbl), (args[-2].m_data.dbl));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_date_sun_info(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("date_sun_info", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_date_sunrise(long, int, double, double, double, double)
_ZN4HPHP14f_date_sunriseElidddd

(return value) => rax
_rv => rdi
timestamp => rsi
format => rdx
latitude => xmm0
longitude => xmm1
zenith => xmm2
gmt_offset => xmm3
*/

TypedValue* fh_date_sunrise(TypedValue* _rv, long timestamp, int format, double latitude, double longitude, double zenith, double gmt_offset) asm("_ZN4HPHP14f_date_sunriseElidddd");

TypedValue * fg1_date_sunrise(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_date_sunrise(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 6
    if ((args-5)->m_type != KindOfDouble) {
      tvCastToDoubleInPlace(args-5);
    }
  case 5:
    if ((args-4)->m_type != KindOfDouble) {
      tvCastToDoubleInPlace(args-4);
    }
  case 4:
    if ((args-3)->m_type != KindOfDouble) {
      tvCastToDoubleInPlace(args-3);
    }
  case 3:
    if ((args-2)->m_type != KindOfDouble) {
      tvCastToDoubleInPlace(args-2);
    }
  case 2:
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
    break;
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  fh_date_sunrise((rv), (long)(args[-0].m_data.num), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (args[-2].m_data.dbl) : (double)(0.0), (count > 3) ? (args[-3].m_data.dbl) : (double)(0.0), (count > 4) ? (args[-4].m_data.dbl) : (double)(0.0), (count > 5) ? (args[-5].m_data.dbl) : (double)(99999.0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_date_sunrise(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 6LL) {
      if ((count <= 5 || (args-5)->m_type == KindOfDouble) && (count <= 4 || (args-4)->m_type == KindOfDouble) && (count <= 3 || (args-3)->m_type == KindOfDouble) && (count <= 2 || (args-2)->m_type == KindOfDouble) && (count <= 1 || (args-1)->m_type == KindOfInt64) && (args-0)->m_type == KindOfInt64) {
        fh_date_sunrise((&(rv)), (long)(args[-0].m_data.num), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (args[-2].m_data.dbl) : (double)(0.0), (count > 3) ? (args[-3].m_data.dbl) : (double)(0.0), (count > 4) ? (args[-4].m_data.dbl) : (double)(0.0), (count > 5) ? (args[-5].m_data.dbl) : (double)(99999.0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_date_sunrise(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("date_sunrise", count, 1, 6, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 6);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_date_sunset(long, int, double, double, double, double)
_ZN4HPHP13f_date_sunsetElidddd

(return value) => rax
_rv => rdi
timestamp => rsi
format => rdx
latitude => xmm0
longitude => xmm1
zenith => xmm2
gmt_offset => xmm3
*/

TypedValue* fh_date_sunset(TypedValue* _rv, long timestamp, int format, double latitude, double longitude, double zenith, double gmt_offset) asm("_ZN4HPHP13f_date_sunsetElidddd");

TypedValue * fg1_date_sunset(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_date_sunset(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 6
    if ((args-5)->m_type != KindOfDouble) {
      tvCastToDoubleInPlace(args-5);
    }
  case 5:
    if ((args-4)->m_type != KindOfDouble) {
      tvCastToDoubleInPlace(args-4);
    }
  case 4:
    if ((args-3)->m_type != KindOfDouble) {
      tvCastToDoubleInPlace(args-3);
    }
  case 3:
    if ((args-2)->m_type != KindOfDouble) {
      tvCastToDoubleInPlace(args-2);
    }
  case 2:
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
    break;
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  fh_date_sunset((rv), (long)(args[-0].m_data.num), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (args[-2].m_data.dbl) : (double)(0.0), (count > 3) ? (args[-3].m_data.dbl) : (double)(0.0), (count > 4) ? (args[-4].m_data.dbl) : (double)(0.0), (count > 5) ? (args[-5].m_data.dbl) : (double)(99999.0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_date_sunset(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 6LL) {
      if ((count <= 5 || (args-5)->m_type == KindOfDouble) && (count <= 4 || (args-4)->m_type == KindOfDouble) && (count <= 3 || (args-3)->m_type == KindOfDouble) && (count <= 2 || (args-2)->m_type == KindOfDouble) && (count <= 1 || (args-1)->m_type == KindOfInt64) && (args-0)->m_type == KindOfInt64) {
        fh_date_sunset((&(rv)), (long)(args[-0].m_data.num), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (args[-2].m_data.dbl) : (double)(0.0), (count > 3) ? (args[-3].m_data.dbl) : (double)(0.0), (count > 4) ? (args[-4].m_data.dbl) : (double)(0.0), (count > 5) ? (args[-5].m_data.dbl) : (double)(99999.0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_date_sunset(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("date_sunset", count, 1, 6, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 6);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_date_time_set(HPHP::Object const&, int, int, int)
_ZN4HPHP15f_date_time_setERKNS_6ObjectEiii

object => rdi
hour => rsi
minute => rdx
second => rcx
*/

void fh_date_time_set(Value* object, int hour, int minute, int second) asm("_ZN4HPHP15f_date_time_setERKNS_6ObjectEiii");

TypedValue * fg1_date_time_set(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_date_time_set(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
    break;
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_date_time_set(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0));
  return rv;
}

TypedValue* fg_date_time_set(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 3LL && count <= 4LL) {
      if ((count <= 3 || (args-3)->m_type == KindOfInt64) && (args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_date_time_set(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0));
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_date_time_set(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("date_time_set", count, 3, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_date_timestamp_get(HPHP::Object const&)
_ZN4HPHP20f_date_timestamp_getERKNS_6ObjectE

(return value) => rax
datetime => rdi
*/

long fh_date_timestamp_get(Value* datetime) asm("_ZN4HPHP20f_date_timestamp_getERKNS_6ObjectE");

TypedValue * fg1_date_timestamp_get(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_date_timestamp_get(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfInt64;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (int64_t)fh_date_timestamp_get(&args[-0].m_data);
  return rv;
}

TypedValue* fg_date_timestamp_get(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_date_timestamp_get(&args[-0].m_data);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_date_timestamp_get(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("date_timestamp_get", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Object HPHP::f_date_timestamp_set(HPHP::Object const&, long)
_ZN4HPHP20f_date_timestamp_setERKNS_6ObjectEl

(return value) => rax
_rv => rdi
datetime => rsi
timestamp => rdx
*/

Value* fh_date_timestamp_set(Value* _rv, Value* datetime, long timestamp) asm("_ZN4HPHP20f_date_timestamp_setERKNS_6ObjectEl");

TypedValue * fg1_date_timestamp_set(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_date_timestamp_set(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_date_timestamp_set((&rv->m_data), &args[-0].m_data, (long)(args[-1].m_data.num));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_date_timestamp_set(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfObject;
        fh_date_timestamp_set((&rv.m_data), &args[-0].m_data, (long)(args[-1].m_data.num));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_date_timestamp_set(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("date_timestamp_set", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_date_timezone_get(HPHP::Object const&)
_ZN4HPHP19f_date_timezone_getERKNS_6ObjectE

(return value) => rax
_rv => rdi
object => rsi
*/

TypedValue* fh_date_timezone_get(TypedValue* _rv, Value* object) asm("_ZN4HPHP19f_date_timezone_getERKNS_6ObjectE");

TypedValue * fg1_date_timezone_get(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_date_timezone_get(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  fh_date_timezone_get((rv), &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_date_timezone_get(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        fh_date_timezone_get((&(rv)), &args[-0].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_date_timezone_get(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("date_timezone_get", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_date_timezone_set(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP19f_date_timezone_setERKNS_6ObjectES2_

object => rdi
timezone => rsi
*/

void fh_date_timezone_set(Value* object, Value* timezone) asm("_ZN4HPHP19f_date_timezone_setERKNS_6ObjectES2_");

TypedValue * fg1_date_timezone_set(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_date_timezone_set(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_date_timezone_set(&args[-0].m_data, &args[-1].m_data);
  return rv;
}

TypedValue* fg_date_timezone_set(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_date_timezone_set(&args[-0].m_data, &args[-1].m_data);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_date_timezone_set(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("date_timezone_set", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_date(HPHP::String const&, long)
_ZN4HPHP6f_dateERKNS_6StringEl

(return value) => rax
_rv => rdi
format => rsi
timestamp => rdx
*/

TypedValue* fh_date(TypedValue* _rv, Value* format, long timestamp) asm("_ZN4HPHP6f_dateERKNS_6StringEl");

TypedValue * fg1_date(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_date(TypedValue* rv, ActRec* ar, int64_t count) {
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
  fh_date((rv), &args[-0].m_data, (count > 1) ? (long)(args[-1].m_data.num) : (long)(TimeStamp::Current()));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_date(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
        fh_date((&(rv)), &args[-0].m_data, (count > 1) ? (long)(args[-1].m_data.num) : (long)(TimeStamp::Current()));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_date(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("date", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_getdate(long)
_ZN4HPHP9f_getdateEl

(return value) => rax
_rv => rdi
timestamp => rsi
*/

Value* fh_getdate(Value* _rv, long timestamp) asm("_ZN4HPHP9f_getdateEl");

TypedValue * fg1_getdate(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_getdate(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfArray;
  tvCastToInt64InPlace(args-0);
  fh_getdate((&rv->m_data), (count > 0) ? (long)(args[-0].m_data.num) : (long)(TimeStamp::Current()));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_getdate(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      if ((count <= 0 || (args-0)->m_type == KindOfInt64)) {
        rv.m_type = KindOfArray;
        fh_getdate((&rv.m_data), (count > 0) ? (long)(args[-0].m_data.num) : (long)(TimeStamp::Current()));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_getdate(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("getdate", 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_gettimeofday(bool)
_ZN4HPHP14f_gettimeofdayEb

(return value) => rax
_rv => rdi
return_float => rsi
*/

TypedValue* fh_gettimeofday(TypedValue* _rv, bool return_float) asm("_ZN4HPHP14f_gettimeofdayEb");

TypedValue * fg1_gettimeofday(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_gettimeofday(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToBooleanInPlace(args-0);
  fh_gettimeofday((rv), (count > 0) ? (bool)(args[-0].m_data.num) : (bool)(false));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_gettimeofday(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      if ((count <= 0 || (args-0)->m_type == KindOfBoolean)) {
        fh_gettimeofday((&(rv)), (count > 0) ? (bool)(args[-0].m_data.num) : (bool)(false));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_gettimeofday(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("gettimeofday", 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_gmdate(HPHP::String const&, long)
_ZN4HPHP8f_gmdateERKNS_6StringEl

(return value) => rax
_rv => rdi
format => rsi
timestamp => rdx
*/

TypedValue* fh_gmdate(TypedValue* _rv, Value* format, long timestamp) asm("_ZN4HPHP8f_gmdateERKNS_6StringEl");

TypedValue * fg1_gmdate(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_gmdate(TypedValue* rv, ActRec* ar, int64_t count) {
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
  fh_gmdate((rv), &args[-0].m_data, (count > 1) ? (long)(args[-1].m_data.num) : (long)(TimeStamp::Current()));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_gmdate(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
        fh_gmdate((&(rv)), &args[-0].m_data, (count > 1) ? (long)(args[-1].m_data.num) : (long)(TimeStamp::Current()));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_gmdate(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("gmdate", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_gmmktime(int, int, int, int, int, int)
_ZN4HPHP10f_gmmktimeEiiiiii

(return value) => rax
_rv => rdi
hour => rsi
minute => rdx
second => rcx
month => r8
day => r9
year => st0
*/

TypedValue* fh_gmmktime(TypedValue* _rv, int hour, int minute, int second, int month, int day, int year) asm("_ZN4HPHP10f_gmmktimeEiiiiii");

TypedValue * fg1_gmmktime(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_gmmktime(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 6
    if ((args-5)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-5);
    }
  case 5:
    if ((args-4)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-4);
    }
  case 4:
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
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
  fh_gmmktime((rv), (count > 0) ? (int)(args[-0].m_data.num) : (int)(INT_MAX), (count > 1) ? (int)(args[-1].m_data.num) : (int)(INT_MAX), (count > 2) ? (int)(args[-2].m_data.num) : (int)(INT_MAX), (count > 3) ? (int)(args[-3].m_data.num) : (int)(INT_MAX), (count > 4) ? (int)(args[-4].m_data.num) : (int)(INT_MAX), (count > 5) ? (int)(args[-5].m_data.num) : (int)(INT_MAX));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_gmmktime(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 6LL) {
      if ((count <= 5 || (args-5)->m_type == KindOfInt64) && (count <= 4 || (args-4)->m_type == KindOfInt64) && (count <= 3 || (args-3)->m_type == KindOfInt64) && (count <= 2 || (args-2)->m_type == KindOfInt64) && (count <= 1 || (args-1)->m_type == KindOfInt64) && (count <= 0 || (args-0)->m_type == KindOfInt64)) {
        fh_gmmktime((&(rv)), (count > 0) ? (int)(args[-0].m_data.num) : (int)(INT_MAX), (count > 1) ? (int)(args[-1].m_data.num) : (int)(INT_MAX), (count > 2) ? (int)(args[-2].m_data.num) : (int)(INT_MAX), (count > 3) ? (int)(args[-3].m_data.num) : (int)(INT_MAX), (count > 4) ? (int)(args[-4].m_data.num) : (int)(INT_MAX), (count > 5) ? (int)(args[-5].m_data.num) : (int)(INT_MAX));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_gmmktime(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("gmmktime", 6, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 6);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_gmstrftime(HPHP::String const&, long)
_ZN4HPHP12f_gmstrftimeERKNS_6StringEl

(return value) => rax
_rv => rdi
format => rsi
timestamp => rdx
*/

Value* fh_gmstrftime(Value* _rv, Value* format, long timestamp) asm("_ZN4HPHP12f_gmstrftimeERKNS_6StringEl");

TypedValue * fg1_gmstrftime(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_gmstrftime(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfString;
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
  fh_gmstrftime((&rv->m_data), &args[-0].m_data, (count > 1) ? (long)(args[-1].m_data.num) : (long)(TimeStamp::Current()));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_gmstrftime(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
        rv.m_type = KindOfString;
        fh_gmstrftime((&rv.m_data), &args[-0].m_data, (count > 1) ? (long)(args[-1].m_data.num) : (long)(TimeStamp::Current()));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_gmstrftime(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("gmstrftime", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_idate(HPHP::String const&, long)
_ZN4HPHP7f_idateERKNS_6StringEl

(return value) => rax
_rv => rdi
format => rsi
timestamp => rdx
*/

TypedValue* fh_idate(TypedValue* _rv, Value* format, long timestamp) asm("_ZN4HPHP7f_idateERKNS_6StringEl");

TypedValue * fg1_idate(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_idate(TypedValue* rv, ActRec* ar, int64_t count) {
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
  fh_idate((rv), &args[-0].m_data, (count > 1) ? (long)(args[-1].m_data.num) : (long)(TimeStamp::Current()));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_idate(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
        fh_idate((&(rv)), &args[-0].m_data, (count > 1) ? (long)(args[-1].m_data.num) : (long)(TimeStamp::Current()));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_idate(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("idate", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_localtime(long, bool)
_ZN4HPHP11f_localtimeElb

(return value) => rax
_rv => rdi
timestamp => rsi
is_associative => rdx
*/

Value* fh_localtime(Value* _rv, long timestamp, bool is_associative) asm("_ZN4HPHP11f_localtimeElb");

TypedValue * fg1_localtime(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_localtime(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfArray;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-1);
    }
  case 1:
    if ((args-0)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-0);
    }
  case 0:
    break;
  }
  fh_localtime((&rv->m_data), (count > 0) ? (long)(args[-0].m_data.num) : (long)(TimeStamp::Current()), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_localtime(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfBoolean) && (count <= 0 || (args-0)->m_type == KindOfInt64)) {
        rv.m_type = KindOfArray;
        fh_localtime((&rv.m_data), (count > 0) ? (long)(args[-0].m_data.num) : (long)(TimeStamp::Current()), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_localtime(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("localtime", 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_microtime(bool)
_ZN4HPHP11f_microtimeEb

(return value) => rax
_rv => rdi
get_as_float => rsi
*/

TypedValue* fh_microtime(TypedValue* _rv, bool get_as_float) asm("_ZN4HPHP11f_microtimeEb");

TypedValue * fg1_microtime(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_microtime(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToBooleanInPlace(args-0);
  fh_microtime((rv), (count > 0) ? (bool)(args[-0].m_data.num) : (bool)(false));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_microtime(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      if ((count <= 0 || (args-0)->m_type == KindOfBoolean)) {
        fh_microtime((&(rv)), (count > 0) ? (bool)(args[-0].m_data.num) : (bool)(false));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_microtime(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("microtime", 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mktime(int, int, int, int, int, int)
_ZN4HPHP8f_mktimeEiiiiii

(return value) => rax
_rv => rdi
hour => rsi
minute => rdx
second => rcx
month => r8
day => r9
year => st0
*/

TypedValue* fh_mktime(TypedValue* _rv, int hour, int minute, int second, int month, int day, int year) asm("_ZN4HPHP8f_mktimeEiiiiii");

TypedValue * fg1_mktime(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mktime(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 6
    if ((args-5)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-5);
    }
  case 5:
    if ((args-4)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-4);
    }
  case 4:
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
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
  fh_mktime((rv), (count > 0) ? (int)(args[-0].m_data.num) : (int)(INT_MAX), (count > 1) ? (int)(args[-1].m_data.num) : (int)(INT_MAX), (count > 2) ? (int)(args[-2].m_data.num) : (int)(INT_MAX), (count > 3) ? (int)(args[-3].m_data.num) : (int)(INT_MAX), (count > 4) ? (int)(args[-4].m_data.num) : (int)(INT_MAX), (count > 5) ? (int)(args[-5].m_data.num) : (int)(INT_MAX));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mktime(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 6LL) {
      if ((count <= 5 || (args-5)->m_type == KindOfInt64) && (count <= 4 || (args-4)->m_type == KindOfInt64) && (count <= 3 || (args-3)->m_type == KindOfInt64) && (count <= 2 || (args-2)->m_type == KindOfInt64) && (count <= 1 || (args-1)->m_type == KindOfInt64) && (count <= 0 || (args-0)->m_type == KindOfInt64)) {
        fh_mktime((&(rv)), (count > 0) ? (int)(args[-0].m_data.num) : (int)(INT_MAX), (count > 1) ? (int)(args[-1].m_data.num) : (int)(INT_MAX), (count > 2) ? (int)(args[-2].m_data.num) : (int)(INT_MAX), (count > 3) ? (int)(args[-3].m_data.num) : (int)(INT_MAX), (count > 4) ? (int)(args[-4].m_data.num) : (int)(INT_MAX), (count > 5) ? (int)(args[-5].m_data.num) : (int)(INT_MAX));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mktime(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("mktime", 6, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 6);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_strftime(HPHP::String const&, long)
_ZN4HPHP10f_strftimeERKNS_6StringEl

(return value) => rax
_rv => rdi
format => rsi
timestamp => rdx
*/

TypedValue* fh_strftime(TypedValue* _rv, Value* format, long timestamp) asm("_ZN4HPHP10f_strftimeERKNS_6StringEl");

TypedValue * fg1_strftime(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_strftime(TypedValue* rv, ActRec* ar, int64_t count) {
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
  fh_strftime((rv), &args[-0].m_data, (count > 1) ? (long)(args[-1].m_data.num) : (long)(TimeStamp::Current()));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_strftime(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
        fh_strftime((&(rv)), &args[-0].m_data, (count > 1) ? (long)(args[-1].m_data.num) : (long)(TimeStamp::Current()));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_strftime(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("strftime", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_strptime(HPHP::String const&, HPHP::String const&)
_ZN4HPHP10f_strptimeERKNS_6StringES2_

(return value) => rax
_rv => rdi
date => rsi
format => rdx
*/

TypedValue* fh_strptime(TypedValue* _rv, Value* date, Value* format) asm("_ZN4HPHP10f_strptimeERKNS_6StringES2_");

TypedValue * fg1_strptime(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_strptime(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_strptime((rv), &args[-0].m_data, &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_strptime(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        fh_strptime((&(rv)), &args[-0].m_data, &args[-1].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_strptime(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("strptime", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_strtotime(HPHP::String const&, long)
_ZN4HPHP11f_strtotimeERKNS_6StringEl

(return value) => rax
_rv => rdi
input => rsi
timestamp => rdx
*/

TypedValue* fh_strtotime(TypedValue* _rv, Value* input, long timestamp) asm("_ZN4HPHP11f_strtotimeERKNS_6StringEl");

TypedValue * fg1_strtotime(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_strtotime(TypedValue* rv, ActRec* ar, int64_t count) {
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
  fh_strtotime((rv), &args[-0].m_data, (count > 1) ? (long)(args[-1].m_data.num) : (long)(TimeStamp::Current()));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_strtotime(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
        fh_strtotime((&(rv)), &args[-0].m_data, (count > 1) ? (long)(args[-1].m_data.num) : (long)(TimeStamp::Current()));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_strtotime(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("strtotime", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_time()
_ZN4HPHP6f_timeEv

(return value) => rax
*/

long fh_time() asm("_ZN4HPHP6f_timeEv");

TypedValue* fg_time(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_type = KindOfInt64;
      rv.m_data.num = (int64_t)fh_time();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("time", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_timezone_abbreviations_list()
_ZN4HPHP29f_timezone_abbreviations_listEv

(return value) => rax
_rv => rdi
*/

Value* fh_timezone_abbreviations_list(Value* _rv) asm("_ZN4HPHP29f_timezone_abbreviations_listEv");

TypedValue* fg_timezone_abbreviations_list(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_type = KindOfArray;
      fh_timezone_abbreviations_list((&rv.m_data));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("timezone_abbreviations_list", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_timezone_identifiers_list()
_ZN4HPHP27f_timezone_identifiers_listEv

(return value) => rax
_rv => rdi
*/

Value* fh_timezone_identifiers_list(Value* _rv) asm("_ZN4HPHP27f_timezone_identifiers_listEv");

TypedValue* fg_timezone_identifiers_list(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_type = KindOfArray;
      fh_timezone_identifiers_list((&rv.m_data));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("timezone_identifiers_list", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_timezone_location_get(HPHP::Object const&)
_ZN4HPHP23f_timezone_location_getERKNS_6ObjectE

(return value) => rax
_rv => rdi
timezone => rsi
*/

Value* fh_timezone_location_get(Value* _rv, Value* timezone) asm("_ZN4HPHP23f_timezone_location_getERKNS_6ObjectE");

TypedValue * fg1_timezone_location_get(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_timezone_location_get(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfArray;
  tvCastToObjectInPlace(args-0);
  fh_timezone_location_get((&rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_timezone_location_get(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfArray;
        fh_timezone_location_get((&rv.m_data), &args[-0].m_data);
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_timezone_location_get(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("timezone_location_get", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_timezone_name_from_abbr(HPHP::String const&, int, bool)
_ZN4HPHP25f_timezone_name_from_abbrERKNS_6StringEib

(return value) => rax
_rv => rdi
abbr => rsi
gmtoffset => rdx
isdst => rcx
*/

TypedValue* fh_timezone_name_from_abbr(TypedValue* _rv, Value* abbr, int gmtoffset, bool isdst) asm("_ZN4HPHP25f_timezone_name_from_abbrERKNS_6StringEib");

TypedValue * fg1_timezone_name_from_abbr(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_timezone_name_from_abbr(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-2);
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
  fh_timezone_name_from_abbr((rv), &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(-1), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(true));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_timezone_name_from_abbr(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfBoolean) && (count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
        fh_timezone_name_from_abbr((&(rv)), &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(-1), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(true));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_timezone_name_from_abbr(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("timezone_name_from_abbr", count, 1, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_timezone_name_get(HPHP::Object const&)
_ZN4HPHP19f_timezone_name_getERKNS_6ObjectE

(return value) => rax
_rv => rdi
object => rsi
*/

Value* fh_timezone_name_get(Value* _rv, Value* object) asm("_ZN4HPHP19f_timezone_name_getERKNS_6ObjectE");

TypedValue * fg1_timezone_name_get(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_timezone_name_get(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfString;
  tvCastToObjectInPlace(args-0);
  fh_timezone_name_get((&rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_timezone_name_get(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfString;
        fh_timezone_name_get((&rv.m_data), &args[-0].m_data);
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_timezone_name_get(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("timezone_name_get", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_timezone_offset_get(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP21f_timezone_offset_getERKNS_6ObjectES2_

(return value) => rax
object => rdi
dt => rsi
*/

long fh_timezone_offset_get(Value* object, Value* dt) asm("_ZN4HPHP21f_timezone_offset_getERKNS_6ObjectES2_");

TypedValue * fg1_timezone_offset_get(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_timezone_offset_get(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfInt64;
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (int64_t)fh_timezone_offset_get(&args[-0].m_data, &args[-1].m_data);
  return rv;
}

TypedValue* fg_timezone_offset_get(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_timezone_offset_get(&args[-0].m_data, &args[-1].m_data);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_timezone_offset_get(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("timezone_offset_get", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Object HPHP::f_timezone_open(HPHP::String const&)
_ZN4HPHP15f_timezone_openERKNS_6StringE

(return value) => rax
_rv => rdi
timezone => rsi
*/

Value* fh_timezone_open(Value* _rv, Value* timezone) asm("_ZN4HPHP15f_timezone_openERKNS_6StringE");

TypedValue * fg1_timezone_open(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_timezone_open(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  tvCastToStringInPlace(args-0);
  fh_timezone_open((&rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_timezone_open(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv.m_type = KindOfObject;
        fh_timezone_open((&rv.m_data), &args[-0].m_data);
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_timezone_open(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("timezone_open", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_timezone_transitions_get(HPHP::Object const&)
_ZN4HPHP26f_timezone_transitions_getERKNS_6ObjectE

(return value) => rax
_rv => rdi
object => rsi
*/

Value* fh_timezone_transitions_get(Value* _rv, Value* object) asm("_ZN4HPHP26f_timezone_transitions_getERKNS_6ObjectE");

TypedValue * fg1_timezone_transitions_get(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_timezone_transitions_get(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfArray;
  tvCastToObjectInPlace(args-0);
  fh_timezone_transitions_get((&rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_timezone_transitions_get(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfArray;
        fh_timezone_transitions_get((&rv.m_data), &args[-0].m_data);
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_timezone_transitions_get(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("timezone_transitions_get", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_timezone_version_get()
_ZN4HPHP22f_timezone_version_getEv

(return value) => rax
_rv => rdi
*/

Value* fh_timezone_version_get(Value* _rv) asm("_ZN4HPHP22f_timezone_version_getEv");

TypedValue* fg_timezone_version_get(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_type = KindOfString;
      fh_timezone_version_get((&rv.m_data));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("timezone_version_get", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Object HPHP::c_DateTime::t_add(HPHP::Object const&)
_ZN4HPHP10c_DateTime5t_addERKNS_6ObjectE

(return value) => rax
_rv => rdi
this_ => rsi
interval => rdx
*/

Value* th_8DateTime_add(Value* _rv, ObjectData* this_, Value* interval) asm("_ZN4HPHP10c_DateTime5t_addERKNS_6ObjectE");

TypedValue* tg1_8DateTime_add(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_8DateTime_add(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  tvCastToObjectInPlace(args-0);
  th_8DateTime_add((&rv->m_data), (this_), &args[-0].m_data);
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_8DateTime_add(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if ((args-0)->m_type == KindOfObject) {
          rv.m_type = KindOfObject;
          th_8DateTime_add((&rv.m_data), (this_), &args[-0].m_data);
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
  new ((void *)inst) c_DateTime(cls);
  return inst;
}

IMPLEMENT_CLASS(DateTime);
/*
void HPHP::c_DateTime::t___construct(HPHP::String const&, HPHP::Object const&)
_ZN4HPHP10c_DateTime13t___constructERKNS_6StringERKNS_6ObjectE

this_ => rdi
time => rsi
timezone => rdx
*/

void th_8DateTime___construct(ObjectData* this_, Value* time, Value* timezone) asm("_ZN4HPHP10c_DateTime13t___constructERKNS_6StringERKNS_6ObjectE");

TypedValue* tg1_8DateTime___construct(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_8DateTime___construct(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
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
  th_8DateTime___construct((this_), (count > 0) ? &args[-0].m_data : (Value*)(&defVal0), (count > 1) ? &args[-1].m_data : (Value*)(&null_object));
  return rv;
}

TypedValue* tg_8DateTime___construct(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count <= 2LL) {
        if ((count <= 1 || (args-1)->m_type == KindOfObject) && (count <= 0 || IS_STRING_TYPE((args-0)->m_type))) {
          rv.m_data.num = 0LL;
          rv.m_type = KindOfNull;
          String defVal0 = "now";
          th_8DateTime___construct((this_), (count > 0) ? &args[-0].m_data : (Value*)(&defVal0), (count > 1) ? &args[-1].m_data : (Value*)(&null_object));
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

TypedValue* tg1_8DateTime_createFromFormat(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue* tg1_8DateTime_createFromFormat(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  th_8DateTime_createFromFormat((&rv->m_data), ("DateTime"), &args[-0].m_data, &args[-1].m_data, (count > 2) ? &args[-2].m_data : (Value*)(&null_object));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_8DateTime_createFromFormat(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfObject) && IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        rv.m_type = KindOfObject;
        th_8DateTime_createFromFormat((&rv.m_data), ("DateTime"), &args[-0].m_data, &args[-1].m_data, (count > 2) ? &args[-2].m_data : (Value*)(&null_object));
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

TypedValue* tg1_8DateTime_diff(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_8DateTime_diff(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  th_8DateTime_diff((&rv->m_data), (this_), &args[-0].m_data, (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_8DateTime_diff(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL && count <= 2LL) {
        if ((count <= 1 || (args-1)->m_type == KindOfBoolean) && (args-0)->m_type == KindOfObject) {
          rv.m_type = KindOfObject;
          th_8DateTime_diff((&rv.m_data), (this_), &args[-0].m_data, (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
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

TypedValue* tg1_8DateTime_format(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_8DateTime_format(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfString;
  tvCastToStringInPlace(args-0);
  th_8DateTime_format((&rv->m_data), (this_), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_8DateTime_format(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_type = KindOfString;
          th_8DateTime_format((&rv.m_data), (this_), &args[-0].m_data);
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

TypedValue* tg_8DateTime_getLastErrors(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_type = KindOfArray;
      th_8DateTime_getLastErrors((&rv.m_data), ("DateTime"));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("DateTime::getLastErrors", 0, 1);
    }
    rv.m_data.num = 0LL;
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

TypedValue* tg_8DateTime_getOffset(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
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

TypedValue* tg_8DateTime_getTimestamp(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
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

TypedValue* tg_8DateTime_getTimezone(ActRec *ar) {
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

TypedValue* tg1_8DateTime_modify(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_8DateTime_modify(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  tvCastToStringInPlace(args-0);
  th_8DateTime_modify((&rv->m_data), (this_), &args[-0].m_data);
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_8DateTime_modify(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_type = KindOfObject;
          th_8DateTime_modify((&rv.m_data), (this_), &args[-0].m_data);
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

TypedValue* tg1_8DateTime_setDate(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_8DateTime_setDate(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  th_8DateTime_setDate((&rv->m_data), (this_), (long)(args[-0].m_data.num), (long)(args[-1].m_data.num), (long)(args[-2].m_data.num));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_8DateTime_setDate(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 3LL) {
        if ((args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfInt64) {
          rv.m_type = KindOfObject;
          th_8DateTime_setDate((&rv.m_data), (this_), (long)(args[-0].m_data.num), (long)(args[-1].m_data.num), (long)(args[-2].m_data.num));
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

TypedValue* tg1_8DateTime_setISODate(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_8DateTime_setISODate(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  th_8DateTime_setISODate((&rv->m_data), (this_), (long)(args[-0].m_data.num), (long)(args[-1].m_data.num), (count > 2) ? (long)(args[-2].m_data.num) : (long)(1));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_8DateTime_setISODate(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 2LL && count <= 3LL) {
        if ((count <= 2 || (args-2)->m_type == KindOfInt64) && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfInt64) {
          rv.m_type = KindOfObject;
          th_8DateTime_setISODate((&rv.m_data), (this_), (long)(args[-0].m_data.num), (long)(args[-1].m_data.num), (count > 2) ? (long)(args[-2].m_data.num) : (long)(1));
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

TypedValue* tg1_8DateTime_setTime(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_8DateTime_setTime(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  th_8DateTime_setTime((&rv->m_data), (this_), (long)(args[-0].m_data.num), (long)(args[-1].m_data.num), (count > 2) ? (long)(args[-2].m_data.num) : (long)(0));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_8DateTime_setTime(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 2LL && count <= 3LL) {
        if ((count <= 2 || (args-2)->m_type == KindOfInt64) && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfInt64) {
          rv.m_type = KindOfObject;
          th_8DateTime_setTime((&rv.m_data), (this_), (long)(args[-0].m_data.num), (long)(args[-1].m_data.num), (count > 2) ? (long)(args[-2].m_data.num) : (long)(0));
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

TypedValue* tg1_8DateTime_setTimestamp(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_8DateTime_setTimestamp(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  tvCastToInt64InPlace(args-0);
  th_8DateTime_setTimestamp((&rv->m_data), (this_), (long)(args[-0].m_data.num));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_8DateTime_setTimestamp(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if ((args-0)->m_type == KindOfInt64) {
          rv.m_type = KindOfObject;
          th_8DateTime_setTimestamp((&rv.m_data), (this_), (long)(args[-0].m_data.num));
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

TypedValue* tg1_8DateTime_setTimezone(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_8DateTime_setTimezone(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  tvCastToObjectInPlace(args-0);
  th_8DateTime_setTimezone((&rv->m_data), (this_), &args[-0].m_data);
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_8DateTime_setTimezone(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if ((args-0)->m_type == KindOfObject) {
          rv.m_type = KindOfObject;
          th_8DateTime_setTimezone((&rv.m_data), (this_), &args[-0].m_data);
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

TypedValue* tg1_8DateTime_sub(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_8DateTime_sub(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  tvCastToObjectInPlace(args-0);
  th_8DateTime_sub((&rv->m_data), (this_), &args[-0].m_data);
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_8DateTime_sub(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if ((args-0)->m_type == KindOfObject) {
          rv.m_type = KindOfObject;
          th_8DateTime_sub((&rv.m_data), (this_), &args[-0].m_data);
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
  new ((void *)inst) c_DateTimeZone(cls);
  return inst;
}

IMPLEMENT_CLASS(DateTimeZone);
/*
void HPHP::c_DateTimeZone::t___construct(HPHP::String const&)
_ZN4HPHP14c_DateTimeZone13t___constructERKNS_6StringE

this_ => rdi
timezone => rsi
*/

void th_12DateTimeZone___construct(ObjectData* this_, Value* timezone) asm("_ZN4HPHP14c_DateTimeZone13t___constructERKNS_6StringE");

TypedValue* tg1_12DateTimeZone___construct(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_12DateTimeZone___construct(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  tvCastToStringInPlace(args-0);
  th_12DateTimeZone___construct((this_), &args[-0].m_data);
  return rv;
}

TypedValue* tg_12DateTimeZone___construct(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_data.num = 0LL;
          rv.m_type = KindOfNull;
          th_12DateTimeZone___construct((this_), &args[-0].m_data);
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

TypedValue* tg_12DateTimeZone_getLocation(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfArray;
        th_12DateTimeZone_getLocation((&rv.m_data), (this_));
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

TypedValue* tg_12DateTimeZone_getName(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfString;
        th_12DateTimeZone_getName((&rv.m_data), (this_));
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

TypedValue* tg1_12DateTimeZone_getOffset(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_12DateTimeZone_getOffset(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfInt64;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (int64_t)th_12DateTimeZone_getOffset((this_), &args[-0].m_data);
  return rv;
}

TypedValue* tg_12DateTimeZone_getOffset(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if ((args-0)->m_type == KindOfObject) {
          rv.m_type = KindOfInt64;
          rv.m_data.num = (int64_t)th_12DateTimeZone_getOffset((this_), &args[-0].m_data);
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

TypedValue* tg_12DateTimeZone_getTransitions(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfArray;
        th_12DateTimeZone_getTransitions((&rv.m_data), (this_));
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

TypedValue* tg_12DateTimeZone_listAbbreviations(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_type = KindOfArray;
      th_12DateTimeZone_listAbbreviations((&rv.m_data), ("DateTimeZone"));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("DateTimeZone::listAbbreviations", 0, 1);
    }
    rv.m_data.num = 0LL;
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

TypedValue* tg_12DateTimeZone_listIdentifiers(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_type = KindOfArray;
      th_12DateTimeZone_listIdentifiers((&rv.m_data), ("DateTimeZone"));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("DateTimeZone::listIdentifiers", 0, 1);
    }
    rv.m_data.num = 0LL;
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
  new ((void *)inst) c_DateInterval(cls);
  return inst;
}

IMPLEMENT_CLASS(DateInterval);
/*
void HPHP::c_DateInterval::t___construct(HPHP::String const&)
_ZN4HPHP14c_DateInterval13t___constructERKNS_6StringE

this_ => rdi
interval_spec => rsi
*/

void th_12DateInterval___construct(ObjectData* this_, Value* interval_spec) asm("_ZN4HPHP14c_DateInterval13t___constructERKNS_6StringE");

TypedValue* tg1_12DateInterval___construct(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_12DateInterval___construct(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  tvCastToStringInPlace(args-0);
  th_12DateInterval___construct((this_), &args[-0].m_data);
  return rv;
}

TypedValue* tg_12DateInterval___construct(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_data.num = 0LL;
          rv.m_type = KindOfNull;
          th_12DateInterval___construct((this_), &args[-0].m_data);
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

TypedValue* tg_12DateInterval___get(ActRec *ar) {
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

TypedValue* tg_12DateInterval___set(ActRec *ar) {
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

TypedValue* tg1_12DateInterval_createFromDateString(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue* tg1_12DateInterval_createFromDateString(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  tvCastToStringInPlace(args-0);
  th_12DateInterval_createFromDateString((&rv->m_data), ("DateInterval"), &args[-0].m_data);
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_12DateInterval_createFromDateString(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv.m_type = KindOfObject;
        th_12DateInterval_createFromDateString((&rv.m_data), ("DateInterval"), &args[-0].m_data);
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

TypedValue* tg1_12DateInterval_format(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_12DateInterval_format(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfString;
  tvCastToStringInPlace(args-0);
  th_12DateInterval_format((&rv->m_data), (this_), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_12DateInterval_format(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_type = KindOfString;
          th_12DateInterval_format((&rv.m_data), (this_), &args[-0].m_data);
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
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}


} // !HPHP

