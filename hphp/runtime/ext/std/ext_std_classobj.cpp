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

#include "hphp/runtime/ext/std/ext_std_classobj.h"
#include "hphp/runtime/base/class-info.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/ext/ext_array.h"
#include "hphp/runtime/ext/ext_string.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////
// helpers

static inline StrNR ctxClassName() {
  Class* ctx = g_context->getContextClass();
  return ctx ? ctx->nameStr() : StrNR(staticEmptyString());
}

static const Class* get_cls(const Variant& class_or_object) {
  Class* cls = nullptr;
  if (class_or_object.is(KindOfObject)) {
    ObjectData* obj = class_or_object.toCObjRef().get();
    cls = obj->getVMClass();
  } else if (class_or_object.is(KindOfArray)) {
    // do nothing but avoid the toString conversion notice
  } else {
    cls = Unit::loadClass(class_or_object.toString().get());
  }
  return cls;
}

///////////////////////////////////////////////////////////////////////////////

Array HHVM_FUNCTION(get_declared_classes) {
  return ClassInfo::GetClasses();
}

Array HHVM_FUNCTION(get_declared_interfaces) {
  return ClassInfo::GetInterfaces();
}

Array HHVM_FUNCTION(get_declared_traits) {
  return ClassInfo::GetTraits();
}

bool HHVM_FUNCTION(class_alias, const String& original, const String& alias,
                                bool autoload /* = true */) {
  auto const origClass =
    autoload ? Unit::loadClass(original.get())
             : Unit::lookupClass(original.get());
  if (!origClass) {
    raise_warning("Class %s not found", original.data());
    return false;
  }
  return Unit::aliasClass(origClass, alias.get());
}

bool HHVM_FUNCTION(class_exists, const String& class_name,
                                 bool autoload /* = true */) {
  return Unit::classExists(class_name.get(), autoload, ClassKind::Class);
}

bool HHVM_FUNCTION(interface_exists, const String& interface_name,
                                     bool autoload /* = true */) {
  return
    Unit::classExists(interface_name.get(), autoload, ClassKind::Interface);
}

bool HHVM_FUNCTION(trait_exists, const String& trait_name,
                                 bool autoload /* = true */) {
  return Unit::classExists(trait_name.get(), autoload, ClassKind::Trait);
}

static void getMethodNamesImpl(const Class* cls,
                               const Class* ctx,
                               Array& out) {

  // The order of these methods is so that the first ones win on
  // case insensitive name conflicts.

  auto const numMethods = cls->numMethods();

  for (Slot i = 0; i < numMethods; ++i) {
    auto const meth = cls->getMethod(i);
    auto const declCls = meth->cls();
    auto addMeth = [&]() {
      auto const methName = Variant(meth->name(), Variant::StaticStrInit{});
      auto const lowerName = f_strtolower(methName.toString());
      if (!out.exists(lowerName)) {
        out.add(lowerName, methName);
      }
    };

    // Only pick methods declared in this class, in order to match
    // Zend's order.  Inherited methods will be inserted in the
    // recursive call later.
    if (declCls != cls) continue;

    // Skip generated, internal methods.
    if (meth->isGenerated()) continue;

    // Public methods are always visible.
    if ((meth->attrs() & AttrPublic)) {
      addMeth();
      continue;
    }

    // In anonymous contexts, only public methods are visible.
    if (!ctx) continue;

    // All methods are visible if the context is the class that
    // declared them.  If the context is not the declCls, protected
    // methods are visible in context classes related the declCls.
    if (declCls == ctx ||
        ((meth->attrs() & AttrProtected) &&
         (ctx->classof(declCls) || declCls->classof(ctx)))) {
      addMeth();
    }
  }

  // Now add the inherited methods.
  if (auto const parent = cls->parent()) {
    getMethodNamesImpl(parent, ctx, out);
  }

  // Add interface methods that the class may not have implemented yet.
  for (auto& iface : cls->declInterfaces()) {
    getMethodNamesImpl(iface.get(), ctx, out);
  }
}

