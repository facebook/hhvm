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

static auto ne_counter = ServiceData::createCounter("admin.named-entities");

rds::Handle NamedFunc::getFuncHandle(const StringData* name) const {
  auto const mode =
    RO::RepoAuthoritative ? rds::Mode::Persistent : rds::Mode::Normal;
  m_cachedFunc.bind(mode, rds::LinkName{"NEFunc", name});
  return m_cachedFunc.handle();
}

void NamedFunc::setCachedFunc(Func* f) {
  *m_cachedFunc = f;
  if (m_cachedFunc.isNormal()) {
    f ? m_cachedFunc.markInit() : m_cachedFunc.markUninit();
  }
}

rds::Handle NamedType::getClassHandle(const StringData* name) const {
  auto const mode =
    RO::RepoAuthoritative ? rds::Mode::Persistent : rds::Mode::Normal;
  m_cachedClass.bind(mode, rds::LinkName{"NEClass", name});
  return m_cachedClass.handle();
}

void NamedType::setCachedClass(Class* f) {
  *m_cachedClass = f;
  if (m_cachedClass.isNormal()) {
    f ? m_cachedClass.markInit() : m_cachedClass.markUninit();
  }
}

bool NamedType::isPersistentTypeAlias() const {
  return m_cachedTypeAlias.bound() && m_cachedTypeAlias.isPersistent();
}

void NamedType::setCachedTypeAlias(const TypeAlias& td) {
  if (!m_cachedTypeAlias.isInit()) {
    m_cachedTypeAlias.initWith(td);
  } else {
    *m_cachedTypeAlias = td;
  }
}

const TypeAlias* NamedType::getCachedTypeAlias() const {
  return m_cachedTypeAlias.bound() &&
         m_cachedTypeAlias.isInit() &&
         m_cachedTypeAlias->name()
    ? m_cachedTypeAlias.get()
    : nullptr;
}

void NamedType::setCachedReifiedGenerics(ArrayData* a) {
  if (!m_cachedReifiedGenerics.isInit()) {
    m_cachedReifiedGenerics.initWith(a);
  } else {
    *m_cachedReifiedGenerics = a;
  }
}

void NamedType::pushClass(Class* cls) {
  assertx(cls->m_next == nullptr);
  cls->m_next = m_clsList;
  m_clsList = cls;
}

void NamedType::removeClass(Class* goner) {
  auto head = m_clsList;
  if (!head) return;

  if (RuntimeOption::EvalEnableReverseDataMap) {
    // This deregisters Classes registered to data_map in Unit::defClass().
    data_map::deregister(goner);
  }

  if (head == goner) {
    m_clsList = head->m_next;
    return;
  }
  auto t = &(head->m_next);
  while (t->get() != goner) {
    assertx(*t);
    t = &((*t)->m_next);
  }
  *t = goner->m_next;
}

namespace {
///////////////////////////////////////////////////////////////////////////////

/*
 * Global NamedType & NamedFunc tables.
 */
NamedType::Map* s_namedTypeMap;
NamedFunc::Map* s_namedFuncMap;

/*
 * Initialize the NamedType & NamedFunc tables.
 */
NEVER_INLINE
void initializeNamedTypeMap() {
  NamedType::Map::Config config;
  config.growthFactor = 1;
  config.entryCountThreadCacheSize = 10;
  s_namedTypeMap = new (vm_malloc(sizeof(NamedType::Map)))
    NamedType::Map(RuntimeOption::EvalInitialTypeTableSize, config);
}

NEVER_INLINE
void initializeNamedFuncMap() {
  NamedFunc::Map::Config config;
  config.growthFactor = 1;
  config.entryCountThreadCacheSize = 10;
  s_namedFuncMap = new (vm_malloc(sizeof(NamedFunc::Map)))
    NamedFunc::Map(RuntimeOption::EvalInitialFuncTableSize, config);
}

/*
 * Insert a NamedType or NamedFunc into its table.
 */
template<typename T>
NEVER_INLINE
T* insertNamedEntity(const StringData* str, typename T::Map* map) {
  if (!str->isStatic()) {
    str = makeStaticString(str);
  }
  auto res = map->insert(str, T());
  ne_counter->increment();
  static std::atomic<bool> signaled{false};
  checkAHMSubMaps(*map, "named entity table", signaled);

  auto const ne = &res.first->second;
  if (res.second && RuntimeOption::EvalEnableReverseDataMap) {
    data_map::register_start(ne);
  }
  return ne;
}

///////////////////////////////////////////////////////////////////////////////
}

