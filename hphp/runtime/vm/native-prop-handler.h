/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#ifndef _incl_HPHP_RUNTIME_VM_NATIVE_PROP_HANDLER_H
#define _incl_HPHP_RUNTIME_VM_NATIVE_PROP_HANDLER_H

#include "hphp/runtime/ext/extension.h"
#include "hphp/util/hash-set.h"

namespace HPHP::Native {
//////////////////////////////////////////////////////////////////////////////
// Class NativePropHandler

struct NativePropHandler {
  typedef Variant (*GetFunc)(const Object& obj, const String& name);
  typedef Variant (*SetFunc)(const Object& obj, const String& name,
                             const Variant& value);
  typedef Variant (*IssetFunc)(const Object& obj, const String& name);
  typedef Variant (*UnsetFunc)(const Object& obj, const String& name);

  GetFunc get;      // native magic prop get
  SetFunc set;      // native magic set
  IssetFunc isset;  // native magic isset
  UnsetFunc unset;  // native magic unset
};

// Sigil value to use in property resolution in case
// if the native accessor didn't handle the prop, and
// we should try user-level magic accessors.
ALWAYS_INLINE Variant prop_not_handled() {
  return uninit_null();
}

NativePropHandler* getNativePropHandler(const StringData* className);

/**
 * Handler for a class with custom handling functions.
 */
void registerNativePropHandler(const StringData* className,
                               NativePropHandler::GetFunc get,
                               NativePropHandler::SetFunc set,
                               NativePropHandler::IssetFunc isset,
                               NativePropHandler::UnsetFunc unset);

/**
 * Default implementations of the accessor hooks. A property handler
 * class is supposed to implement `getProp`, `setProp`, `issetProp`,
 * `unsetProp`, and  `isPropSupported`. If a method cannot handle property,
 * it should return sigil `Native::prop_not_handled` value.
 *
 * Example:
 *
 * struct ElementPropHandler {
 *   static Variant getProp(const Object& this_, const String& name) {
 *     // get `name` prop
 *   }
 *   ...
 * }
 */

// A guard to stop handling in case if the property is not supported
// for this operation, and we should go to the user-level magic hooks.

#define CHECK_NATIVE_PROP_SUPPORTED(name, op)                                  \
  if (!T::isPropSupported(name, op)) {                                         \
    return Native::prop_not_handled();                                         \
  }

// Default getProp.

template<class T>
Variant nativePropHandlerGet(const Object& obj, const String& name) {
  CHECK_NATIVE_PROP_SUPPORTED(name, "get")
  return T::getProp(obj, name);
}

// Default setProp.

template<class T>
Variant nativePropHandlerSet(const Object& obj,
                             const String& name,
                             const Variant& value) {
  CHECK_NATIVE_PROP_SUPPORTED(name, "set")
  return T::setProp(obj, name, value);
}

// Default issetProp.

template<class T>
Variant nativePropHandlerIsset(const Object& obj, const String& name) {
  CHECK_NATIVE_PROP_SUPPORTED(name, "isset")
  return T::issetProp(obj, name);
}

// Default unsetProp.

template<class T>
Variant nativePropHandlerUnset(const Object& obj, const String& name) {
  CHECK_NATIVE_PROP_SUPPORTED(name, "unset")
  return T::unsetProp(obj, name);
}

/**
 * Default registering for a class name.
 * Example: Native::registerNativePropHandler<HandlerClassName>(className);
 */
template<class T>
void registerNativePropHandler(const StringData* className) {
  registerNativePropHandler(
    className,
    &nativePropHandlerGet<T>,
    &nativePropHandlerSet<T>,
    &nativePropHandlerIsset<T>,
    &nativePropHandlerUnset<T>
  );
}

template<class T>
void registerNativePropHandler(const String& className) {
  registerNativePropHandler<T>(className.get());
}

/**
 * Base prop handler class, to be extended by actual prop handlers.
 * By default handlers are "noop", that can be overridden by
 * child classes.
 */
struct BasePropHandler {
  static Variant getProp(const Object& /*this_*/, const String& /*name*/) {
    return Native::prop_not_handled();
  }
  static Variant setProp(const Object& /*this_*/, const String& /*name*/,
                         const Variant& /*value*/) {
    return Native::prop_not_handled();
  }
  static Variant issetProp(const Object& /*this_*/, const String& /*name*/) {
    return Native::prop_not_handled();
  }
  static Variant unsetProp(const Object& /*this_*/, const String& /*name*/) {
    return Native::prop_not_handled();
  }
  static bool isPropSupported(const String& /*name*/, const String& /*op*/) {
    return false;
  }
};

#define CHECK_ACCESSOR(accesor, opstr, classname, propname)                    \
  if (!accesor) {                                                              \
    raise_error("Cannot directly %s the property %s::$%s",                     \
                 opstr, classname->data(), propname.data());                   \
  }

/**
 * Base prop handler class, that uses `Native::PropAccessorMap`.
 * Derived classes provide the handling map with accessort per each property.
 */
template <class Derived>
struct MapPropHandler : BasePropHandler {

