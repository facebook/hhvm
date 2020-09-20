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
#pragma once

#include <atomic>
#include <type_traits>

#include <boost/iterator/iterator_facade.hpp>

#include <folly/Bits.h>

#include "hphp/runtime/vm/treadmill.h"
#include "hphp/util/assertions.h"

namespace HPHP {

extern uint64_t g_emptyTable;

/*
 * A hashtable safe for multiple concurrent readers, even while writes are
 * happening, but with at most one concurrent writer.  Reads and writes both
 * are wait-free, but that there is only at most one writer will generally
 * require synchronization outside of this class.  (E.g. the translator write
 * lease.)
 *
 * Key must be an atomically loadable/storable type.  The Value must be a
 * trivially copyable and assignable type, and it must be legal to copy from it
 * without synchronization (this table may do this during a rehash).  Also,
 * assumes Key == 0 is invalid (i.e. the empty key).
 *
 * Insertions must be unique.  It is an error to insert the same key more than
 * once.
 *
 * Uses the treadmill to collect garbage.
 */

template<class Key, class Val,
         class HashFunc = std::hash<Key>,
         class Alloc = std::allocator<char>>
struct TreadHashMap : private HashFunc,
                      private Alloc::template rebind<char>::other {
  using Allocator = typename Alloc::template rebind<char>::other;
  using value_type = std::pair<std::atomic<Key>,Val>;
  static_assert(
    std::is_trivially_destructible<Key>::value &&
    std::is_trivially_destructible<Val>::value,
    "TreadHashMap only supports trivially destructible keys and values"
  );

private:
  struct Table {
    uint32_t capac;
    uint32_t size;
    value_type entries[0];
  };

  Table* staticEmptyTable() {
    static_assert(sizeof(Table) == sizeof(g_emptyTable), "");
    return reinterpret_cast<Table*>(&g_emptyTable);
  }

public:
  explicit TreadHashMap(uint32_t initialCapacity)
    : m_table(staticEmptyTable())
    , m_initialSize(folly::nextPowTwo(initialCapacity))
  {}

  ~TreadHashMap() {
    auto t = m_table.load(std::memory_order_acquire);
    if (t != staticEmptyTable()) {
      freeTable(t);
    }
  }

  TreadHashMap(const TreadHashMap&) = delete;
  TreadHashMap& operator=(const TreadHashMap&) = delete;

  template<class IterVal>
  struct thm_iterator
    : boost::iterator_facade<thm_iterator<IterVal>,IterVal,
                             boost::forward_traversal_tag>
  {
    explicit thm_iterator() : m_table(nullptr) {}

    // Conversion constructor for interoperability between iterator
    // and const_iterator.  The enable_if<> magic prevents the
    // constructor from existing if we're going in the const_iterator
    // to iterator direction.
    template<class OtherVal>
    thm_iterator(const thm_iterator<OtherVal>& o,
                 typename std::enable_if<
                   std::is_convertible<OtherVal*,IterVal*>::value
                 >::type* = 0)
      : m_table(o.m_table)
      , m_offset(o.m_offset)
    {}

    explicit thm_iterator(Table* tab, size_t offset)
      : m_table(tab)
      , m_offset(offset)
    {
      advancePastEmpty();
    }

  private:
    friend struct TreadHashMap;
    friend class boost::iterator_core_access;

    void increment() {
      ++m_offset;
      advancePastEmpty();
    }

    bool equal(const thm_iterator& o) const {
      return m_table == o.m_table && m_offset == o.m_offset;
    }

    IterVal& dereference() const {
      return m_table->entries[m_offset];
    }

  private:
    void advancePastEmpty() {
      while (m_offset < m_table->capac &&
        m_table->entries[m_offset].first.load(std::memory_order_acquire) == 0) {
        ++m_offset;
      }
    }

  private:
    Table* m_table;
    size_t m_offset;
  };

  typedef thm_iterator<value_type> iterator;
  typedef thm_iterator<const value_type> const_iterator;

  size_t size() const {
    return m_table.load(std::memory_order_acquire)->size;
  }

  iterator begin() {
    return iterator(m_table.load(std::memory_order_acquire), 0);
  }

  iterator end() {
    auto tab = m_table.load(std::memory_order_acquire);
    return iterator(tab, tab->capac);
  }

  const_iterator begin() const {
    return const_cast<TreadHashMap*>(this)->begin();
  }
  const_iterator end() const {
    return const_cast<TreadHashMap*>(this)->end();
  }

  Val* insert(Key key, Val val) {
    assertx(key != 0);
    return insertImpl(acquireAndGrowIfNeeded(), key, val);
  }

  Val* find(Key key) const {
    assertx(key != 0);

    auto tab = m_table.load(std::memory_order_acquire);
    if (tab->size == 0) return nullptr; // empty
    assertx(tab->capac > tab->size);
    auto idx = project(tab, key);
    for (;;) {
      auto& entry = tab->entries[idx];
      Key currentProbe = entry.first.load(std::memory_order_acquire);
      if (currentProbe == key) return &entry.second;
      if (currentProbe == 0) return nullptr;
      if (++idx == tab->capac) idx = 0;
    }
  }

  template<typename F> void filter_keys(F fn) {
    auto const old = m_table.load(std::memory_order_acquire);
    if (UNLIKELY(old == staticEmptyTable())) return;

    Table* newTable = allocTable(old->capac);
    for (auto i = uint32_t{}; i < old->capac; ++i) {
      auto const ent = &old->entries[i];
      auto const key = ent->first.load(std::memory_order_acquire);
      if (key && !fn(key)) insertImpl(newTable, key, ent->second);
    }
    Treadmill::enqueue([this, old] { freeTable(old); });
    m_table.store(newTable, std::memory_order_release);
  }

private:
  Val* insertImpl(Table* const tab, Key newKey, Val newValue) {
    auto probe = &tab->entries[project(tab, newKey)];
    assertx(size_t(probe - tab->entries) < tab->capac);

    while (Key currentProbe = probe->first.load(std::memory_order_acquire)) {
      assertx(currentProbe != newKey); // insertions must be unique
      assertx(probe <= (tab->entries + tab->capac));
      // acquireAndGrowIfNeeded ensures there's at least one empty slot.
      (void)currentProbe;
      if (++probe == (tab->entries + tab->capac)) probe = tab->entries;
    }

    // Copy over the value before we publish.
    probe->second = newValue;

    // Make it visible.
    probe->first.store(newKey, std::memory_order_release);

    // size is only written to by the writer thread, relaxed memory
    // ordering is ok.
    ++tab->size;

    return &probe->second;
  }

  Table* acquireAndGrowIfNeeded() {
    auto old = m_table.load(std::memory_order_acquire);

    // 50% occupancy, avoiding the FPU.
    if (LIKELY((old->size) < (old->capac / 2))) {
      return old;
    }

    Table* newTable;
    if (UNLIKELY(old == staticEmptyTable())) {
      newTable = allocTable(m_initialSize);
    } else {
      newTable = allocTable(old->capac * 2);
      for (uint32_t i = 0; i < old->capac; ++i) {
        auto const ent = old->entries + i;
        auto const key = ent->first.load(std::memory_order_acquire);
        if (key) insertImpl(newTable, key, ent->second);
      }
      Treadmill::enqueue([this, old] { freeTable(old); });
    }
    assertx(newTable->size == old->size); // only one writer thread
    m_table.store(newTable, std::memory_order_release);
    return newTable;
  }

  size_t project(Table* tab, Key key) const {
    assertx(folly::isPowTwo(tab->capac));
    return HashFunc::operator()(key) & (tab->capac - 1);
  }

  constexpr size_t allocSize(uint32_t cap) {
    return cap * sizeof(value_type) + sizeof(Table);
  }

  Table* allocTable(uint32_t capacity) {
    auto size = allocSize(capacity);
    auto ret = reinterpret_cast<Table*>(Allocator::allocate(size));
    memset(ret, 0, size);
    ret->capac = capacity;
    ret->size = 0;
    return ret;
  }

  void freeTable(Table* t) {
    Allocator::deallocate(reinterpret_cast<char*>(t), allocSize(t->capac));
  }

private:
  std::atomic<Table*> m_table;
  uint32_t m_initialSize;
};

//////////////////////////////////////////////////////////////////////

}

