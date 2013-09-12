/*
   +----------------------------------------------------------------------+
   | Zend Engine                                                          |
   +----------------------------------------------------------------------+
   | Copyright (c) 1998-2013 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
   | Authors: Andi Gutmans <andi@zend.com>                                |
   |          Zeev Suraski <zeev@zend.com>                                |
   +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef ZEND_OPERATORS_H
#define ZEND_OPERATORS_H

#include <errno.h>
#include <math.h>
#include <assert.h>

#ifdef __GNUC__
#include <stddef.h>
#endif

#ifdef HAVE_IEEEFP_H
#include <ieeefp.h>
#endif

#include "zend_alloc.h"

#include "hphp/runtime/base/zend-strtod.h"

#define LONG_SIGN_MASK (1L << (8*sizeof(long)-1))

BEGIN_EXTERN_C()
ZEND_API inline zend_bool instanceof_function_ex(const zend_class_entry *instance_ce, const zend_class_entry *ce, zend_bool interfaces_only TSRMLS_DC) {
  return instance_ce->classof(ce);
}
ZEND_API inline zend_bool instanceof_function(const zend_class_entry *instance_ce, const zend_class_entry *ce TSRMLS_DC) {
  return instanceof_function_ex(instance_ce, ce, 0 TSRMLS_CC);
}
END_EXTERN_C()

#if ZEND_DVAL_TO_LVAL_CAST_OK
# define zend_dval_to_lval(d) ((long) (d))
#elif SIZEOF_LONG == 4
zend_always_inline long zend_dval_to_lval(double d)
{
  if (d > LONG_MAX || d < LONG_MIN) {
    double  two_pow_32 = pow(2., 32.),
        dmod;

    dmod = fmod(d, two_pow_32);
    if (dmod < 0) {
      /* we're going to make this number positive; call ceil()
       * to simulate rounding towards 0 of the negative number */
      dmod = ceil(dmod) + two_pow_32;
    }
    return (long)(unsigned long)dmod;
  }
  return (long)d;
}
#else
zend_always_inline long zend_dval_to_lval(double d)
{
  /* >= as (double)LONG_MAX is outside signed range */
  if (d >= LONG_MAX || d < LONG_MIN) {
    double  two_pow_64 = pow(2., 64.),
        dmod;

    dmod = fmod(d, two_pow_64);
    if (dmod < 0) {
      /* no need to call ceil; original double must have had no
       * fractional part, hence dmod does not have one either */
      dmod += two_pow_64;
    }
    return (long)(unsigned long)dmod;
  }
  return (long)d;
}
#endif
/* }}} */

#define ZEND_IS_DIGIT(c) ((c) >= '0' && (c) <= '9')
#define ZEND_IS_XDIGIT(c) (((c) >= 'A' && (c) <= 'F') || ((c) >= 'a' && (c) <= 'f'))

/**
 * Checks whether the string "str" with length "length" is numeric. The value
 * of allow_errors determines whether it's required to be entirely numeric, or
 * just its prefix. Leading whitespace is allowed.
 *
 * The function returns 0 if the string did not contain a valid number; IS_LONG
 * if it contained a number that fits within the range of a long; or IS_DOUBLE
 * if the number was out of long range or contained a decimal point/exponent.
 * The number's value is returned into the respective pointer, *lval or *dval,
 * if that pointer is not NULL.
 *
 * This variant also gives information if a string that represents an integer
 * could not be represented as such due to overflow. It writes 1 to oflow_info
 * if the integer is larger than LONG_MAX and -1 if it's smaller than LONG_MIN.
 */
zend_uchar is_numeric_string_ex(const char *str, int length, long *lval, double *dval, int allow_errors, int *oflow_info);
inline zend_uchar is_numeric_string(const char *str, int length, long *lval, double *dval, int allow_errors) {
  return is_numeric_string_ex(str, length, lval, dval, allow_errors, NULL);
}

BEGIN_EXTERN_C()
ZEND_API inline void _convert_to_string(zval *op ZEND_FILE_LINE_DC) {
  tvCastToStringInPlace(op);
}
ZEND_API inline void convert_to_long(zval *op) {
  tvCastToInt64InPlace(op);
}
ZEND_API inline void convert_to_double(zval *op) {
  tvCastToDoubleInPlace(op);
}
ZEND_API inline void convert_to_boolean(zval *op) {
  tvCastToBooleanInPlace(op);
}
#define convert_to_cstring(op) if (Z_TYPE_P(op) != IS_STRING) { _convert_to_cstring((op) ZEND_FILE_LINE_CC); }
#define convert_to_string(op) if (Z_TYPE_P(op) != IS_STRING) { _convert_to_string((op) ZEND_FILE_LINE_CC); }

END_EXTERN_C()

#define convert_to_ex_master(ppzv, lower_type, upper_type)  \
  if (Z_TYPE_PP(ppzv)!=IS_##upper_type) {          \
    SEPARATE_ZVAL_IF_NOT_REF(ppzv);            \
    convert_to_##lower_type(*ppzv);            \
  }

