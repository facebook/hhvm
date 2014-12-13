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
#ifndef incl_HPHP_PERSISTENT_RESOURCE_STORE_H_
#define incl_HPHP_PERSISTENT_RESOURCE_STORE_H_

#include <string>
#include <map>

#include "hphp/util/thread-local.h"

namespace HPHP {

/*
 * The PersistentResourceStore is used to allow some extension resources
 * to last across requests.  These "persistent resources" are
 * thread-local, but not request local, and are managed using this
 * singleton.
 *
 * Take care when creating these resources that they do not maintain
 * pointers into the request local ("smart") heap across requests.
 * The resources themselves also must be allocated outside of the
 * request local heap.
 */
template<class K, class T>
struct PersistentResourceStore {
  PersistentResourceStore() = default;
  PersistentResourceStore(const PersistentResourceStore&) = delete;
  PersistentResourceStore& operator=(const PersistentResourceStore&) = delete;
  ~PersistentResourceStore() {
    for (auto& e : m_objects) removeObject(e.second);
  }

  void set(K name, T obj);
  T get(K name) { return m_objects[name]; }
  void remove(K name);

  const std::map<K,T>& getMap() {
    return m_objects;
  }

private:
  void removeObject(T data);

private:
  std::map<K,T> m_objects;
};

template<class K, class T>
void PersistentResourceStore<K,T>::removeObject(T r) {
  if (!r) return;
  if (!decRefRes(r)) r->decPersistent();
}

template<class K, class T>
void PersistentResourceStore<K,T>::set(K name, T resource) {
  auto iter = m_objects.find(name);
  if (iter != m_objects.end()) {
    if (iter->second == resource) {
      return; // we are setting the same object
    }
    removeObject(iter->second);
    m_objects.erase(iter);
  }
  if (resource) {
    resource->incRefCount();
    resource->incPersistent();
    m_objects[name] = resource;
  }
}

template<class K, class T>
void PersistentResourceStore<K,T>::remove(K name) {
  auto iter = m_objects.find(name);
  if (iter != m_objects.end()) {
    removeObject(iter->second);
    m_objects.erase(iter);
  }
}
}
#endif
