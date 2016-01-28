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

#ifndef incl_HPHP_RUNTIME_VM_NAMEVALUETABLE_H_
#define incl_HPHP_RUNTIME_VM_NAMEVALUETABLE_H_

#include <folly/Bits.h>

#include "hphp/runtime/base/typed-value.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

struct ActRec;
struct StringData;

/*
 * This class implements a name to TypedValue map.  Basically a hashtable from
 * StringData* to TypedValue.
 *
 * This is for use in variable environments in bytecode.cpp, and is also used
 * for the global variable environment ($GLOBALS via GlobalsArray).
 *
 * The table may be optionally attached to an ActRec, in which case it will
 * contain a kNamedLocalDataType TypedValue per every named local defined in
 * ActRec's function.  This is to keep storage for locals in functions with a
 * VarEnv in their normal location, but still make them accessible by name
 * through this table.
 */
struct NameValueTable {
  struct Iterator {
    explicit Iterator(const NameValueTable* tab);
    static Iterator getLast(const NameValueTable* tab);
    static Iterator getEnd(const NameValueTable* tab);

    /*
     * The following two constructors are primarily for using this with
     * the ArrayData interface (see GlobalsArray), which
     * expects iterators to be represented by a ssize_t.
     *
     * The constructor taking `pos' must be given a value previously
     * returned from toInteger().
     *
     * The constructor taking a const StringData* starts iteration at
     * the key given, or returns an invalid iterator if that key does
     * not exist.
     */
    explicit Iterator(const NameValueTable* tab, ssize_t pos);
    explicit Iterator(const NameValueTable* tab, const StringData* start);

    ssize_t toInteger() const;
    bool valid() const;
    const StringData* curKey() const;
    const TypedValue* curVal() const;
    void next();
    void prev();

  private:
    explicit Iterator() {}
    bool atEmpty() const;

  private:
    const NameValueTable* m_tab;
    ssize_t m_idx;
  };

  /*
   * Create a global NameValueTable.
   */
  explicit NameValueTable();

  /*
   * Create a NameValueTable attached to an already existing ActRec
   * and populate the table with ActRec's locals.
   */
  explicit NameValueTable(ActRec* fp);

  /**
   * Clone NameValueTable.
   */
  explicit NameValueTable(const NameValueTable& nvTable, ActRec* fp);

  ~NameValueTable();

  NameValueTable(const NameValueTable&) = delete;
  NameValueTable& operator=(const NameValueTable&) = delete;

  /**
   * Suspend locals into an in-resumable ActRec.
   */
  void suspend(const ActRec* oldFP, ActRec* newFP);

  /**
   * Attach to a new ActRec and populate its locals with TypedValues stored
   * in this NameValueTable.
   */
  void attach(ActRec* fp);

  /**
   * Detach from the current ActRec and steal its named locals.
   */
  void detach(ActRec* fp);

  ActRec* getFP() const { return m_fp; }

  /*
   * Explicitly request letting go of all elements in the
   * NameValueTable without decrefing them.
   *
   * This is intended for use when destroying the global scope, where
   * we shouldn't be running destructors.
   */
  void leak();
  bool leaked() const { return !m_table; }

  /*
   * Set the slot for the supplied name to `val', allocating it if
   * necessary.
   */
  TypedValue* set(const StringData* name, const TypedValue* val);

  /*
   * Bind the slot for the supplied name to `val', allocating it and
   * boxing it first if necessary.
   */
  TypedValue* bind(const StringData* name, TypedValue* val);

  /*
   * Remove an element from this table.  All elements added always
   * occupy storage, so this is done by setting the element to
   * KindOfUninit.
   */
  void unset(const StringData* name);

  /*
   * Lookup a name, returning null if it doesn't exist in this table.
   */
  TypedValue* lookup(const StringData* name);

  /*
   * Insert a name to value entry with KindOfNull for the value, or
   * return what is already there if the key already exists in the
   * table.
   */
  TypedValue* lookupAdd(const StringData* name);

private:
  // Dummy DT for named locals; keep out of conflict with actual DataTypes in
  // base/datatype.h.
  static constexpr auto kNamedLocalDataType = kExtraInvalidDataType;

  // Element type for the name/value hashtable.
  struct Elm {
    TypedValue        m_tv;
    const StringData* m_name;
    template<class F> void scan(F& mark) const {
      if (m_name) {
        mark(m_name);
        if (m_tv.m_type != kNamedLocalDataType) {
          mark(m_tv);
        }
      }
    }
  };

private:
  void reserve(size_t desiredSize);
  void allocate(const size_t newCapac);
  TypedValue* derefNamedLocal(TypedValue* tv) const;
  TypedValue* findTypedValue(const StringData* name);
  Elm* insertImpl(const StringData* name);
  Elm* insert(const StringData* name);
  void rehash(Elm* const oldTab, const size_t oldMask);
  Elm* findElm(const StringData* name) const;

public:
  template<class F> void scan(F& mark) const {
    // TODO #6511877 need to access ActRec::scan() here.
    //m_fp->scan(mark);
    if (leaked()) return;
    for (unsigned i = 0, n = m_tabMask+1; i < n; ++i) {
      m_table[i].scan(mark);
    }
  }

private:
  ActRec* m_fp{nullptr};
  Elm* m_table{nullptr}; // Power of two sized hashtable.
  uint32_t m_tabMask{0};
  uint32_t m_elms{0};
};

//////////////////////////////////////////////////////////////////////

}

#endif
