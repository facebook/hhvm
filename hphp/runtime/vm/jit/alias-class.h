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

#include "hphp/runtime/base/rds.h"

#include "hphp/runtime/vm/minstr-state.h"

#include "hphp/runtime/vm/jit/alias-id-set.h"
#include "hphp/runtime/vm/jit/stack-offsets.h"

#include <folly/Optional.h>

#include <bitset>
#include <string>
#include <cstdint>

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
 *                    +-------+-------+
 *                    |               |
 *                 UnknownTV         Iter
 *                    |
 *                    |
 *                    |
 *      +---------+---+---------------+-------------------+----------+
 *      |         |                   |                   |          |
 *      |         |                   |                   |          |
 *      |         |                   |                   |       RdsAny
 *      |         |                   |                   |          |
 *      |         |                HeapAny*               |         ...
 *      |         |                   |                   |
 *      |         |            +------+------+            |
 *      |         |            |             |            |
 *   FrameAny  StackAny     ElemAny       PropAny     MIStateAny
 *      |         |          /    \          |            |
 *     ...       ...   ElemIAny  ElemSAny   ...           |
 *                        |         |                     |
 *                       ...       ...             +------+------+
 *                                                 |             |
 *                                             MITempBase     MIBase**
 *
 *
 *   (*) AHeapAny contains some things other than ElemAny, and PropAny
 *       that don't have explicit nodes in the lattice yet.  (Like the
 *       lvalBlackhole, etc.)  It's hard for this to matter to client code for
 *       now because we don't expose an intersection or difference operation.
 *
 *  (**) MIBase is a pointer, so it's not UnknownTV, but it's hard to find
 *       the right spot for it in this diagram.
 */
struct AliasClass;

//////////////////////////////////////////////////////////////////////

struct ALocal;
struct AIter;

namespace detail {
FPRelOffset frame_base_offset(SSATmp* fp);

static constexpr uint32_t kSlotsPerAIter = 4;
static constexpr uint32_t kAIterBaseOffset = 0;
static constexpr uint32_t kAIterTypeOffset = 1;
static constexpr uint32_t kAIterPosOffset = 2;
static constexpr uint32_t kAIterEndOffset = 3;

}

struct UFrameBase {
  UFrameBase(SSATmp* fp) : base{detail::frame_base_offset(fp)} {}
  UFrameBase(FPRelOffset off) : base{off} {}
  FPRelOffset base;
};

#define FRAME_RELATIVE(Name)                                                  \
  struct Name : UFrameBase {                                                  \
    Name(SSATmp* fp, AliasIdSet ids, FPInvOffset off = FPInvOffset{0})        \
      : UFrameBase{detail::frame_base_offset(fp) - off.offset}, ids{ids} {}   \
    Name(FPRelOffset off, AliasIdSet ids) : UFrameBase{off}, ids{ids} {}      \
    AliasIdSet ids;                                                           \
  }

#define FRAME_RELATIVE0(Name)                                                 \
  struct Name : UFrameBase {                                                  \
    using UFrameBase::UFrameBase;                                             \
    Name() = default;                                                         \
    /* implicit */ Name(const UFrameBase& base) : UFrameBase{base} {}         \
    Name& operator=(const UFrameBase& b) { base = b.base; return *this; }     \
  }

/*
 * Special data for locations known to be a set of locals on the frame `fp'.
 */
FRAME_RELATIVE(ALocal);

/*
 * Iterator state. We model the different fields of the iterator as 4
 * adjacent slots (with each iterator starting at a multiple of 4).
 */
FRAME_RELATIVE(AIter);

inline AIter aiter_base(SSATmp* fp, uint32_t id,
                        FPInvOffset off = FPInvOffset{0}) {
  using namespace detail;
  return AIter { fp, id * kSlotsPerAIter + kAIterBaseOffset, off };
}
inline AIter aiter_type(SSATmp* fp, uint32_t id,
                        FPInvOffset off = FPInvOffset{0}) {
  using namespace detail;
  return AIter { fp, id * kSlotsPerAIter + kAIterTypeOffset, off };
}
inline AIter aiter_pos(SSATmp* fp, uint32_t id,
                       FPInvOffset off = FPInvOffset{0}) {
  using namespace detail;
  return AIter { fp, id * kSlotsPerAIter + kAIterPosOffset, off };
}
inline AIter aiter_end(SSATmp* fp, uint32_t id,
                       FPInvOffset off = FPInvOffset{0}) {
  using namespace detail;
  return AIter { fp, id * kSlotsPerAIter + kAIterEndOffset, off };
}

inline AIter aiter_all(SSATmp* fp, uint32_t id,
                       FPInvOffset off = FPInvOffset{0}) {
  using namespace detail;
  return AIter {
    fp,
    AliasIdSet::IdRange(id * kSlotsPerAIter, (id + 1) * kSlotsPerAIter),
    off
  };
}

