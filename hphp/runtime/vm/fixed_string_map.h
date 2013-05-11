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

#include <util/base.h>
#include <runtime/base/string_data.h>

namespace HPHP {
namespace VM {
///////////////////////////////////////////////////////////////////////////////

template <typename V, bool case_sensitive> class FixedStringMap {
public:
  explicit FixedStringMap(int num) : m_table(0) { init(num); }
  FixedStringMap() : m_mask(0), m_table(0) {}
  ~FixedStringMap() { free(m_table); }

  void init(int num);
  void add(const StringData* s, const V& v);
  V *find(const StringData* s) const;

private:
  FixedStringMap(const FixedStringMap&);
  const FixedStringMap& operator=(const FixedStringMap&);

  struct Elm {
    const StringData* sd;
    V data;
  };

  /* Elements. */
  unsigned m_mask;
  Elm*     m_table;
};

///////////////////////////////////////////////////////////////////////////////
} }

#endif // incl_HPHP_FIXED_STRING_MAP_H_
