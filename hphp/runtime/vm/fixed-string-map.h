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

#ifndef incl_HPHP_FIXED_STRING_MAP_H_
#define incl_HPHP_FIXED_STRING_MAP_H_

#include "hphp/util/base.h"
#include "hphp/runtime/base/string-data.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

template<class V, bool case_sensitive, class ExtraType = int32_t>
struct FixedStringMap {
  explicit FixedStringMap(int num) : m_table(0) { init(num); }
  FixedStringMap() : m_mask(0), m_table(0) {}
  ~FixedStringMap() { clear(); }

  FixedStringMap(const FixedStringMap&) = delete;
  const FixedStringMap& operator=(const FixedStringMap&) = delete;

  void clear();
  void init(int num);
  void add(const StringData* s, const V& v);
  V* find(const StringData* s) const;

  ExtraType& extra() { return m_extra; }
  const ExtraType& extra() const { return m_extra; }

private:
  struct Elm {
    const StringData* sd;
    V data;
  };

  /*
   * The fields we need here are just m_mask and m_table.  This would
   * leave 4 byte padding hole, though, which some users
   * (e.g. IndexedStringMap) might have a use for, so we expose it as
   * a slot for our users.
   */
  uint32_t  m_mask;
  ExtraType m_extra;  // not ours
  Elm*      m_table;
};

//////////////////////////////////////////////////////////////////////

}

#endif
