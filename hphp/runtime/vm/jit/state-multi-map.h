/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_JIT_STATE_MULTIMAP_H_
#define incl_HPHP_JIT_STATE_MULTIMAP_H_

#include <type_traits>
#include <map>

#include "hphp/runtime/vm/jit/ir-unit.h"

namespace HPHP { namespace jit {

//////////////////////////////////////////////////////////////////////

/*
 * Utility to keep a multi-map of state about each key, indexed by
 * key->id(), where key can be an IRInstruction, Block, or SSATmp.
 */
template<class Key, class Info>
struct StateMultiMap {
  using InfoMap        = std::multimap<uint32_t, Info>;
  using iterator       = typename InfoMap::iterator;
  using const_iterator = typename InfoMap::const_iterator;
  using range          = std::pair<iterator, iterator>;
  using const_range    = std::pair<const_iterator, const_iterator>;

  static_assert(
    std::is_same<Key,Block>::value ||
    std::is_same<Key,IRInstruction>::value ||
    std::is_same<Key,SSATmp>::value,
    "StateMultiMap can only be used with Block, IRInstruction, or SSATmp"
  );

  StateMultiMap() { }

  StateMultiMap(const StateMultiMap&) = default;
  StateMultiMap(StateMultiMap&&) = default;
  StateMultiMap& operator=(const StateMultiMap&) = delete;
  StateMultiMap& operator=(StateMultiMap&&) = default;

  void clear() { m_info.clear(); }

  range operator[](uint32_t id) {
    return m_info.equal_range(id);
  }

  const_range operator[](uint32_t id) const {
    return m_info.equal_range(id);
  }

  range operator[](const Key& k) { return (*this)[k.id()]; }
  range operator[](const Key* k) { return (*this)[k->id()]; }

  const_range operator[](const Key& k) const { return (*this)[k.id()]; }
  const_range operator[](const Key* k) const { return (*this)[k->id()]; }

  iterator insert(const Key& k, const Info& info) {
    return insert(k.id(), info);
  }
  iterator insert(uint32_t id, const Info& info) {
    return m_info.insert(std::make_pair(id, info));
  }

  iterator begin()              { return m_info.begin(); }
  iterator end()                { return m_info.end(); }
  const_iterator begin()  const { return m_info.begin(); }
  const_iterator end()    const { return m_info.end(); }
  const_iterator cbegin() const { return m_info.cbegin(); }
  const_iterator cend()   const { return m_info.cend(); }

private:
  InfoMap m_info;
};

//////////////////////////////////////////////////////////////////////

}}

#endif
