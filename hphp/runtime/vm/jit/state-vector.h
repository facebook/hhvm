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

#ifndef incl_HPHP_JIT_STATE_VECTOR_H_
#define incl_HPHP_JIT_STATE_VECTOR_H_

#include <type_traits>

#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/ir-unit.h"

namespace HPHP { namespace jit {

//////////////////////////////////////////////////////////////////////

/*
 * Utility to keep a vector of state about each key, indexed by
 * key->id(), where key can be an IRInstruction, Block, or SSATmp.
 *
 * Takes an `init' element, which everything is defaulted to.
 */
template<class Key, class Info>
struct StateVector {
  using InfoVector      = jit::vector<Info>;
  using iterator        = typename InfoVector::iterator;
  using const_iterator  = typename InfoVector::const_iterator;
  using reference       = typename InfoVector::reference;
  using const_reference = typename InfoVector::const_reference;

  static_assert(
    std::is_same<Key,Block>::value ||
    std::is_same<Key,IRInstruction>::value ||
    std::is_same<Key,SSATmp>::value,
    "StateVector can only be used with Block, IRInstruction, or SSATmp"
  );

  StateVector(const IRUnit& unit, Info init)
    : m_unit(&unit)
    , m_init(std::move(init))
    , m_info(unit.numIds(nullKey), m_init)
  {}

  StateVector(const StateVector&) = default;
  StateVector(StateVector&&) = default;
  StateVector& operator=(const StateVector&) = delete;
  StateVector& operator=(StateVector&&) = default;

  reference operator[](uint32_t id) {
    if (id >= m_info.size()) grow();
    return m_info[id];
  }

  const_reference operator[](uint32_t id) const {
    assertx(id < m_unit->numIds(nullKey));
    return id < m_info.size() ? m_info[id] : m_init;
  }

  reference operator[](const Key& k) { return (*this)[k.id()]; }
  reference operator[](const Key* k) { return (*this)[k->id()]; }

  const_reference operator[](const Key& k) const { return (*this)[k.id()]; }
  const_reference operator[](const Key* k) const { return (*this)[k->id()]; }

  iterator begin()              { return m_info.begin(); }
  iterator end()                { return m_info.end(); }
  const_iterator begin()  const { return m_info.begin(); }
  const_iterator end()    const { return m_info.end(); }
  const_iterator cbegin() const { return m_info.cbegin(); }
  const_iterator cend()   const { return m_info.cend(); }

  const IRUnit& unit() const { return *m_unit; }

private:
  void grow() {
    m_info.resize(m_unit->numIds(nullKey), m_init);
  }

private:
  static constexpr Key* nullKey { nullptr };
  const IRUnit* m_unit;
  Info m_init;
  InfoVector m_info;
};

//////////////////////////////////////////////////////////////////////

}}

#endif
