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
#ifndef incl_HPHP_ALIAS_CLASS_H_
#define incl_HPHP_ALIAS_CLASS_H_

#include <string>
#include <cstdint>

#include <folly/Optional.h>

namespace HPHP { struct StringData; }
namespace HPHP { namespace jit {

struct SSATmp;

//////////////////////////////////////////////////////////////////////

/*
 * AliasClass represents a lattice of abstract memory locations.  These support
 * type-system-like operations (operator<= for subset, operator| for union, and
 * maybe() to test for non-zero intersections).
 *
 * AUnknown is the top of the lattice (is the class of all possible memory
 * locations we care about).  We don't subdivide very much yet, so many things
 * should end up there.  AEmpty is the bottom.  The various special location
 * types AFoo all have an AFooAny, which is the superset of all AFoos.
 */
struct AliasClass;

//////////////////////////////////////////////////////////////////////

/*
 * Special data for locations known to be exactly a specific local on the frame
 * `fp'.
 */
struct AFrame { SSATmp* fp; uint32_t id; };

/*
 * A location inside of an object property, with base `obj' and byte offset
 * `offset' from the ObjectData*.
 */
struct AProp  { SSATmp* obj; uint32_t offset; };

/*
 * A integer index inside of an array, with base `arr'.  The `arr' tmp is any
 * kind of array (not necessarily kPackedKind or whatnot).
 */
struct AElemI { SSATmp* arr; uint64_t idx; };

/*
 * A location inside of an array, with base `arr', with a string key.  The
 * `arr' tmp is any kind of array.
 */
struct AElemS { SSATmp* arr; const StringData* key; };

/*
 * A range of the stack, starting at `offset' from a base pointer, and
 * extending `size' slots deeper into the stack (toward lower memory
 * addresses).  The base pointer may either be a StkPtr or a FramePtr (see
 * below).  The reason ranges extend downward is that it is common to need to
 * refer to the class of all stack locations below some depth (this can be done
 * by putting INT32_MAX in the size).
 *
 * Some notes on how the evaluation stack is treated for alias analysis:
 *
 *   o Unlike AFrame locations, in general AStack locations with different
 *     bases may alias each other, even if the offsets are the same.  This is
 *     because we define new StkPtrs in the middle of a region for the same
 *     function, but there is only one FramePtr for a function.
 *
 *   o We represent canonicalized AStack locations as offsets off of the
 *     FramePtr, to address the above aliasing issue.  See canonicalize() in
 *     the .cpp.  AStack locations based on different FramePtrs are presumed
 *     never to alias, and also are naturally presumed never to alias AFrame
 *     locations.  Either of these things 'could' be done, but it is illegal to
 *     generate IR that accesses eval stack locations using offsets from a
 *     FramePtr, or accesses frame locals using offsets from a StkPtr.  (It
 *     would break things in generators, among other things.)
 *
 *   o In situations with inlined calls, we may in fact have AFrame locations
 *     that refer to the same concrete memory locations (in the evaluation
 *     stack) as other AStack locations in the same HHIR program.  However, the
 *     portions of the HHIR program that may use that memory location in either
 *     of those ways may not overlap, based on the HHBC semantics of function
 *     calls and eval stack usage.  (This may seem odd, but it's not really any
 *     different from knowing that a physical portion of the heap may be
 *     treated as both an AElemI or AProp, but not at the same time based on
 *     HHBC-level semantics.)
 *
 *   o IR instructions that may re-enter can generally potentially write to the
 *     evaluation stack below some logical depth.  Right now that depth is
 *     available in the BCMarker for each instruction, but there is no way to
 *     find which frame or stack pointer it is relative to.  In memory_effects
 *     right now this means we generally must include AStackAny in the
 *     may-store set for any instruction that can re-enter.
 */
struct AStack { SSATmp* base; int32_t offset; int32_t size; };

//////////////////////////////////////////////////////////////////////

struct AliasClass {
  enum rep : uint32_t {  // bits for various location classes
    BEmpty   = 0,

    BFrame   = 1 << 0,
    BProp    = 1 << 1,
    BElemI   = 1 << 2,
    BElemS   = 1 << 3,
    BStack   = 1 << 4,

    BElem    = BElemI | BElemS,

    BHeap    = ~(BFrame|BStack),

