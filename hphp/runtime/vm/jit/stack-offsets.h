/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
 * FPRel: Offset in cells relative to the frame pointer in address order.
 * FPInv: Offset in cells relative to the frame pointer in reverse address
 *        order (i.e. a positive offset indicates lower address).
 * IRSP: Offset in cells relative to the IR stack pointer in address order.
 * BCSP: Offset in cells relative to the top of the stack at the start of the
 *       current bytecode in address order.
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
 *
 * FPInvOffsets are usually used for the IR and BC stack pointer offsets,
 * relative to the frame pointer.  {IR,BC}SPOffsets are then offsets relative
 * to those stack pointers.
 */

namespace HPHP { namespace jit {

struct BCSPOffset;
struct IRSPOffset;
struct FPInvOffset;
struct FPRelOffset;

///////////////////////////////////////////////////////////////////////////////

struct BCSPOffset {
  int32_t offset;

  /*
   * Comparisons.
   *
   * A "lower" BCSPOffset means "lower address" (i.e., farther from fp).
   */
  bool operator==(BCSPOffset o) const { return offset == o.offset; }
  bool operator<(BCSPOffset other) const { return offset < other.offset; }
  bool operator>(BCSPOffset other) const { return offset > other.offset; }

  /*
   * Move up and down the eval stack.
   */
  BCSPOffset operator+(int32_t d) const { return BCSPOffset{offset + d}; }
  BCSPOffset operator-(int32_t d) const { return BCSPOffset{offset - d}; }

  BCSPOffset& operator+=(size_t d) {
    offset += safe_cast<int32_t>(d);
    return *this;
  }

  /*
   * Increment/decrement.
   */
  BCSPOffset& operator++() {
    offset++;
    return *this;
  }
  BCSPOffset operator++(int) {
    auto before = *this;
    offset++;
    return before;
  }
  BCSPOffset& operator--() {
    offset--;
    return *this;
  }
  BCSPOffset operator--(int) {
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

struct IRSPOffset {
  int32_t offset;

  bool operator==(IRSPOffset o) const { return offset == o.offset; }

  IRSPOffset operator+(int32_t d) const { return IRSPOffset{offset + d}; }
  IRSPOffset operator-(int32_t d) const { return IRSPOffset{offset - d}; }

  /*
   * Convert to an FP*Off, given that the IR stack pointer is at `sp' relative
   * to the frame pointer.
   *
   * `this' is presumed relative to that IR stack pointer.
   */
  template<typename FPOff>
  FPOff to(FPInvOffset sp) const;
};

inline IRSPOffset operator+(int32_t lhs, IRSPOffset rhs) {
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
  FPInvOffset& operator--() {
    offset--;
    return *this;
  }

  /*
   * Difference.
   */
  int32_t operator-(FPInvOffset o) const { return offset - o.offset; }

  /*
   * Invert to an FPRelOffset.
   */
  FPRelOffset operator-() const;

  /*
   * Convert to an *SPOff, where the (bytecode or IR) stack pointer is at `sp'
   * relative to the frame pointer.
   */
  template<typename SPOff>
  SPOff to(FPInvOffset sp) const;
};

///////////////////////////////////////////////////////////////////////////////

struct FPRelOffset {
  int32_t offset;

  FPRelOffset operator+(int32_t x) const { return FPRelOffset{offset + x}; }
  FPRelOffset operator-(int32_t x) const { return FPRelOffset{offset - x}; }

  /*
   * Invert to an FPInvOffset.
   */
  FPInvOffset operator-() const;

  /*
   * Convert to an *SPOff, where the (bytecode or IR) stack pointer is at `sp'
   * relative to the frame pointer.
   */
  template<typename SPOff>
  SPOff to(FPInvOffset sp) const;
};

inline FPRelOffset operator+(int32_t lhs, FPRelOffset rhs) {
  return rhs + lhs;
}

///////////////////////////////////////////////////////////////////////////////
/*
 * Interconversions.
 */

/*
 * FPInvOffset <-> FPRelOffset
 */
inline FPRelOffset FPInvOffset::operator-() const {
  return FPRelOffset{-offset};
}
inline FPInvOffset FPRelOffset::operator-() const {
  return FPInvOffset{-offset};
}

/*
 * {BC,IR}SPOffset -> FPInvOffset
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
BCSPOffset::to<FPInvOffset>(FPInvOffset sp) const {
  return sp - offset;
}
template<> inline FPInvOffset
IRSPOffset::to<FPInvOffset>(FPInvOffset sp) const {
  return sp - offset;
}

/*
 * {BC,IR}SPOffset -> FPRelOffset
 */
template<> inline FPRelOffset
BCSPOffset::to<FPRelOffset>(FPInvOffset sp) const {
  return -to<FPInvOffset>(sp);
}
template<> inline FPRelOffset
IRSPOffset::to<FPRelOffset>(FPInvOffset sp) const {
  return -to<FPInvOffset>(sp);
}

/*
 * FPInvOffset -> {BC,IR}SPOffset
 */
template<> inline BCSPOffset
FPInvOffset::to<BCSPOffset>(FPInvOffset sp) const {
  return BCSPOffset{sp.offset - offset};
}
template<> inline IRSPOffset
FPInvOffset::to<IRSPOffset>(FPInvOffset sp) const {
  return IRSPOffset{sp.offset - offset};
}

/*
 * FPRelOffset -> {BC,IR}SPOffset
 */
template<> inline BCSPOffset
FPRelOffset::to<BCSPOffset>(FPInvOffset sp) const {
  return (-*this).to<BCSPOffset>(sp);
}
template<> inline IRSPOffset
FPRelOffset::to<IRSPOffset>(FPInvOffset sp) const {
  return (-*this).to<IRSPOffset>(sp);
}

///////////////////////////////////////////////////////////////////////////////

}}

#endif // incl_HPHP_JIT_STACK_OFFSETS_H_
