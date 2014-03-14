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
#ifndef HPHP_EXT_FILTER_FILTER_PRIVATE_H
#define HPHP_EXT_FILTER_FILTER_PRIVATE_H

#include "hphp/runtime/base/types.h"

#define PHP_INPUT_FILTER_PARAM_DECL const String& value, long flags, \
                                    const Variant& option_array

#define RETURN_VALIDATION_FAILED    \
  if (flags & k_FILTER_NULL_ON_FAILURE) {   \
    return uninit_null();   \
  } else {    \
    return false;  \
  }

#define PHP_FILTER_TRIM_DEFAULT(p, len) PHP_FILTER_TRIM_DEFAULT_EX(p, len, 1);

#define PHP_FILTER_TRIM_DEFAULT_EX(p, len, return_if_empty) { \
  while ((len > 0)  && (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\v' || \
         *p == '\n')) { \
    p++; \
    len--; \
  } \
  if (len < 1 && return_if_empty) { \
    RETURN_VALIDATION_FAILED \
  } \
  if (len > 0) { \
    while (p[len-1] == ' ' || p[len-1] == '\t' || p[len-1] == '\r' || \
           p[len-1] == '\v' || p[len-1] == '\n') { \
      len--; \
    } \
  } \
}

#define FETCH_STRING_OPTION(var_name, option_name)       \
  var_name = nullptr;                                    \
  var_name##_set = 0;                                    \
  var_name##_len = 0;                                    \
  if (option_array.isArray() &&                          \
      !option_array.toArray().empty()) {                 \
    const Array& option_array_arr = option_array.toArray();   \
    if (option_array_arr.exists(option_name)) {          \
      Variant option_val(option_array_arr[option_name]); \
      if (option_val.isString()) {                       \
        var_name = option_val.toString().data();         \
        var_name##_len = option_val.toString().length(); \
        var_name##_set = 1;                              \
      }                                                  \
    }                                                    \
  }

#define FETCH_LONG_OPTION(var_name, option_name)         \
  var_name = 0;                                          \
  var_name##_set = 0;                                    \
  if (option_array.isArray() &&                          \
      !option_array.toArray().empty()) {                 \
    const Array& option_array_arr = option_array.toArray();   \
    if (option_array_arr.exists(option_name)) {          \
      Variant option_val(option_array_arr[option_name]); \
      if (option_val.isInteger()) {                      \
        var_name = option_val.toInt64();                 \
        var_name##_set = 1;                              \
      }                                                  \
    }                                                    \
  }

#endif /* HPHP_EXT_FILTER_FILTER_PRIVATE_H */
