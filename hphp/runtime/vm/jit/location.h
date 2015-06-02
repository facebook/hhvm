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

#ifndef incl_HPHP_JIT_LOCATION_H_
#define incl_HPHP_JIT_LOCATION_H_

#include <vector>

#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/jit/region-selection.h"
#include "hphp/util/safe-cast.h"

namespace HPHP { namespace jit {

// Location --
//   A user-program-visible, and bytecode ISA addressable place for a PHP
//   value.
struct Location {
  enum Space {
    Invalid,  // Unknown location
    Stack,    // Stack; offset == delta from top
    Local,    // Stack frame's registers; offset == local register
    Iter,     // Stack frame's iterators
    Litstr,   // Literal string pseudo-location
    Litint,   // Literal int pseudo-location
    This,     // $this in the current frame
  };

  explicit Location(Space spc)
    : space(spc)
  {
    assertx(spc == This);
    offset = 0;
  }

  Location(Space spc, int64_t off)
    : space(spc)
  {
    assertx(spc != Stack);
    offset = off;
  }

  explicit Location(BCSPOffset offset)
    : space(Stack)
  {
    bcRelOffset = offset;
  }

  Location()
    : space(Invalid)
  {
    offset = -1;
  }

  int cmp(const Location &r) const {
#define CMP(field) do { \
  if (field > r.field)      { return 1; } \
  else if (field < r.field) { return -1; } } while(0)
    CMP(space);
    switch (space) {
    case Stack:
      CMP(bcRelOffset);
      return 0;
    default:
      CMP(offset);
      return 0;
    }
#undef CMP
  }

  bool operator==(const Location& r) const {
    return cmp(r) == 0;
  }

  bool operator!=(const Location& r) const {
    return cmp(r) != 0;
  }

  bool operator<(const Location& r) const {
    return cmp(r) < 0;
  }

  // Hash function.
  size_t operator()(const Location& l) const {
    switch (space) {
    case Stack:
      return HPHP::hash_int64_pair(l.space, l.bcRelOffset.offset);
    default:
      return HPHP::hash_int64_pair(l.space, l.offset);
    }
  }

  const char *spaceName() const {
    switch(space) {
    case Stack:  return "Stk";
    case Local:  return "Local";
    case Iter:   return "Iter";
    case Litstr: return "Litstr";
    case Litint: return "Litint";
    case This:   return "This";
    case Invalid:return "*invalid*";
    default:     not_reached();
    }
  }

  std::string pretty() const {
    switch (space) {
    case Stack:
      return folly::format("(Location {} {})",
        spaceName(), bcRelOffset.offset).str();
    default:
      return folly::format("(Location {} {})", spaceName(), offset).str();
    }
  }

  bool isStack() const {
    return space == Stack;
  }

  bool isLocal() const {
    return space == Local;
  }

  bool isInvalid() const {
    return space == Invalid;
  }

  bool isValid() const {
    return !isInvalid();
  }

  bool isLiteral() const {
    return space == Litstr || space == Litint;
  }

  bool isThis() const {
    return space == This;
  }

  bool isIter() const {
    return space == Iter;
  }

public:
  Space space;
  union {
    BCSPOffset bcRelOffset;
    int64_t offset;
  };
};

} }

#endif // incl_HPHP_JIT_LOCATION_H_