/*
 * These singleton values are each associated with a given frame. AFContext
 * is the m_thisUnsafe field of ActRec, AFFunc is m_func, AFMeta holds the
 * flags, hardware, and VM return fields (m_savedRip and m_callOffAndFlags).
 */
FRAME_RELATIVE0(AFContext);
FRAME_RELATIVE0(AFFunc);
FRAME_RELATIVE0(AFMeta);

/*
 * AActRec is a union of AFContext, AFFunc, and AFMeta
 */
FRAME_RELATIVE0(AActRec);

/*
 * A location inside of an object property, with base `obj' at physical index
 * `index' from the ObjectData*.
 */
struct AProp  { SSATmp* obj; uint16_t offset; };

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
 * addresses).  As an example, `acls = AStack { fp, FPRelOffset{-1}, 3 }`
 * represents the following:
 *
 *         ___________________
 *        | (i am an actrec)  |
 *  high  |___________________| ___...fp points here        __
 *    ^   |   local 0         |                               \
 *    |   |___________________| ___...start counting here: 1  |
 *    |   |   local 1         |                               | acls
 *    |   |___________________| ___...2                       |
 *    |   |   local 2         |                               |
 *   low  |___________________| ___...3; we're done         __/
 *        |   local 3         |
 *        |___________________|
 *
 * The frame pointer is the same for all stack ranges in the IR unit, and thus
 * is not stored here.  The reason ranges extend downward is that it is common
 * to need to refer to the class of all stack locations below some depth (this
 * can be done by putting INT32_MAX in the size).
 *
 * Some notes on how the evaluation stack is treated for alias analysis:
 *
 *   o Since AStack locations are always canonicalized, different AStack
 *     locations must not alias if there is no overlap in the ranges.
 *
 *   o In situations with inlined calls, we may in fact have ALocal locations
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
  // We can create an AStack from either a stack pointer or a frame pointer.
  // These constructors canonicalize the offset to be relative to the outermost
  // frame pointer.
  explicit AStack(SSATmp* fp, FPRelOffset offset, int32_t size);
  explicit AStack(SSATmp* sp, IRSPRelOffset offset, int32_t size);
  explicit AStack(FPRelOffset o, int32_t s) : offset(o), size(s) {}

  FPRelOffset offset;
  int32_t size;
};

/*
 * A TypedValue stored in rds.
 *
 * Assumes this handle uniquely identifies a TypedValue in rds - it's
 * not required that the tv is at the start of the rds storage.
 */
struct ARds { rds::Handle handle; };

#undef FRAME_RELATIVE
#undef FRAME_RELATIVE0

#define ALIAS_CLASS_SPEC          \
  O(None,     BEmpty)             \
  O(Local,    (BLocal | BActRec)) \
  O(Iter,     (BIter | BActRec))  \
  O(Prop,     BProp)              \
  O(ElemI,    BElemI)             \
  O(ElemS,    BElemS)             \
  O(Stack,    BStack)             \
  O(Rds,      BRds)               \
  O(FrameAll, BActRec)            \
/**/

//////////////////////////////////////////////////////////////////////

struct AliasClass {
  enum rep : uint32_t {  // bits for various location classes
    BEmpty    = 0,
    // The relative order of the values are used in operator| to decide
    // which specialization is more useful.
    BLocal      = 1U << 0,
    BIter       = 1U << 1,
    BProp       = 1U << 2,
    BElemI      = 1U << 3,
    BElemS      = 1U << 4,
    BStack      = 1U << 5,
    BRds        = 1U << 6,
    BFContext   = 1U << 7,
    BFFunc      = 1U << 8,
    BFMeta      = 1U << 9,

    // Have no specialization, put them last.
    BMITempBase = 1U << 10,
    BMIBase     = 1U << 11,
    BFBasePtr   = 1U << 12,

    BElem      = BElemI | BElemS,
    BHeap      = BElem | BProp,
    BMIState   = BMITempBase | BMIBase,

    BActRec = BFContext | BFFunc | BFMeta,

    BUnknownTV = ~(BIter | BMIBase | BActRec),

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
  /* implicit */ AliasClass(ALocal);
  /* implicit */ AliasClass(AIter);
  /* implicit */ AliasClass(AProp);
  /* implicit */ AliasClass(AElemI);
  /* implicit */ AliasClass(AElemS);
  /* implicit */ AliasClass(AStack);
  /* implicit */ AliasClass(ARds);
  /* implicit */ AliasClass(AFContext);
  /* implicit */ AliasClass(AFFunc);
  /* implicit */ AliasClass(AFMeta);
  /* implicit */ AliasClass(AActRec);

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
  AliasClass& operator|=(AliasClass o) { return *this = *this | o; }

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
   * Returns whether the alias class contains a single location.
   */
  bool isSingleLocation() const;

