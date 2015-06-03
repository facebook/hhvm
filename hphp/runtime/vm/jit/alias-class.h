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
 * locations we care about).  AEmpty is the bottom.  AUnknownTV is a union of
 * all the classes that contain TypedValue-style storage of PHP values.  The
 * various special location types AFoo all have an AFooAny, which is the
 * superset of all AFoos.
 *
 * Part of the lattice currently looks like this:
 *
 *                         Unknown
 *                            |
 *                            |
 *                    +-------+-------+----------+
 *                    |               |          |
 *                 UnknownTV      IterPosAny  IterBaseAny
 *                    |               |          |
 *                    |              ...        ...
 *                    |
 *      +---------+---+---------------+-------------------------+
 *      |         |                   |                         |
 *      |         |                   |                         |
 *      |         |                   |                         |
 *      |         |                   |                         |
 *      |         |                HeapAny*                     |
 *      |         |                   |                         |
 *      |         |            +------+------+---------+        |
 *      |         |            |             |         |        |
 *   FrameAny  StackAny     ElemAny       PropAny   RefAny  MIStateAny
 *      |         |          /    \          |         |        |
 *     ...       ...   ElemIAny  ElemSAny   ...       ...      ...
 *                        |         |
 *                       ...       ...
 *
 *   (*) AHeapAny contains some things other than ElemAny, PropAny and RefAny
 *       that don't have explicit nodes in the lattice yet.  (Like the
 *       lvalBlackhole, etc.)  It's hard for this to matter to client code for
 *       now because we don't expose an intersection or difference operation.
 */
struct AliasClass;

//////////////////////////////////////////////////////////////////////

/*
 * Special data for locations known to be exactly a specific local on the frame
 * `fp'.
 */
struct AFrame { SSATmp* fp; uint32_t id; };

/*
 * A specific php iterator's position value (m_pos).
 */
struct AIterPos  { SSATmp* fp; uint32_t id; };

/*
 * A specific php iterator's base and initialization state, for non-mutable
 * iterators.
 *
 * Instances of this AliasClass cover both the memory storing the pointer to
 * the object being iterated, and the initialization flags (itype and next
 * helper)---the reason for this is that nothing may load/store the
 * initialization state if it isn't also going to load/store the base pointer.
 */
struct AIterBase { SSATmp* fp; uint32_t id; };

/*
 * A location inside of an object property, with base `obj' and byte offset
 * `offset' from the ObjectData*.
 */
struct AProp  { SSATmp* obj; uint32_t offset; };

/*
 * A integer index inside of an array, with base `arr'.  The `arr' tmp is any
 * kind of array (not necessarily kPackedKind or whatnot).
 */
struct AElemI { SSATmp* arr; int64_t idx; };

/*
 * A location inside of an array, with base `arr', with a string key.  The
 * `arr' tmp is any kind of array.
 */
struct AElemS { SSATmp* arr; const StringData* key; };

/*
 * A range of the stack, starting at `offset' from the outermost frame pointer,
 * and extending `size' slots deeper into the stack (toward lower memory
 * addresses).  The frame pointer is the same for all stack ranges in the IR
 * unit, and thus is not stored here.  The reason ranges extend downward is
 * that it is common to need to refer to the class of all stack locations below
 * some depth (this can be done by putting INT32_MAX in the size).
 *
 * Some notes on how the evaluation stack is treated for alias analysis:
 *
 *   o Since AStack locations are always canonicalized, different AStack
 *     locations must not alias if there is no overlap in the ranges.
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
 */
struct AStack {
  // We can create an AStack from either a stack pointer or a frame
  // pointer. This constructor canonicalizes the offset to base on the
  // outermost frame pointer.
  explicit AStack(SSATmp* base, int32_t offset, int32_t size);
  explicit AStack(int32_t o, int32_t s) : offset(o), size(s) {}

  int32_t offset;
  int32_t size;
};

/*
 * One of the MInstrState TypedValues, at a particular offset in bytes.
 */
struct AMIState { int32_t offset; };

/*
 * A RefData referenced by a BoxedCell.
 */
struct ARef { SSATmp* boxed; };

//////////////////////////////////////////////////////////////////////

struct AliasClass {
  enum rep : uint32_t {  // bits for various location classes
    BEmpty    = 0,
    // The relative order of the values are used in operator| to decide
    // which specialization is more useful.
    BFrame    = 1 << 0,
    BIterPos  = 1 << 1,
    BIterBase = 1 << 2,
    BProp     = 1 << 3,
    BElemI    = 1 << 4,
    BElemS    = 1 << 5,
    BStack    = 1 << 6,
    BMIState  = 1 << 7,
    BRef      = 1 << 8,

    BElem     = BElemI | BElemS,
    BHeap     = BElem | BProp | BRef,

    BUnknownTV = ~(BIterPos | BIterBase),

