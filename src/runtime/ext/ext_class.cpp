/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <runtime/ext/ext_class.h>
#include <runtime/base/class_info.h>
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

Array f_get_declared_traits() {
  return ClassInfo::GetTraits(true);
}

bool f_class_exists(CStrRef class_name, bool autoload /* = true */) {
  const ClassInfo *info = ClassInfo::FindClass(class_name);

  if (info) {
    if (!(info->getAttribute() & ClassInfo::IsVolatile)) return true;
    if (info->isClassInfoRedeclared()) {
      const ClassInfo *actual = info->getCurrent();
      if (actual != info) {
        return !(actual->getAttribute() & (ClassInfo::IsInterface|
                                         ClassInfo::IsTrait));
      }
    } else if (info->isDeclared()) {
      return true;
    }
    // volatile and not yet loaded and not an interface
  } else {
    info = ClassInfo::FindInterface(class_name);
    if (info) {
      if (!(info->getAttribute() & ClassInfo::IsVolatile) ||
          info->isDeclared()) {
        return false;
      }
    }
  }

  if (!autoload) return false;

  AutoloadHandler::s_instance->invokeHandler(class_name);
  return f_class_exists(class_name, false);
}

bool f_interface_exists(CStrRef interface_name, bool autoload /* = true */) {
  const ClassInfo *info = ClassInfo::FindInterface(interface_name);

  if (info) {
    if (!(info->getAttribute() & ClassInfo::IsVolatile)) return true;
    if (info->isClassInfoRedeclared()) {
      const ClassInfo *actual = info->getCurrent();
      if (actual != info) {
        return actual->getAttribute() & ClassInfo::IsInterface;
      }
    } else if (info->isDeclared()) {
      return true;
    }
    // volatile and not yet loaded and not a class
  } else {
    info = ClassInfo::FindClass(interface_name);
    if (info) {
      if (!(info->getAttribute() & ClassInfo::IsVolatile)) return false;
      if (info->isClassInfoRedeclared()) {
        const ClassInfo *actual = info->getCurrent();
        if (actual != info) {
          return actual->getAttribute() & ClassInfo::IsInterface;
        }
      } else if (info->isDeclared()) {
        return false;
      }
    }
  }

  if (!autoload) return false;

  AutoloadHandler::s_instance->invokeHandler(interface_name);
  return f_interface_exists(interface_name, false);
}

bool f_trait_exists(CStrRef trait_name, bool autoload /* = true */) {
  const ClassInfo *info = ClassInfo::FindTrait(trait_name);

  if (autoload && (!info || (info->getAttribute() & ClassInfo::IsVolatile))) {
    AutoloadHandler::s_instance->invokeHandler(trait_name);
    if (!info && ClassInfo::FindTrait(trait_name)) {
      return true;
    }
  }

  if (info) {
    return info->isDeclared();
  }

  // look for traits redeclared by classes
  info = ClassInfo::FindClass(trait_name);
  if (info && info->isClassInfoRedeclared()) {
    if (!info->isDeclared()) return false;
    return info->getCurrent()->isClassInfoRedeclared();
  }

  return false;
}

Array f_get_class_methods(CVarRef class_or_object) {
  ClassInfo::MethodVec methods;
  CStrRef class_name = get_classname(class_or_object);
  if (!ClassInfo::GetClassMethods(methods, class_name)) return Array();
  CStrRef klass = FrameInjection::GetClassName(true);

  bool allowPrivate = !klass.empty() && klass->isame(class_name.get());
  Array ret = Array::Create();
  for (unsigned int i = 0; i < methods.size(); i++) {
    if ((methods[i]->attribute & ClassInfo::IsPublic) || allowPrivate) {
      ret.set(methods[i]->name, true);
    }
  }
  return ret.keys();
}

Array f_get_class_constants(CStrRef class_name) {
  const ClassInfo *cls = ClassInfo::FindClass(class_name);
  Array ret = Array::Create();
  if (cls) {
    const ClassInfo::ConstantVec &constants = cls->getConstantsVec();
    for (ClassInfo::ConstantVec::const_iterator iter = constants.begin();
         iter != constants.end(); ++iter) {
      ret.set((*iter)->name, (*iter)->getValue());
    }
  }
  return ret;
}

Array f_get_class_vars(CStrRef class_name) {
  ClassInfo::PropertyVec properties;
  ClassInfo::GetClassProperties(properties, class_name);
  CStrRef context = FrameInjection::GetClassName(true);
  const ClassInfo *cls = NULL;
  if (!context.empty()) {
    cls = ClassInfo::FindClass(context);
  }

  Array ret = Array::Create();
  // PHP has instance variables appear before static variables...
  for (unsigned int i = 0; i < properties.size(); i++) {
    if (!(properties[i]->attribute & ClassInfo::IsStatic) &&
        properties[i]->isVisible(cls)) {
      ret.set(properties[i]->name,
              get_class_var_init(class_name, properties[i]->name));
    }
  }
  for (unsigned int i = 0; i < properties.size(); i++) {
    if (properties[i]->attribute & ClassInfo::IsStatic &&
        properties[i]->isVisible(cls)) {
      ret.set(properties[i]->name,
              get_static_property(class_name, properties[i]->name));
    }
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
    CStrRef parentClass = classInfo->getParentClass();
    if (!parentClass.empty()) {
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
    ClassInfo *defClass;
    return classInfo->hasMethod(method_name, defClass) != NULL;
  }
  return false;
}

bool f_property_exists(CVarRef class_or_object, CStrRef property) {
  if (class_or_object.isObject()) {
    // Call o_exists for objects, to include dynamic properties.
    return class_or_object.toObject()->o_propExists(property);
  }
  const ClassInfo *classInfo =
    ClassInfo::FindClass(get_classname(class_or_object));
  while (classInfo) {
    if (classInfo->hasProperty(property)) {
      return true;
    } else {
      classInfo = classInfo->getParentClassInfo();
    }
  }
  return false;
}

Variant f_get_object_vars(CVarRef object) {
  if (object.isObject()) {
    return object.toObject()->o_toIterArray(FrameInjection::GetClassName(true));
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////

Variant f_call_user_method_array(CStrRef method_name, VRefParam obj,
                                 CArrRef paramarr) {
  return obj.toObject()->o_invoke(method_name, paramarr, -1);
}

Variant f_call_user_method(int _argc, CStrRef method_name, VRefParam obj,
                           CArrRef _argv /* = null_array */) {
  return obj.toObject()->o_invoke(method_name, _argv, -1);
}

///////////////////////////////////////////////////////////////////////////////
}