template NamedType* NamedType::get<true>(const StringData*, String*);
template NamedType* NamedType::get<false>(const StringData*, String*);

template<bool AllowCreate>
NamedType* NamedType::get(const StringData* str, String* normalizedStr /* = nullptr */) {
  if (str->isSymbol()) {
    if (auto const result = str->getNamedType()) return result;
  }

  if (UNLIKELY(!s_namedTypeMap)) {
    initializeNamedTypeMap();
  }

  auto const result = [&]() -> NamedType* {
    auto it = s_namedTypeMap->find(str);
    if (LIKELY(it != s_namedTypeMap->end())) {
      return &it->second;
    }

    if (needsNSNormalization(str)) {
      auto normStr = normalizeNS(StrNR(str).asString());
      if (normalizedStr) {
        *normalizedStr = normStr;
      }
      return get<AllowCreate>(normStr.get(), normalizedStr);
    }

    if (AllowCreate) {
      return insertNamedEntity<NamedType>(str, s_namedTypeMap);
    }
    return nullptr;
  }();

  if (str->isSymbol() && result) {
    const_cast<StringData*>(str)->setNamedType(result);
  }
  return result;
}

template NamedFunc* NamedFunc::get<true>(const StringData*, String*);
template NamedFunc* NamedFunc::get<false>(const StringData*, String*);

template<bool AllowCreate>
NamedFunc* NamedFunc::get(const StringData* str, String* normalizedStr /* = nullptr */) {
  if (UNLIKELY(!s_namedFuncMap)) {
    initializeNamedFuncMap();
  }

  auto it = s_namedFuncMap->find(str);
  if (LIKELY(it != s_namedFuncMap->end())) {
    return &it->second;
  }

  if (needsNSNormalization(str)) {
    auto normStr = normalizeNS(StrNR(str).asString());
    if (normalizedStr) {
      *normalizedStr = normStr;
    }
    return get<AllowCreate>(normStr.get(), normalizedStr);
  }

  if (AllowCreate) {
    return insertNamedEntity<NamedFunc>(str, s_namedFuncMap);
  }
  return nullptr;
}

NamedType::Map* NamedType::types() {
  return s_namedTypeMap;
}

NamedFunc::Map* NamedFunc::funcs() {
  return s_namedFuncMap;
}

size_t namedEntityTableSize() {
  return (s_namedTypeMap ? s_namedTypeMap->size() : 0) +
         (s_namedFuncMap ? s_namedFuncMap->size() : 0);
}

std::vector<std::pair<const char*, int64_t>> namedEntityStats() {
  std::vector<std::pair<const char*, int64_t>> stats;
  if (!s_namedTypeMap && !s_namedFuncMap) return stats;
  auto numSubMaps = (s_namedTypeMap ? s_namedTypeMap->numSubMaps() : 0) +
                    (s_namedFuncMap ? s_namedFuncMap->numSubMaps() : 0);
  auto capacity = (s_namedTypeMap ? s_namedTypeMap->capacity() : 0) +
                  (s_namedFuncMap ? s_namedFuncMap->capacity() : 0);
  stats.emplace_back("submaps", numSubMaps);
  stats.emplace_back("capacity", capacity);
  return stats;
}

ServiceData::CounterCallback s_counters(
  [](std::map<std::string, int64_t>& counters) {
    // avoid duplicating code for now. When we actually clean up the admin
    // port, we can just inline namedEntityStats()
    for (const auto& pair : namedEntityStats()) {
      counters[folly::sformat("admin.named_entities_{}", pair.first)] = pair.second;
    }
  }
);

///////////////////////////////////////////////////////////////////////////////
}
