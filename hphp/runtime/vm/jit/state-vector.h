/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/smart-containers.h"
#include "hphp/runtime/vm/jit/ir-unit.h"

namespace HPHP { namespace JIT {

//////////////////////////////////////////////////////////////////////

/*
 * Utility to keep a vector of state about each key, indexed by
 * key->id(), where key can be an IRInstruction, Block, or SSATmp.
 *
 * Takes an `init' element, which everything is defaulted to.  Calls
 * to reset() restore all entries to this state.
 */
template<class Key, class Info>
struct StateVector {
  typedef smart::vector<Info> InfoVector;
  typedef typename InfoVector::iterator iterator;
  typedef typename InfoVector::const_iterator const_iterator;
  typedef typename InfoVector::reference reference;
  typedef typename InfoVector::const_reference const_reference;

  static_assert(
    std::is_same<Key,Block>::value ||
    std::is_same<Key,IRInstruction>::value ||
    std::is_same<Key,SSATmp>::value,
    "StateVector can only be used with Block, IRInstruction, or SSATmp"
  );

  StateVector(const IRUnit& unit, Info init)
    : m_unit(unit)
    , m_info(numIds(unit, static_cast<Key*>(nullptr)), init)
    , m_init(init) {
  }

  StateVector(const StateVector& other)
    : m_unit(other.m_unit)
    , m_info(other.m_info)
    , m_init(other.m_init)
  {}

  StateVector& operator=(const StateVector& other) {
    assert(&other.m_unit == &m_unit);
    m_info = other.m_info;
    m_init = other.m_init;
    return *this;
  }

  void reset() {
    m_info.assign(m_info.size(), m_init);
    grow();
  }

  reference operator[](const Key& k) { return (*this)[&k]; }
  reference operator[](const Key* k) {
    auto id = k->id();
    if (id >= m_info.size()) grow();
    assert(id < m_info.size());
    return m_info[id];
  }

  const_reference operator[](const Key& k) const { return (*this)[&k]; }
  const_reference operator[](const Key* k) const {
    assert(k->id() < numIds(m_unit, (Key*)nullptr));
    auto id = k->id();
    return id < m_info.size() ? m_info[id] : m_init;
  }

  iterator begin()              { return m_info.begin(); }
  iterator end()                { return m_info.end(); }
  const_iterator begin()  const { return m_info.begin(); }
  const_iterator end()    const { return m_info.end(); }
  const_iterator cbegin() const { return m_info.cbegin(); }
  const_iterator cend()   const { return m_info.cend(); }

private:
  static unsigned numIds(const IRUnit& unit, IRInstruction*) {
    return unit.numInsts();
  }
  static unsigned numIds(const IRUnit& unit, SSATmp*) {
    return unit.numTmps();
  }
  static unsigned numIds(const IRUnit& unit, Block*) {
    return unit.numBlocks();
  }

private:
  void grow() {
    m_info.resize(numIds(m_unit, static_cast<Key*>(nullptr)),
                  m_init);
  }

private:
  const IRUnit& m_unit;
  InfoVector m_info;
  Info m_init;
};

//////////////////////////////////////////////////////////////////////

}}

#endif
