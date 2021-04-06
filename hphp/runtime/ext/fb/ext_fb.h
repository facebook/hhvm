/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#pragma once

#include "hphp/runtime/ext/extension.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

extern const int64_t k_FB_SERIALIZE_HACK_ARRAYS;
extern const int64_t k_FB_SERIALIZE_HACK_ARRAYS_AND_KEYSETS;
extern const int64_t k_FB_SERIALIZE_VARRAY_DARRAY;
extern const int64_t k_FB_SERIALIZE_POST_HACK_ARRAY_MIGRATION;

Variant HHVM_FUNCTION(fb_serialize, const Variant& thing, int64_t options = 0);
Variant HHVM_FUNCTION(fb_unserialize,
                      const Variant& thing,
                      bool& success,
                      int64_t options = 0);
Variant HHVM_FUNCTION(
    fb_compact_serialize, const Variant& thing, int64_t options = 0);
Variant HHVM_FUNCTION(fb_compact_unserialize,
                      const Variant& thing, bool& success,
                      Variant& errcode);
bool HHVM_FUNCTION(fb_intercept, const String& name, const Variant& handler,
                   const Variant& data = uninit_variant);
bool HHVM_FUNCTION(fb_intercept2, const String& name, const Variant& handler);
bool HHVM_FUNCTION(fb_rename_function, const String& orig_func_name,
                          const String& new_func_name);
bool HHVM_FUNCTION(fb_utf8ize, Variant& input);
int64_t HHVM_FUNCTION(fb_utf8_strlen_deprecated, const String& input);
int64_t HHVM_FUNCTION(fb_utf8_strlen, const String& input);
String HHVM_FUNCTION(fb_utf8_substr, const String& str,
                     int64_t start, int64_t length = INT_MAX);
Variant HHVM_FUNCTION(fb_get_code_coverage, bool flush);
void HHVM_FUNCTION(fb_enable_code_coverage);
Array HHVM_FUNCTION(fb_disable_code_coverage);
Array HHVM_FUNCTION(HH_disable_code_coverage_with_frequency);
bool HHVM_FUNCTION(fb_output_compression, bool new_value);
void HHVM_FUNCTION(fb_set_exit_callback, const Variant& function);
int64_t HHVM_FUNCTION(fb_get_last_flush_size);
Variant HHVM_FUNCTION(fb_lazy_lstat, const String& filename);
Variant HHVM_FUNCTION(fb_lazy_realpath, const String& filename);
int64_t HHVM_FUNCTION(HH_non_crypto_md5_upper, StringArg str);
int64_t HHVM_FUNCTION(HH_non_crypto_md5_lower, StringArg str);
int64_t HHVM_FUNCTION(HH_int_mul_overflow, int64_t a, int64_t b);
int64_t HHVM_FUNCTION(HH_int_mul_add_overflow,
                      int64_t a, int64_t b, int64_t bias);

///////////////////////////////////////////////////////////////////////////////

Variant fb_unserialize(const char* str,
                       int len,
                       bool& success,
                       int64_t options);
String fb_compact_serialize(const Variant& thing, int64_t options);
Variant fb_compact_unserialize(const char* str, int len,
                               bool& success,
                               Variant& errcode);

///////////////////////////////////////////////////////////////////////////////
}

