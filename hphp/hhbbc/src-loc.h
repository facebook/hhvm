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
#ifndef incl_HHBBC_SRC_LOC_H_
#define incl_HHBBC_SRC_LOC_H_

#include <cstdint>

namespace HPHP { namespace HHBBC {

//////////////////////////////////////////////////////////////////////

namespace php {

using LineNumber = uint32_t;
using ColNumber  = uint32_t;
using LineRange  = std::tuple<LineNumber,LineNumber>;

struct SrcPos {
  bool operator==(SrcPos o) const {
    return line == o.line && col == o.col;
  }
  bool operator!=(SrcPos o) const { return !(*this == o); }

  LineNumber line;
  ColNumber col;
};

struct SrcLoc {
  SrcLoc()
    : start{0,0}
    , past{0,0}
  {}

  SrcLoc(SrcPos start, SrcPos past)
    : start(start)
    , past(past)
  {}

  bool isValid() const { return past != SrcPos{0,0}; }

  bool operator==(SrcLoc o) const {
    return start == o.start && past == o.past;
  }

  bool operator!=(SrcLoc o) const { return !(*this == o); }

  SrcPos start;
  SrcPos past;
};

std::string show(SrcLoc);

}

//////////////////////////////////////////////////////////////////////

}}

#endif
