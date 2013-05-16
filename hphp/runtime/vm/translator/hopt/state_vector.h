/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include "hphp/runtime/base/memory/memory_manager.h"

#include "hphp/runtime/vm/translator/hopt/irfactory.h"

namespace HPHP {  namespace JIT {

//////////////////////////////////////////////////////////////////////

/*
 * Utility to keep a vector of state about each key, indexed by
 * key->getId(), where key can be an IRInstruction, Block, or SSATmp.
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

  StateVector(const IRFactory* factory, Info init)
    : m_factory(factory)
    , m_info(numIds(factory, (Key*)nullptr), init)
    , m_init(init) {
  }

  void reset() {
    m_info.assign(m_info.size(), m_init);
    grow();
  }

  reference operator[](const Key& k) { return (*this)[&k]; }
  reference operator[](const Key* k) {
    auto id = k->getId();
    if (id >= m_info.size()) grow();
    assert(id < m_info.size());
    return m_info[id];
  }

  const_reference operator[](const Key& k) const { return (*this)[&k]; }
  const_reference operator[](const Key* k) const {
    assert(k->getId() < numIds(m_factory, (Key*)nullptr));
    auto id = k->getId();
    return id < m_info.size() ? m_info[id] : m_init;
  }

  iterator begin()              { return m_info.begin(); }
  iterator end()                { return m_info.end(); }
  const_iterator begin()  const { return m_info.begin(); }
  const_iterator end()    const { return m_info.end(); }
  const_iterator cbegin() const { return m_info.cbegin(); }
  const_iterator cend()   const { return m_info.cend(); }

private:
  static unsigned numIds(const IRFactory* factory, IRInstruction*) {
    return factory->numInsts();
  }
  static unsigned numIds(const IRFactory* factory, SSATmp*) {
    return factory->numTmps();
  }
  static unsigned numIds(const IRFactory* factory, Block*) {
    return factory->numBlocks();
  }

private:
  void grow() {
    m_info.resize(numIds(m_factory, static_cast<Key*>(nullptr)),
                  m_init);
  }

private:
  const IRFactory* m_factory;
  InfoVector m_info;
  Info m_init;
};

//////////////////////////////////////////////////////////////////////

}}

#endif
