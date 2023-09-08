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

#include "hphp/runtime/vm/native-prop-handler.h"

namespace HPHP::Native {
//////////////////////////////////////////////////////////////////////////////
using NativePropHandlerMap = hphp_fast_map
  <const StringData*, NativePropHandler>;

static NativePropHandlerMap s_nativePropHandlerMap;

void registerNativePropHandler(const StringData* className,
                               NativePropHandler::GetFunc get,
                               NativePropHandler::SetFunc set,
                               NativePropHandler::IssetFunc isset,
                               NativePropHandler::UnsetFunc unset) {

  assertx(s_nativePropHandlerMap.find(className) ==
    s_nativePropHandlerMap.end());

  NativePropHandler propHandler;
  propHandler.get = get;
  propHandler.set = set;
  propHandler.isset = isset;
  propHandler.unset = unset;

  s_nativePropHandlerMap[className] = propHandler;
}

NativePropHandler* getNativePropHandler(const StringData* className) {
  auto it = s_nativePropHandlerMap.find(className);
  if (it == s_nativePropHandlerMap.end()) {
    return nullptr;
  }
  return &it->second;
}

////////////////////////////////////////////////////////////////////////////////
// PropAccessorMap

PropAccessorMap::PropAccessorMap(PropAccessor* props,
                                 PropAccessorMap *base) {
  if (base) {
    *this = *base;
  }
  for (PropAccessor *p = props; p->name; p++) {
    this->insert(p);
  }
}

bool PropAccessorMap::isPropSupported(const String& name) {
  return lookupProp(name) != end();
}

Variant (*PropAccessorMap::get(const String& name))(const Object& this_) {
  return (*lookupProp(name))->get;
}

void (*PropAccessorMap::set(const String& name))
     (const Object& this_, const Variant& value) {
  return (*lookupProp(name))->set;
}

bool (*PropAccessorMap::isset(const String& name))(const Object& this_) {
  return (*lookupProp(name))->isset;
}

void (*PropAccessorMap::unset(const String& name))(const Object& this_) {
  return (*lookupProp(name))->unset;
}

PropAccessorMap::const_iterator PropAccessorMap::lookupProp(
  const String& name) {
  auto pa = PropAccessor {
    name.data(), nullptr, nullptr, nullptr, nullptr
  };
  return find(&pa);
}

////////////////////////////////////////////////////////////////////////////////
// API to call from object-data at property resolution.

Variant getProp(const Object& obj, const String& name) {
  auto nph = obj->getVMClass()->getNativePropHandler();
  assertx(nph);
  assertx(nph->get);
  return nph->get(obj, name);
}

Variant setProp(const Object& obj, const String& name, const Variant& value) {
  auto nph = obj->getVMClass()->getNativePropHandler();
  assertx(nph);
  assertx(nph->set);
  return nph->set(obj, name, value);
}

Variant issetProp(const Object& obj, const String& name) {
  auto nph = obj->getVMClass()->getNativePropHandler();
  assertx(nph);
  assertx(nph->isset);
  return nph->isset(obj, name);
}

Variant unsetProp(const Object& obj, const String& name) {
  auto nph = obj->getVMClass()->getNativePropHandler();
  assertx(nph);
  assertx(nph->unset);
  return nph->unset(obj, name);
}

//////////////////////////////////////////////////////////////////////////////
} // namespace HPHP::Native
