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

#include "hphp/runtime/ext/ext_class.h"
#include "hphp/runtime/base/class-info.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/ext/util.h"
#include "hphp/util/util.h"

namespace HPHP {

using Transl::CallerFrame;
using Transl::VMRegAnchor;

///////////////////////////////////////////////////////////////////////////////
// helpers

static String get_classname(CVarRef class_or_object) {
  if (class_or_object.is(KindOfObject)) {
    return class_or_object.toCObjRef().get()->o_getClassName();
  }
  return class_or_object.toString();
}

static inline CStrRef ctxClassName() {
  Class* ctx = g_vmContext->getContextClass();
  return ctx ? ctx->nameRef() : empty_string;
}

static const Class* get_cls(CVarRef class_or_object) {
  Class* cls = NULL;
  if (class_or_object.is(KindOfObject)) {
    ObjectData* obj = class_or_object.toCObjRef().get();
    cls = obj->getVMClass();
  } else {
    cls = Unit::loadClass(class_or_object.toString().get());
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

bool f_class_alias(CStrRef original,
                   CStrRef alias,
                   bool autoload /* = true */) {
  auto const origClass =
    autoload ? Unit::loadClass(original.get())
             : lookup_class(original.get());
  if (!origClass) {
    raise_warning("Class %s not found", original->data());
    return false;
  }
  return Unit::aliasClass(origClass, alias.get());
}

bool f_class_exists(CStrRef class_name, bool autoload /* = true */) {
  return Unit::classExists(class_name.get(), autoload, AttrNone);
}

bool f_interface_exists(CStrRef interface_name, bool autoload /* = true */) {
  return Unit::classExists(interface_name.get(), autoload,
                               AttrInterface);
}

bool f_trait_exists(CStrRef trait_name, bool autoload /* = true */) {
  return Unit::classExists(trait_name.get(), autoload, AttrTrait);
}

Array f_get_class_methods(CVarRef class_or_object) {
  const Class* cls = get_cls(class_or_object);
  if (!cls) return Array();
  VMRegAnchor _;

  auto retVal      = HphpArray::MakeReserve(cls->numMethods());
  auto arrayHolder = Array::attach(retVal);
  cls->getMethodNames(arGetContextClassFromBuiltin(g_vmContext->getFP()),
                      retVal);
  return arrayHolder.keys();
}

Array f_get_class_constants(CStrRef className) {
  auto const cls = Unit::loadClass(className.get());
  if (cls == NULL) {
    return Array::attach(HphpArray::MakeReserve(0));
  }

  auto const numConstants = cls->numConstants();
  ArrayInit arrayInit(numConstants);

  auto const consts = cls->constants();
  for (size_t i = 0; i < numConstants; i++) {
    // Note: hphpc doesn't include inherited constants in
    // get_class_constants(), so mimic that behavior
    if (consts[i].m_class == cls) {
      auto const name  = const_cast<StringData*>(consts[i].m_name);
      auto value = &consts[i].m_val;
      // Handle dynamically set constants
      if (value->m_type == KindOfUninit) {
        value = cls->clsCnsGet(consts[i].m_name);
      }
      arrayInit.set(name, tvAsCVarRef(value), true /* isKey */);
    }
  }

  return arrayInit.toArray();
}

Variant f_get_class_vars(CStrRef className) {
  const Class* cls = Unit::loadClass(className.get());
  if (!cls) {
    return false;
  }
  cls->initialize();

  const Class::SProp* sPropInfo = cls->staticProperties();
  const size_t numSProps = cls->numStaticProperties();
  const Class::Prop* propInfo = cls->declProperties();
  const size_t numDeclProps = cls->numDeclProperties();

  // The class' instance property initialization template is in different
  // places, depending on whether it has any request-dependent initializers
  // (i.e. constants)
  const Class::PropInitVec& declPropInitVec = cls->declPropInit();
  const Class::PropInitVec* propVals = !cls->pinitVec().empty()
    ? cls->getPropData() : &declPropInitVec;
  assert(propVals != NULL);
  assert(propVals->size() == numDeclProps);

  // For visibility checks
  CallerFrame cf;
  Class* ctx = arGetContextClass(cf());

  ArrayInit arr(numDeclProps + numSProps);

  for (size_t i = 0; i < numDeclProps; ++i) {
    StringData* name = const_cast<StringData*>(propInfo[i].m_name);
    // Empty names are used for invisible/private parent properties; skip them
    assert(name->size() != 0);
    if (Class::IsPropAccessible(propInfo[i], ctx)) {
      const TypedValue* value = &((*propVals)[i]);
      arr.set(name, tvAsCVarRef(value), true /* isKey */);
    }
  }

  for (size_t i = 0; i < numSProps; ++i) {
    bool vis, access;
    TypedValue* value = cls->getSProp(ctx, sPropInfo[i].m_name, vis, access);
    if (access) {
      arr.set(const_cast<StringData*>(sPropInfo[i].m_name),
        tvAsCVarRef(value), true /* isKey */);
    }
  }

  return arr.toArray();
}

///////////////////////////////////////////////////////////////////////////////

Variant f_get_class(CVarRef object /* = null_variant */) {
  if (object.isNull()) {
    // No arg passed.
    String ret;
    CallerFrame cf;
    Class* cls = arGetContextClassImpl<true>(cf());
    if (cls) {
      ret = CStrRef(cls->nameRef());
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
  if (!object.isInitialized()) {
    CallerFrame cf;
    Class* cls = arGetContextClass(cf());
    if (cls && cls->parent()) {
      return CStrRef(cls->parentRef());
    }
    return false;
  }

  Variant class_name;
  if (object.isObject()) {
    class_name = f_get_class(object);
  } else if (object.isString()) {
    class_name = object;
  } else {
    return false;
  }

  const Class* cls = lookup_class(class_name.toString().get());
  if (cls) {
    auto& parentClass = *(const String*)(&cls->parentRef());
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

  const Class* cls = get_cls(class_or_object);
  if (!cls) return false;
  if (cls->attrs() & AttrTrait) return false;
  const Class* other = lookup_class(class_name.get());
  if (!other) return false;
  if (other->attrs() & AttrTrait) return false;
  if (other == cls) return !subclass_only;
  return cls->classof(other);
}

bool f_is_a(CVarRef class_or_object, CStrRef class_name, bool allow_string /* = false */) {
  return is_a_impl(class_or_object, class_name, allow_string, false);
}

bool f_is_subclass_of(CVarRef class_or_object, CStrRef class_name, bool allow_string /* = true */) {
  return is_a_impl(class_or_object, class_name, allow_string, true);
}

bool f_method_exists(CVarRef class_or_object, CStrRef method_name) {
  const Class* cls = get_cls(class_or_object);
  if (!cls) return false;
  if (cls->lookupMethod(method_name.get()) != NULL) return true;
  if (cls->attrs() & AttrAbstract) {
    const Class::InterfaceMap& ifaces = cls->allInterfaces();
    for (int i = 0, size = ifaces.size(); i < size; i++) {
      if (ifaces[i]->lookupMethod(method_name.get())) return true;
    }
  }
  return false;
}

Variant f_property_exists(CVarRef class_or_object, CStrRef property) {
  if (class_or_object.isObject()) {
    CStrRef context = ctxClassName();
    return (bool)class_or_object.toObject()->o_realProp(
      property, ObjectData::RealPropExist, context);
  }
  if (!class_or_object.isString()) {
    raise_warning(
      "First parameter must either be an object or the name of an existing class"
    );
    return Variant(Variant::NullInit());
  }

  Class* cls = lookup_class(get_classname(class_or_object).get());
  if (!cls) {
    return false;
  }
  bool accessible;
  auto propInd = cls->getDeclPropIndex(cls, property.get(), accessible);
  if (propInd != kInvalidSlot) {
    return true;
  }
  propInd = cls->lookupSProp(property.get());
  return (propInd != kInvalidSlot);
}

Variant f_get_object_vars(CObjRef object) {
  return object->o_toIterArray(ctxClassName());
}

///////////////////////////////////////////////////////////////////////////////

Variant f_call_user_method_array(CStrRef method_name, VRefParam obj,
                                 CArrRef paramarr) {
  return obj.toObject()->o_invoke(method_name, paramarr);
}

Variant f_call_user_method(int _argc, CStrRef method_name, VRefParam obj,
                           CArrRef _argv /* = null_array */) {
  return obj.toObject()->o_invoke(method_name, _argv);
}

///////////////////////////////////////////////////////////////////////////////
}
