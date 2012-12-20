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
#include <runtime/vm/translator/translator.h>
#include <runtime/vm/translator/translator-inline.h>
#include <util/util.h>

namespace HPHP {

using VM::Transl::CallerFrame;
using VM::Transl::VMRegAnchor;

///////////////////////////////////////////////////////////////////////////////
// helpers

static String get_classname(CVarRef class_or_object) {
  if (class_or_object.is(KindOfObject)) {
    return class_or_object.toCObjRef().get()->o_getClassName();
  }
  return class_or_object.toString();
}

static inline CStrRef ctxClassName() {
  return hhvm ?
    g_vmContext->getContextClassName() :
    FrameInjection::GetClassName(true);
}

static const VM::Class* get_cls(CVarRef class_or_object) {
  VM::Class* cls = NULL;
  if (class_or_object.is(KindOfObject)) {
    ObjectData* obj = class_or_object.toCObjRef().get();
    cls = obj->getVMClass();
  } else {
    cls = VM::Unit::loadClass(class_or_object.toString().get());
  }
  return cls;
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
  if (hhvm) {
    return VM::Unit::classExists(class_name.get(), autoload, VM::AttrNone);
  }
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
  if (hhvm) {
    return VM::Unit::classExists(interface_name.get(), autoload,
                                 VM::AttrInterface);
  }
  const ClassInfo *info = ClassInfo::FindClassInterfaceOrTrait(interface_name);

  if (info) {
    return info->getAttribute() & ClassInfo::IsInterface;
  }

  if (!autoload) return false;

  AutoloadHandler::s_instance->invokeHandler(interface_name);
  return f_interface_exists(interface_name, false);
}

bool f_trait_exists(CStrRef trait_name, bool autoload /* = true */) {
  if (hhvm) {
    return VM::Unit::classExists(trait_name.get(), autoload, VM::AttrTrait);
  }
  const ClassInfo *info = ClassInfo::FindClassInterfaceOrTrait(trait_name);

  if (info) {
    return info->getAttribute() & ClassInfo::IsTrait;
  }

  if (!autoload) return false;

  AutoloadHandler::s_instance->invokeHandler(trait_name);
  return f_trait_exists(trait_name, false);
}

Array f_get_class_methods(CVarRef class_or_object) {
  if (hhvm) {
    const VM::Class* cls = get_cls(class_or_object);
    if (!cls) return Array();
    VMRegAnchor _;

    HphpArray* retVal = NEW(HphpArray)(cls->numMethods());
    cls->getMethodNames(arGetContextClassFromBuiltin(g_vmContext->getFP()),
                        retVal);
    return Array(retVal).keys();
  }

  ClassInfo::MethodVec methods;
  CStrRef class_name = get_classname(class_or_object);
  const ClassInfo *classInfo = NULL;
  for (int i = 0; ; ++i) {
    classInfo = ClassInfo::FindClassInterfaceOrTrait(class_name);
    if (classInfo) break;
    if (i) return Array();
    AutoloadHandler::s_instance->invokeHandler(class_name);
  }

  if (!ClassInfo::GetClassMethods(methods, classInfo)) return Array();

  CStrRef klass = ctxClassName();
  bool allowPrivate = !klass.empty() && klass->isame(class_name.get());

  Array ret = Array::Create();
  for (unsigned int i = 0; i < methods.size(); i++) {
    if ((methods[i]->attribute & ClassInfo::IsPublic) || allowPrivate) {
      ret.set(methods[i]->name, true);
    }
  }
  return ret.keys();
}

Array vm_get_class_constants(CStrRef className) {
  HPHP::VM::Class* cls = HPHP::VM::Unit::loadClass(className.get());
  if (cls == NULL) {
    return NEW(HphpArray)(0);
  }

  size_t numConstants = cls->numConstants();
  HphpArray* retVal = NEW(HphpArray)(numConstants);
  const VM::Class::Const* consts = cls->constants();
  for (size_t i = 0; i < numConstants; i++) {
    // Note: hphpc doesn't include inherited constants in
    // get_class_constants(), so mimic that behavior
    if (consts[i].m_class == cls) {
      StringData* name  = const_cast<StringData*>(consts[i].m_name);
      TypedValue* value = cls->clsCnsGet(consts[i].m_name);
      retVal->nvSet(name, value, false);
    }
  }

  return retVal;
}

Array f_get_class_constants(CStrRef class_name) {
  if (hhvm) {
    return vm_get_class_constants(class_name.get());
  }
  const ClassInfo *cls = ClassInfo::FindClass(class_name);
  if (!cls) {
    AutoloadHandler::s_instance->invokeHandler(class_name);
    cls = ClassInfo::FindClass(class_name);
  }
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
  HPHP::VM::Class* cls = HPHP::VM::Unit::lookupClass(className.get());
  if (cls == NULL) {
    raise_error("Unknown class %s", className->data());
  }
  cls->initialize();

  const VM::Class::SProp* sPropInfo = cls->staticProperties();
  const size_t numSProps = cls->numStaticProperties();
  const VM::Class::Prop* propInfo = cls->declProperties();
  const size_t numDeclProps = cls->numDeclProperties();

  // The class' instance property initialization template is in different
  // places, depending on whether it has any request-dependent initializers
  // (i.e. constants)
  const VM::Class::PropInitVec& declPropInitVec = cls->declPropInit();
  const VM::Class::PropInitVec* propVals = !cls->pinitVec().empty()
    ? cls->getPropData() : &declPropInitVec;
  ASSERT(propVals != NULL);
  ASSERT(propVals->size() == numDeclProps);

  // For visibility checks
  CallerFrame cf;
  HPHP::VM::Class* ctx = arGetContextClass(cf());

  HphpArray* ret = NEW(HphpArray)(numDeclProps + numSProps);

  for (size_t i = 0; i < numDeclProps; ++i) {
    StringData* name = const_cast<StringData*>(propInfo[i].m_name);
    // Empty names are used for invisible/private parent properties; skip them
    ASSERT(name->size() != 0);
    if (VM::Class::IsPropAccessible(propInfo[i], ctx)) {
      const TypedValue* value = &((*propVals)[i]);
      ret->nvSet(name, value, false);
    }
  }

  for (size_t i = 0; i < numSProps; ++i) {
    bool vis, access;
    TypedValue* value = cls->getSProp(ctx, sPropInfo[i].m_name, vis, access);
    if (access) {
      ret->nvSet(const_cast<StringData*>(sPropInfo[i].m_name), value, false);
    }
  }

  return ret;
}

Array f_get_class_vars(CStrRef class_name) {
  if (hhvm) return vm_get_class_vars(class_name.get());

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
  if (object.isNull()) {
    // No arg passed.
    String ret;
    if (hhvm) {
      CallerFrame cf;
      HPHP::VM::Class* cls = HPHP::VM::arGetContextClassImpl<true>(cf());
      if (cls) {
        ret = CStrRef(cls->nameRef());
      }
    } else {
      ret = FrameInjection::GetClassName(true);
    }
    if (ret.empty()) {
      raise_warning("get_class() called without object from outside a class");
      return false;
    }
    return ret;
  }
  if (!object.isObject()) return false;
  return object.toObject()->o_getClassName();
}

Variant f_get_parent_class(CVarRef object /* = null_variant */) {
  if (hhvm) {
    if (!object.isInitialized()) {
      CallerFrame cf;
      HPHP::VM::Class* cls = arGetContextClass(cf());
      if (cls && cls->parent()) {
        return CStrRef(cls->parentRef());
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

static bool is_a_impl(CVarRef class_or_object, CStrRef class_name,
                      bool allow_string, bool subclass_only) {
  if (class_or_object.isString() && !allow_string) {
    return false;
  }

  if (hhvm) {
    const VM::Class* cls = get_cls(class_or_object);
    if (!cls) return false;
    if (cls->attrs() & VM::AttrTrait) return false;
    const VM::Class* other = VM::Unit::lookupClass(class_name.get());
    if (!other) return false;
    if (other->attrs() & VM::AttrTrait) return false;
    if (other == cls) return !subclass_only;
    return cls->classof(other);
  }

  /* Resolve subject class, autoloading if needed */
  CStrRef this_class = get_classname(class_or_object);
  const ClassInfo *classInfo =
        ClassInfo::FindClassInterfaceOrTrait(this_class);
  if (!classInfo) {
    AutoloadHandler::s_instance->invokeHandler(this_class);
    classInfo = ClassInfo::FindClassInterfaceOrTrait(this_class);
    if (!classInfo) return false;
  }
  if (classInfo->getAttribute() & ClassInfo::IsTrait) return false;

  /* Resolve target class, don't bother to autoload as it
   * couldn't be an ancestor of an already loaded class
   */
  const ClassInfo *otherInfo =
        ClassInfo::FindClassInterfaceOrTrait(class_name);
  if (!otherInfo) return false;
  if (otherInfo->getAttribute() & ClassInfo::IsTrait) return false;

  /* Derives from returns true for same classes
   * override that by explicitly checking and
   * returning false before dispatching to derivesFrom()
   */
  if (this_class->isame(class_name.get())) {
    return !subclass_only;
  }

  return classInfo->derivesFrom(class_name, true);
}

bool f_is_a(CVarRef class_or_object, CStrRef class_name, bool allow_string /* = false */) {
  return is_a_impl(class_or_object, class_name, allow_string, false);
}

bool f_is_subclass_of(CVarRef class_or_object, CStrRef class_name, bool allow_string /* = true */) {
  return is_a_impl(class_or_object, class_name, allow_string, true);
}

bool f_method_exists(CVarRef class_or_object, CStrRef method_name) {
  if (hhvm) {
    const VM::Class* cls = get_cls(class_or_object);
    if (!cls) return false;
    if (cls->lookupMethod(method_name.get()) != NULL) return true;
    if (cls->attrs() & VM::AttrAbstract) {
      const VM::ClassSet& ifaces = cls->allInterfaces();
      for (VM::ClassSet::const_iterator it = ifaces.begin();
           it != ifaces.end();
           ++it) {
        if ((*it)->lookupMethod(method_name.get())) return true;
      }
    }
    return false;
  }
  CStrRef class_name = get_classname(class_or_object);
  for (int i = 0; ; ++i) {
    const ClassInfo *classInfo =
      ClassInfo::FindClassInterfaceOrTrait(class_name);
    if (classInfo) {
      ClassInfo *defClass;
      return classInfo->hasMethod(method_name, defClass) != NULL;
    }
    if (i) break;
    AutoloadHandler::s_instance->invokeHandler(class_name);
  }
  return false;
}

bool f_property_exists(CVarRef class_or_object, CStrRef property) {
  if (class_or_object.isObject()) {
    CStrRef context = ctxClassName();
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
    return object.toObject()->o_toIterArray(ctxClassName());
  }
  raise_warning("get_object_vars() expects parameter 1 to be object");
  return Variant(Variant::nullInit);
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
