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
#ifndef incl_HPHP_SHAPE_H_
#define incl_HPHP_SHAPE_H_

#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/property-table.h"
#include "hphp/util/hash-map-typedefs.h"

#include <tbb/spin_rw_mutex.h>

namespace HPHP {

class Shape {
public:
  static Shape* emptyShape();
  static Shape* create(StringData** properties, uint32_t numProperties);
  static Shape* clone(Shape* from);
  Shape* transition(StringData* property);

  size_t size() const;
  size_t capacity() const;

  bool transitionRequiresGrowth() const;
  size_t suggestedNewCapacity() const;

  uint32_t offsetFor(const StringData*) const;
  bool hasOffsetFor(const StringData*) const;
  const StringData* keyForOffset(uint32_t) const;

private:
  static Shape* create(); // Empty shape.

  Shape();
  Shape(const Shape&);

  void addProperty(StringData* property);
  bool shouldCacheTransition() const;
  size_t maxCachedTransitions(size_t size);

  typedef hphp_hash_map<StringData*, Shape*> TransitionMap;

  size_t m_size;
  size_t m_capacity;
  PropertyTable m_table;

  static const size_t kMaxTransitionCount = 64;
  static const size_t kMaxTotalShapes = 32 * 1024 * 1024;
  static std::atomic<size_t> s_totalShapes;

  // The tree of Shapes is global to the VM, so it can be accessed concurrently
  // across multiple requests/threads. Reading (i.e. traversing the tree) is
  // much more common than adding new things, so the transition map is guarded
  // by a read-write spin lock. This prevents writers from moving the map from
  // under any readers currently reading. It would probably be okay to add
  // properties to the table concurrently. A reader that got a stale answer
  // might end up doing a little extra work, but wouldn't do anything
  // technically incorrect. However, this would require a fancier hash table,
  // and the simple solution seemed like a good place to start.
  typedef tbb::spin_rw_mutex MapLock;
  MapLock m_lock;
  TransitionMap m_cachedTransitions;
};

inline bool Shape::transitionRequiresGrowth() const {
  return m_size == m_capacity;
}

inline size_t Shape::suggestedNewCapacity() const {
  // 1.5x growth
  return m_capacity ? m_capacity + (m_capacity >> 1) : 2;
}

inline size_t Shape::size() const {
  return m_size;
}

inline size_t Shape::capacity() const {
  return m_capacity;
}

inline const StringData* Shape::keyForOffset(uint32_t offset) const {
  return m_table.keyForOffset(offset);
}

inline uint32_t Shape::offsetFor(const StringData* property) const {
  return m_table.offsetFor(property);
}

inline bool Shape::hasOffsetFor(const StringData* property) const {
  return offsetFor(property) != PropertyTable::kInvalidOffset;
}

inline Shape* Shape::create(StringData** properties, uint32_t numProperties) {
  Shape* start = emptyShape();
  Shape* curr = start;
  for (uint32_t i = 0; i < numProperties; ++i) {
    curr = curr->transition(properties[i]);
    if (!curr) return nullptr;
  }
  return curr;
}

inline void Shape::addProperty(StringData* property) {
  m_table.add(property);
  if (transitionRequiresGrowth()) {
    m_capacity = suggestedNewCapacity();
  }
  m_size++;
}

inline Shape* Shape::transition(StringData* property) {
  assert(property->isStatic());

  {
    MapLock::scoped_lock locker(m_lock, false);
    auto iter = m_cachedTransitions.find(property);
    if (iter != m_cachedTransitions.end()) {
      return iter->second;
    }
  }

  if (s_totalShapes.load() > kMaxTotalShapes) return nullptr;

  if (m_size + 1 > kMaxTransitionCount) return nullptr;

  Shape* newShape = clone(this);
  newShape->addProperty(property);

  MapLock::scoped_lock locker(m_lock, true);
  auto result = m_cachedTransitions.insert(std::make_pair(property, newShape));
  if (result.second) {
    s_totalShapes++;
  } else {
    delete newShape;
  }
  return result.first->second;
}

std::string show(const Shape& shape);

}

#endif
