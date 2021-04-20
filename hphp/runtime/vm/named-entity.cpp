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

#include "hphp/runtime/vm/named-entity.h"

#include "hphp/runtime/base/perf-warning.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/type-string.h"

#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/reverse-data-map.h"
#include "hphp/runtime/vm/type-alias.h"
#include "hphp/runtime/vm/unit-util.h"

#include <folly/AtomicHashMap.h>

#include <atomic>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

rds::Handle NamedEntity::getFuncHandle(const StringData* name) const {
  auto const mode =
    RO::RepoAuthoritative ? rds::Mode::Persistent : rds::Mode::Normal;
  m_cachedFunc.bind(mode, rds::LinkName{"NEFunc", name});
  return m_cachedFunc.handle();
}

void NamedEntity::setCachedFunc(Func* f) {
  *m_cachedFunc = f;
  if (m_cachedFunc.isNormal()) {
    f ? m_cachedFunc.markInit() : m_cachedFunc.markUninit();
  }
}

rds::Handle NamedEntity::getClassHandle(const StringData* name) const {
  auto const mode =
    RO::RepoAuthoritative ? rds::Mode::Persistent : rds::Mode::Normal;
  m_cachedClass.bind(mode, rds::LinkName{"NEClass", name});
  return m_cachedClass.handle();
}

rds::Handle NamedEntity::getRecordDescHandle(const StringData* name) const {
  auto const mode =
    RO::RepoAuthoritative ? rds::Mode::Persistent : rds::Mode::Normal;
  m_cachedRecordDesc.bind(mode, rds::LinkName{"NERecord", name});
  return m_cachedRecordDesc.handle();
}

void NamedEntity::setCachedClass(Class* f) {
  *m_cachedClass = f;
  if (m_cachedClass.isNormal()) {
    f ? m_cachedClass.markInit() : m_cachedClass.markUninit();
  }
}

void NamedEntity::setCachedRecordDesc(RecordDesc* r) {
  *m_cachedRecordDesc = r;
  if (m_cachedRecordDesc.isNormal()) {
    r ? m_cachedRecordDesc.markInit() : m_cachedRecordDesc.markUninit();
  }
}

bool NamedEntity::isPersistentTypeAlias() const {
  return m_cachedTypeAlias.bound() && m_cachedTypeAlias.isPersistent();
}

void NamedEntity::setCachedTypeAlias(const TypeAlias& td) {
  if (!m_cachedTypeAlias.isInit()) {
    m_cachedTypeAlias.initWith(td);
  } else {
    *m_cachedTypeAlias = td;
  }
}

const TypeAlias* NamedEntity::getCachedTypeAlias() const {
  return m_cachedTypeAlias.bound() &&
         m_cachedTypeAlias.isInit() &&
         m_cachedTypeAlias->name()
    ? m_cachedTypeAlias.get()
    : nullptr;
}

void NamedEntity::setCachedReifiedGenerics(ArrayData* a) {
  if (!m_cachedReifiedGenerics.isInit()) {
    m_cachedReifiedGenerics.initWith(a);
  } else {
    *m_cachedReifiedGenerics = a;
  }
}

namespace {
template<typename T>
typename std::enable_if<std::is_same<T, Class>::value, void>::type
deregister(T* goner) {
  if (RuntimeOption::EvalEnableReverseDataMap) {
    // This deregisters Classes registered to data_map in Unit::defClass().
    data_map::deregister(goner);
  }
}
template<typename T>
typename std::enable_if<!std::is_same<T, Class>::value, void>::type
deregister(T* goner) {}

template<class T>
void pushImpl(T* type, NamedEntity::ListType<T>& list) {
  assertx(type->m_next == nullptr);
  type->m_next = list;
  list = type;
}

template<class T>
void removeImpl(T* goner, NamedEntity::ListType<T>& list) {
  T* head = list;
  if (!head) return;

  deregister(goner);

  if (head == goner) {
    list = head->m_next;
    return;
  }
  auto t = &(head->m_next);
  while (t->get() != goner) {
    assertx(*t);
    t = &((*t)->m_next);
  }
  *t = goner->m_next;
}
}

void NamedEntity::pushRecordDesc(RecordDesc* rec) {
  pushImpl(rec, m_recordList);
}

void NamedEntity::removeRecordDesc(RecordDesc* goner) {
  removeImpl(goner, m_recordList);
}

void NamedEntity::pushClass(Class* cls) {
  pushImpl(cls, m_clsList);
}

void NamedEntity::removeClass(Class* goner) {
  removeImpl(goner, m_clsList);
}

void NamedEntity::setUniqueFunc(Func* func) {
  assertx(func && func->isUnique());
  auto const DEBUG_ONLY old = m_uniqueFunc;
  assertx(!old || func == old);
  m_uniqueFunc = func;
}

namespace {
///////////////////////////////////////////////////////////////////////////////

/*
 * Global NamedEntity table.
 */
NamedEntity::Map* s_namedDataMap;

/*
 * Initialize the NamedEntity table.
 */
NEVER_INLINE
void initializeNamedDataMap() {
  NamedEntity::Map::Config config;
  config.growthFactor = 1;
  config.entryCountThreadCacheSize = 10;

  s_namedDataMap = new (vm_malloc(sizeof(NamedEntity::Map)))
    NamedEntity::Map(RuntimeOption::EvalInitialNamedEntityTableSize, config);
}

/*
 * Insert a NamedEntity into the table.
 */
NEVER_INLINE
NamedEntity* insertNamedEntity(const StringData* str) {
  if (!str->isStatic()) {
    str = makeStaticString(str);
  }
  auto res = s_namedDataMap->insert(str, NamedEntity());
  static std::atomic<bool> signaled{false};
  checkAHMSubMaps(*s_namedDataMap, "named entity table", signaled);

  auto const ne = &res.first->second;
  if (res.second && RuntimeOption::EvalEnableReverseDataMap) {
    data_map::register_start(ne);
  }
  return ne;
}

///////////////////////////////////////////////////////////////////////////////
}

NamedEntity* NamedEntity::get(const StringData* str,
                              bool allowCreate /* = true */,
                              String* normalizedStr /* = nullptr */) {
  if (str->isSymbol()) {
    if (auto const result = str->getNamedEntity()) return result;
  }

  if (UNLIKELY(!s_namedDataMap)) {
    initializeNamedDataMap();
  }

  auto const result = [&]() -> NamedEntity* {
    auto it = s_namedDataMap->find(str);
    if (LIKELY(it != s_namedDataMap->end())) {
      return &it->second;
    }

    if (needsNSNormalization(str)) {
      auto normStr = normalizeNS(StrNR(str).asString());
      if (normalizedStr) {
        *normalizedStr = normStr;
      }
      return get(normStr.get(), allowCreate, normalizedStr);
    }

    if (LIKELY(allowCreate)) {
      return insertNamedEntity(str);
    }
    return nullptr;
  }();

  if (str->isSymbol() && result) {
    const_cast<StringData*>(str)->setNamedEntity(result);
  }
  return result;
}

NamedEntity::Map* NamedEntity::table() {
  return s_namedDataMap;
}

size_t NamedEntity::tableSize() {
  return s_namedDataMap ? s_namedDataMap->size() : 0;
}

std::vector<std::pair<const char*, int64_t>> NamedEntity::tableStats() {
  std::vector<std::pair<const char*, int64_t>> stats;
  if (!s_namedDataMap) return stats;

  stats.emplace_back("submaps", s_namedDataMap->numSubMaps());
  stats.emplace_back("capacity", s_namedDataMap->capacity());

  return stats;
}

///////////////////////////////////////////////////////////////////////////////
}
