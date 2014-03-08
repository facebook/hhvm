/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/base-includes.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// type testing

bool f_is_bool(const Variant& v);
bool f_is_int(const Variant& v);
bool f_is_integer(const Variant& v);
bool f_is_long(const Variant& v);
bool f_is_double(const Variant& v);
bool f_is_float(const Variant& v);
bool f_is_numeric(const Variant& v);
bool f_is_real(const Variant& v);
bool f_is_string(const Variant& v);
bool f_is_scalar(const Variant& v);
bool f_is_array(const Variant& v);
bool f_is_object(const Variant& v);
bool f_is_resource(const Variant& v);
bool f_is_null(const Variant& v);

String f_gettype(const Variant& v);
String f_get_resource_type(const Resource& handle);

///////////////////////////////////////////////////////////////////////////////
// type conversion

bool f_boolval(const Variant& v);
int64_t f_intval(const Variant& v, int64_t base = 10);
double f_doubleval(const Variant& v);
double f_floatval(const Variant& v);
String f_strval(const Variant& v);

bool f_settype(VRefParam var, const String& type);

///////////////////////////////////////////////////////////////////////////////
// input/output

Variant f_print_r(const Variant& expression, bool ret = false);
Variant f_var_export(const Variant& expression, bool ret = false);
void f_var_dump(const Variant& v);
void f_var_dump(int _argc, const Variant& expression, const Array& _argv = null_array);
void f_debug_zval_dump(const Variant& variable);
String f_serialize(const Variant& value);
Variant f_unserialize(const String& str,
                      const Array& class_whitelist = empty_array);

///////////////////////////////////////////////////////////////////////////////
// variable table

Array f_get_defined_vars();

bool f_import_request_variables(const String& types, const String& prefix = "");

#define EXTR_OVERWRITE          0
#define EXTR_SKIP               1
#define EXTR_PREFIX_SAME        2
#define EXTR_PREFIX_ALL         3
#define EXTR_PREFIX_INVALID     4
#define EXTR_PREFIX_IF_EXISTS   5
#define EXTR_IF_EXISTS          6
#define EXTR_REFS               0x100

int64_t f_extract(const Array& var_array, int extract_type = EXTR_OVERWRITE,
                  const String& prefix = "");

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_VARIABLE_H_