#define convert_to_boolean_ex(ppzv)  convert_to_ex_master(ppzv, boolean, BOOL)
#define convert_to_long_ex(ppzv)  convert_to_ex_master(ppzv, long, LONG)
#define convert_to_double_ex(ppzv)  convert_to_ex_master(ppzv, double, DOUBLE)
#define convert_to_string_ex(ppzv)  convert_to_ex_master(ppzv, string, STRING)
#define convert_to_array_ex(ppzv)  convert_to_ex_master(ppzv, array, ARRAY)
#define convert_to_object_ex(ppzv)  convert_to_ex_master(ppzv, object, OBJECT)
#define convert_to_null_ex(ppzv)  convert_to_ex_master(ppzv, null, NULL)

inline zval& zval_follow_ref(zval &z) {
  return (z.m_type == HPHP::KindOfRef) ? *z.m_data.pref->tv() : z;
}
inline const zval& zval_follow_ref(const zval &z) {
  return (z.m_type == HPHP::KindOfRef) ? *z.m_data.pref->tv() : z;
}

#define Z_LVAL(zval)      zval_follow_ref(zval).m_data.num
#define Z_BVAL(zval)      ((zend_bool)zval_follow_ref(zval).m_data.num)
#define Z_DVAL(zval)      zval_follow_ref(zval).m_data.dbl
#define Z_STRVAL(zval)      zval_follow_ref(zval).m_data.pstr->data()
#define Z_STRLEN(zval)      zval_follow_ref(zval).m_data.pstr->size()
#define Z_ARRVAL(zval)      zval_follow_ref(zval).m_data.parr
#define Z_OBJVAL(zval)      zval_follow_ref(zval).m_data.pobj
#define Z_OBJ_HANDLE(zval)    Z_OBJVAL(zval)->o_getId()
#define Z_OBJ_HT(zval)      Z_OBJVAL(zval)
#define Z_OBJCE(zval)      zend_get_class_entry(&(zval) TSRMLS_CC)
#define Z_OBJPROP(zval)      Z_OBJ_HT((zval))->get_properties(&(zval) TSRMLS_CC)
#define Z_OBJ_HANDLER(zval, hf) Z_OBJ_HT((zval))->hf
#define Z_RESVAL(zval)      zval_get_resource_id(zval)
#define Z_OBJDEBUG(zval,is_tmp)  (Z_OBJ_HANDLER((zval),get_debug_info)?Z_OBJ_HANDLER((zval),get_debug_info)(&(zval),&is_tmp TSRMLS_CC):(is_tmp=0,Z_OBJ_HANDLER((zval),get_properties)?Z_OBJPROP(zval):NULL))

#define Z_LVAL_P(zval_p)    Z_LVAL(*zval_p)
#define Z_BVAL_P(zval_p)    Z_BVAL(*zval_p)
#define Z_DVAL_P(zval_p)    Z_DVAL(*zval_p)
#define Z_STRVAL_P(zval_p)    Z_STRVAL(*zval_p)
#define Z_STRLEN_P(zval_p)    Z_STRLEN(*zval_p)
#define Z_ARRVAL_P(zval_p)    Z_ARRVAL(*zval_p)
#define Z_OBJPROP_P(zval_p)    Z_OBJPROP(*zval_p)
#define Z_OBJCE_P(zval_p)    Z_OBJCE(*zval_p)
#define Z_RESVAL_P(zval_p)    Z_RESVAL(*zval_p)
#define Z_OBJVAL_P(zval_p)    Z_OBJVAL(*zval_p)
#define Z_OBJ_HANDLE_P(zval_p)  Z_OBJ_HANDLE(*zval_p)
#define Z_OBJ_HT_P(zval_p)    Z_OBJ_HT(*zval_p)
#define Z_OBJ_HANDLER_P(zval_p, h)  Z_OBJ_HANDLER(*zval_p, h)
#define Z_OBJDEBUG_P(zval_p,is_tmp)  Z_OBJDEBUG(*zval_p,is_tmp)

#define Z_LVAL_PP(zval_pp)    Z_LVAL(**zval_pp)
#define Z_BVAL_PP(zval_pp)    Z_BVAL(**zval_pp)
#define Z_DVAL_PP(zval_pp)    Z_DVAL(**zval_pp)
#define Z_STRVAL_PP(zval_pp)  Z_STRVAL(**zval_pp)
#define Z_STRLEN_PP(zval_pp)  Z_STRLEN(**zval_pp)
#define Z_ARRVAL_PP(zval_pp)  Z_ARRVAL(**zval_pp)
#define Z_OBJPROP_PP(zval_pp)  Z_OBJPROP(**zval_pp)
#define Z_OBJCE_PP(zval_pp)    Z_OBJCE(**zval_pp)
#define Z_RESVAL_PP(zval_pp)  Z_RESVAL(**zval_pp)
#define Z_OBJVAL_PP(zval_pp)  Z_OBJVAL(**zval_pp)
#define Z_OBJ_HANDLE_PP(zval_p)  Z_OBJ_HANDLE(**zval_p)
#define Z_OBJ_HT_PP(zval_p)    Z_OBJ_HT(**zval_p)
#define Z_OBJ_HANDLER_PP(zval_p, h)    Z_OBJ_HANDLER(**zval_p, h)
#define Z_OBJDEBUG_PP(zval_pp,is_tmp)  Z_OBJDEBUG(**zval_pp,is_tmp)

HPHP::DataType& Z_TYPE(const zval& z);
#define Z_TYPE_P(zval_p)  Z_TYPE(*zval_p)
#define Z_TYPE_PP(zval_pp)  Z_TYPE(**zval_pp)

#endif
