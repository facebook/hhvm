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
  return ClassInfo::GetClasses();
}

Array f_get_declared_interfaces() {
  return ClassInfo::GetInterfaces();
}

Array f_get_declared_traits() {
  return ClassInfo::GetTraits();
}

bool f_class_exists(CStrRef class_name, bool autoload /* = true */) {
  const ClassInfo *info = ClassInfo::FindClassInterfaceOrTrait(class_name);

  if (info) {
    ClassInfo::Attribute attr = info->getAttribute();
    return !(attr & (ClassInfo::IsInterface|ClassInfo::IsTrait));
  }

  if (!autoload) return false;

  AutoloadHandler::s_instance->invokeHandler(class_name);
  return f_class_exists(class_name, false);
}

bool f_interface_exists(CStrRef interface_name, bool autoload /* = true */) {
  const ClassInfo *info = ClassInfo::FindClassInterfaceOrTrait(interface_name);

  if (info) {
    return info->getAttribute() & ClassInfo::IsInterface;
  }

  if (!autoload) return false;

  AutoloadHandler::s_instance->invokeHandler(interface_name);
  return f_interface_exists(interface_name, false);
}

bool f_trait_exists(CStrRef trait_name, bool autoload /* = true */) {
  const ClassInfo *info = ClassInfo::FindClassInterfaceOrTrait(trait_name);

  if (info) {
    return info->getAttribute() & ClassInfo::IsTrait;
  }

  if (!autoload) return false;

  AutoloadHandler::s_instance->invokeHandler(trait_name);
  return f_trait_exists(trait_name, false);
}

Array f_get_class_methods(CVarRef class_or_object) {
  ClassInfo::MethodVec methods;
  CStrRef class_name = get_classname(class_or_object);
  if (!ClassInfo::GetClassMethods(methods, class_name)) return Array();
  CStrRef klass = hhvm
                  ? g_context->getContextClassName(true)
                  : FrameInjection::GetClassName(true);
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

Array vm_get_class_vars(CStrRef className) {
  HPHP::VM::Class* cls = g_context->lookupClass(className.get());
  if (cls == NULL) {
    raise_error("Unknown class %s", className->data());
  }
  cls->initialize();

  HPHP::VM::Class::SPropInfoVec& sPropInfo = cls->m_sPropInfo;
  HPHP::VM::Class::PropInfoVec& propInfo = cls->m_declPropInfo;

  // The class' instance property initialization template is in different
  // places, depending on whether it has any request-dependent initializers
  // (i.e. constants)
  HPHP::VM::Class::PropInitVec* propVals =
    (cls->m_pinitVec.size() > 0 ?
     g_context->getPropData(cls) : &cls->m_declPropInit);
  ASSERT(propVals != NULL);
  ASSERT(propVals->size() == propInfo.size());

  // For visibility checks
  HPHP::VM::ActRec* fp = vm_get_previous_frame();
  HPHP::VM::PreClass* ctx = fp->m_func->m_preClass;
  const ClassInfo* ctxCI =
    (ctx == NULL ? NULL : g_context->findClassInfo(CStrRef(ctx->m_name)));
  ClassInfo::PropertyMap propMap =
    g_context->findClassInfo(className)->getProperties();

  HphpArray* ret = NEW(HphpArray)(propInfo.size() + sPropInfo.size());

  for (unsigned int i = 0; i < propInfo.size(); ++i) {
    StringData* name = const_cast<StringData*>(propInfo[i].m_name);
    if (propMap[String(name)]->isVisible(ctxCI)) {
      TypedValue* value = &((*propVals)[i]);
      ret->nvSet(name, value, false);
    }
  }

  for (unsigned int i = 0; i < sPropInfo.size(); ++i) {
    bool vis, access;
    TypedValue* value = cls->getSProp(ctx, sPropInfo[i].m_name, vis, access);
    if (vis) {
      ret->nvSet(const_cast<StringData*>(sPropInfo[i].m_name), value, false);
    }
  }

  return ret;
}

Array f_get_class_vars(CStrRef class_name) {
  if (hhvm) {
    return vm_get_class_vars(class_name.get());
  } else {
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
}

///////////////////////////////////////////////////////////////////////////////

Variant f_get_class(CVarRef object /* = null_variant */) {
  // hphpc passes in an *uninitialized* null for:
  //
  //   get_class(null)
  //
  // Therefore, limit the following block of code to hhvm.
  if (hhvm) {
    if (!object.isInitialized()) {
      // No arg passed.
      String ret;
      HPHP::VM::ActRec* fp = vm_get_previous_frame();
      HPHP::VM::PreClass* preClass = arGetContextPreClass(fp);
      if (preClass) {
        ret = CStrRef(preClass->m_name);
      }
      if (ret.empty()) {
        raise_warning("get_class() called without object from outside a class");
        return false;
      }
      return ret;
    }
  }
  if (!object.isObject()) return false;
  return object.toObject()->o_getClassName();
}

Variant f_get_parent_class(CVarRef object /* = null_variant */) {
  if (hhvm) {
    if (!object.isInitialized()) {
      HPHP::VM::ActRec* fp = vm_get_previous_frame();
      HPHP::VM::PreClass* preClass = arGetContextPreClass(fp);
      if (preClass && preClass->m_parent && !(preClass->m_parent->empty())) {
        return CStrRef(preClass->m_parent);
      }
      return false;
    }
  }
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
    CStrRef context = hhvm
                      ? g_context->getContextClassName(true)
                      : FrameInjection::GetClassName(true);
    // Call o_exists for objects, to include dynamic properties.
    return class_or_object.toObject()->o_propExists(property, context);
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
    if (hhvm) {
      return object.toObject()->o_toIterArray(
        g_context->getContextClassName(true));
    } else {
      return object.toObject()->o_toIterArray(
        FrameInjection::GetClassName(true));
    }
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
