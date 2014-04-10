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

void PersistentResourceStore::removeObject(ResourceData* data) {
  if (!data) return;
  if (!decRefRes(data)) {
    SweepableResourceData *sw = dynamic_cast<SweepableResourceData*>(data);
    if (sw) {
      sw->decPersistent();
    }
  }
}

PersistentResourceStore::~PersistentResourceStore() {
  for (auto iter = m_objects.begin();
       iter != m_objects.end();
       ++iter) {
    auto const& resources = iter->second;
    for (auto iterInner = resources.begin();
         iterInner != resources.end();
         ++iterInner) {
      removeObject(iterInner->second);
    }
  }
}

int PersistentResourceStore::size() const {
  int total = 0;
  for (auto iter = m_objects.begin();
       iter != m_objects.end();
       ++iter) {
    total += iter->second.size();
  }
  return total;
}

void PersistentResourceStore::set(const char *type, const char *name,
                                ResourceData *obj) {
  assert(type && *type);
  assert(name);
  {
    auto& resources = m_objects[type];
    auto iter = resources.find(name);
    if (iter != resources.end()) {
      if (iter->second == obj) {
        return; // we are setting the same object
      }
      removeObject(iter->second);
      resources.erase(iter);
    }
  }
  if (obj) {
    obj->incRefCount();
    SweepableResourceData *sw = dynamic_cast<SweepableResourceData*>(obj);
    if (sw) {
      sw->incPersistent();
    }
    m_objects[type][name] = obj;
  }
}

ResourceData *PersistentResourceStore::get(const char *type, const char *name) {
  assert(type && *type);
  assert(name);
  auto& resources = m_objects[type];
  auto iter = resources.find(name);
  if (iter == resources.end()) {
    return nullptr;
  }
  return iter->second;
}

void PersistentResourceStore::remove(const char *type, const char *name) {
  assert(type && *type);
  assert(name);
  auto& resources = m_objects[type];
  auto iter = resources.find(name);
  if (iter != resources.end()) {
    removeObject(iter->second);
    resources.erase(iter);
  }
}

const std::map<std::string,ResourceData*>&
PersistentResourceStore::getMap(const char *type) {
  assert(type && *type);
  return m_objects[type];
}

//////////////////////////////////////////////////////////////////////

}

