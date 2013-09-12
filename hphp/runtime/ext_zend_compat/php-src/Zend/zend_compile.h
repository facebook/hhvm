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

#ifndef ZEND_COMPILE_H
#define ZEND_COMPILE_H

#include "zend.h"

#ifdef HAVE_STDARG_H
# include <stdarg.h>
#endif

typedef struct _zend_literal {
  zval       constant;
  zend_ulong hash_value;
  zend_uint  cache_slot;
} zend_literal;

#if SIZEOF_LONG == 8
#define THIS_HASHVAL 210728972157UL
#else
#define THIS_HASHVAL 275574653UL
#endif

/* method flags (types) */
#define ZEND_ACC_STATIC      0x01
#define ZEND_ACC_ABSTRACT    0x02
#define ZEND_ACC_FINAL      0x04
#define ZEND_ACC_IMPLEMENTED_ABSTRACT    0x08

/* class flags (types) */
/* ZEND_ACC_IMPLICIT_ABSTRACT_CLASS is used for abstract classes (since it is set by any abstract method even interfaces MAY have it set, too). */
/* ZEND_ACC_EXPLICIT_ABSTRACT_CLASS denotes that a class was explicitly defined as abstract by using the keyword. */
#define ZEND_ACC_IMPLICIT_ABSTRACT_CLASS  0x10
#define ZEND_ACC_EXPLICIT_ABSTRACT_CLASS  0x20
#define ZEND_ACC_FINAL_CLASS              0x40
#define ZEND_ACC_INTERFACE                0x80
#define ZEND_ACC_TRAIT            0x120

/* op_array flags */
#define ZEND_ACC_INTERACTIVE        0x10

/* method flags (visibility) */
/* The order of those must be kept - public < protected < private */
#define ZEND_ACC_PUBLIC    0x100
#define ZEND_ACC_PROTECTED  0x200
#define ZEND_ACC_PRIVATE  0x400
#define ZEND_ACC_PPP_MASK  (ZEND_ACC_PUBLIC | ZEND_ACC_PROTECTED | ZEND_ACC_PRIVATE)

#define ZEND_ACC_CHANGED  0x800
#define ZEND_ACC_IMPLICIT_PUBLIC  0x1000

/* method flags (special method detection) */
#define ZEND_ACC_CTOR    0x2000
#define ZEND_ACC_DTOR    0x4000
#define ZEND_ACC_CLONE    0x8000

/* method flag (bc only), any method that has this flag can be used statically and non statically. */
#define ZEND_ACC_ALLOW_STATIC  0x10000

/* shadow of parent's private method/property */
#define ZEND_ACC_SHADOW 0x20000

/* deprecation flag */
#define ZEND_ACC_DEPRECATED 0x40000

/* class implement interface(s) flag */
#define ZEND_ACC_IMPLEMENT_INTERFACES 0x80000
#define ZEND_ACC_IMPLEMENT_TRAITS    0x400000

/* class constants updated */
#define ZEND_ACC_CONSTANTS_UPDATED    0x100000

/* user class has methods with static variables */
#define ZEND_HAS_STATIC_IN_METHODS    0x800000


#define ZEND_ACC_CLOSURE              0x100000
#define ZEND_ACC_GENERATOR            0x800000

/* function flag for internal user call handlers __call, __callstatic */
#define ZEND_ACC_CALL_VIA_HANDLER     0x200000

/* disable inline caching */
#define ZEND_ACC_NEVER_CACHE          0x400000

#define ZEND_ACC_PASS_REST_BY_REFERENCE 0x1000000
#define ZEND_ACC_PASS_REST_PREFER_REF  0x2000000

#define ZEND_ACC_RETURN_REFERENCE    0x4000000
#define ZEND_ACC_DONE_PASS_TWO      0x8000000

typedef struct _zend_arg_info {
  const char *name;
  zend_uint name_len;
  const char *class_name;
  zend_uint class_name_len;
  zend_uchar type_hint;
  zend_bool allow_null;
  zend_bool pass_by_reference;
} zend_arg_info;

#define ZEND_RETURN_VALUE        0
#define ZEND_RETURN_REFERENCE      1

typedef union _zend_function {
  zend_uchar type;  /* MUST be the first element of this struct! */

  struct {
    zend_uchar type;  /* never used */
    const char *function_name;
    zend_class_entry *scope;
    zend_uint fn_flags;
    union _zend_function *prototype;
    zend_uint num_args;
    zend_uint required_num_args;
    zend_arg_info *arg_info;
  } common;
} zend_function;

ZEND_API zend_bool zend_is_auto_global(const char *name, uint name_len TSRMLS_DC);

#include "zend_globals_macros.h"

/* var status for backpatching */
#define BP_VAR_R			0
#define BP_VAR_W			1
#define BP_VAR_RW			2
#define BP_VAR_IS			3
#define BP_VAR_NA			4	/* if not applicable */
#define BP_VAR_FUNC_ARG		5
#define BP_VAR_UNSET		6

#define ZEND_SEND_BY_VAL     0
#define ZEND_SEND_BY_REF     1
#define ZEND_SEND_PREFER_REF 2

#endif /* ZEND_COMPILE_H */
