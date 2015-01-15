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

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class Func;

///////////////////////////////////////////////////////////////////////////////

namespace FSM {

extern const StringData* null_key;

template<bool CaseSensitive>
inline bool strEqual(const StringData* sd1, const StringData* sd2) {
  if (sd1 == sd2) return true;
  return CaseSensitive ? sd1->same(sd2) : sd1->isame(sd2);
}

}

template<class V, bool CaseSensitive, class E>
NEVER_INLINE
void FixedStringMap<V,CaseSensitive,E>::clear() {
  if (m_table && m_table != (Elm*)&FSM::null_key + 1) {
    free(m_table - m_mask - 1);
  }
  m_table = nullptr;
  m_mask = 0;
}

template<class V, bool CaseSensitive, class E>
NEVER_INLINE
void FixedStringMap<V,CaseSensitive,E>::init(int num, uint32_t numExtraBytes) {
  if (!num && !numExtraBytes) {
    m_table = (Elm*)&FSM::null_key + 1;
    m_mask = 0;
    return;
  }

  static const double kLoadFactor = 0.80;
  int capac = 1;
  while (num >= kLoadFactor * capac) {
    capac *= 2;
  }
  TRACE_MOD(Trace::runtime, 1, "FixedStringMap::init: %d -> %d\n", num, capac);
  assert(!m_table);
  m_table = (Elm*)calloc(capac * sizeof(Elm) + numExtraBytes, 1) + capac;
  assert(m_table);
  m_mask = capac - 1;
}

template<class V, bool CaseSensitive, class E>
NEVER_INLINE
void FixedStringMap<V,CaseSensitive,E>::add(const StringData* sd, const V& v) {
  assert(sd->isStatic());

  Elm* elm = &m_table[-1 - int32_t(sd->hash() & m_mask)];
  UNUSED unsigned numProbes = 0;
  while (elm->sd) {
    assert(numProbes++ < m_mask + 1);
    // Semantics for multiple insertion: new value wins.
    if (FSM::strEqual<CaseSensitive>(elm->sd, sd)) break;
    if (UNLIKELY(++elm == m_table)) elm -= m_mask + 1;
  }
  elm->sd = sd;
  elm->data = v;
}

template<class V, bool CaseSensitive, class E>
NEVER_INLINE
V* FixedStringMap<V,CaseSensitive,E>::find(const StringData* sd) const {
  Elm* elm = &m_table[-1 - int32_t(sd->hash() & m_mask)];
  UNUSED unsigned numProbes = 0;
  for(;;) {
    assert(numProbes++ < m_mask + 1);
    if (UNLIKELY(nullptr == elm->sd)) return nullptr;
    if (FSM::strEqual<CaseSensitive>(elm->sd, sd)) return &elm->data;
    if (UNLIKELY(++elm == m_table)) elm -= m_mask + 1;
  }
}

///////////////////////////////////////////////////////////////////////////////

template<class T, class V, bool case_sensitive, class E>
inline
T& FixedStringMapBuilder<T,V,case_sensitive,E>::operator[](V idx) {
  assert(idx >= 0);
  assert(size_t(idx) < m_list.size());
  return m_list[idx];
}

template<class T, class V, bool case_sensitive, class E>
inline
const T& FixedStringMapBuilder<T,V,case_sensitive,E>::operator[](V idx) const {
  return (*const_cast<FixedStringMapBuilder*>(this))[idx];
}

template<class T, class V, bool case_sensitive, class E>
inline
void FixedStringMapBuilder<T,V,case_sensitive,E>::add(const StringData* name,
                                                      const T& t) {
  if (m_list.size() >= size_t(std::numeric_limits<V>::max())) {
    assert(false && "FixedStringMap::Builder overflowed");
    abort();
  }
  m_map[name] = m_list.size();
  m_list.push_back(t);
}

template<class T, class V, bool case_sensitive, class E>
inline
void FixedStringMapBuilder<T,V,case_sensitive,E>::addUnnamed(const T& t) {
  m_list.push_back(t);
}

template<class T, class V, bool case_sensitive, class E>
inline
void FixedStringMapBuilder<T,V,case_sensitive,E>::create(FSMap& map) {
  map.extra() = size();
  map.init(size(), 0);
  if (!size()) {
    return;
  }
  for (const_iterator it = begin(); it != end(); ++it) {
    map.add(it->first, it->second);
  }
}

///////////////////////////////////////////////////////////////////////////////
}
