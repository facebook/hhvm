/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/base-includes.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Array f_get_declared_classes();
Array f_get_declared_interfaces();
Array f_get_declared_traits();
bool f_class_alias(CStrRef original, CStrRef alias, bool autoload = true);
bool f_class_exists(CStrRef class_name, bool autoload = true);
bool f_interface_exists(CStrRef interface_name, bool autoload = true);
bool f_trait_exists(CStrRef trait_name, bool autoload = true);
Array f_get_class_methods(CVarRef class_or_object);
Variant f_get_class_vars(CStrRef class_name);
Array f_get_class_constants(CStrRef class_name);

///////////////////////////////////////////////////////////////////////////////

Variant f_get_class(CVarRef object = null_variant);
Variant f_get_parent_class(CVarRef object = null_variant);
bool f_is_a(CVarRef class_or_object, CStrRef class_name, bool allow_string = false);
bool f_is_subclass_of(CVarRef class_or_object, CStrRef class_name, bool allow_string = true);
bool f_method_exists(CVarRef class_or_object, CStrRef method_name);
Variant f_property_exists(CVarRef class_or_object, CStrRef property);
Variant f_get_object_vars(CObjRef object);

///////////////////////////////////////////////////////////////////////////////

Variant f_call_user_method_array(CStrRef method_name, VRefParam obj,
                                 CArrRef paramarr);
Variant f_call_user_method(int _argc, CStrRef method_name, VRefParam obj, CArrRef _argv = null_array);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_CLASS_H_
