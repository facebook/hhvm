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

#include <folly/Bits.h>

#include "hphp/runtime/base/tv-val.h"
#include "hphp/runtime/base/typed-value.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

struct StringData;

/*
 * This class implements a name to TypedValue map. Basically a hashtable from
 * StringData* to TypedValue. Used by variable environments in bytecode.cpp.
 */
struct NameValueTable {
  struct Iterator {
    explicit Iterator(const NameValueTable* tab);
    static Iterator getLast(const NameValueTable* tab);
    static Iterator getEnd(const NameValueTable* tab);

    ssize_t toInteger() const;
    bool valid() const;
    const StringData* curKey() const;
    tv_rval curVal() const;
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

  ~NameValueTable();

  NameValueTable(const NameValueTable&) = delete;
  NameValueTable& operator=(const NameValueTable&) = delete;

  /*
   * Set the slot for the supplied name to `val', allocating it if
   * necessary.
   */
  tv_lval set(const StringData* name, tv_rval val);

  /*
   * Remove an element from this table.  All elements added always
   * occupy storage, so this is done by setting the element to
   * KindOfUninit.
   */
  void unset(const StringData* name);

  /*
   * Lookup a name, returning null if it doesn't exist in this table.
   */
  tv_lval lookup(const StringData* name);

  /*
   * Insert a name to value entry with KindOfNull for the value, or
   * return what is already there if the key already exists in the
   * table.
   */
  tv_lval lookupAdd(const StringData* name);

private:
  // Element type for the name/value hashtable.
  struct Elm {
    TypedValue        m_tv;
    const StringData* m_name;
    TYPE_SCAN_CUSTOM() {
      // m_tv is only valid if m_name != null
      if (m_name) {
        scanner.scan(m_name);
        scanner.scan(m_tv);
      }
    }
  };

private:
  void reserve(size_t desiredSize);
  void allocate(const size_t newCapac);
  tv_lval findTypedValue(const StringData* name);
  Elm* insertImpl(const StringData* name);
  Elm* insert(const StringData* name);
  void rehash(Elm* const oldTab, const size_t oldMask);
  Elm* findElm(const StringData* name) const;

private:
  Elm* m_table{nullptr};
  uint32_t m_tabMask{0};
  uint32_t m_elms{0};

  TYPE_SCAN_CUSTOM() {
    scanner.scan(m_table);
  }
};

//////////////////////////////////////////////////////////////////////

}

