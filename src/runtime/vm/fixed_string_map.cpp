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

#include "runtime/vm/core_types.h"
#include "runtime/vm/fixed_string_map.h"
#include "runtime/base/macros.h"

namespace HPHP {
namespace VM {

TRACE_SET_MOD(runtime);

///////////////////////////////////////////////////////////////////////////////

class Func;

inline bool strEqual(bool case_sensitive,
                     const StringData* sd1, const StringData* sd2) {
  if (sd1 == sd2) return true;
  if (sd1->size() != sd2->size()) return false;
  return case_sensitive ?
      (0 == memcmp(sd1->data(), sd2->data(), sd1->size())) :
      bstrcaseeq(sd1->data(), sd2->data(), sd1->size());
}

template <typename V, bool case_sensitive>
void FixedStringMap<V, case_sensitive>::init(int num) {
  static const double kLoadFactor = 0.80;
  int capac = 1;
  while (num >= kLoadFactor * capac) {
    capac *= 2;
  }
  TRACE(1, "FixedStringMap::init: %d -> %d\n", num, capac);
  ASSERT(!m_table);
  m_table = (Elm*)calloc(capac * sizeof(Elm), 1);
  ASSERT(m_table);
  m_mask = capac - 1;
}

template <typename V, bool case_sensitive>
void FixedStringMap<V, case_sensitive>::add(const StringData* sd, const V& v) {
  ASSERT(sd->isStatic());

  Elm* elm = &m_table[sd->hash() & m_mask];
  UNUSED unsigned numProbes = 0;
  while (elm->sd) {
    ASSERT(numProbes++ < m_mask + 1);
    // Semantics for multiple insertion: new value wins.
    if (strEqual(case_sensitive, elm->sd, sd)) break;
    if (UNLIKELY(++elm == &m_table[m_mask + 1])) elm = m_table;
  }
  elm->sd = sd;
  elm->data = v;
}

template <typename V, bool case_sensitive>
V* FixedStringMap<V, case_sensitive>::find(const StringData* sd) const {
  Elm* elm = &m_table[sd->hash() & m_mask];
  UNUSED unsigned numProbes = 0;
  for(;;) {
    ASSERT(numProbes++ < m_mask + 1);
    if (UNLIKELY(NULL == elm->sd)) return NULL;
    if (strEqual(case_sensitive, elm->sd, sd)) return &elm->data;
    if (UNLIKELY(++elm == &m_table[m_mask + 1])) elm = m_table;
  }
}

template class FixedStringMap<Slot, false>;
template class FixedStringMap<Slot, true>;
template class FixedStringMap<Id, false>;
template class FixedStringMap<Id, true>;
template class FixedStringMap<Func*, false>;
template class FixedStringMap<unsigned char* /* TCA */, true>;

///////////////////////////////////////////////////////////////////////////////

} }
