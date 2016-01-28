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

#ifndef incl_HPHP_EXT_FB_H_
#define incl_HPHP_EXT_FB_H_

#include "hphp/runtime/ext/extension.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Variant HHVM_FUNCTION(fb_serialize, const Variant& thing);
Variant HHVM_FUNCTION(fb_unserialize, const Variant& thing, VRefParam success);
Variant HHVM_FUNCTION(fb_compact_serialize, const Variant& thing);
Variant HHVM_FUNCTION(fb_compact_unserialize,
                      const Variant& thing, VRefParam success,
                      VRefParam errcode = null_variant);
bool HHVM_FUNCTION(fb_intercept, const String& name, const Variant& handler,
                    const Variant& data = null_variant);
bool HHVM_FUNCTION(fb_rename_function, const String& orig_func_name,
                          const String& new_func_name);
bool HHVM_FUNCTION(fb_utf8ize, VRefParam input);
int64_t HHVM_FUNCTION(fb_utf8_strlen_deprecated, const String& input);
int64_t HHVM_FUNCTION(fb_utf8_strlen, const String& input);
String HHVM_FUNCTION(fb_utf8_substr, const String& str,
                     int64_t start, int64_t length = INT_MAX);
Variant HHVM_FUNCTION(fb_get_code_coverage, bool flush);
void HHVM_FUNCTION(fb_enable_code_coverage);
Variant HHVM_FUNCTION(fb_disable_code_coverage);
bool HHVM_FUNCTION(fb_output_compression, bool new_value);
void HHVM_FUNCTION(fb_set_exit_callback, const Variant& function);
int64_t HHVM_FUNCTION(fb_get_last_flush_size);
Variant HHVM_FUNCTION(fb_lazy_lstat, const String& filename);
String HHVM_FUNCTION(fb_lazy_realpath, const String& filename);

Array HHVM_FUNCTION(fb_call_user_func_safe,
                    const Variant& function,
                    const Array& argv);
Variant HHVM_FUNCTION(fb_call_user_func_safe_return,
                      const Variant& function,
                      const Variant& def,
                      const Array& argv);
Array HHVM_FUNCTION(fb_call_user_func_array_safe,
                    const Variant& function,
                    const Array& params);

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
