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
#include "hphp/runtime/base/persistent-resource-store.h"

#include "hphp/runtime/base/complex-types.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

IMPLEMENT_THREAD_LOCAL_NO_CHECK(PersistentResourceStore, g_persistentResources);

//////////////////////////////////////////////////////////////////////

void PersistentResourceStore::removeObject(SweepableResourceData* r) {
  if (!r) return;
  if (!decRefRes(r)) r->decPersistent();
}

PersistentResourceStore::~PersistentResourceStore() {
  for (auto& e : m_objects) {
    for (auto& f : e.second) {
      removeObject(f.second);
    }
  }
}

void PersistentResourceStore::set(std::string type, std::string name,
                                  SweepableResourceData *resource) {
  {
    auto& resources = m_objects[type];
    auto iter = resources.find(name);
    if (iter != resources.end()) {
      if (iter->second == resource) {
        return; // we are setting the same object
      }
      removeObject(iter->second);
      resources.erase(iter);
    }
  }
  if (resource) {
    resource->incRefCount();
    resource->incPersistent();
    m_objects[type][name] = resource;
  }
}

SweepableResourceData*
PersistentResourceStore::get(std::string type, std::string name) {
  auto& resources = m_objects[type];
  auto iter = resources.find(name);
  return iter != resources.end() ? iter->second : nullptr;
}

void PersistentResourceStore::remove(std::string type, std::string name) {
  auto& resources = m_objects[type];
  auto iter = resources.find(name);
  if (iter != resources.end()) {
    removeObject(iter->second);
    resources.erase(iter);
  }
}

const std::map<std::string,SweepableResourceData*>&
PersistentResourceStore::getMap(std::string type) {
  return m_objects[type];
}

//////////////////////////////////////////////////////////////////////

}

