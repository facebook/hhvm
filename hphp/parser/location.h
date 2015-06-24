/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_PARSER_LOCATION_H_
#define incl_HPHP_PARSER_LOCATION_H_

#include <cstring>
#include <memory>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class Location {
public:
  Location() = default;
  Location(int l0, int c0, int l1, int c1)
      : r(l0, c0, l1, c1) {}

  struct Range {
    Range() = default;
    Range(int l0, int c0, int l1, int c1)
        : line0(l0), char0(c0), line1(l1), char1(c1) {}
    int line0{1};
    int char0{1};
    int line1{1};
    int char1{1};

    int compare(const Range& other) const {
      if (line0 < other.line0) return -1; if (line0 > other.line0) return 1;
      if (char0 < other.char0) return -1; if (char0 > other.char0) return 1;
      if (line1 < other.line1) return -1; if (line1 > other.line1) return 1;
      if (char1 < other.char1) return -1; if (char1 > other.char1) return 1;
      return 0;
    }
  };

  const char *file{""};
  Range r;
  int cursor{0};

  void first(int line, char pos) {
    r.line0 = line; r.char0 = pos;
  }
  void first(Location &loc) {
    r.line0 = loc.r.line0; r.char0 = loc.r.char0;
  }
  void last(int line, char pos) {
    r.line1 = line; r.char1 = pos;
  }
  void last(Location &loc) {
    r.line1 = loc.r.line1; r.char1 = loc.r.char1;
  }

  /**
   * This only guarantees consistent result between two locations, whether or
   * not it makes sense, because we're comparing those integers first for
   * quicker sorting.
   */
  int compare(const Location *loc) const {
    if (auto d = r.compare(loc->r)) return d;
    return strcmp(file, loc->file);
  }
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_PARSER_LOCATION_H_
