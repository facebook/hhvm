/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#ifndef __EXT_CLASS_H__
#define __EXT_CLASS_H__

#include <runtime/base/base_includes.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Array f_get_declared_classes();
Array f_get_declared_interfaces();
bool f_class_exists(CStrRef class_name, bool autoload = true);
bool f_interface_exists(CStrRef interface_name, bool autoload = true);
Array f_get_class_methods(CVarRef class_or_object);
Array f_get_class_vars(CStrRef class_name);

///////////////////////////////////////////////////////////////////////////////

Variant f_get_class(CVarRef object = null_variant);
Variant f_get_parent_class(CVarRef object = null_variant);
bool f_is_a(CObjRef object, CStrRef class_name);
bool f_is_subclass_of(CVarRef class_or_object, CStrRef class_name);
bool f_method_exists(CVarRef class_or_object, CStrRef method_name);
bool f_property_exists(CVarRef class_or_object, CStrRef property);
Array f_get_object_vars(CObjRef object);

///////////////////////////////////////////////////////////////////////////////

Variant f_call_user_method_array(CStrRef method_name, Variant obj,
                                 CArrRef paramarr);
Variant f_call_user_method(int _argc, CStrRef method_name, Variant obj, CArrRef _argv = null_array);

///////////////////////////////////////////////////////////////////////////////
}

#endif // __EXT_CLASS_H__
