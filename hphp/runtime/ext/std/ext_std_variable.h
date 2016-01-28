/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
void HHVM_FUNCTION(var_dump,
                   const Variant& v, const Array& _argv = null_array);
void HHVM_FUNCTION(debug_zval_dump, const Variant& variable);
String HHVM_FUNCTION(serialize, const Variant& value);
Variant HHVM_FUNCTION(unserialize, const String& str,
                      const Array& options = empty_array_ref);

///////////////////////////////////////////////////////////////////////////////
// variable table

int64_t constexpr EXTR_OVERWRITE        = 0;
int64_t constexpr EXTR_SKIP             = 1;
int64_t constexpr EXTR_PREFIX_SAME      = 2;
int64_t constexpr EXTR_PREFIX_ALL       = 3;
int64_t constexpr EXTR_PREFIX_INVALID   = 4;
int64_t constexpr EXTR_PREFIX_IF_EXISTS = 5;
int64_t constexpr EXTR_IF_EXISTS        = 6;
int64_t constexpr EXTR_REFS             = 0x100;

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_VARIABLE_H_
