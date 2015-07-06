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

#include "hphp/runtime/vm/name-value-table.h"

#include <limits>
#include <folly/Bits.h>

#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/runtime.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

NameValueTable::NameValueTable() {
  allocate(folly::nextPowTwo(RuntimeOption::EvalVMInitialGlobalTableSize));
}

NameValueTable::NameValueTable(ActRec* fp)
  : m_fp(fp)
{
  assert(m_fp);
  const auto func = m_fp->m_func;
  const Id numNames = func->numNamedLocals();

  /**
   * Reserve space for all named locals plus one additional variable
   * to avoid reallocations if one extra dynamic variable is used.
   */
  reserve(numNames + 1);

  for (Id i = 0; i < numNames; ++i) {
    assert(func->lookupVarId(func->localVarName(i)) == i);

    auto elm = insert(func->localVarName(i));
    assert(elm && elm->m_tv.m_type == kInvalidDataType);
    elm->m_tv.m_type = kNamedLocalDataType;
    elm->m_tv.m_data.num = i;
  }
}

NameValueTable::NameValueTable(const NameValueTable& nvTable, ActRec* fp)
  : m_fp(fp)
  , m_elms(nvTable.m_elms)
{
  allocate(nvTable.m_tabMask + 1);
  assert(m_tabMask == nvTable.m_tabMask);

  for (int i = 0; i <= m_tabMask; ++i) {
    Elm& src = nvTable.m_table[i];
    Elm& dst = m_table[i];

    dst.m_name = src.m_name;
    if (dst.m_name) {
      dst.m_name->incRefCount();
      if (src.m_tv.m_type == kNamedLocalDataType) {
        dst.m_tv.m_type = kNamedLocalDataType;
        dst.m_tv.m_data.num = src.m_tv.m_data.num;
      } else {
        tvDupFlattenVars(&src.m_tv, &dst.m_tv);
      }
    }
  }
}

NameValueTable::~NameValueTable() {
  if (!m_table) return;

  for (Elm* elm = &m_table[m_tabMask]; elm != &m_table[-1]; --elm) {
    if (elm->m_name) {
      decRefStr(const_cast<StringData*>(elm->m_name));
      if (elm->m_tv.m_type != kNamedLocalDataType) {
        tvRefcountedDecRef(elm->m_tv);
      }
    }
  }
  req::free(m_table);
}

void NameValueTable::suspend(const ActRec* oldFP, ActRec* newFP) {
  assert(m_fp == oldFP);
  assert(oldFP->func() == newFP->func());
  assert(!oldFP->resumed());
  assert(newFP->resumed());

  m_fp = newFP;
}

void NameValueTable::attach(ActRec* fp) {
  assert(m_fp == nullptr);
  m_fp = fp;

  const auto func = fp->m_func;
  const Id numNames = func->numNamedLocals();
  TypedValue* loc = frame_local(fp, 0);

  for (Id i = 0; i < numNames; ++i, --loc) {
    assert(func->lookupVarId(func->localVarName(i)) == i);

    auto elm = insert(func->localVarName(i));
    assert(elm);
    if (elm->m_tv.m_type != kInvalidDataType) {
      assert(elm->m_tv.m_type != kNamedLocalDataType);
      tvCopy(elm->m_tv, *loc);
    }

    elm->m_tv.m_type = kNamedLocalDataType;
    elm->m_tv.m_data.num = i;
  }
}

void NameValueTable::detach(ActRec* fp) {
  assert(m_fp == fp);
  m_fp = nullptr;

  const auto func = fp->m_func;
  const Id numNames = func->numNamedLocals();
  TypedValue* loc = frame_local(fp, 0);

  for (Id i = 0; i < numNames; ++i, --loc) {
    assert(func->lookupVarId(func->localVarName(i)) == i);

    auto elm = findElm(func->localVarName(i));
    assert(elm && elm->m_tv.m_type == kNamedLocalDataType);
    tvCopy(*loc, elm->m_tv);
    tvDebugTrash(loc);
  }
}

