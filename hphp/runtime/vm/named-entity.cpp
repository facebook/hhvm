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

#include "hphp/runtime/vm/named-entity.h"

#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/type-alias.h"
#include "hphp/runtime/vm/unit-util.h"

#include <folly/AtomicHashMap.h>

#include <atomic>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

void NamedEntity::setCachedFunc(Func* f) {
  *m_cachedFunc = f;
}

Func* NamedEntity::getCachedFunc() const {
  return LIKELY(m_cachedFunc.bound()) ? *m_cachedFunc : nullptr;
}

void NamedEntity::setCachedClass(Class* f) {
  *m_cachedClass = f;
}

Class* NamedEntity::getCachedClass() const {
  return LIKELY(m_cachedClass.bound()) ? *m_cachedClass : nullptr;
}

bool NamedEntity::isPersistentTypeAlias() const {
  return m_cachedTypeAlias.isPersistent();
}

void NamedEntity::setCachedTypeAlias(const TypeAliasReq& td) {
  *m_cachedTypeAlias = td;
}

const TypeAliasReq* NamedEntity::getCachedTypeAlias() const {
  // TODO(#2103214): Support persistent type aliases.
  return m_cachedTypeAlias.bound() && m_cachedTypeAlias->name
    ? m_cachedTypeAlias.get()
    : nullptr;
}

void NamedEntity::pushClass(Class* cls) {
  assert(!cls->m_nextClass);
  cls->m_nextClass = m_clsList.load(std::memory_order_acquire);
  m_clsList.store(cls, std::memory_order_release);
}

void NamedEntity::removeClass(Class* goner) {
  Class* head = m_clsList.load(std::memory_order_acquire);
  if (!head) return;
  if (head == goner) {
    return m_clsList.store(head->m_nextClass, std::memory_order_release);
  }
  LowPtr<Class>* cls = &head->m_nextClass;
  while (cls->get() != goner) {
    assert(*cls);
    cls = &(*cls)->m_nextClass;
  }
  *cls = goner->m_nextClass;
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

  s_namedDataMap = new NamedEntity::Map(
      RuntimeOption::EvalInitialNamedEntityTableSize, config);
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
  return &res.first->second;
}

///////////////////////////////////////////////////////////////////////////////
}

NamedEntity* NamedEntity::get(const StringData* str,
                              bool allowCreate /* = true */,
                              String* normalizedStr /* = nullptr */) {
  if (UNLIKELY(!s_namedDataMap)) {
    initializeNamedDataMap();
  }

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
}

NamedEntity::Map* NamedEntity::table() {
  return s_namedDataMap;
}

size_t NamedEntity::tableSize() {
  return s_namedDataMap ? s_namedDataMap->size() : 0;
}

///////////////////////////////////////////////////////////////////////////////
}
