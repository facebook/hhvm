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

#include "hphp/runtime/vm/name-value-table.h"

#include <limits>
#include <folly/Bits.h>

#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/runtime.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

NameValueTable::NameValueTable() {
  allocate(folly::nextPowTwo(RuntimeOption::EvalVMInitialGlobalTableSize));
}

NameValueTable::~NameValueTable() {
  for (Elm* elm = &m_table[m_tabMask]; elm != &m_table[-1]; --elm) {
    if (elm->m_name) {
      decRefStr(const_cast<StringData*>(elm->m_name));
      tvDecRefGen(elm->m_tv);
    }
  }
  req::free(m_table);
}

tv_lval NameValueTable::set(const StringData* name, tv_rval val) {
  auto const target = findTypedValue(name);
  tvSet(*val, target);
  return target;
}

void NameValueTable::unset(const StringData* name) {
  Elm* elm = findElm(name);
  if (!elm) return;
  tvUnset(&elm->m_tv);
}

tv_lval NameValueTable::lookup(const StringData* name) {
  Elm* elm = findElm(name);
  if (!elm) return nullptr;
  auto const lval = &elm->m_tv;
  return type(lval) == KindOfUninit ? tv_lval{} : lval;
}

tv_lval NameValueTable::lookupAdd(const StringData* name) {
  auto const val = findTypedValue(name);
  if (type(val) == KindOfUninit) {
    tvWriteNull(val);
  }
  return val;
}

void NameValueTable::reserve(size_t desiredSize) {
  /*
   * Reserve space for size * 4 / 3 because we limit our max load
   * factor to .75.
   *
   * Add one because the way we compute whether there is enough
   * space on lookupAdd will look at m_elms + 1.
   */
  const size_t reqCapac = desiredSize * 4 / 3 + 1;
  const size_t curCapac = m_tabMask + 1;

  if (reqCapac > curCapac) {
    allocate(folly::nextPowTwo(reqCapac));
  }
}

void NameValueTable::allocate(const size_t newCapac) {
  assertx(folly::isPowTwo(newCapac));
  assertx(newCapac - 1 <= std::numeric_limits<uint32_t>::max());

  Elm* oldTab = m_table;
  const size_t oldMask = m_tabMask;

  m_table = req::calloc_raw_array<Elm>(newCapac);
  m_tabMask = uint32_t(newCapac - 1);

  if (oldTab) {
    rehash(oldTab, oldMask);
    req::free(oldTab);
  }
}

tv_lval NameValueTable::findTypedValue(const StringData* name) {
  Elm* elm = insert(name);
  if (elm->m_tv.m_type == kInvalidDataType) {
    tvWriteNull(elm->m_tv);
  }
  return &elm->m_tv;
}

NameValueTable::Elm* NameValueTable::insertImpl(const StringData* name) {
  Elm* elm = &m_table[name->hash() & m_tabMask];
  UNUSED size_t numProbes = 0;
  for (;;) {
    assertx(numProbes++ < m_tabMask + 1);
    if (nullptr == elm->m_name) {
      elm->m_name = name;
      elm->m_tv.m_type = kInvalidDataType;
      return elm;
    }
    if (name->same(elm->m_name)) {
      return elm;
    }
    if (UNLIKELY(++elm == &m_table[m_tabMask + 1])) {
      elm = m_table;
    }
  }
}

NameValueTable::Elm* NameValueTable::insert(const StringData* name) {
  reserve(m_elms + 1);
  Elm* elm = insertImpl(name);
  if (elm->m_tv.m_type == kInvalidDataType) {
    ++m_elms;
    name->incRefCount();
  }
  return elm;
}

void NameValueTable::rehash(Elm* const oldTab, const size_t oldMask) {
  for (Elm* srcElm = &oldTab[oldMask]; srcElm != &oldTab[-1]; --srcElm) {
    if (srcElm->m_name) {
      assertx(tvIsPlausible(srcElm->m_tv));
      Elm* dstElm = insertImpl(srcElm->m_name);
      dstElm->m_name = srcElm->m_name;
      dstElm->m_tv = srcElm->m_tv;
    }
  }
}

NameValueTable::Elm* NameValueTable::findElm(const StringData* name) const {
  Elm* elm = &m_table[name->hash() & m_tabMask];
  UNUSED size_t numProbes = 0;
  for (;;) {
    assertx(numProbes++ < m_tabMask + 1);
    if (UNLIKELY(nullptr == elm->m_name)) {
      return nullptr;
    }
    if (name->same(elm->m_name)) return elm;
    if (UNLIKELY(++elm == &m_table[m_tabMask + 1])) {
      elm = m_table;
    }
  }
}

//////////////////////////////////////////////////////////////////////

NameValueTable::Iterator::Iterator(const NameValueTable* tab)
  : m_tab(tab)
  , m_idx(0)
{
  if (!valid()) next();
}

NameValueTable::Iterator
NameValueTable::Iterator::getLast(const NameValueTable* tab) {
  Iterator it;
  it.m_tab = tab;
  it.m_idx = tab->m_tabMask + 1;
  it.prev();
  return it;
}

NameValueTable::Iterator
NameValueTable::Iterator::getEnd(const NameValueTable* tab) {
  Iterator it;
  it.m_tab = tab;
  it.m_idx = tab->m_tabMask + 1;
  return it;
}

ssize_t NameValueTable::Iterator::toInteger() const {
  const ssize_t invalid = m_tab->m_tabMask + 1;
  return valid() ? m_idx : invalid;
}

bool NameValueTable::Iterator::valid() const {
  return m_idx >= 0 && size_t(m_idx) < m_tab->m_tabMask + 1 && !atEmpty();
}

const StringData* NameValueTable::Iterator::curKey() const {
  assertx(valid());
  return m_tab->m_table[m_idx].m_name;
}

tv_rval NameValueTable::Iterator::curVal() const {
  assertx(valid());
  return &m_tab->m_table[m_idx].m_tv;
}

void NameValueTable::Iterator::next() {
  size_t const sz = m_tab->m_tabMask + 1;
  if (m_idx + 1 >= sz) {
    m_idx = sz;
    return;
  }
  ++m_idx;
  if (LIKELY(!atEmpty())) {
    return;
  }
  do {
    ++m_idx;
  } while (size_t(m_idx) < sz && atEmpty());
}

void NameValueTable::Iterator::prev() {
  do {
    --m_idx;
  } while (m_idx >= 0 && atEmpty());
}

bool NameValueTable::Iterator::atEmpty() const {
  if (!m_tab->m_table[m_idx].m_name) {
    return true;
  }
  auto const val = &m_tab->m_table[m_idx].m_tv;
  return type(val) == KindOfUninit;
}

//////////////////////////////////////////////////////////////////////

}
