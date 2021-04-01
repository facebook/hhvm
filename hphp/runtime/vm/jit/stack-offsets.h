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

#include "hphp/util/safe-cast.h"

#include <cstdint>

/*
 * We compute and store 'stack offsets' in many different places in the jit,
 * and there are a number of different frames of reference for these offsets.
 * This header declares some structs that allow us to be explicit about which
 * frame of reference we're using and provides a little bit of type safety when
 * converting between these various reference frames.  Below is a diagram of
 * how the stack is generally laid out along with an example of what some
 * offsets in each reference frame refers to.
 *
 * Stack reference frames:
 *
 * Actual memory:
 *
 *    |  ActRec     |                    higher addresses
 *    |  ActRec     |                           |
 *    |  ActRec     |                           |
 *    +-------------+ <--- fp                   |
 *    | evalslot AA |                           |
 *    +-------------+                           |
 *    | evalslot BB |                           |
 *    +-------------+                           |
 *    | evalslot CC |                           |
 *    +-------------+ <--- sp                   |
 *    | evalslot DD |                           |
 *    +-------------+                           |
 *    | evalslot EE |                           |
 *    +-------------+                           |
 *    | ........... |                           |
 *                                      lower addresses
 *
 * FPInv:   Offset in cells relative to the frame pointer in reverse address
 *          order (i.e. a positive offset indicates lower address).
 * IRSPRel: Offset in cells relative to the IR stack pointer in address order.
 * BCSPRel: Offset in cells relative to the top of the stack at the start of
 *          the current bytecode in address order.
 *
 * Supposing we're translating a bytecode instruction where EE is "top of
 * stack" in HHBC semantics, then here are some examples:
 *
 *  slot    FPInv   IRSPRel   BCSPRel
 *    EE      5       -2         0
 *    DD      4       -1         1
 *    CC      3        0         2
 *    BB      2        1         3
 *    AA      1        2         4
 *
 * FPInvOffsets are usually used for the IR and BC stack pointer offsets,
 * relative to the frame pointer.  {IR,BC}SPRelOffsets are then offsets
 * relative to those stack pointers.
 */

namespace HPHP { namespace jit {

struct BCSPRelOffset;
struct IRSPRelOffset;
struct FPInvOffset;

///////////////////////////////////////////////////////////////////////////////

struct BCSPRelOffset {
  int32_t offset;

  /*
   * Comparisons.
   *
   * A "lower" BCSPRelOffset means "lower address" (i.e., farther from fp).
   */
  bool operator==(BCSPRelOffset o) const { return offset == o.offset; }
  bool operator<(BCSPRelOffset other) const { return offset < other.offset; }
  bool operator>(BCSPRelOffset other) const { return offset > other.offset; }

  /*
   * Move up and down the eval stack.
   */
  BCSPRelOffset operator+(int32_t d) const { return BCSPRelOffset{offset + d}; }
  BCSPRelOffset operator-(int32_t d) const { return BCSPRelOffset{offset - d}; }

  BCSPRelOffset& operator+=(size_t d) {
    offset += safe_cast<int32_t>(d);
    return *this;
  }

  /*
   * Increment/decrement.
   */
  BCSPRelOffset& operator++() {
    offset++;
    return *this;
  }
  BCSPRelOffset operator++(int) {
    auto before = *this;
    offset++;
    return before;
  }
  BCSPRelOffset& operator--() {
    offset--;
    return *this;
  }
  BCSPRelOffset operator--(int) {
    auto before = *this;
    offset--;
    return before;
  }

  /*
   * Convert to an FP*Off, given that the bytecode stack pointer is at `sp'
   * relative to the frame pointer.
   *
   * `this' is presumed relative to that bytecode stack pointer.
   */
  template<typename FPOff>
  FPOff to(FPInvOffset sp) const;
};

///////////////////////////////////////////////////////////////////////////////

struct IRSPRelOffset {
  int32_t offset;

  /*
   * Comparisons.
   *
   * A "lower" IRSPRelOffset means "lower address".
   */
  bool operator==(IRSPRelOffset o) const { return offset == o.offset; }
  bool operator!=(IRSPRelOffset o) const { return offset != o.offset; }
  bool operator< (IRSPRelOffset o) const { return offset <  o.offset; }
  bool operator<=(IRSPRelOffset o) const { return offset <= o.offset; }
  bool operator> (IRSPRelOffset o) const { return offset >  o.offset; }
  bool operator>=(IRSPRelOffset o) const { return offset >= o.offset; }

