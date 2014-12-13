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

#include "hphp/runtime/vm/native-prop-handler.h"
#include "hphp/runtime/base/complex-types.h"

namespace HPHP { namespace Native {
//////////////////////////////////////////////////////////////////////////////
typedef std::unordered_map
  <const StringData*, NativePropHandler> NativePropHandlerMap;

static NativePropHandlerMap s_nativePropHandlerMap;

void registerNativePropHandler(const StringData* className,
                               NativePropHandler::GetFunc get,
                               NativePropHandler::SetFunc set,
                               NativePropHandler::IssetFunc isset,
                               NativePropHandler::UnsetFunc unset) {

  assert(s_nativePropHandlerMap.find(className) ==
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

// API to call from object-data at property resolution.

Variant getProp(ObjectData* obj, const StringData* name) {
  auto nph = obj->getVMClass()->getNativePropHandler();
  assert(nph);
  assert(nph->get);
  return nph->get(obj, name);
}

Variant setProp(ObjectData* obj, const StringData* name, Variant& value) {
  auto nph = obj->getVMClass()->getNativePropHandler();
  assert(nph);
  assert(nph->set);
  return nph->set(obj, name, value);
}

Variant issetProp(ObjectData* obj, const StringData* name) {
  auto nph = obj->getVMClass()->getNativePropHandler();
  assert(nph);
  assert(nph->isset);
  return nph->isset(obj, name);
}

Variant unsetProp(ObjectData* obj, const StringData* name) {
  auto nph = obj->getVMClass()->getNativePropHandler();
  assert(nph);
  assert(nph->unset);
  return nph->unset(obj, name);
}

//////////////////////////////////////////////////////////////////////////////
}} // namespace HPHP::Native
