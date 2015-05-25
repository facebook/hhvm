/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HPHP_JIT_STACK_OFFSETS_H_
#define incl_HPHP_JIT_STACK_OFFSETS_H_

#include "hphp/util/safe-cast.h"

/*
 * We compute and store 'stack offsets' in many different places in the jit,
 * and there are a number of different frames of reference for these offsets.
 * This header declares some structs that allow us to be explicit about which
 * frame of reference we're using and provides a little bit of type safety when
 * converting between these various reference frames. Below is a diagram of how
 * the stack is generally laid out along with an example of what some offsets in
 * each reference frame refers to.
 *
 * Stack reference frames:
 *
 * Actual memory:
 *
 *    |  ActRec     |                    higher addresses
 *    |  ActRec     |                           |
 *    |  ActRec     |  <--- fp                  |
 *    +-------------+                           |
 *    | evalslot AA |                           |
 *    +-------------+                           |
 *    | evalslot BB |                           |
 *    +-------------+                           |
 *    | evalslot CC |  <--- stkptr              |
 *    +-------------+                           |
 *    | evalslot DD |                           |
 *    +-------------+                           |
 *    | evalslot EE |                           |
 *    +-------------+                           |
 *    | ........... |                           |
 *                                      lower addresses
 *
 * FPRel: Offset in cells relative to the frame pointer in address order.
 * FPInv: Offset in cells relative to the frame pointer in reverse address
 *  order (i.e. a positive offset indicates lower address).
 * IRSP: Offset in cells relative to the IR stack pointer in address order.
 * BCSP: Offset in cells relative to the top of the stack at the start of the
 *  current bytecode in address order.
 *
 * Supposing we're translating a bytecode instruction where EE is "top of
 * stack" in HHBC semantics, then here are some examples:
 *
 *  slot       FPRel      FPInv         IRSP      BCSP
 *    EE         -5          5           -2         0
 *    DD         -4          4           -1         1
 *    CC         -3          3            0         2
 *    BB         -2          2            1         3
 *    AA         -1          1            2         4
 */

/*
* NB: The operators that are present in the various offset structs were added on
* an as-needed basis. If you find that something obvious is missing, it's
* probably because nobody was doing it previously. Do keep in mind, however,
* that it doesn't always make sense to add all operators for all offset types.
* For example, it doesn't really make sense to add an IRSPOffset and a
* BCSPOffset together.
*/

namespace HPHP { namespace jit {

struct IRSPOffset {
  int32_t offset;

  bool operator==(IRSPOffset o) const { return offset == o.offset; }
  bool operator==(int32_t other) const { return offset == other; }

  IRSPOffset operator+(int32_t x) const { return IRSPOffset{offset + x}; }
  IRSPOffset operator-(int32_t x) const { return IRSPOffset{offset - x}; }

  bool operator<(IRSPOffset o) const { return offset < o.offset; }
  bool operator>(IRSPOffset o) const { return offset > o.offset; }

  IRSPOffset& operator+=(size_t delta) {
    offset += delta;
    return *this;
  }
};

inline IRSPOffset operator+(int32_t lhs, IRSPOffset rhs) {
  return rhs + lhs;
}
inline IRSPOffset operator-(int32_t lhs, IRSPOffset rhs) {
  return IRSPOffset{lhs - rhs.offset};
}

struct BCSPOffset {
  int32_t offset;

  bool operator==(BCSPOffset o) const { return offset == o.offset; }
  bool operator==(int32_t otherOffset) const { return offset == otherOffset; }
  bool operator>=(int32_t otherOffset) const { return offset >= otherOffset; }
  bool operator<(BCSPOffset other) const { return offset < other.offset; }
  bool operator>(BCSPOffset other) const { return offset > other.offset; }
  bool operator<(int32_t otherOffset) const { return offset < otherOffset; }

  BCSPOffset& operator++() {
    offset++;
    return *this;
  }

  BCSPOffset operator++(int) {
    auto before = BCSPOffset{offset};
    offset++;
    return before;
  }

  BCSPOffset& operator+=(size_t delta) {
    offset += safe_cast<int32_t>(delta);
    return *this;
  }

  BCSPOffset operator-() const { return BCSPOffset{-offset}; }

  BCSPOffset operator-(int32_t delta) { return BCSPOffset{offset - delta}; }
};

struct FPInvOffset {
  int32_t offset;

  static constexpr FPInvOffset invalid() { return FPInvOffset{INT_MIN}; }

  bool operator==(FPInvOffset o) const { return offset == o.offset; }

  bool isValid() const {
    return offset != INT_MIN;
  }

  FPInvOffset operator+(int32_t delta) const {
    return FPInvOffset{offset + delta};
  }

  // Return an int32_t because the difference between two FPInvOffsets is no
  // longer an FPInvOffset.
  int32_t operator-(FPInvOffset o) const { return offset - o.offset; }
  FPInvOffset operator-(BCSPOffset o) const {
    return FPInvOffset{offset - o.offset};
  }
  FPInvOffset operator-(IRSPOffset o) {
    return FPInvOffset{offset - o.offset};
  }
  FPInvOffset operator-(int32_t delta) const {
    return FPInvOffset{offset - delta};
  }

  FPInvOffset& operator++() {
    offset++;
    return *this;
  }

  FPInvOffset& operator--() {
    offset--;
    return *this;
  }

  FPInvOffset& operator+=(int32_t delta) {
    offset += delta;
    return *this;
  }
  FPInvOffset& operator-=(int32_t delta) {
    offset -= delta;
    return *this;
  }

  bool operator>(FPInvOffset o) const { return offset > o.offset; }
  bool operator>(int32_t otherOffset) const { return offset > otherOffset; }
  bool operator>=(int32_t otherOffset) const { return offset >= otherOffset; }
  bool operator<(FPInvOffset other) const { return offset < other.offset; }
  bool operator<(int32_t otherOffset) const { return offset < otherOffset; }
};

inline bool operator>=(size_t lhs, FPInvOffset rhs) {
  return safe_cast<int32_t>(lhs) >= rhs.offset;
}

struct FPRelOffset {
  int32_t offset;

  FPRelOffset operator+(int32_t x) const { return FPRelOffset{offset + x}; }
  FPRelOffset operator-(int32_t x) const { return FPRelOffset{offset - x}; }
};

inline FPRelOffset operator+(int32_t lhs, FPRelOffset rhs) {
  return rhs + lhs;
}

} } // HPHP::jit

#endif // incl_HPHP_JIT_STACK_OFFSETS_H_
