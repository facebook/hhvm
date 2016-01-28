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

#ifndef incl_HPHP_EXT_CLASS_H_
#define incl_HPHP_EXT_CLASS_H_

#include "hphp/runtime/ext/std/ext_std.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Array HHVM_FUNCTION(get_declared_classes);
Array HHVM_FUNCTION(get_declared_interfaces);
Array HHVM_FUNCTION(get_declared_traits);
bool HHVM_FUNCTION(class_alias, const String& original, const String& alias,
                                bool autoload = true);
bool HHVM_FUNCTION(class_exists, const String& class_name,
                                 bool autoload = true);
bool HHVM_FUNCTION(interface_exists, const String& interface_name,
                                     bool autoload = true);
bool HHVM_FUNCTION(trait_exists, const String& trait_name,
                                 bool autoload = true);
Variant HHVM_FUNCTION(get_class_methods, const Variant& class_or_object);
Array HHVM_FUNCTION(get_class_constants, const String& className);
Variant HHVM_FUNCTION(get_class_vars, const String& className);
Variant HHVM_FUNCTION(get_class, const Variant& object = null_variant);
Variant HHVM_FUNCTION(get_called_class);
Variant HHVM_FUNCTION(get_parent_class, const Variant& object = null_variant);
bool HHVM_FUNCTION(is_a, const Variant& class_or_object,
                         const String& class_name,
                         bool allow_string = false);
bool HHVM_FUNCTION(is_subclass_of, const Variant& class_or_object,
                                   const String& class_name,
                                   bool allow_string = true);
bool HHVM_FUNCTION(method_exists, const Variant& class_or_object,
                                  const String& method_name);
Variant HHVM_FUNCTION(property_exists, const Variant& class_or_object,
                                       const String& property);
Array HHVM_FUNCTION(get_object_vars, const Object& object);
Variant HHVM_FUNCTION(call_user_method_array, const String& method_name,
                                              VRefParam obj,
                                              const Variant& paramarr);

void getMethodNames(Class* cls, Class* ctx, Array& result);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_CLASS_H_
