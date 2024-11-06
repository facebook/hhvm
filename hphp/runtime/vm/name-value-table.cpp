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

#include "hphp/runtime/vm/jit/prof-data-serialize.h"
#include "hphp/runtime/vm/jit/type.h"

#include "hphp/util/configs/eval.h"

#include <tbb/concurrent_hash_map.h>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

NameValueTable::NameValueTable() {
  allocate(folly::nextPowTwo(Cfg::Eval::VMInitialGlobalTableSize));
}

NameValueTable::~NameValueTable() {
  for (Elm* elm = &m_table[m_tabMask]; elm != &m_table[-1]; --elm) {
    if (elm->m_name) {
      decRefStr(const_cast<StringData*>(elm->m_name));
      if (elm->m_link.bound()) {
        if (elm->m_link.isInitNoProfile()) {
          tvDecRefGen(elm->m_link.getNoProfile());
          elm->m_link.markUninit();
        }
      } else {
        tvDecRefGen(elm->m_tv);
      }
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
  if (elm->m_link.bound()) {
    if (!elm->m_link.isInitNoProfile()) return;
    tvDecRefGen(elm->m_link.getNoProfile());
    elm->m_link.markUninit();
    return;
  }
  tvUnset(&elm->m_tv);
}

tv_lval NameValueTable::lookup(const StringData* name) {
  Elm* elm = findElm(name);
  if (!elm) return tv_lval{};
  if (elm->m_link.bound()) {
    if (!elm->m_link.isInitNoProfile()) return tv_lval{};
    return elm->m_link.getNoProfile();
  }
  auto const lval = &elm->m_tv;
  return type(lval) == KindOfUninit ? tv_lval{} : lval;
}

tv_lval NameValueTable::lookupAdd(const StringData* name) {
  auto const val = findTypedValue(name);
  if (type(val) == KindOfUninit) tvWriteNull(val);
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
  if (elm->m_link.bound()) {
    if (!elm->m_link.isInitNoProfile()) {
      elm->m_link.initWith(make_tv<KindOfNull>());
    }
    return elm->m_link.getNoProfile();
  }
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
      elm->m_link = decltype(elm->m_link){};
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
    // Attempt to hook this entry up to an existing RDS slot.
    elm->m_link = linkForGlobal(name);
    ++m_elms;
    name->incRefCount();
  }
  return elm;
}

void NameValueTable::rehash(Elm* const oldTab, const size_t oldMask) {
  for (Elm* srcElm = &oldTab[oldMask]; srcElm != &oldTab[-1]; --srcElm) {
    if (srcElm->m_name) {
      if (debug) {
        if (srcElm->m_link.bound()) {
          if (srcElm->m_link.isInitNoProfile()) {
            always_assert(tvIsPlausible(*srcElm->m_link.getNoProfile()));
          }
        } else {
          always_assert(tvIsPlausible(srcElm->m_tv));
        }
      }
      Elm* dstElm = insertImpl(srcElm->m_name);
      dstElm->m_name = srcElm->m_name;
      dstElm->m_tv = srcElm->m_tv;
      dstElm->m_link = srcElm->m_link;
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
  auto const& elem = m_tab->m_table[m_idx];
  if (elem.m_link.bound()) {
    assertx(elem.m_link.isInitNoProfile());
    return elem.m_link.getNoProfile();
  }
  return &elem.m_tv;
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
  auto const& elem = m_tab->m_table[m_idx];
  if (!elem.m_name) return true;
  if (elem.m_link.bound()) return !elem.m_link.isInitNoProfile();
  return type(elem.m_tv) == KindOfUninit;
}

//////////////////////////////////////////////////////////////////////

namespace {

struct ProfileMeta {
  uint64_t count{0};
  uint64_t present{0};
  jit::Type type{jit::TBottom};
};

using ProfileMap = tbb::concurrent_hash_map<
  const StringData*,
  ProfileMeta,
  pointer_hash<StringData>
>;
ProfileMap s_profiling;

struct PreAllocElem {
  rds::Link<TypedValue, rds::Mode::Normal> link;
  jit::Type type;
  bool mainlyPresent;
};

hphp_fast_map<
  const StringData*,
  PreAllocElem,
  hphp_string_hash,
  hphp_string_same
> s_prealloc;

}

bool shouldProfileGlobals() {
  return isJitSerializing() && Cfg::Eval::ProfileGlobalsLimit > 0;
}

void profileGlobal(const StringData* name) {
  // Look up the global. Record if it's present or not. If it is
  // present, record it's type.
  assertx(shouldProfileGlobals());
  if (!name->isStatic()) name = makeStaticString(name);
  auto const tv = g_context->m_globalNVTable->lookup(name);
  ProfileMap::accessor acc;
  s_profiling.insert(acc, name);
  ++acc->second.count;
  if (tv) {
    ++acc->second.present;
    acc->second.type |= jit::typeFromTV(tv, nullptr);
  }
}

rds::Link<TypedValue, rds::Mode::Normal> linkForGlobal(const StringData* name) {
  auto const it = s_prealloc.find(name);
  if (it == s_prealloc.end()) return rds::Link<TypedValue, rds::Mode::Normal>{};
  return it->second.link;
}

bool globalMainlyPresent(const StringData* name) {
  auto const it = s_prealloc.find(name);
  if (it == s_prealloc.end()) return false;
  return it->second.mainlyPresent;
}

jit::Type predictedTypeForGlobal(const StringData* name) {
  auto const it = s_prealloc.find(name);
  if (it == s_prealloc.end()) return jit::TCell;
  return it->second.type;
}

void writeGlobalProfiles(jit::ProfDataSerializer& ser) {
  if (!shouldProfileGlobals()) {
    // Always write an entry, even if we're not profiling. This means
    // we can safely load if the options are different.
    jit::write_raw(ser, uint32_t(0));
    return;
  }

  // Sort the profiled data by the total count:
  struct SortedElem {
    const StringData* name;
    ProfileMeta meta;
  };
  std::vector<SortedElem> sorted;
  sorted.reserve(s_profiling.size());
  for (auto const& s : s_profiling) {
    sorted.emplace_back(SortedElem{s.first, s.second});
  }
  std::sort(
    sorted.begin(),
    sorted.end(),
    [&] (const SortedElem& v1, const SortedElem& v2) {
      if (v1.meta.count != v2.meta.count) {
        return v1.meta.count > v2.meta.count;
      }
      return strcmp(v1.name->data(), v2.name->data()) < 0;
    }
  );

  // Only write out the first EvalProfileGlobalsLimit entries. These
  // will be the hottest.
  auto const size =
    std::min<uint32_t>(Cfg::Eval::ProfileGlobalsLimit, sorted.size());
  jit::write_raw(ser, uint32_t{size});
  for (uint32_t i = 0; i < size; ++i) {
    jit::write_string(ser, sorted[i].name);

    // Record whether this global was present enough to side-exit for
    // the missing case.
    auto const success = sorted[i].meta.present / double(sorted[i].meta.count);
    jit::write_raw(ser, success >= Cfg::Eval::ProfileGlobalsSlowExitThreshold);

    auto type = sorted[i].meta.type;
    // TBottom means it was never present. Nothing to guard on this
    // case.
    if (type == jit::TBottom) type = jit::TCell;
    type = jit::relaxToGuardable(type);
    type.serialize(ser);
  }
}

void readGlobalProfiles(jit::ProfDataDeserializer& ser) {
  auto const size = jit::read_raw<uint32_t>(ser);
  for (uint32_t i = 0; i < size; ++i) {
    auto const name = jit::read_string(ser);
    auto const mainlyPresent = jit::read_raw<bool>(ser);
    auto const type = jit::Type::deserialize(ser);
    if (Cfg::Eval::ProfileGlobalsLimit <= 0) continue;
    auto const link = rds::alloc<TypedValue, rds::Mode::Normal>();
    s_prealloc.emplace(name, PreAllocElem{link, type, mainlyPresent});
  }
}

//////////////////////////////////////////////////////////////////////

}