  static Variant getProp(const Object& this_, const String& name) {
    auto get = Derived::map.get(name);
    CHECK_ACCESSOR(get, "get", this_->getVMClass()->name(), name)
    return get(this_);
  }

  static Variant setProp(const Object& this_,
                         const String& name,
                         const Variant& value) {
    auto set = Derived::map.set(name);
    CHECK_ACCESSOR(set, "set", this_->getVMClass()->name(), name);
    set(this_, value);
    return true;
  }

  static Variant issetProp(const Object& this_, const String& name) {
    auto isset = Derived::map.isset(name);
    // If there is special `isset`, call it.
    if (isset) {
      return isset(this_);
    }
    // Otherwise, fallback to `null` check of the result from `get`.
    auto get = Derived::map.get(name);
    CHECK_ACCESSOR(get, "get", this_->getVMClass()->name(), name)
    return !get(this_).isNull();
  }

  static Variant unsetProp(const Object& this_, const String& name) {
    auto unset = Derived::map.unset(name);
    CHECK_ACCESSOR(unset, "unset", this_->getVMClass()->name(), name);
    unset(this_);
    return true;
  }

  static bool isPropSupported(const String& name, const String& /*op*/) {
    return Derived::map.isPropSupported(name);
  }
};

/**
 * An entry in the `PropAccessorMap`, contains handlers per property.
 */
struct PropAccessor {
  const char* name;
  Variant (*get)(const Object& this_);
  void    (*set)(const Object& this_, const Variant& value);
  bool    (*isset)(const Object& this_);
  void    (*unset)(const Object& this_);
};

struct hashNPA {
  size_t operator()(const PropAccessor* pa) const {
    return hash_string_i(pa->name, strlen(pa->name));
  }
};
struct cmpNPA {
  bool operator()(const PropAccessor* pa1,
                  const PropAccessor* pa2) const {
    return strcasecmp(pa1->name, pa2->name) == 0;
  }
};

/**
 * Map-based handling of property accessors. Callers may organize handlers
 * into a map with handling function per each property. Example:
 *
 * static Native::PropAccessor elementPropAccessors[] = {
 *   {"nodeValue", elementNodeValueGet, elementNodeValueSet, ...},
 *   {"localName", elementLocaleNameGet, nullptr, ...},
 *   ...
 *   {nullptr, ...}
 * };
 */
struct PropAccessorMap :
      private hphp_hash_set<PropAccessor*, hashNPA, cmpNPA> {

  explicit PropAccessorMap(PropAccessor* props,
                           PropAccessorMap *base = nullptr);

  bool isPropSupported(const String& name);

  Variant (*get(const String& name))(const Object& this_);

  void    (*set(const String& name))(const Object& this_,
                                     const Variant& value);

  bool    (*isset(const String& name))(const Object& this_);
  void    (*unset(const String& name))(const Object& this_);

private:
  const_iterator lookupProp(const String& name);
};

/**
 * API methods to call at property resolution (from `object-data`).
 * Example: Native::getProp(this, propName);
 */
Variant getProp(const Object& obj, const String& name);
Variant setProp(const Object& obj, const String& name, const Variant& value);
Variant issetProp(const Object& obj, const String& name);
Variant unsetProp(const Object& obj, const String& name);

//////////////////////////////////////////////////////////////////////////////
} // namespace HPHP::Native

#endif // _incl_HPHP_RUNTIME_VM_NATIVE_PROP_HANDLER_H