    BUnknown   = static_cast<uint32_t>(-1),
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
  /* implicit */ AliasClass(AIterPos);
  /* implicit */ AliasClass(AIterBase);
  /* implicit */ AliasClass(AProp);
  /* implicit */ AliasClass(AElemI);
  /* implicit */ AliasClass(AElemS);
  /* implicit */ AliasClass(AStack);
  /* implicit */ AliasClass(AMIState);
  /* implicit */ AliasClass(ARef);

  /*
   * Exact equality.
   */
  bool operator==(AliasClass) const;
  bool operator!=(AliasClass o) const { return !(*this == o); }

  /*
   * Return an AliasClass that is the precise union of this class and another
   * class, or folly::none if that precise union cannot be represented.
   *
   * Guaranteed to be commutative.
   */
  folly::Optional<AliasClass> precise_union(AliasClass) const;

  /*
   * Create an alias class that is at least as big as the true union of this
   * alias class and another one.
   *
   * If this.precise_union(o) is not folly::none, this function is guaranteed
   * to return *this.precise_union(o).  Otherwise it may return an arbitrary
   * AliasClass bigger than the (unrepresentable) true union---callers should
   * not rely on specifics about how much bigger it is.
   *
   * Guaranteed to be commutative.
   */
  AliasClass operator|(AliasClass o) const;

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
  folly::Optional<AFrame>    frame() const;
  folly::Optional<AIterPos>  iterPos() const;
  folly::Optional<AIterBase> iterBase() const;
  folly::Optional<AProp>     prop() const;
  folly::Optional<AElemI>    elemI() const;
  folly::Optional<AElemS>    elemS() const;
  folly::Optional<AStack>    stack() const;
  folly::Optional<AMIState>  mis() const;
  folly::Optional<ARef>      ref() const;

  /*
   * Conditionally access specific known information, but also checking that
   * that is the only major category contained in this AliasClass.
   *
   * I.e., cls.is_foo() is semantically equivalent to:
   *
   *   cls <= AFooAny ? cls.foo() : folly::none
   */
  folly::Optional<AFrame>    is_frame() const;
  folly::Optional<AIterPos>  is_iterPos() const;
  folly::Optional<AIterBase> is_iterBase() const;
  folly::Optional<AProp>     is_prop() const;
  folly::Optional<AElemI>    is_elemI() const;
  folly::Optional<AElemS>    is_elemS() const;
  folly::Optional<AStack>    is_stack() const;
  folly::Optional<AMIState>  is_mis() const;
  folly::Optional<ARef>      is_ref() const;

private:
  enum class STag {
    None,
    Frame,
    IterPos,
    IterBase,
    Prop,
    ElemI,
    ElemS,
    Stack,
    MIState,
    Ref,

    IterBoth,  // A union of base and pos for the same iter.
  };
  struct UIterBoth { SSATmp* fp; uint32_t id; };

private:
  friend std::string show(AliasClass);
  friend AliasClass canonicalize(AliasClass);
  bool checkInvariants() const;
  bool equivData(AliasClass) const;
  bool subclassData(AliasClass) const;
  bool diffSTagSubclassData(rep relevant_bits, AliasClass) const;
  bool maybeData(AliasClass) const;
  bool diffSTagMaybeData(rep relevant_bits, AliasClass) const;
  folly::Optional<UIterBoth> asUIter() const;
  bool refersToSameIterHelper(AliasClass) const;
  static folly::Optional<AliasClass>
    precise_diffSTag_unionData(rep newBits, AliasClass, AliasClass);
  static AliasClass unionData(rep newBits, AliasClass, AliasClass);
  static rep stagBits(STag tag);

private:
  rep m_bits;
  STag m_stag{STag::None};
  union {
    AFrame    m_frame;
    AIterPos  m_iterPos;
    AIterBase m_iterBase;
    AProp     m_prop;
    AElemI    m_elemI;
    AElemS    m_elemS;
    AStack    m_stack;
    AMIState  m_mis;
    ARef      m_ref;

    UIterBoth m_iterBoth;
  };
};

//////////////////////////////////////////////////////////////////////

auto const AEmpty       = AliasClass{AliasClass::BEmpty};
auto const AFrameAny    = AliasClass{AliasClass::BFrame};
auto const AIterPosAny  = AliasClass{AliasClass::BIterPos};
auto const AIterBaseAny = AliasClass{AliasClass::BIterBase};
auto const APropAny     = AliasClass{AliasClass::BProp};
auto const AHeapAny     = AliasClass{AliasClass::BHeap};
auto const ARefAny      = AliasClass{AliasClass::BRef};
auto const AStackAny    = AliasClass{AliasClass::BStack};
auto const AElemIAny    = AliasClass{AliasClass::BElemI};
auto const AElemSAny    = AliasClass{AliasClass::BElemS};
auto const AElemAny     = AliasClass{AliasClass::BElem};
auto const AMIStateAny  = AliasClass{AliasClass::BMIState};
auto const AUnknownTV   = AliasClass{AliasClass::BUnknownTV};
auto const AUnknown     = AliasClass{AliasClass::BUnknown};

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