Variant HHVM_FUNCTION(get_class_methods, const Variant& class_or_object) {
  auto const cls = get_cls(class_or_object);
  if (!cls) return init_null();
  VMRegAnchor _;

  auto retVal = Array::attach(MixedArray::MakeReserve(cls->numMethods()));
  getMethodNamesImpl(
    cls,
    arGetContextClassFromBuiltin(vmfp()),
    retVal
  );
  return f_array_values(retVal).toArray();
}

Array HHVM_FUNCTION(get_class_constants, const String& className) {
  auto const cls = Unit::loadClass(className.get());
  if (cls == NULL) {
    return Array::attach(MixedArray::MakeReserve(0));
  }

  auto const numConstants = cls->numConstants();
  ArrayInit arrayInit(numConstants, ArrayInit::Map{});

  auto const consts = cls->constants();
  for (size_t i = 0; i < numConstants; i++) {
    // Note: hphpc doesn't include inherited constants in
    // get_class_constants(), so mimic that behavior
    if (consts[i].m_class == cls) {
      auto const name  = const_cast<StringData*>(consts[i].m_name.get());
      Cell value = consts[i].m_val;
      // Handle dynamically set constants
      if (value.m_type == KindOfUninit) {
        value = cls->clsCnsGet(consts[i].m_name);
      }
      assert(value.m_type != KindOfUninit);
      arrayInit.set(name, cellAsCVarRef(value));
    }
  }

  return arrayInit.toArray();
}

Variant HHVM_FUNCTION(get_class_vars, const String& className) {
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

  ArrayInit arr(numDeclProps + numSProps, ArrayInit::Map{});

  for (size_t i = 0; i < numDeclProps; ++i) {
    StringData* name = const_cast<StringData*>(propInfo[i].m_name.get());
    // Empty names are used for invisible/private parent properties; skip them
    assert(name->size() != 0);
    if (Class::IsPropAccessible(propInfo[i], ctx)) {
      const TypedValue* value = &((*propVals)[i]);
      arr.set(name, tvAsCVarRef(value));
    }
  }

  for (size_t i = 0; i < numSProps; ++i) {
    bool vis, access;
    TypedValue* value = cls->getSProp(ctx, sPropInfo[i].m_name, vis, access);
    if (access) {
      arr.set(const_cast<StringData*>(sPropInfo[i].m_name.get()),
        tvAsCVarRef(value));
    }
  }

  return arr.toArray();
}

///////////////////////////////////////////////////////////////////////////////

Variant HHVM_FUNCTION(get_class, const Variant& object /* = null_variant */) {
  if (object.isNull()) {
    // No arg passed.
    String ret;
    CallerFrame cf;
    Class* cls = arGetContextClassImpl<true>(cf());
    if (cls) {
      ret = String(cls->nameStr());
    }

    if (ret.empty()) {
      raise_warning("get_class() called without object from outside a class");
      return false;
    }
    return ret;
  }
  if (!object.isObject()) return false;
  return VarNR(object.toObject()->o_getClassName());
}

Variant HHVM_FUNCTION(get_called_class) {
  EagerCallerFrame cf;
  ActRec* ar = cf();
  if (ar) {
    if (ar->hasThis()) {
      return Variant(ar->getThis()->o_getClassName());
    }
    if (ar->hasClass()) {
      return Variant(ar->getClass()->preClass()->name(),
        Variant::StaticStrInit{});
    }
  }
  return Variant(false);
}

Variant HHVM_FUNCTION(get_parent_class,
                      const Variant& object /* = null_variant */) {
  if (object.isNull()) {
    CallerFrame cf;
    Class* cls = arGetContextClass(cf());
    if (cls && cls->parent()) {
      return String(cls->parentStr());
    }
    return false;
  }

  Variant class_name;
  if (object.isObject()) {
    class_name = HHVM_FN(get_class)(object);
  } else if (object.isString()) {
    class_name = object;
  } else {
    return false;
  }

  const Class* cls = Unit::loadClass(class_name.toString().get());
  if (cls) {
    auto parentClass = cls->parentStr();
    if (!parentClass.empty()) {
      return VarNR(parentClass);
    }
  }
  return false;
}

