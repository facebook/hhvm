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

#include "hphp/runtime/vm/fixed-string-map.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/macros.h"

namespace HPHP {

TRACE_SET_MOD(runtime);

///////////////////////////////////////////////////////////////////////////////

class Func;

static const StringData* null_key;

template<bool CaseSensitive>
inline bool strEqual(const StringData* sd1, const StringData* sd2) {
  if (sd1 == sd2) return true;
  return CaseSensitive ? sd1->same(sd2) : sd1->isame(sd2);
}

template<class V, bool CaseSensitive, class E>
void FixedStringMap<V,CaseSensitive,E>::clear() {
  if (m_table != (Elm*)&null_key) free(m_table);
  m_table = nullptr;
  m_mask = 0;
}

template<class V, bool CaseSensitive, class E>
void FixedStringMap<V,CaseSensitive,E>::init(int num) {
  if (!num) {
    m_table = (Elm*)&null_key;
    m_mask = 0;
    return;
  }

  static const double kLoadFactor = 0.80;
  int capac = 1;
  while (num >= kLoadFactor * capac) {
    capac *= 2;
  }
  TRACE(1, "FixedStringMap::init: %d -> %d\n", num, capac);
  assert(!m_table);
  m_table = (Elm*)calloc(capac * sizeof(Elm), 1);
  assert(m_table);
  m_mask = capac - 1;
}

template<class V, bool CaseSensitive, class E>
void FixedStringMap<V,CaseSensitive,E>::add(const StringData* sd, const V& v) {
  assert(sd->isStatic());

  Elm* elm = &m_table[sd->hash() & m_mask];
  UNUSED unsigned numProbes = 0;
  while (elm->sd) {
    assert(numProbes++ < m_mask + 1);
    // Semantics for multiple insertion: new value wins.
    if (strEqual<CaseSensitive>(elm->sd, sd)) break;
    if (UNLIKELY(++elm == &m_table[m_mask + 1])) elm = m_table;
  }
  elm->sd = sd;
  elm->data = v;
}

template<class V, bool CaseSensitive, class E>
V* FixedStringMap<V,CaseSensitive,E>::find(const StringData* sd) const {
  Elm* elm = &m_table[sd->hash() & m_mask];
  UNUSED unsigned numProbes = 0;
  for(;;) {
    assert(numProbes++ < m_mask + 1);
    if (UNLIKELY(nullptr == elm->sd)) return nullptr;
    if (strEqual<CaseSensitive>(elm->sd, sd)) return &elm->data;
    if (UNLIKELY(++elm == &m_table[m_mask + 1])) elm = m_table;
  }
}

template class FixedStringMap<Slot,false,Slot>;
template class FixedStringMap<Slot,true,Slot>;
template class FixedStringMap<Id,false,Id>;
template class FixedStringMap<Id,true,Id>;
template class FixedStringMap<Func*,false,int32_t>;
template class FixedStringMap<unsigned char* /* TCA */,true,int32_t>;

///////////////////////////////////////////////////////////////////////////////

 }