void NameValueTable::leak() {
  m_elms = 0;
  m_tabMask = 0;
  req::free(m_table);
  m_table = nullptr;
}

TypedValue* NameValueTable::set(const StringData* name, const TypedValue* val) {
  TypedValue* target = findTypedValue(name);
  tvSet(*tvToCell(val), *target);
  return target;
}

TypedValue* NameValueTable::bind(const StringData* name, TypedValue* val) {
  TypedValue* target = findTypedValue(name);
  if (val->m_type != KindOfRef) {
    tvBox(val);
  }
  tvBind(val, target);
  return target;
}

void NameValueTable::unset(const StringData* name) {
  Elm* elm = findElm(name);
  if (!elm) return;
  tvUnset(derefNamedLocal(&elm->m_tv));
}

TypedValue* NameValueTable::lookup(const StringData* name) {
  Elm* elm = findElm(name);
  if (!elm) return nullptr;
  TypedValue* tv = derefNamedLocal(&elm->m_tv);
  return tv->m_type == KindOfUninit ? nullptr : tv;
}

TypedValue* NameValueTable::lookupAdd(const StringData* name) {
  auto tv = findTypedValue(name);
  if (tv->m_type == KindOfUninit) {
    tvWriteNull(tv);
  }
  return tv;
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
  assert(folly::isPowTwo(newCapac));
  assert(newCapac - 1 <= std::numeric_limits<uint32_t>::max());

  Elm* oldTab = m_table;
  const size_t oldMask = m_tabMask;

  m_table = static_cast<Elm*>(req::calloc(sizeof(Elm), newCapac));
  m_tabMask = uint32_t(newCapac - 1);

  if (oldTab) {
    rehash(oldTab, oldMask);
    req::free(oldTab);
  }
}

TypedValue* NameValueTable::derefNamedLocal(TypedValue* tv) const {
  return tv->m_type == kNamedLocalDataType
    ? frame_local(m_fp, tv->m_data.num)
    : tv;
}

TypedValue* NameValueTable::findTypedValue(const StringData* name) {
  Elm* elm = insert(name);
  if (elm->m_tv.m_type == kInvalidDataType) {
    tvWriteNull(&elm->m_tv);
    return &elm->m_tv;
  }
  return derefNamedLocal(&elm->m_tv);
}

NameValueTable::Elm* NameValueTable::insertImpl(const StringData* name) {
  Elm* elm = &m_table[name->hash() & m_tabMask];
  UNUSED size_t numProbes = 0;
  for (;;) {
    assert(numProbes++ < m_tabMask + 1);
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
      assert(srcElm->m_tv.m_type == kNamedLocalDataType ||
             tvIsPlausible(srcElm->m_tv));
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
    assert(numProbes++ < m_tabMask + 1);
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

NameValueTable::Iterator::Iterator(const NameValueTable* tab, ssize_t pos)
  : m_tab(tab)
  , m_idx(pos)
{
  assert(pos >= 0);
  if (!valid()) next();
}

NameValueTable::Iterator::Iterator(const NameValueTable* tab,
                                   const StringData* start)
  : m_tab(tab)
{
  Elm* e = m_tab->findElm(start);
  m_idx = e ? e - m_tab->m_table : (m_tab->m_tabMask + 1);
}

ssize_t NameValueTable::Iterator::toInteger() const {
  const ssize_t invalid = m_tab->m_tabMask + 1;
  return valid() ? m_idx : invalid;
}

bool NameValueTable::Iterator::valid() const {
  return m_idx >= 0 && size_t(m_idx) < m_tab->m_tabMask + 1 && !atEmpty();
}

const StringData* NameValueTable::Iterator::curKey() const {
  assert(valid());
  return m_tab->m_table[m_idx].m_name;
}

const TypedValue* NameValueTable::Iterator::curVal() const {
  assert(valid());
  return m_tab->derefNamedLocal(&m_tab->m_table[m_idx].m_tv);
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
  const auto tv = m_tab->derefNamedLocal(&m_tab->m_table[m_idx].m_tv);
  return tv->m_type == KindOfUninit;
}

//////////////////////////////////////////////////////////////////////

}
