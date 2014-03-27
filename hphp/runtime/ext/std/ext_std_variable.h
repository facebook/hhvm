/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_VARIABLE_H_
#define incl_HPHP_VARIABLE_H_

#include "hphp/runtime/ext/std/ext_std.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// type testing

bool HHVM_FUNCTION(is_bool, const Variant& v);
bool HHVM_FUNCTION(is_int, const Variant& v);
bool HHVM_FUNCTION(is_float, const Variant& v);
bool HHVM_FUNCTION(is_numeric, const Variant& v);
bool HHVM_FUNCTION(is_string, const Variant& v);
bool HHVM_FUNCTION(is_scalar, const Variant& v);
bool HHVM_FUNCTION(is_array, const Variant& v);
bool HHVM_FUNCTION(is_object, const Variant& v);
bool HHVM_FUNCTION(is_resource, const Variant& v);

String HHVM_FUNCTION(gettype, const Variant& v);
String HHVM_FUNCTION(get_resource_type, const Resource& handle);

///////////////////////////////////////////////////////////////////////////////
// type conversion

bool HHVM_FUNCTION(settype, VRefParam var, const String& type);

///////////////////////////////////////////////////////////////////////////////
// input/output

Variant HHVM_FUNCTION(print_r, const Variant& expression, bool ret = false);
Variant HHVM_FUNCTION(var_export, const Variant& expression, bool ret = false);
void f_var_dump(const Variant& v);
void f_var_dump(int _argc, const Variant& expression,
                const Array& _argv /* = null_array */);
void HHVM_FUNCTION(debug_zval_dump, const Variant& variable);
String HHVM_FUNCTION(serialize, const Variant& value);
Variant HHVM_FUNCTION(unserialize, const String& str,
                      const Array& class_whitelist = empty_array);

///////////////////////////////////////////////////////////////////////////////
// variable table

Array HHVM_FUNCTION(get_defined_vars);

#define EXTR_OVERWRITE          0
#define EXTR_SKIP               1
#define EXTR_PREFIX_SAME        2
#define EXTR_PREFIX_ALL         3
#define EXTR_PREFIX_INVALID     4
#define EXTR_PREFIX_IF_EXISTS   5
#define EXTR_IF_EXISTS          6
#define EXTR_REFS               0x100

int64_t HHVM_FUNCTION(extract, const Array& var_array,
                               int extract_type = EXTR_OVERWRITE,
                               const String& prefix = empty_string);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_VARIABLE_H_
