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

#ifndef incl_HPHP_ROOT_MAP_H_
#define incl_HPHP_ROOT_MAP_H_

#include "hphp/runtime/base/req-hash-map.h"
#include "hphp/runtime/base/req-ptr.h"

namespace HPHP {

/*
 * RootMap wraps a simple id->object hashtable, useful for extensions
 * that need pass 'handles' to objects or resources into 3rd party apis
 * masked as integers or void* pointers. The map is intended to be a
 * field of a thread local, where it will be automatically scanned by
 * the gc, and must be reset() between requests.
 */
template<class T> struct RootMap {
  using RootId = uintptr_t;
  using Map = req::fast_map<RootId,req::ptr<T>>;

  RootId addRoot(req::ptr<T>&& ptr) {
    assertx(ptr);
    auto const tok = reinterpret_cast<uintptr_t>(ptr.get());
    assertx(!containsKey(tok));
    getMap().emplace(tok, std::move(ptr));
    return tok;
  }

  RootId addRoot(const req::ptr<T>& ptr) {
    assertx(ptr);
    auto const tok = reinterpret_cast<uintptr_t>(ptr.get());
    assertx(!containsKey(tok));
    getMap()[tok] = ptr;
    return tok;
    static_assert(sizeof(tok) <= sizeof(RootId), "");
  }

  bool containsKey(RootId tok) const {
    if (!m_map) return false;
    auto& map = *m_map;
    auto it = map.find(tok);
    return it != map.end();
  }

  req::ptr<T> lookupRoot(const void* vp) const {
    return lookupRoot(reinterpret_cast<RootId>(vp));
  }

  req::ptr<T> lookupRoot(RootId tok) const {
    if (!m_map) return nullptr;
    auto& map = *m_map;
    auto it = map.find(tok);
    return it != map.end() ? unsafe_cast_or_null<T>(it->second) : nullptr;
  }

  req::ptr<T> removeRoot(RootId tok) {
    if (m_map) {
      auto it = m_map->find(tok);
      if (it != m_map->end()) {
        auto ptr = std::move(it->second);
        m_map->erase(it);
        return unsafe_cast_or_null<T>(ptr);
      }
    }
    return nullptr;
  }

  req::ptr<T> removeRoot(const void* vp) {
    return removeRoot(reinterpret_cast<RootId>(vp));
  }

  bool removeRoot(const req::ptr<T>& ptr) {
    auto const tok = reinterpret_cast<uintptr_t>(ptr.get());
    return removeRoot(tok) != nullptr;
  }

  bool removeRoot(const T* ptr) {
    auto const tok = reinterpret_cast<uintptr_t>(ptr);
    return removeRoot(tok) != nullptr;
  }

  void reset() {
    m_map = nullptr;
  }

private:
  Map& getMap() {
    if (UNLIKELY(!m_map)) {
      m_map = req::make_raw<Map>();
    }
    return *m_map;
  }
  Map* m_map{nullptr};
};

}
#endif
