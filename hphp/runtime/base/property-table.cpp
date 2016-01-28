/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/base/property-table.h"

namespace HPHP {

PropertyTable::PropertyTable()
  : m_nextOffset(0)
  , m_size(0)
  , m_capacity(0)
  , m_base(nullptr)
{
}

PropertyTable::PropertyTable(const PropertyTable& other)
  : m_nextOffset(other.m_nextOffset)
  , m_size(other.m_size)
  , m_capacity(other.m_capacity)
  , m_base(nullptr)
{
  if (!other.m_base) {
    assert(!m_capacity);
    assert(!other.m_capacity);
    return;
  }

  assert(m_capacity == other.m_capacity);
  m_base = malloc(bytesForCapacity(m_capacity));
  memcpy(m_base, other.m_base, bytesForCapacity(m_capacity));
}

PropertyTable::~PropertyTable() {
  free(m_base);
}

uint32_t PropertyTable::add(const StringData* property) {
  if (shouldGrow()) grow();
  assert(offsetFor(property) == kInvalidOffset);
  assert((m_size + 1) < m_capacity);

  auto offset = m_nextOffset++;
  assert(offset != kInvalidOffset);
  auto bucketIndex = bucketFor<Static>(property);
  offsets()[offset] = bucketIndex;
  entries()[bucketIndex] = Entry{ property, offset, property->hash() };
  m_size++;
  return offset;
}

void PropertyTable::grow() {
  assert(shouldGrow());
  if (!m_capacity) {
    m_capacity = 2;
    assert(!m_base);
    m_base = malloc(bytesForCapacity(m_capacity));
    memset(m_base, 0, bytesForCapacity(m_capacity));
    return;
  }

  auto newCapacity = m_capacity << 1;
  void* oldBase = m_base;
  void* newBase = malloc(bytesForCapacity(newCapacity));
  memset(newBase, 0, bytesForCapacity(newCapacity));
  rehash(oldBase, m_capacity, newBase, newCapacity);
  m_base = newBase;
  m_capacity = newCapacity;
  free(oldBase);
}

void PropertyTable::rehash(
  void* oldBase,
  size_t oldCapacity,
  void* newBase,
  size_t newCapacity
) {
  auto oldOffsets = baseToOffsets(oldBase);
  auto oldEntries = baseToEntries(oldBase, oldCapacity);
  auto newOffsets = baseToOffsets(newBase);
  auto newEntries = baseToEntries(newBase, newCapacity);

  for (auto i = 0; i < m_size; ++i) {
    Entry& oldEntry = oldEntries[oldOffsets[i]];
    auto bucketIndex = bucketFor<Static>(oldEntry.key, newEntries, newCapacity);
    newOffsets[i] = bucketIndex;
    newEntries[bucketIndex] = oldEntry;
  }
}

}
