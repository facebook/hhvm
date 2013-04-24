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
bool HPHP::f_is_bool(HPHP::Variant const&)
_ZN4HPHP9f_is_boolERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_bool(TypedValue* var) asm("_ZN4HPHP9f_is_boolERKNS_7VariantE");

TypedValue* fg_is_bool(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv.m_type = KindOfBoolean;
      rv.m_data.num = (fh_is_bool((args-0))) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("is_bool", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_is_int(HPHP::Variant const&)
_ZN4HPHP8f_is_intERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_int(TypedValue* var) asm("_ZN4HPHP8f_is_intERKNS_7VariantE");

TypedValue* fg_is_int(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv.m_type = KindOfBoolean;
      rv.m_data.num = (fh_is_int((args-0))) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("is_int", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_is_integer(HPHP::Variant const&)
_ZN4HPHP12f_is_integerERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_integer(TypedValue* var) asm("_ZN4HPHP12f_is_integerERKNS_7VariantE");

TypedValue* fg_is_integer(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv.m_type = KindOfBoolean;
      rv.m_data.num = (fh_is_integer((args-0))) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("is_integer", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_is_long(HPHP::Variant const&)
_ZN4HPHP9f_is_longERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_long(TypedValue* var) asm("_ZN4HPHP9f_is_longERKNS_7VariantE");

TypedValue* fg_is_long(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv.m_type = KindOfBoolean;
      rv.m_data.num = (fh_is_long((args-0))) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("is_long", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_is_double(HPHP::Variant const&)
_ZN4HPHP11f_is_doubleERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_double(TypedValue* var) asm("_ZN4HPHP11f_is_doubleERKNS_7VariantE");

TypedValue* fg_is_double(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv.m_type = KindOfBoolean;
      rv.m_data.num = (fh_is_double((args-0))) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("is_double", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_is_float(HPHP::Variant const&)
_ZN4HPHP10f_is_floatERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_float(TypedValue* var) asm("_ZN4HPHP10f_is_floatERKNS_7VariantE");

TypedValue* fg_is_float(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv.m_type = KindOfBoolean;
      rv.m_data.num = (fh_is_float((args-0))) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("is_float", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_is_numeric(HPHP::Variant const&)
_ZN4HPHP12f_is_numericERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_numeric(TypedValue* var) asm("_ZN4HPHP12f_is_numericERKNS_7VariantE");

TypedValue* fg_is_numeric(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv.m_type = KindOfBoolean;
      rv.m_data.num = (fh_is_numeric((args-0))) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("is_numeric", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_is_real(HPHP::Variant const&)
_ZN4HPHP9f_is_realERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_real(TypedValue* var) asm("_ZN4HPHP9f_is_realERKNS_7VariantE");

TypedValue* fg_is_real(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv.m_type = KindOfBoolean;
      rv.m_data.num = (fh_is_real((args-0))) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("is_real", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_is_string(HPHP::Variant const&)
_ZN4HPHP11f_is_stringERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_string(TypedValue* var) asm("_ZN4HPHP11f_is_stringERKNS_7VariantE");

TypedValue* fg_is_string(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv.m_type = KindOfBoolean;
      rv.m_data.num = (fh_is_string((args-0))) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("is_string", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_is_scalar(HPHP::Variant const&)
_ZN4HPHP11f_is_scalarERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_scalar(TypedValue* var) asm("_ZN4HPHP11f_is_scalarERKNS_7VariantE");

TypedValue* fg_is_scalar(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv.m_type = KindOfBoolean;
      rv.m_data.num = (fh_is_scalar((args-0))) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("is_scalar", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_is_array(HPHP::Variant const&)
_ZN4HPHP10f_is_arrayERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_array(TypedValue* var) asm("_ZN4HPHP10f_is_arrayERKNS_7VariantE");

TypedValue* fg_is_array(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv.m_type = KindOfBoolean;
      rv.m_data.num = (fh_is_array((args-0))) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("is_array", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_is_object(HPHP::Variant const&)
_ZN4HPHP11f_is_objectERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_object(TypedValue* var) asm("_ZN4HPHP11f_is_objectERKNS_7VariantE");

TypedValue* fg_is_object(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv.m_type = KindOfBoolean;
      rv.m_data.num = (fh_is_object((args-0))) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("is_object", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_is_resource(HPHP::Variant const&)
_ZN4HPHP13f_is_resourceERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_resource(TypedValue* var) asm("_ZN4HPHP13f_is_resourceERKNS_7VariantE");

TypedValue* fg_is_resource(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv.m_type = KindOfBoolean;
      rv.m_data.num = (fh_is_resource((args-0))) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("is_resource", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_is_null(HPHP::Variant const&)
_ZN4HPHP9f_is_nullERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_null(TypedValue* var) asm("_ZN4HPHP9f_is_nullERKNS_7VariantE");

TypedValue* fg_is_null(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv.m_type = KindOfBoolean;
      rv.m_data.num = (fh_is_null((args-0))) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("is_null", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_gettype(HPHP::Variant const&)
_ZN4HPHP9f_gettypeERKNS_7VariantE

(return value) => rax
_rv => rdi
v => rsi
*/

Value* fh_gettype(Value* _rv, TypedValue* v) asm("_ZN4HPHP9f_gettypeERKNS_7VariantE");

TypedValue* fg_gettype(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv.m_type = KindOfString;
      fh_gettype((&rv.m_data), (args-0));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("gettype", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_get_resource_type(HPHP::Object const&)
_ZN4HPHP19f_get_resource_typeERKNS_6ObjectE

(return value) => rax
_rv => rdi
handle => rsi
*/

Value* fh_get_resource_type(Value* _rv, Value* handle) asm("_ZN4HPHP19f_get_resource_typeERKNS_6ObjectE");

TypedValue * fg1_get_resource_type(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_get_resource_type(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfString;
  tvCastToObjectInPlace(args-0);
  fh_get_resource_type((&rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_get_resource_type(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfString;
        fh_get_resource_type((&rv.m_data), &args[-0].m_data);
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_get_resource_type(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("get_resource_type", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_intval(HPHP::Variant const&, long)
_ZN4HPHP8f_intvalERKNS_7VariantEl

(return value) => rax
v => rdi
base => rsi
*/

long fh_intval(TypedValue* v, long base) asm("_ZN4HPHP8f_intvalERKNS_7VariantEl");

TypedValue * fg1_intval(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_intval(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfInt64;
  tvCastToInt64InPlace(args-1);
  rv->m_data.num = (int64_t)fh_intval((args-0), (count > 1) ? (long)(args[-1].m_data.num) : (long)(10));
  return rv;
}

TypedValue* fg_intval(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64)) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_intval((args-0), (count > 1) ? (long)(args[-1].m_data.num) : (long)(10));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_intval(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("intval", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_doubleval(HPHP::Variant const&)
_ZN4HPHP11f_doublevalERKNS_7VariantE

(return value) => xmm0
v => rdi
*/

double fh_doubleval(TypedValue* v) asm("_ZN4HPHP11f_doublevalERKNS_7VariantE");

TypedValue* fg_doubleval(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv.m_type = KindOfDouble;
      rv.m_data.dbl = fh_doubleval((args-0));
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("doubleval", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_floatval(HPHP::Variant const&)
_ZN4HPHP10f_floatvalERKNS_7VariantE

(return value) => xmm0
v => rdi
*/

double fh_floatval(TypedValue* v) asm("_ZN4HPHP10f_floatvalERKNS_7VariantE");

TypedValue* fg_floatval(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv.m_type = KindOfDouble;
      rv.m_data.dbl = fh_floatval((args-0));
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("floatval", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_strval(HPHP::Variant const&)
_ZN4HPHP8f_strvalERKNS_7VariantE

(return value) => rax
_rv => rdi
v => rsi
*/

Value* fh_strval(Value* _rv, TypedValue* v) asm("_ZN4HPHP8f_strvalERKNS_7VariantE");

TypedValue* fg_strval(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv.m_type = KindOfString;
      fh_strval((&rv.m_data), (args-0));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("strval", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_settype(HPHP::VRefParamValue const&, HPHP::String const&)
_ZN4HPHP9f_settypeERKNS_14VRefParamValueERKNS_6StringE

(return value) => rax
var => rdi
type => rsi
*/

bool fh_settype(TypedValue* var, Value* type) asm("_ZN4HPHP9f_settypeERKNS_14VRefParamValueERKNS_6StringE");

TypedValue * fg1_settype(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_settype(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToStringInPlace(args-1);
  rv->m_data.num = (fh_settype((args-0), &args[-1].m_data)) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_settype(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type)) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_settype((args-0), &args[-1].m_data)) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_settype(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("settype", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_print_r(HPHP::Variant const&, bool)
_ZN4HPHP9f_print_rERKNS_7VariantEb

(return value) => rax
_rv => rdi
expression => rsi
ret => rdx
*/

TypedValue* fh_print_r(TypedValue* _rv, TypedValue* expression, bool ret) asm("_ZN4HPHP9f_print_rERKNS_7VariantEb");

TypedValue * fg1_print_r(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_print_r(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToBooleanInPlace(args-1);
  fh_print_r((rv), (args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_print_r(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfBoolean)) {
        fh_print_r((&(rv)), (args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_print_r(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("print_r", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_var_export(HPHP::Variant const&, bool)
_ZN4HPHP12f_var_exportERKNS_7VariantEb

(return value) => rax
_rv => rdi
expression => rsi
ret => rdx
*/

TypedValue* fh_var_export(TypedValue* _rv, TypedValue* expression, bool ret) asm("_ZN4HPHP12f_var_exportERKNS_7VariantEb");

TypedValue * fg1_var_export(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_var_export(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToBooleanInPlace(args-1);
  fh_var_export((rv), (args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_var_export(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfBoolean)) {
        fh_var_export((&(rv)), (args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_var_export(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("var_export", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_var_dump(int, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP10f_var_dumpEiRKNS_7VariantERKNS_5ArrayE

_argc => rdi
expression => rsi
_argv => rdx
*/

void fh_var_dump(int64_t _argc, TypedValue* expression, Value* _argv) asm("_ZN4HPHP10f_var_dumpEiRKNS_7VariantERKNS_5ArrayE");

TypedValue* fg_var_dump(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL) {
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
      fh_var_dump((count), (args-0), (Value*)(&extraArgs));
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_missing_arguments_nr("var_dump", 1, count, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_debug_zval_dump(HPHP::Variant const&)
_ZN4HPHP17f_debug_zval_dumpERKNS_7VariantE

variable => rdi
*/

void fh_debug_zval_dump(TypedValue* variable) asm("_ZN4HPHP17f_debug_zval_dumpERKNS_7VariantE");

TypedValue* fg_debug_zval_dump(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv.m_data.num = 0LL;
      rv.m_type = KindOfNull;
      fh_debug_zval_dump((args-0));
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("debug_zval_dump", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_unserialize(HPHP::String const&, HPHP::Array const&)
_ZN4HPHP13f_unserializeERKNS_6StringERKNS_5ArrayE

(return value) => rax
_rv => rdi
str => rsi
class_whitelist => rdx
*/

TypedValue* fh_unserialize(TypedValue* _rv, Value* str, Value* class_whitelist) asm("_ZN4HPHP13f_unserializeERKNS_6StringERKNS_5ArrayE");

TypedValue * fg1_unserialize(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_unserialize(TypedValue* rv, ActRec* ar, int64_t count) {
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
  Array defVal1 = empty_array;
  fh_unserialize((rv), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&defVal1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_unserialize(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfArray) && IS_STRING_TYPE((args-0)->m_type)) {
        Array defVal1 = empty_array;
        fh_unserialize((&(rv)), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&defVal1));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_unserialize(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("unserialize", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_get_defined_vars()
_ZN4HPHP18f_get_defined_varsEv

(return value) => rax
_rv => rdi
*/

Value* fh_get_defined_vars(Value* _rv) asm("_ZN4HPHP18f_get_defined_varsEv");

TypedValue* fg_get_defined_vars(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_type = KindOfArray;
      fh_get_defined_vars((&rv.m_data));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("get_defined_vars", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_import_request_variables(HPHP::String const&, HPHP::String const&)
_ZN4HPHP26f_import_request_variablesERKNS_6StringES2_

(return value) => rax
types => rdi
prefix => rsi
*/

bool fh_import_request_variables(Value* types, Value* prefix) asm("_ZN4HPHP26f_import_request_variablesERKNS_6StringES2_");

TypedValue * fg1_import_request_variables(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_import_request_variables(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
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
  rv->m_data.num = (fh_import_request_variables(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&empty_string))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_import_request_variables(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && IS_STRING_TYPE((args-0)->m_type)) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_import_request_variables(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&empty_string))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_import_request_variables(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("import_request_variables", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_extract(HPHP::Array const&, int, HPHP::String const&)
_ZN4HPHP9f_extractERKNS_5ArrayEiRKNS_6StringE

(return value) => rax
var_array => rdi
extract_type => rsi
prefix => rdx
*/

long fh_extract(Value* var_array, int extract_type, Value* prefix) asm("_ZN4HPHP9f_extractERKNS_5ArrayEiRKNS_6StringE");

TypedValue * fg1_extract(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_extract(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfInt64;
  switch (count) {
  default: // count >= 3
    if (!IS_STRING_TYPE((args-2)->m_type)) {
      tvCastToStringInPlace(args-2);
    }
  case 2:
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
    break;
  }
  if ((args-0)->m_type != KindOfArray) {
    tvCastToArrayInPlace(args-0);
  }
  rv->m_data.num = (int64_t)fh_extract(&args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(k_EXTR_OVERWRITE), (count > 2) ? &args[-2].m_data : (Value*)(&empty_string));
  return rv;
}

TypedValue* fg_extract(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 3LL) {
      if ((count <= 2 || IS_STRING_TYPE((args-2)->m_type)) && (count <= 1 || (args-1)->m_type == KindOfInt64) && (args-0)->m_type == KindOfArray) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_extract(&args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(k_EXTR_OVERWRITE), (count > 2) ? &args[-2].m_data : (Value*)(&empty_string));
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_extract(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("extract", count, 1, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}




} // !HPHP

