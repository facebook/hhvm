/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HPHP_VM_TREAD_HASH_MAP_H_
#define incl_HPHP_VM_TREAD_HASH_MAP_H_

#include <atomic>
#include <boost/noncopyable.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/type_traits/is_convertible.hpp>
#include <boost/utility/enable_if.hpp>
#include <utility>
#include "hphp/util/util.h"
#include "hphp/util/atomic.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

namespace Treadmill { void deferredFree(void*); }

//////////////////////////////////////////////////////////////////////

/*
 * A hashtable safe for multiple concurrent readers, even while writes
 * are happening, but with at most one concurrent writer.  Reads and
 * writes both are wait-free, but that there is only at most one
 * writer will generally require synchronization outside of this
 * class.  (E.g. the translator write lease.)
 *
 * Key must be an atomically loadable/storable type.  The Value must
 * be a trivially copyable and assignable type, and it must be legal
 * to copy from it without synchronization (this table may do this
 * during a rehash).  Also, assumes Key == 0 is invalid (i.e. the
 * empty key).
 *
 * Insertions must be unique.  It is an error to insert the same key
 * more than once.
 *
 * Uses the treadmill to collect garbage.
 */
template<class Key, class Val, class HashFunc>
struct TreadHashMap : private boost::noncopyable {
  typedef std::pair<std::atomic<Key>,Val> value_type;

private:
  struct Table {
    size_t capac;
    size_t size;
    value_type entries[0];
  };

public:
  explicit TreadHashMap(size_t initialCapacity)
    : m_table(allocTable(initialCapacity))
  {}

  ~TreadHashMap() {
    free(m_table);
  }

  template<class IterVal>
  struct thm_iterator
    : boost::iterator_facade<thm_iterator<IterVal>,IterVal,
                             boost::forward_traversal_tag>
  {
    explicit thm_iterator() : m_table(0) {}

    // Conversion constructor for interoperability between iterator
    // and const_iterator.  The enable_if<> magic prevents the
    // constructor from existing if we're going in the const_iterator
    // to iterator direction.
    template<class OtherVal>
    thm_iterator(const thm_iterator<OtherVal>& o,
                 typename boost::enable_if<
                   boost::is_convertible<OtherVal*,IterVal*>
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
    friend class TreadHashMap;
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
    assert(key != 0);
    return insertImpl(acquireAndGrowIfNeeded(), key, val);
  }

  Val* find(Key key) const {
    assert(key != 0);

    auto tab = m_table.load(std::memory_order_consume);
    assert(tab->capac > tab->size);
    auto idx = project(tab, key);
    for (;;) {
      auto& entry = tab->entries[idx];
      Key currentProbe = entry.first.load(std::memory_order_acquire);
      if (currentProbe == key) return &entry.second;
      if (currentProbe == 0) return 0;
      if (++idx == tab->capac) idx = 0;
    }
  }

private:
  Val* insertImpl(Table* const tab, Key newKey, Val newValue) {
    auto probe = &tab->entries[project(tab, newKey)];
    assert(size_t(probe - tab->entries) < tab->capac);

    // Since we're the only thread allowed to write, we're allowed to
    // do a relaxed load here.  (No need for an acquire/release
    // handshake with ourselves.)
    while (Key currentProbe = probe->first) {
      assert(currentProbe != newKey); // insertions must be unique
      assert(probe <= (tab->entries + tab->capac));
      // can't loop forever; acquireAndGrowIfNeeded ensures there's
      // some slack.
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
    // Relaxed load is ok---there's only one writer thread.
    auto old = m_table.load(std::memory_order_relaxed);

    // 75% occupancy, avoiding the FPU.
    if (LIKELY((old->size) < (old->capac / 4 + old->capac / 2))) {
      return old;
    }

    // Rehash from old to new.
    auto newTable = allocTable(old->capac * 2);
    for (auto i = 0; i < old->capac; ++i) {
      value_type* ent = old->entries + i;
      if (ent->first) {
        insertImpl(newTable, ent->first, ent->second);
      }
    }
    assert(newTable->capac == old->capac * 2);
    assert(newTable->size == old->size); // only one writer thread
    m_table.store(newTable, std::memory_order_release);
    Treadmill::deferredFree(old);
    return newTable;
  }

  size_t project(Table* tab, Key key) const {
    assert(Util::isPowerOfTwo(tab->capac));
    return m_hash(key) & (tab->capac - 1);
  }

  static Table* allocTable(size_t capacity) {
    auto ret = static_cast<Table*>(
      calloc(1, capacity * sizeof(value_type) + sizeof(Table)));
    ret->capac = capacity;
    ret->size = 0;
    return ret;
  }

private:
  HashFunc m_hash;
  std::atomic<Table*> m_table;
};

//////////////////////////////////////////////////////////////////////

}

#endif
