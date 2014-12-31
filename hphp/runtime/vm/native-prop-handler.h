/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/base-includes.h"

namespace HPHP { namespace Native {
//////////////////////////////////////////////////////////////////////////////
// Class NativePropHandler

struct NativePropHandler {
  typedef Variant (*GetFunc)(ObjectData *obj, const StringData* name);
  typedef Variant (*SetFunc)(ObjectData *obj, const StringData* name,
                             Variant& value);
  typedef Variant (*IssetFunc)(ObjectData *obj, const StringData* name);
  typedef Variant (*UnsetFunc)(ObjectData *obj, const StringData* name);

  GetFunc get;      // native magic prop get (analogue of user's `__get`)
  SetFunc set;      // native magic set (user's `__set`)
  IssetFunc isset;  // native magic isset (user's `__isset`)
  UnsetFunc unset;  // native magic unset (user's `__unset`)
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
 * and `unsetProp`. If a method cannot handle property, it should
 * return sigil `Native::prop_not_handled` value.
 *
 * Example:
 *
 * class ElementPropHandler {
 *   static Variant getProp(ObjectData* this_, const StringData* name) {
 *     // get `name` prop
 *   }
 *   ...
 * }
 */

// Default getProp.

template<class T>
Variant nativePropHandlerGet(ObjectData* obj, const StringData* name) {
  return T::getProp(obj, name);
}

// Default setProp.

template<class T>
Variant nativePropHandlerSet(ObjectData* obj,
                             const StringData* name,
                             Variant& value) {
  return T::setProp(obj, name, value);
}

// Default issetProp.

template<class T>
Variant nativePropHandlerIsset(ObjectData* obj, const StringData* name) {
  return T::issetProp(obj, name);
}

// Default unsetProp.

template<class T>
Variant nativePropHandlerUnset(ObjectData* obj, const StringData* name) {
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

/**
 * API methods to call at property resolution (from `object-data`).
 * Example: Native::getProp(this, propName);
 */
Variant getProp(ObjectData* obj, const StringData* name);
Variant setProp(ObjectData* obj, const StringData* name, Variant& value);
Variant issetProp(ObjectData* obj, const StringData* name);
Variant unsetProp(ObjectData* obj, const StringData* name);

inline bool isPropHandled(Variant& propResult) {
  return propResult.isInitialized();
}

//////////////////////////////////////////////////////////////////////////////
}} // namespace HPHP::Native

#endif // _incl_HPHP_RUNTIME_VM_NATIVE_PROP_HANDLER_H
