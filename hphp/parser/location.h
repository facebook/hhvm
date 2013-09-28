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

#ifndef incl_HPHP_PARSER_LOCATION_H_
#define incl_HPHP_PARSER_LOCATION_H_

#include "hphp/util/base.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(Location);
class Location {
public:
  Location() : file(""), line0(1), char0(1), line1(1), char1(1), cursor(0) {}

  const char *file;
  int line0;
  int char0;
  int line1;
  int char1;
  int cursor;

  void first(int line, char pos) {
    line0 = line; char0 = pos;
  }
  void first(Location &loc) {
    line0 = loc.line0; char0 = loc.char0;
  }
  void last(int line, char pos) {
    line1 = line; char1 = pos;
  }
  void last(Location &loc) {
    line1 = loc.line1; char1 = loc.char1;
  }

  /**
   * This only guarantees consistent result between two locations, whether or
   * not it makes sense, because we're comparing those integers first for
   * quicker sorting.
   */
  int compare(Location *loc) {
    if (line0 < loc->line0) return -1; if (line0 > loc->line0) return 1;
    if (char0 < loc->char0) return -1; if (char0 > loc->char0) return 1;
    if (line1 < loc->line1) return -1; if (line1 > loc->line1) return 1;
    if (char1 < loc->char1) return -1; if (char1 > loc->char1) return 1;
    return strcmp(file, loc->file);
  }
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_PARSER_LOCATION_H_
