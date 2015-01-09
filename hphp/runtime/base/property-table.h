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
#ifndef incl_HPHP_PROPERTY_TABLE_H_
#define incl_HPHP_PROPERTY_TABLE_H_

#include "hphp/runtime/base/string-data.h"

#include <cstdint>

namespace HPHP {

// Hash table that preserves insertion order.
class PropertyTable {
public:
  PropertyTable();
  PropertyTable(const PropertyTable&);
  ~PropertyTable();

  PropertyTable& operator=(const PropertyTable&) = delete;

  uint32_t add(const StringData*);
  uint32_t offsetFor(const StringData*) const;
  const StringData* keyForOffset(uint32_t) const;

  static constexpr uint32_t kInvalidOffset = UINT_MAX;

private:
  struct Entry {
    const StringData* key;
    uint32_t offset;
    strhash_t hash;
  };

  static uint32_t* baseToOffsets(void* base);
  uint32_t* offsets();
  const uint32_t* offsets() const;

  static Entry* baseToEntries(void* base, size_t capacity);
  Entry* entries();
  const Entry* entries() const;

  enum PropertyTypeTag {
    Static,
    NotStatic
  };

  template<PropertyTypeTag>
  uint32_t bucketFor(const StringData*);
  template<PropertyTypeTag>
  uint32_t bucketFor(const StringData*) const;
  template<PropertyTypeTag>
  uint32_t bucketFor(const StringData*, const Entry*, size_t capacity);
  template<PropertyTypeTag>
  uint32_t bucketFor(const StringData*, const Entry*, size_t capacity) const;

  static size_t bytesForCapacity(size_t capacity);
  double loadFactor() const;
  bool shouldGrow() const;
  void grow();
  void rehash(void* oldBase, size_t oldCap, void* newBase, size_t newCap);

  // The capacity below which we ignore the load factor. The table
  // depends on having at least one free slot to signal the end of a hash
  // lookup.  If we always followed the load factor, table sizes below a
  // certain limit would be guaranteed to fill completely, violating this
  // invariant.
  static constexpr size_t kLoadFactorCapacityThreshold = 4;
  static constexpr double kMaxLoadFactor =
    1.0 - (1.0/static_cast<double>(kLoadFactorCapacityThreshold));

  uint32_t m_nextOffset;
  size_t m_size;
  size_t m_capacity;
  void* m_base;
};

inline size_t PropertyTable::bytesForCapacity(size_t capacity) {
  return capacity * (sizeof(uint32_t) + sizeof(Entry));
}

inline uint32_t* PropertyTable::baseToOffsets(void* base) {
  return static_cast<uint32_t*>(base);
}

inline PropertyTable::Entry* PropertyTable::baseToEntries(
  void* base,
  size_t capacity
) {
  return reinterpret_cast<Entry*>(baseToOffsets(base) + capacity);
}

inline uint32_t* PropertyTable::offsets() {
  return const_cast<uint32_t*>(
    const_cast<const PropertyTable*>(this)->baseToOffsets(m_base));
}

inline const uint32_t* PropertyTable::offsets() const {
  return baseToOffsets(m_base);
}

inline PropertyTable::Entry* PropertyTable::entries() {
  return const_cast<PropertyTable::Entry*>(
    const_cast<const PropertyTable*>(this)->entries());
}

inline const PropertyTable::Entry* PropertyTable::entries() const {
  return baseToEntries(m_base, m_capacity);
}

inline double PropertyTable::loadFactor() const {
  return static_cast<double>(m_size) / static_cast<double>(m_capacity);
}

inline bool PropertyTable::shouldGrow() const {
  if (m_capacity >= kLoadFactorCapacityThreshold) {
    return loadFactor() >= kMaxLoadFactor;
  }
  // We have to keep at least one extra slot free so that we're guaranteed to
  // eventually hit it when doing the hash lookup.
  return (m_size + 1) >= m_capacity;
}

template<PropertyTable::PropertyTypeTag propType>
inline uint32_t PropertyTable::bucketFor(const StringData* property) {
  return bucketFor<propType>(property, entries(), m_capacity);
}

template<PropertyTable::PropertyTypeTag propType>
inline uint32_t PropertyTable::bucketFor(const StringData* property) const {
  return bucketFor<propType>(property, entries(), m_capacity);
}

inline const StringData* PropertyTable::keyForOffset(uint32_t offset) const {
  if (offset >= m_size) return nullptr;
  return entries()[offsets()[offset]].key;
}

inline uint32_t PropertyTable::offsetFor(const StringData* property) const {
  auto bucketIndex = property->isStatic() ? bucketFor<Static>(property)
                                          : bucketFor<NotStatic>(property);
  auto& bucket = entries()[bucketIndex];
  return bucket.key ? bucket.offset : kInvalidOffset;
}

template<PropertyTable::PropertyTypeTag propType>
inline uint32_t PropertyTable::bucketFor(
  const StringData* property,
  const Entry* entries,
  size_t capacity
) {
  return const_cast<const PropertyTable*>(this)->bucketFor<propType>(
    property, entries, capacity);
}

template<PropertyTable::PropertyTypeTag propType>
inline uint32_t PropertyTable::bucketFor(
  const StringData* property,
  const Entry* entries,
  size_t capacity
) const {
  assert(propType == NotStatic || property->isStatic());
  assert(m_size < m_capacity);
  auto hash = property->hash();
  auto start = hash & (capacity - 1);
  auto current = start;
  while (true) {
    const auto key = entries[current].key;
    if (!key) break;
    if (key == property) break;
    if (propType == NotStatic) {
      if (entries[current].hash == property->hash()) {
        if (key->same(property)) break;
      }
    }
    current = (current + 1) & (capacity - 1);
  }
  return current;
}

}

#endif
