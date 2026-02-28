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

#include "hphp/runtime/ext/std/ext_std.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

bool HHVM_FUNCTION(class_exists, const String& class_name,
                                 bool autoload = true);
// If allow_str_cls is true, class_or_object can be a class or a classname.
// Otherwise, it must be an object.
bool HHVM_FUNCTION(is_a, const Variant& class_or_object,
                         const String& class_name,
                         bool allow_str_cls = false);
bool HHVM_FUNCTION(is_subclass_of, const Variant& class_or_object,
                                   const String& class_name,
                                   bool allow_str_cls = true);
Array HHVM_FUNCTION(get_object_vars, const Object& object);

Func* getFuncFromMethCallerFunc(const Func*);
Func* getFuncFromMethCallerHelperClass(const ObjectData*);
Func* getFuncFromDynMethCallerHelperClass(const ObjectData*);

///////////////////////////////////////////////////////////////////////////////
}
