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

#ifndef incl_HPHP_RUNTIME_VM_NAMEVALUETABLE_H_
#define incl_HPHP_RUNTIME_VM_NAMEVALUETABLE_H_

#include <boost/noncopyable.hpp>
#include <deque>
#include <limits>

#include "hphp/util/util.h"
#include "hphp/util/exp-arena.h"
#include "hphp/runtime/base/complex-types.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * This class implements a name to TypedValue map.  Basically a
 * hashtable from StringData* to TypedValue.
 *
 * This is for use in variable environments in bytecode.cpp, and is
 * also used for the global variable environment.  It also ends up
 * used (via NameValueTableWrapper) for initialization of class
 * properties (see class.cpp).
 *
 * Notes:
 *
 *   - All elements are guaranteed to have a fixed address for the
 *     lifetime of the table.  (However this address may at some times
 *     contain a TypedValue with KindOfIndirect, see next point.)  The
 *     lookupRaw* functions access this address.
 *
 *   - Supports migrating storage for values outside of the table.
 *     This currently has a few uses:
 *
 *        o Keeps storage for locals in functions with a VarEnv in
 *          their normal location, but still accessible by name
 *          through this table.
 *
 *        o When invoking 86pinit or 86sinit on classes, this is used
 *          to provide a name to value mapping to these generated
 *          functions, although the storage lives elsewhere.
 *
 *        o Some special global variables have storage outside of the
 *          NameValueTable.  (This is currently because the runtime
 *          accesses these values by raw pointer in several
 *          extensions, and might be removable (#1158383).)
 *
 *   - Adding and then unsetting an element will still leave its space
 *     allocated for the lifetime of the NameValueTable (with the type
 *     set to KindOfUninit).  This is because of the first note: we
 *     hand out pointers that need to be valid for the lifetime of the
 *     table, so we can't really reclaim any of the KindOfUninit
 *     elements.
 *
 */
struct NameValueTable : private boost::noncopyable {
  /*
   * Create a NameValueTable with room for `size' elements before
   * rehash is necessary.
   */
  explicit NameValueTable(size_t size)
    : m_table(0)
    , m_tabMask(0)
    , m_elms(0)
    , m_storage(size)
  {
    /*
     * Reserve space for size * 4 / 3 because we limit our max load
     * factor to .75.
     *
     * Add one because the way we compute whether there is enough
     * space on lookupAdd will look at m_elms + 1, and this
     * constructor is sometimes used with an exact final size count so
     * we'd like to avoid reallocation in that case.
     *
     * Finally, never allocate less than 4 slots.  In the case of a
     * NameValueTable created with an exact size of a single element
     * (not uncommon for a VarEnv), this avoids reallocation when
     * someone does a lookupAdd() on the single element.
     */
    allocate(std::max(Util::roundUpToPowerOfTwo(size * 4 / 3 + 1), 4ul));
  }
  ~NameValueTable();

  struct Iterator;

  /*
   * Map a name via KindOfIndirect to a specific storage location.
   *
   * This is for use when we want a name to be possible to look up in
   * this table, but the actual storage to exist somewhere else
   * (e.g. the execution stack or an instance property slot).
   *
   * The `migrateSet' version will retain the value currently at
   * `loc': we set the mapping in this table to point at `loc' but
   * don't change the current contents of loc.  The `migrate' version
   * also copies whatever current value is in the table to `loc', and
   * assumes `loc' is currently dead.
   *
   * Both functions return a pointer to the previous storage location
   * mapped for this name.  If the previous storage location was
   * inside this NameValueTable, they return null.  This is to support
   * nesting of migrate/resettle pairs in a scoped fashion.
   */
  TypedValue* migrate(const StringData* name, TypedValue* loc) {
    return migrateImpl<true>(name, loc);
  }
  TypedValue* migrateSet(const StringData* name, TypedValue* loc) {
    return migrateImpl<false>(name, loc);
  }

  /*
   * Explicitly request letting go of all elements in the
   * NameValueTable without decrefing them.
   *
   * This is intended for use when destroying the global scope, where
   * we shouldn't be running destructors.
   */
  void leak() {
    m_storage.clear();
    m_elms = 0;
    m_tabMask = 0;
    free(m_table);
    m_table = 0;
  }

  /*
   * Opposite of migrate.  Remaps `name' to point at `origLoc', or if
   * `origLoc' is null copies from the current external storage back
   * inside this table.
   *
   * It is illegal to call resettle for a name that hasn't been
   * migrated.
   */
  void resettle(const StringData* name, TypedValue* origLoc) {
    Elm* e = findElm(name);
    assert(e);
    TypedValue* tv = e->m_tv;
    assert(tv->m_type == KindOfIndirect);
    if (!origLoc) {
      *tv = *tv->m_data.pind;
    } else {
      *origLoc = *tv->m_data.pind;
      tv->m_data.pind = origLoc;
    }
  }

  /*
   * Set the slot for the supplied name to `val', allocating it if
   * necessary.
   */
  TypedValue* set(const StringData* name, const TypedValue* val) {
    TypedValue* target = findTypedValue(name);
    tvSet(*(val->m_type == KindOfRef ? val->m_data.pref->tv() : val),
          *target);
    return target;
  }

  /*
   * Bind the slot for the supplied name to `val', allocating it and
   * boxing it first if necessary.
   */
  TypedValue* bind(const StringData* name, TypedValue* val) {
    TypedValue* target = findTypedValue(name);
    if (val->m_type != KindOfRef) {
      tvBox(val);
    }
    tvBind(val, target);
    return target;
  }

  /*
   * Remove an element from this table.  All elements added always
   * occupy storage, so this is done by setting the element to
   * KindOfUninit.
   */
  void unset(const StringData* name) {
    TypedValue* tv = lookupRawPointer(name);
    if (tv) {
      tvUnset(tvDerefIndirect(tv));
    }
  }

  /*
   * Look up the actual storage for this name in this table, returning
   * null if it doesn't exist.
   *
   * The returned TypedValue might have KindOfIndirect if this name
   * has been used with migrate/resettle.
   *
   * The returned pointer is guaranteed to remain valid for the
   * lifetime of this NameValueTable.
   */
  TypedValue* lookupRawPointer(const StringData* name) {
    Elm* elm = findElm(name);
    if (!elm) return 0;
    assert(elm->m_tv);
    return elm->m_tv;
  }

  /*
   * Same as lookupRawPointer, but add an entry with a null value if
   * it didn't already exist.
   */
  TypedValue* lookupAddRawPointer(const StringData* name) {
    Elm* elm = insert(name);
    if (!elm->m_tv) {
      addStorage(elm);
      tvWriteNull(elm->m_tv);
    }
    return elm->m_tv;
  }

  /*
   * Lookup a name, returning null if it doesn't exist in this table.
   */
  TypedValue* lookup(const StringData* name) {
    Elm* elm = findElm(name);
    if (!elm) return 0;
    TypedValue* tv = elm->m_tv;
    tv = tvDerefIndirect(tv);
    return tv->m_type == KindOfUninit ? 0 : tv;
  }

  /*
   * Insert a name to value entry with KindOfNull for the value, or
   * return what is already there if the key already exists in the
   * table.
   */
  TypedValue* lookupAdd(const StringData* name) {
    Elm* elm = insert(name);
    if (!elm->m_tv) {
      addStorage(elm);
      tvWriteNull(elm->m_tv);
      return elm->m_tv;
    }
    TypedValue* ret = tvDerefIndirect(elm->m_tv);
    if (ret->m_type == KindOfUninit) {
      tvWriteNull(ret);
    }
    return ret;
  }

private:
  struct Elm {
    const StringData* m_name;
    TypedValue*       m_tv;
  };

private:
  void reserve(size_t desiredSize) {
    const size_t curCapac = m_tabMask + 1;
    if (desiredSize < curCapac * 3 / 4) { // .75 load factor
      return;
    }
    allocate(Util::roundUpToPowerOfTwo(curCapac + 1));
  }

  void allocate(const size_t newCapac) {
    assert(Util::isPowerOfTwo(newCapac));
    const size_t newMask = newCapac - 1;
    assert(newMask <= std::numeric_limits<uint32_t>::max());
    Elm* newTab = static_cast<Elm*>(calloc(sizeof(Elm), newCapac));
    if (m_table) {
      rehash(newTab, newMask);
      free(m_table);
    }
    m_table = newTab;
    m_tabMask = uint32_t(newMask);
  }

  TypedValue* findTypedValue(const StringData* name) {
    return tvDerefIndirect(lookupAddRawPointer(name));
  }

  static Elm* insertImpl(Elm* const table,
                         const size_t tabMask,
                         const StringData* name) {
    Elm* elm = &table[name->hash() & tabMask];
    UNUSED size_t numProbes = 0;
    for (;;) {
      assert(numProbes++ < tabMask + 1);
      if (0 == elm->m_name) {
        elm->m_name = name;
        return elm;
      }
      if (name->same(elm->m_name)) {
        return elm;
      }
      if (UNLIKELY(++elm == &table[tabMask + 1])) {
        elm = table;
      }
    }
  }

  Elm* insert(const StringData* name) {
    reserve(m_elms + 1);
    Elm* e = insertImpl(m_table, m_tabMask, name);
    if (!e->m_tv) {
      ++m_elms;
      name->incRefCount();
    }
    return e;
  }

  template<bool retainValue>
  TypedValue* migrateImpl(const StringData* name, TypedValue* loc) {
    Elm* elm = insert(name);
    if (!elm->m_tv) {
      addStorage(elm);
      tvWriteUninit(elm->m_tv);
      tvBindIndirect(elm->m_tv, loc);
      return 0;
    }

    TypedValue* ours = elm->m_tv;
    if (ours->m_type == KindOfIndirect) {
      TypedValue* old = ours->m_data.pind;
      ours->m_data.pind = loc;
      if (retainValue) {
        *loc = *old;
      }
      return old;
    }

    // Current value is actually stored inside the array.
    if (retainValue) {
      // loc is assumed dead; no need for a decRef.
      *loc = *ours;
    }
    tvBindIndirect(ours, loc);
    return 0;
  }

  void rehash(Elm* const target, const size_t targetMask) {
    for (size_t i = 0; i <= m_tabMask; ++i) {
      if (const StringData* sd = m_table[i].m_name) {
        Elm* targetElm = insertImpl(target, targetMask, sd);
        targetElm->m_name = sd;
        targetElm->m_tv = m_table[i].m_tv;
      }
    }
  }

  Elm* findElm(const StringData* name) const {
    Elm* elm = &m_table[name->hash() & m_tabMask];
    UNUSED size_t numProbes = 0;
    for (;;) {
      assert(numProbes++ < m_tabMask + 1);
      if (UNLIKELY(0 == elm->m_name)) {
        return 0;
      }
      if (name->same(elm->m_name)) return elm;
      if (UNLIKELY(++elm == &m_table[m_tabMask + 1])) {
        elm = m_table;
      }
    }
  }

  void addStorage(Elm* elm) {
    assert(!elm->m_tv);
    elm->m_tv = static_cast<TypedValue*>(
      m_storage.alloc(sizeof(TypedValue)));
    // Require 16-byte alignment so we can access globals with movdqa.
    assert(reinterpret_cast<uintptr_t>(elm->m_tv) % 16 == 0);
  }

private:
  // Power of two sized hashtable.
  Elm* m_table;
  uint32_t m_tabMask;
  uint32_t m_elms;
  ExpArena m_storage;
};

//////////////////////////////////////////////////////////////////////

struct NameValueTable::Iterator {
  explicit Iterator(const NameValueTable* tab)
    : m_tab(tab)
    , m_idx(0)
  {
    if (!valid()) next();
  }

  static Iterator getEnd(const NameValueTable* tab) {
    Iterator it;
    it.m_tab = tab;
    it.m_idx = tab->m_tabMask + 1;
    it.prev();
    return it;
  }

  /*
   * The following two constructors are primarily for using this with
   * the ArrayData interface (see NameValueTableWrapper), which
   * expects iterators to be represented by a ssize_t.
   *
   * The constructor taking `pos' must be given a value previously
   * returned from toInteger().
   *
   * The constructor taking a const StringData* starts iteration at
   * the key given, or returns an invalid iterator if that key does
   * not exist.
   */

  explicit Iterator(const NameValueTable* tab, ssize_t pos)
    : m_tab(tab)
    , m_idx(pos)
  {
    assert(pos >= 0);
    if (!valid()) next();
  }

  explicit Iterator(const NameValueTable* tab, const StringData* start)
    : m_tab(tab)
  {
    Elm* e = m_tab->findElm(start);
    m_idx = e ? e - m_tab->m_table : (m_tab->m_tabMask + 1);
  }

  ssize_t toInteger() const {
    const ssize_t invalid = ArrayData::invalid_index;
    return valid() ? m_idx : invalid;
  }

  bool valid() const {
    return m_idx >= 0 && size_t(m_idx) < m_tab->m_tabMask + 1 && !atEmpty();
  }

  const StringData* curKey() const {
    assert(valid());
    return m_tab->m_table[m_idx].m_name;
  }

  const TypedValue* curVal() const {
    assert(valid());
    return tvDerefIndirect(m_tab->m_table[m_idx].m_tv);
  }

  const TypedValue* curValRaw() const {
    assert(valid());
    return m_tab->m_table[m_idx].m_tv;
  }

  void next() {
    size_t const sz = m_tab->m_tabMask + 1;
    do {
      ++m_idx;
    } while (size_t(m_idx) < sz && atEmpty());
  }

  void prev() {
    do {
      --m_idx;
    } while (m_idx >= 0 && atEmpty());
  }

private:
  Iterator() {}

  bool atEmpty() const {
    if (!m_tab->m_table[m_idx].m_name) {
      return true;
    }
    const TypedValue* tv = m_tab->m_table[m_idx].m_tv;
    tv = tvDerefIndirect(tv);
    return tv->m_type == KindOfUninit;
  }

private:
  const NameValueTable* m_tab;
  ssize_t m_idx;
};

//////////////////////////////////////////////////////////////////////

inline NameValueTable::~NameValueTable() {
  if (!m_table) return;

  for (Iterator iter(this); iter.valid(); iter.next()) {
    decRefStr(const_cast<StringData*>(iter.curKey()));
    const TypedValue* tv = iter.curValRaw();
    if (tv->m_type != KindOfIndirect) {
      tvRefcountedDecRef(const_cast<TypedValue*>(tv));
    }
  }
  free(m_table);
}

//////////////////////////////////////////////////////////////////////

}

#endif
