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

#ifndef incl_HPHP_EXT_FB_H_
#define incl_HPHP_EXT_FB_H_

#include "hphp/runtime/ext/extension.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Variant f_fb_serialize(const Variant& thing);
Variant f_fb_unserialize(const Variant& thing, VRefParam success);
Variant f_fb_compact_serialize(const Variant& thing);
Variant f_fb_compact_unserialize(const Variant& thing, VRefParam success,
                                 VRefParam errcode = null_variant);
bool f_fb_intercept(const String& name, const Variant& handler,
                    const Variant& data = null_variant);
bool f_fb_rename_function(const String& orig_func_name,
                          const String& new_func_name);
bool f_fb_utf8ize(VRefParam input);
int64_t f_fb_utf8_strlen_deprecated(const String& input);
int64_t f_fb_utf8_strlen(const String& input);
String f_fb_utf8_substr(const String& str, int start, int length = INT_MAX);
Array f_fb_call_user_func_safe(int _argc, const Variant& function,
                               const Array& _argv = null_array);
Variant f_fb_call_user_func_safe_return(
  int _argc, const Variant& function, const Variant& def, const Array& _argv = null_array);
Array f_fb_call_user_func_array_safe(const Variant& function, const Array& params);
Variant f_fb_get_code_coverage(bool flush);
void f_fb_enable_code_coverage();
Variant f_fb_disable_code_coverage();
void f_xhprof_enable(int flags = 0, const Array& args = null_array);
Variant f_xhprof_disable();
void f_xhprof_network_enable();
Variant f_xhprof_network_disable();
void f_xhprof_frame_begin(const String& name);
void f_xhprof_frame_end();
Variant f_xhprof_run_trace(const String& packedTrace, int flags);
void f_xhprof_sample_enable();
Variant f_xhprof_sample_disable();
void f_fb_setprofile(const Variant& callback);
bool f_fb_output_compression(bool new_value);
void f_fb_set_exit_callback(const Variant& function);
int64_t f_fb_get_last_flush_size();
Variant f_fb_lazy_lstat(const String& filename);
String f_fb_lazy_realpath(const String& filename);
extern const int64_t k_FB_UNSERIALIZE_NONSTRING_VALUE;
extern const int64_t k_FB_UNSERIALIZE_UNEXPECTED_END;
extern const int64_t k_FB_UNSERIALIZE_UNRECOGNIZED_OBJECT_TYPE;
extern const int64_t k_FB_UNSERIALIZE_UNEXPECTED_ARRAY_KEY_TYPE;
extern const int64_t k_XHPROF_FLAGS_NO_BUILTINS;
extern const int64_t k_XHPROF_FLAGS_CPU;
extern const int64_t k_XHPROF_FLAGS_MEMORY;
extern const int64_t k_XHPROF_FLAGS_VTSC;
extern const int64_t k_XHPROF_FLAGS_TRACE;
extern const int64_t k_XHPROF_FLAGS_MEASURE_XHPROF_DISABLE;
extern const int64_t k_XHPROF_FLAGS_MALLOC;
extern const int64_t k_XHPROF_FLAGS_I_HAVE_INFINITE_MEMORY;
extern const bool k_HHVM_FACEBOOK;

///////////////////////////////////////////////////////////////////////////////

enum FBCompactSerializeBehavior {
  Base,
  MemoizeParam,
};

Variant fb_unserialize(const char* str, int len, VRefParam success);
String fb_compact_serialize(const Variant& thing,
                            FBCompactSerializeBehavior behavior);
Variant fb_compact_unserialize(const char* str, int len,
                               VRefParam success,
                               VRefParam errcode = null_variant);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_FB_H_