    BNonFrame = ~BFrame,
    BNonStack = ~BStack,

    BUnknown = static_cast<uint32_t>(-1),
  };

  /*
   * A hashing function for alias classes.
   */
  struct Hash { size_t operator()(AliasClass) const; };

  /*
   * Create an alias class from a bit representation.  Usually you should use
   * one of the other functions to create these.
   */
  explicit AliasClass(rep bits) : m_bits{bits} {}

  /*
   * Create an alias class with more precise specialized information about
   * where it is.
   */
  /* implicit */ AliasClass(AFrame);
  /* implicit */ AliasClass(AProp);
  /* implicit */ AliasClass(AElemI);
  /* implicit */ AliasClass(AElemS);
  /* implicit */ AliasClass(AStack);

  /*
   * Exact equality.
   */
  bool operator==(AliasClass) const;
  bool operator!=(AliasClass o) const { return !(*this == o); }

  /*
   * Create an alias class that is at least as big as the true union of this
   * alias class and another one.  Guaranteed to be commutative.
   */
  AliasClass operator|(AliasClass) const;

  /*
   * Returns whether this alias class is a non-strict-subset of another one.
   */
  bool operator<=(AliasClass) const;

  /*
   * Returns whether an alias class could possibly refer to the same concrete
   * memory location as another one.  Basically, do they have a non-empty
   * intersection.
   */
  bool maybe(AliasClass) const;

  /*
   * Conditionally access specific known information of various kinds.
   *
   * Returns folly::none if this alias class has no specialization in that way.
   */
  folly::Optional<AFrame> frame() const;
  folly::Optional<AProp>  prop() const;
  folly::Optional<AElemI> elemI() const;
  folly::Optional<AElemS> elemS() const;
  folly::Optional<AStack> stack() const;

  /*
   * Conditionally access specific known information, but also checking that
   * that is the only major category contained in this AliasClass.
   *
   * I.e., cls.is_foo() is semantically equivalent to:
   *
   *   cls <= AFooAny ? cls.foo() : folly::none
   */
  folly::Optional<AFrame> is_frame() const;
  folly::Optional<AProp>  is_prop() const;
  folly::Optional<AElemI> is_elemI() const;
  folly::Optional<AElemS> is_elemS() const;
  folly::Optional<AStack> is_stack() const;

private:
  enum class STag {
    None,
    Frame,
    Prop,
    ElemI,
    ElemS,
    Stack,
  };

private:
  friend std::string show(AliasClass);
  friend AliasClass canonicalize(AliasClass);
  bool checkInvariants() const;
  bool equivData(AliasClass) const;
  bool subclassData(AliasClass) const;
  bool maybeData(AliasClass) const;
  static AliasClass unionData(rep newBits, AliasClass, AliasClass);
  static rep stagBit(STag tag);

private:
  rep m_bits;
  STag m_stag{STag::None};
  union {
    AFrame   m_frame;
    AProp    m_prop;
    AElemI   m_elemI;
    AElemS   m_elemS;
    AStack   m_stack;
  };
};

//////////////////////////////////////////////////////////////////////

auto const AEmpty    = AliasClass{AliasClass::BEmpty};
auto const AFrameAny = AliasClass{AliasClass::BFrame};
auto const APropAny  = AliasClass{AliasClass::BProp};
auto const AHeapAny  = AliasClass{AliasClass::BHeap};
auto const ANonFrame = AliasClass{AliasClass::BNonFrame};
auto const ANonStack = AliasClass{AliasClass::BNonStack};
auto const AStackAny = AliasClass{AliasClass::BStack};
auto const AElemIAny = AliasClass{AliasClass::BElemI};
auto const AElemSAny = AliasClass{AliasClass::BElemS};
auto const AElemAny  = AliasClass{AliasClass::BElem};
auto const AUnknown  = AliasClass{AliasClass::BUnknown};

//////////////////////////////////////////////////////////////////////

/*
 * Replace any SSATmps in an AliasClass with their canonical name (chasing
 * passthrough instructions as with canonical() from analysis.h.)
 */
AliasClass canonicalize(AliasClass);

/*
 * Produce a debug string for an alias class.
 */
std::string show(AliasClass);

//////////////////////////////////////////////////////////////////////

}}

#endif
