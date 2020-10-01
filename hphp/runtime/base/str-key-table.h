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

#include <bitset>

namespace HPHP {

struct StringData;

struct StrKeyTable {

public:
  static constexpr size_t kStrKeyTableSize = 64;
  // The mask is the bottom log2(kStrKeyTableSize) bits
  static constexpr int32_t kStrKeyTableMask = kStrKeyTableSize - 1;

  // Returns true if the given sd is not static
  bool mayContain(const StringData* sd) const;
  void add(const StringData* sd);
  void reset();
private:
  std::bitset<kStrKeyTableSize> m_table;

};

} // namespace HPHP