  /*
   * Conditionally access specific known information of various kinds.
   *
   * Returns folly::none if this alias class has no specialization in that way.
   */
  folly::Optional<ALocal>          local() const;
  folly::Optional<AIter>           iter() const;
  folly::Optional<AProp>           prop() const;
  folly::Optional<AElemI>          elemI() const;
  folly::Optional<AElemS>          elemS() const;
  folly::Optional<AStack>          stack() const;
  folly::Optional<ARds>            rds() const;
  folly::Optional<AFContext>       fcontext() const;
  folly::Optional<AFFunc>          ffunc() const;
  folly::Optional<AFMeta>          fmeta() const;
  folly::Optional<AActRec>         actrec() const;

  /*
   * Conditionally access specific known information, but also checking that
   * that is the only major category contained in this AliasClass.
   *
   * I.e., cls.is_foo() is semantically equivalent to:
   *
   *   cls <= AFooAny ? cls.foo() : folly::none
   */
  folly::Optional<ALocal>          is_local() const;
  folly::Optional<AIter>           is_iter() const;
  folly::Optional<AProp>           is_prop() const;
  folly::Optional<AElemI>          is_elemI() const;
  folly::Optional<AElemS>          is_elemS() const;
  folly::Optional<AStack>          is_stack() const;
  folly::Optional<ARds>            is_rds() const;
  folly::Optional<AFContext>       is_fcontext() const;
  folly::Optional<AFFunc>          is_ffunc() const;
  folly::Optional<AFMeta>          is_fmeta() const;
  folly::Optional<AActRec>         is_actrec() const;

  /*
   * Like the other foo() and is_foo() methods, but since we don't have an
   * AMIState anymore, these return AliasClass instead.
   */
  folly::Optional<AliasClass> mis() const;
  folly::Optional<AliasClass> is_mis() const;
  folly::Optional<AliasClass> frame_base() const;
  folly::Optional<AliasClass> is_frame_base() const;

private:
  enum class STag {
#define O(name, ...) name,
    ALIAS_CLASS_SPEC
#undef O
  };
private:
  friend std::string show(AliasClass);
  friend AliasClass canonicalize(AliasClass);

  void framelike_union_set(ALocal);
  void framelike_union_set(AIter);

  template <typename T>
  static AliasClass framelike_union(rep newBits, rep frm, T a, T b);

  bool checkInvariants() const;
  bool equivData(AliasClass) const;
  bool subclassData(AliasClass) const;
  bool maybeData(AliasClass) const;
  bool diffSTagSubclassData(rep relevant_bits, AliasClass) const;
  bool diffSTagMaybeData(rep relevant_bits, AliasClass) const;
  folly::Optional<UFrameBase> asUFrameBase() const;
  static AliasClass unionData(rep newBits, AliasClass, AliasClass);
  static STag stagFor(rep bits);

private:
  rep m_bits;
  rep m_stagBits{BEmpty};
  union {
    ALocal          m_local;
    AIter           m_iter;
    AProp           m_prop;
    AElemI          m_elemI;
    AElemS          m_elemS;
    AStack          m_stack;
    ARds            m_rds;

    UFrameBase      m_frameAll;
  };
};

//////////////////////////////////////////////////////////////////////

/* General alias classes. */
auto const AEmpty             = AliasClass{AliasClass::BEmpty};
auto const ALocalAny          = AliasClass{AliasClass::BLocal};
auto const AIterAny           = AliasClass{AliasClass::BIter};
auto const APropAny           = AliasClass{AliasClass::BProp};
auto const AHeapAny           = AliasClass{AliasClass::BHeap};
auto const AStackAny          = AliasClass{AliasClass::BStack};
auto const ARdsAny            = AliasClass{AliasClass::BRds};
auto const AElemIAny          = AliasClass{AliasClass::BElemI};
auto const AElemSAny          = AliasClass{AliasClass::BElemS};
auto const AElemAny           = AliasClass{AliasClass::BElem};
auto const AMIStateAny        = AliasClass{AliasClass::BMIState};
auto const AFContextAny       = AliasClass{AliasClass::BFContext};
auto const AFFuncAny          = AliasClass{AliasClass::BFFunc};
auto const AFMetaAny          = AliasClass{AliasClass::BFMeta};
auto const AActRecAny         = AliasClass{AliasClass::BActRec};
auto const AUnknownTV         = AliasClass{AliasClass::BUnknownTV};
auto const AUnknown           = AliasClass{AliasClass::BUnknown};

/* Alias classes for specific MInstrState fields. */
auto const AMIStateTempBase   = AliasClass{AliasClass::BMITempBase};
auto const AMIStateBase       = AliasClass{AliasClass::BMIBase};

/* Alias class for the frame base register */
auto const AFBasePtr          = AliasClass{AliasClass::BFBasePtr};

//////////////////////////////////////////////////////////////////////

/*
 * Creates an AliasClass given an offset into MInstrState.
 */
AliasClass mis_from_offset(size_t);

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

