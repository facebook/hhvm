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

#include <cpp/ext/ext_class.h>
#include <cpp/base/class_info.h>
#include <util/util.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// helpers

static String get_classname(Variant class_or_object) {
  if (class_or_object.is(KindOfObject)) {
    return class_or_object.toObject()->o_getClassName();
  }
  return class_or_object.toString();
}

///////////////////////////////////////////////////////////////////////////////

Array f_get_declared_classes() {
  return ClassInfo::GetClasses(true);
}

Array f_get_declared_interfaces() {
  return ClassInfo::GetInterfaces(true);
}

bool f_class_exists(CStrRef class_name, bool autoload /* = false */) {
  const ClassInfo::ClassInfo *info =
    ClassInfo::FindClass(class_name.data());
  return info && info->isDeclared();
}

bool f_interface_exists(CStrRef interface_name, bool autoload /* = false */) {
  const ClassInfo::ClassInfo *info =
    ClassInfo::FindInterface(interface_name.data());
  return info && info->isDeclared();
}

Array f_get_class_methods(CVarRef class_or_object) {
  ClassInfo::MethodVec methods;
  ClassInfo::GetClassMethods(methods, get_classname(class_or_object));

  Array ret = Array::Create();
  for (unsigned int i = 0; i < methods.size(); i++) {
    ret.append(methods[i]->name);
  }
  return ret;
}

Array f_get_class_vars(CStrRef class_name) {
  ClassInfo::PropertyMap properties;
  ClassInfo::GetClassProperties(properties, class_name);

  Array ret = Array::Create();
  for (ClassInfo::PropertyMap::const_iterator iter = properties.begin();
       iter != properties.end(); ++iter) {
    ret.append(iter->first);
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////

Variant f_get_class(CVarRef object /* = null_variant */) {
  if (!object.isObject()) return false;
  return object.toObject()->o_getClassName();
}

Variant f_get_parent_class(CVarRef object /* = null_variant */) {
  Variant class_name;
  if (object.isObject()) {
    class_name = f_get_class(object);
  } else if (object.isString()) {
    class_name = object;
  } else {
    return false;
  }
  const ClassInfo *classInfo = ClassInfo::FindClass(class_name.toString());
  if (classInfo) {
    const char *parentClass = classInfo->getParentClass();
    if (parentClass && parentClass[0]) {
      return parentClass;
    }
  }
  return false;
}

bool f_is_a(CObjRef object, CStrRef class_name) {
  return object.instanceof(class_name);
}

bool f_is_subclass_of(CVarRef class_or_object, CStrRef class_name) {
  const ClassInfo *classInfo =
    ClassInfo::FindClass(get_classname(class_or_object));
  if (classInfo) {
    return classInfo->derivesFrom(class_name, false);
  }
  return false;
}

bool f_method_exists(CVarRef class_or_object, CStrRef method_name) {
  const ClassInfo *classInfo =
    ClassInfo::FindClass(get_classname(class_or_object));
  if (classInfo) {
    return classInfo->hasMethod(method_name);
  }
  return false;
}

bool f_property_exists(CVarRef class_or_object, CStrRef property) {
  const ClassInfo *classInfo =
    ClassInfo::FindClass(get_classname(class_or_object));
  while (classInfo) {
    if (classInfo->hasProperty(property)) {
      return true;
    } else {
      classInfo = ClassInfo::FindClass(classInfo->getParentClass());
    }
  }
  return false;
}

Array f_get_object_vars(CObjRef object) {
  return object->o_toIterArray(FrameInjection::getClassName(true));
}

///////////////////////////////////////////////////////////////////////////////

Variant f_call_user_method_array(CStrRef method_name, Variant obj,
                                 CArrRef paramarr) {
  return obj.toObject()->o_invoke(method_name, paramarr, -1);
}

Variant f_call_user_method(int _argc, CStrRef method_name, Variant obj,
                           CArrRef _argv /* = null_array */) {
  return obj.toObject()->o_invoke(method_name, _argv, -1);
}

///////////////////////////////////////////////////////////////////////////////
}
