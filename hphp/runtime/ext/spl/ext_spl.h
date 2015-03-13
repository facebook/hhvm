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

#ifndef incl_HPHP_EXT_SPL_H_
#define incl_HPHP_EXT_SPL_H_

#include "hphp/runtime/ext/extension.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

String HHVM_FUNCTION(spl_object_hash, const Object& obj);
int64_t HHVM_FUNCTION(hphp_object_pointer, const Object& obj);
Variant HHVM_FUNCTION(hphp_get_this);
Variant HHVM_FUNCTION(class_implements, const Variant& obj,
                                        bool autoload = true);
Variant HHVM_FUNCTION(class_parents, const Variant& obj, bool autoload = true);
Variant HHVM_FUNCTION(class_uses, const Variant& obj, bool autoload = true);
void HHVM_FUNCTION(spl_autoload_call, const String& class_name);
String HHVM_FUNCTION(spl_autoload_extensions, const String& file_extensions =
                                                null_string);
Variant HHVM_FUNCTION(spl_autoload_functions);
Variant HHVM_FUNCTION(iterator_apply, const Variant& obj, const Variant& func,
                                      const Array& params = null_array);
Variant HHVM_FUNCTION(iterator_count, const Variant& obj);
Array HHVM_FUNCTION(iterator_to_array, const Variant& obj,
                                         bool use_keys = true);
bool HHVM_FUNCTION(spl_autoload_register,
                   const Variant& autoload_function = null_variant,
                   bool throws = true,
                   bool prepend = false
                  );
bool HHVM_FUNCTION(spl_autoload_unregister, const Variant& autoload_function);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_SPL_H_