static bool is_a_impl(const Variant& class_or_object, const String& class_name,
                      bool allow_string, bool subclass_only) {
  if (class_or_object.isString() && !allow_string) {
    return false;
  }

  const Class* cls = get_cls(class_or_object);
  if (!cls) return false;
  if (cls->attrs() & AttrTrait) return false;
  const Class* other = Unit::lookupClass(class_name.get());
  if (!other) return false;
  if (other->attrs() & AttrTrait) return false;
  if (other == cls) return !subclass_only;
  return cls->classof(other);
}

bool HHVM_FUNCTION(is_a, const Variant& class_or_object,
                         const String& class_name,
                         bool allow_string /* = false */) {
  return is_a_impl(class_or_object, class_name, allow_string, false);
}

bool HHVM_FUNCTION(is_subclass_of, const Variant& class_or_object,
                                   const String& class_name,
                                   bool allow_string /* = true */) {
  return is_a_impl(class_or_object, class_name, allow_string, true);
}

bool HHVM_FUNCTION(method_exists, const Variant& class_or_object,
                                  const String& method_name) {
  const Class* cls = get_cls(class_or_object);
  if (!cls) return false;
  if (cls->lookupMethod(method_name.get()) != NULL) return true;
  if (cls->attrs() & (AttrAbstract | AttrInterface)) {
    const Class::InterfaceMap& ifaces = cls->allInterfaces();
    for (int i = 0, size = ifaces.size(); i < size; i++) {
      if (ifaces[i]->lookupMethod(method_name.get())) return true;
    }
  }
  return false;
}

Variant HHVM_FUNCTION(property_exists, const Variant& class_or_object,
                                       const String& property) {
  Class* cls = nullptr;
  ObjectData* obj = nullptr;
  if (class_or_object.isObject()) {
    obj = class_or_object.getObjectData();
    cls = obj->getVMClass();
    assert(cls);
  } else if (class_or_object.isString()) {
    cls = Unit::loadClass(class_or_object.toString().get());
    if (!cls) return false;
  } else {
    raise_warning(
      "First parameter must either be an object"
      " or the name of an existing class"
    );
    return Variant(Variant::NullInit());
  }

  bool accessible;
  auto propInd = cls->getDeclPropIndex(cls, property.get(), accessible);
  if (propInd != kInvalidSlot) {
    return true;
  }
  if (obj &&
      UNLIKELY(obj->getAttribute(ObjectData::HasDynPropArr)) &&
      obj->dynPropArray()->nvGet(property.get())) {
    return true;
  }
  propInd = cls->lookupSProp(property.get());
  return (propInd != kInvalidSlot);
}

Array HHVM_FUNCTION(get_object_vars, const Object& object) {
  return object->o_toIterArray(ctxClassName());
}

///////////////////////////////////////////////////////////////////////////////

Variant HHVM_FUNCTION(call_user_method_array, const String& method_name,
                                              VRefParam obj,
                                              const Variant& paramarr) {
  return obj.toObject()->o_invoke(method_name, paramarr);
}

///////////////////////////////////////////////////////////////////////////////

void StandardExtension::initClassobj() {
  HHVM_FE(get_declared_classes);
  HHVM_FE(get_declared_interfaces);
  HHVM_FE(get_declared_traits);
  HHVM_FE(class_alias);
  HHVM_FE(class_exists);
  HHVM_FE(interface_exists);
  HHVM_FE(trait_exists);
  HHVM_FE(get_class_methods);
  HHVM_FE(get_class_constants);
  HHVM_FE(get_class_vars);
  HHVM_FE(get_class);
  HHVM_FE(get_called_class);
  HHVM_FE(get_parent_class);
  HHVM_FE(is_a);
  HHVM_FE(is_subclass_of);
  HHVM_FE(method_exists);
  HHVM_FE(property_exists);
  HHVM_FE(get_object_vars);
  HHVM_FE(call_user_method_array);

  loadSystemlib("std_classobj");
}


}