  IRSPRelOffset operator+(int32_t d) const { return IRSPRelOffset{offset + d}; }
  IRSPRelOffset operator-(int32_t d) const { return IRSPRelOffset{offset - d}; }

  /*
   * Increment.
   */
  IRSPRelOffset& operator++() {
    offset++;
    return *this;
  }

  /*
   * Difference.
   */
  int32_t operator-(IRSPRelOffset o) const { return offset - o.offset; }

  /*
   * Convert to an FP*Off, given that the IR stack pointer is at `sp' relative
   * to the frame pointer.
   *
   * `this' is presumed relative to that IR stack pointer.
   */
  template<typename FPOff>
  FPOff to(FPInvOffset sp) const;
};

inline IRSPRelOffset operator+(int32_t lhs, IRSPRelOffset rhs) {
  return rhs + lhs;
}

///////////////////////////////////////////////////////////////////////////////

struct FPInvOffset {
  int32_t offset;

  /*
   * Sentinel invalid offset.
   */
  static constexpr FPInvOffset invalid() { return FPInvOffset{INT_MIN}; }
  bool isValid() const { return offset != INT_MIN; }

  /*
   * Comparisons.
   *
   * A "lower" FPInvOffset means "closer to fp" (i.e., higher address).
   */
  bool operator==(FPInvOffset o) const { return offset == o.offset; }
  bool operator!=(FPInvOffset o) const { return offset != o.offset; }
  bool operator< (FPInvOffset o) const { return offset <  o.offset; }
  bool operator<=(FPInvOffset o) const { return offset <= o.offset; }
  bool operator> (FPInvOffset o) const { return offset >  o.offset; }
  bool operator>=(FPInvOffset o) const { return offset >= o.offset; }

  /*
   * Move up and down the stack space for the frame.
   */
  FPInvOffset operator+(int32_t d) const { return FPInvOffset{offset + d}; }
  FPInvOffset operator-(int32_t d) const { return FPInvOffset{offset - d}; }
  FPInvOffset& operator+=(int32_t d) { offset += d; return *this; }
  FPInvOffset& operator-=(int32_t d) { offset -= d; return *this; }

  /*
   * Increment/decrement.
   */
  FPInvOffset& operator++() {
    offset++;
    return *this;
  }
  FPInvOffset operator++(int) {
    auto before = *this;
    offset++;
    return before;
  }
  FPInvOffset& operator--() {
    offset--;
    return *this;
  }
  FPInvOffset operator--(int) {
    auto before = *this;
    offset--;
    return before;
  }

  /*
   * Difference.
   */
  int32_t operator-(FPInvOffset o) const { return offset - o.offset; }

  /*
   * Convert to an *SPOff, where the (bytecode or IR) stack pointer is at `sp'
   * relative to the frame pointer.
   */
  template<typename SPOff>
  SPOff to(FPInvOffset sp) const;
};

///////////////////////////////////////////////////////////////////////////////

/*
 * Interconversions.
 */

/*
 * {BC,IR}SPRelOffset -> FPInvOffset
 *
 *    +-----------------+ <-[fp]------------------
 *    |       ...       |           |     |
 *    +-----------------+           |     |
 *    |                 |           |     v (res)
 *    +-----------------+ <-[this]--+-------------
 *    |       ...       |           | ^
 *    +-----------------+           | |
 *    |                 |           v |
 *    +-----------------+ <-[sp]------------------
 */
template<> inline FPInvOffset
BCSPRelOffset::to<FPInvOffset>(FPInvOffset sp) const {
  return sp - offset;
}
template<> inline FPInvOffset
IRSPRelOffset::to<FPInvOffset>(FPInvOffset sp) const {
  return sp - offset;
}

/*
 * FPInvOffset -> {BC,IR}SPRelOffset
 */
template<> inline BCSPRelOffset
FPInvOffset::to<BCSPRelOffset>(FPInvOffset sp) const {
  return BCSPRelOffset{sp.offset - offset};
}
template<> inline IRSPRelOffset
FPInvOffset::to<IRSPRelOffset>(FPInvOffset sp) const {
  return IRSPRelOffset{sp.offset - offset};
}

///////////////////////////////////////////////////////////////////////////////

}}

