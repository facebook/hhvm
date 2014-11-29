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

#ifndef incl_HPHP_JIT_TYPE_H_
#define incl_HPHP_JIT_TYPE_H_

#include "hphp/runtime/base/repo-auth-type.h"
#include "hphp/runtime/base/repo-auth-type-array.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/jit/types.h"

#include "hphp/util/data-block.h"

#include <folly/Optional.h>

#include <cstdint>
#include <cstring>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct Func;

namespace jit {
///////////////////////////////////////////////////////////////////////////////

struct DynLocation;

///////////////////////////////////////////////////////////////////////////////

namespace constToBits_detail {
  template<class T>
  struct needs_promotion
    : std::integral_constant<
        bool,
        std::is_integral<T>::value ||
          std::is_same<T,bool>::value ||
          std::is_enum<T>::value
      >
  {};

  template<class T>
  typename std::enable_if<needs_promotion<T>::value,uint64_t>::type
  promoteIfNeeded(T t) { return static_cast<uint64_t>(t); }

  template<class T>
  typename std::enable_if<!needs_promotion<T>::value,T>::type
  promoteIfNeeded(T t) { return t; }
}

///////////////////////////////////////////////////////////////////////////////

/*
 * The Ptr enum is a sub-lattice for the PtrToFoo types, giving types like
 * PtrToFrameInt, PtrToGblBool, etc.
 *
 * The values must be less than 32, for packing into Type below (we are using 5
 * bits for it right now).  We have a pointer "kind" for each of the major
 * segregated locations in which php values can live, and for most of them a
 * union of that location with "inside of a Ref".  These classify PtrTo* types
 * into some categories that cannot possibly alias, without any smarter
 * analysis needed to prove it.  There is also a union for the various
 * locations things can point after a fully generic member operation (see Mem
 * below).
 *
 * The reason we have the category "Ref|Foo" for each of these Foos is that it
 * is very common to need to do a generic unbox on some value if you have no
 * type information.  For example:
 *
 *     t1 = LdPropAddr ...
 *     t2 = UnboxPtr t1
 *
 * At this point, t2 is a pointer into either an object property or a inner
 * RefData, which will be a PtrToRPropCell, which means it still can't alias,
 * for example, a PtrToStkGen or a PtrToGblGen.  (Although it could generally
 * alias a PtrToRGblGen because both could be inside the same RefData.)
 */
enum class Ptr : uint8_t {
  Unk     = 0x00,

  Frame   = 0x01,
  Stk     = 0x02,
  Gbl     = 0x03,
  Prop    = 0x04,
  Arr     = 0x05,
  SProp   = 0x06,
  MIS     = 0x07,
  /*
   * Mem is a number of possible locations that result from the more generic
   * types of member operations.
   *
   * This is a pointer to something living either an object property, an array
   * element, a collection instance, the MinstrState, a object's dynamic
   * property array, the init_null_variant or null_variant, or the
   * lvalBlackHole.
   *
   * Fortunately they still can't alias the eval stack or frame locals, or
   * globals or sprops.  And unless it has the R bit it can't point to an
   * inner-RefData either.
   */
  Memb    = 0x08,
  /*
   * Pointer to class property initializer data.  These can never be refs, so
   * we don't have a RClsInit type.
   */
  ClsInit = 0x09,
  /*
   * Pointer to class constant values in RDS.
   */
  ClsCns  = 0x0a,

  RFrame  = 0x11,
  RStk    = 0x12,
  RGbl    = 0x13,
  RProp   = 0x14,
  RArr    = 0x15,
  RSProp  = 0x16,
  RMIS    = 0x17,
  RMemb   = 0x18,

  Ref     = 0x10,
};

///////////////////////////////////////////////////////////////////////////////

#define IRT_BOXES_AND_PTRS(name, bits)                        \
  IRT(name,              (bits))                              \
  IRT(Boxed##name,       (bits) << kBoxShift)                 \
  IRT(PtrTo##name,       (bits) << kPtrShift)                 \
  IRT(PtrToBoxed##name,  (bits) << kPtrBoxShift)              \
                                                              \
  IRTP(PtrToFrame##name,      Frame, (bits) << kPtrShift)     \
  IRTP(PtrToFrameBoxed##name, Frame, (bits) << kPtrBoxShift)  \
  IRTP(PtrToStk##name,          Stk, (bits) << kPtrShift)     \
  IRTP(PtrToStkBoxed##name,     Stk, (bits) << kPtrBoxShift)  \
  IRTP(PtrToGbl##name,          Gbl, (bits) << kPtrShift)     \
  IRTP(PtrToGblBoxed##name,     Gbl, (bits) << kPtrBoxShift)  \
  IRTP(PtrToProp##name,        Prop, (bits) << kPtrShift)     \
  IRTP(PtrToPropBoxed##name,   Prop, (bits) << kPtrBoxShift)  \
  IRTP(PtrToArr##name,          Arr, (bits) << kPtrShift)     \
  IRTP(PtrToArrBoxed##name,     Arr, (bits) << kPtrBoxShift)  \
  IRTP(PtrToSProp##name,      SProp, (bits) << kPtrShift)     \
  IRTP(PtrToSPropBoxed##name, SProp, (bits) << kPtrBoxShift)  \
  IRTP(PtrToMIS##name,          MIS, (bits) << kPtrShift)     \
  IRTP(PtrToMISBoxed##name,     MIS, (bits) << kPtrBoxShift)  \
  IRTP(PtrToMemb##name,        Memb, (bits) << kPtrShift)     \
  IRTP(PtrToMembBoxed##name,   Memb, (bits) << kPtrBoxShift)  \
  IRTP(PtrToClsInit##name,  ClsInit, (bits) << kPtrShift)     \
                                                              \
  IRTP(PtrToRFrame##name,    RFrame, (bits) << kPtrShift)     \
  IRTP(PtrToRStk##name,        RStk, (bits) << kPtrShift)     \
  IRTP(PtrToRGbl##name,        RGbl, (bits) << kPtrShift)     \
  IRTP(PtrToRProp##name,      RProp, (bits) << kPtrShift)     \
  IRTP(PtrToRArr##name,        RArr, (bits) << kPtrShift)     \
  IRTP(PtrToRSProp##name,    RSProp, (bits) << kPtrShift)     \
  IRTP(PtrToRMemb##name,      RMemb, (bits) << kPtrShift)     \
                                                              \
  IRTP(PtrToRef##name,          Ref, (bits) << kPtrShift)

#define IRT_WITH_ANY(name, bits)                          \
  IRT_BOXES_AND_PTRS(name, bits)                          \
  IRT(Any##name, k##name | kBoxed##name | kPtrTo##name |  \
                 kPtrToBoxed##name)

#define IRT_PHP(c)                                                      \
  c(Uninit,       1ULL << 0)                                            \
  c(InitNull,     1ULL << 1)                                            \
  c(Bool,         1ULL << 2)                                            \
  c(Int,          1ULL << 3)                                            \
  c(Dbl,          1ULL << 4)                                            \
  c(StaticStr,    1ULL << 5)                                            \
  c(CountedStr,   1ULL << 6)                                            \
  c(StaticArr,    1ULL << 7)                                            \
  c(CountedArr,   1ULL << 8)                                            \
  c(Obj,          1ULL << 9)                                            \
  c(Res,          1ULL << 10)
// Boxed*:        11-21
// PtrTo*:        22-32
// PtrToBoxed*:   33-43

/*
 * This list should be in non-decreasing order of specificity.
 */
#define IRT_PHP_UNIONS(c)                                               \
  c(Null,          kUninit|kInitNull)                                   \
  c(Str,           kStaticStr|kCountedStr)                              \
  c(Arr,           kStaticArr|kCountedArr)                              \
  c(NullableObj,   kObj|kInitNull|kUninit)                              \
  c(UncountedInit, kInitNull|kBool|kInt|kDbl|kStaticStr|kStaticArr)     \
  c(Uncounted,     kUncountedInit|kUninit)                              \
  c(InitCell,      kUncountedInit|kStr|kArr|kObj|kRes)                  \
  c(Cell,          kInitCell|kUninit)

#define IRT_RUNTIME                                                     \
  IRT(Cls,         1ULL << 44)                                          \
  IRT(Func,        1ULL << 45)                                          \
  IRT(VarEnv,      1ULL << 46)                                          \
  IRT(NamedEntity, 1ULL << 47)                                          \
  IRT(Cctx,        1ULL << 48) /* Class* with the lowest bit set,  */   \
                               /* as stored in ActRec.m_cls field  */   \
  IRT(RetAddr,     1ULL << 49) /* Return address */                     \
  IRT(StkPtr,      1ULL << 50) /* Stack pointer */                      \
  IRT(FramePtr,    1ULL << 51) /* Frame pointer */                      \
  IRT(TCA,         1ULL << 52)                                          \
  IRT(ActRec,      1ULL << 53)                                          \
  IRT(RDSHandle,   1ULL << 54) /* RDS::Handle */                        \
  IRT(Nullptr,     1ULL << 55)                                          \
  IRT(ABC,         1ULL << 56) /* AsioBlockableChain */
  /* bits 58-62 are pointer kind */

#define IRT_UNIONS                                                      \
  IRT(Ctx,         kObj|kCctx)                                          \

/*
 * Gen, Counted, PtrToGen, and PtrToCounted are here instead of IRT_PHP_UNIONS
 * because boxing them (e.g., BoxedGen, PtrToBoxedGen) would yield nonsense
 * types.
 */
#define IRT_SPECIAL                                               \
  IRT(Bottom,       0)                                            \
  IRT(Top,          0xffffffffffffffffULL)                        \
  IRT(Counted,      kCountedStr|kCountedArr|kObj|kRes|kBoxedCell) \
  IRT(PtrToCounted, kCounted << kPtrShift)                        \
  IRT(Gen,          kCell|kBoxedCell)                             \
  IRT(StackElem,    kGen|kCls)                                    \
  IRT(Init,         kGen & ~kUninit)                              \
  IRT(PtrToGen,     kGen << kPtrShift)                            \
  IRT(PtrToInit,    kInit << kPtrShift)                           \
                                                                  \
  IRTP(PtrToFrameGen,    Frame, kGen << kPtrShift)                \
  IRTP(PtrToFrameInit,   Frame, kInit << kPtrShift)               \
  IRTP(PtrToStkGen,        Stk, kGen << kPtrShift)                \
  IRTP(PtrToStkInit,       Stk, kInit << kPtrShift)               \
  IRTP(PtrToGblGen,        Gbl, kGen << kPtrShift)                \
  IRTP(PtrToGblInit,       Gbl, kInit << kPtrShift)               \
  IRTP(PtrToPropGen,      Prop, kGen << kPtrShift)                \
  IRTP(PtrToPropInit,     Prop, kInit << kPtrShift)               \
  IRTP(PtrToArrGen,        Arr, kGen << kPtrShift)                \
  IRTP(PtrToArrInit,       Arr, kInit << kPtrShift)               \
  IRTP(PtrToSPropGen,    SProp, kGen << kPtrShift)                \
  IRTP(PtrToSPropInit,   SProp, kInit << kPtrShift)               \
  IRTP(PtrToMISGen,        MIS, kGen << kPtrShift)                \
  IRTP(PtrToMISInit,       MIS, kInit << kPtrShift)               \
  IRTP(PtrToMemGen,       Memb, kGen << kPtrShift)                \
  IRTP(PtrToMemInit,      Memb, kInit << kPtrShift)               \
                                                                  \
  IRTP(PtrToRFrameGen,  RFrame, kGen << kPtrShift)                \
  IRTP(PtrToRFrameInit, RFrame, kInit << kPtrShift)               \
  IRTP(PtrToRStkGen,      RStk, kGen << kPtrShift)                \
  IRTP(PtrToRStkInit,     RStk, kInit << kPtrShift)               \
  IRTP(PtrToRGblGen,      RGbl, kGen << kPtrShift)                \
  IRTP(PtrToRGblInit,     RGbl, kInit << kPtrShift)               \
  IRTP(PtrToRPropGen,    RProp, kGen << kPtrShift)                \
  IRTP(PtrToRPropInit,   RProp, kInit << kPtrShift)               \
  IRTP(PtrToRArrGen,      RArr, kGen << kPtrShift)                \
  IRTP(PtrToRArrInit,     RArr, kInit << kPtrShift)               \
  IRTP(PtrToRSPropGen,  RSProp, kGen << kPtrShift)                \
  IRTP(PtrToRSPropInit, RSProp, kInit << kPtrShift)               \
  IRTP(PtrToRMISGen,      RMIS, kGen << kPtrShift)                \
  IRTP(PtrToRMISInit,     RMIS, kInit << kPtrShift)               \
  IRTP(PtrToRMembGen,     RMemb, kGen << kPtrShift)               \
  IRTP(PtrToRMembInit,    RMemb, kInit << kPtrShift)              \
                                                                  \
  IRTP(PtrToRefGen,        Ref, kGen << kPtrShift)                \
  IRTP(PtrToRefInit,       Ref, kInit << kPtrShift)

/*
 * All types with just a single bit set.
 */
#define IRT_PRIMITIVE IRT_PHP(IRT_BOXES_AND_PTRS) IRT_RUNTIME

/*
 * All types.
 */
#define IR_TYPES                                \
  IRT_PHP(IRT_WITH_ANY)                         \
  IRT_PHP_UNIONS(IRT_WITH_ANY)                  \
  IRT_RUNTIME                                   \
  IRT_UNIONS                                    \
  IRT_SPECIAL

///////////////////////////////////////////////////////////////////////////////

struct ConstCctx {
  static ConstCctx cctx(const Class* c) {
    return ConstCctx { reinterpret_cast<uintptr_t>(c) | 0x1 };
  }

  const Class* cls() const {
    return reinterpret_cast<const Class*>(m_val & ~0x1);
  }

  uintptr_t m_val;
};

/*
 * Type is used to represent the types of values in the jit. Every Type
 * represents a set of types, with Type::Top being a superset of all Types and
 * Type::Bottom being a subset of all Types. The elements forming these sets of
 * types come from the types of PHP-visible values (Str, Obj, Int, ...) and
 * runtime-internal types (Func, TCA, ActRec, ...).
 *
 * Types can be constructed from the predefined constants or by composing
 * existing Types in various ways. Unions, intersections, and subtractions are
 * all supported, though for implementation-specific reasons certain
 * combinations of specialized types cannot be represented. A type is
 * considered specialized if it refers to a specific Class or a
 * ArrayData::ArrayKind. As an example, if A and B are unrelated Classes,
 * Obj<A> | Obj<B> is impossible to represent. However, if B is a subclass of
 * A, Obj<A> | Obj<B> == Obj<B>, which can be represented as a Type.
 */
struct Type {
  /*
   * Predefined constants for all primitive types and many common unions.
   */
#define IRT(name, ...) static const Type name;
#define IRTP(name, ...) IRT(name)
  IR_TYPES
#undef IRT
#undef IRTP


  /////////////////////////////////////////////////////////////////////////////
  // Type tags and metadata.

private:
  using bits_t = uint64_t;

  static constexpr size_t kBoxShift     = 11;
  static constexpr size_t kPtrShift     = kBoxShift + kBoxShift;
  static constexpr size_t kPtrBoxShift  = kBoxShift + kPtrShift;

  enum TypedBits {
#define IRT(name, bits)       k##name = (bits),
#define IRTP(name, ptr, bits) k##name = (bits),
    IR_TYPES
#undef IRT
#undef IRTP
  };

  /*
   * An ArrayKind in the top 16 bits, optional RepoAuthType::Array* in
   * the lower 48 bits, and the low bit that says whether the kind is
   * valid.
   */
  enum class ArrayInfo : uintptr_t {};

  /*
   * Tag that tells us if we're exactly equal to, or a subtype of, a Class*.
   */
  enum class ClassTag : uint8_t { Sub, Exact };

  /*
   * A const Class* with the low bit set if this is an exact type, otherwise a
   * subtype.
   */
  struct ClassInfo {
    ClassInfo(const Class* cls, ClassTag tag);

    const Class* get() const;
    bool isExact() const;
    bool operator==(const ClassInfo& rhs) const;

  private:
    uintptr_t m_bits;
  };


  /////////////////////////////////////////////////////////////////////////////
  // Basic methods.

public:
  /*
   * Default bottom constructor.
   */
  Type();

  /*
   * Regular constructors.
   */
  explicit Type(DataType outer, DataType inner = KindOfUninit);
  explicit Type(DataType outer, KindOfAny);

  /*
   * Assignment.
   */
  Type& operator=(Type b);

  /*
   * Hash the Type as a bitfield.
   */
  size_t hash() const;

  /*
   * Stringify the Type.
   *
   * constValString: @requires: isConst()
   */
  std::string toString() const;
  std::string constValString() const;
  static std::string debugString(Type t);


  /////////////////////////////////////////////////////////////////////////////
  // Comparisons.                                                       [const]

  /*
   * Does this represent a non-strict subset of `t2'?
   */
  bool subtypeOf(Type t2) const;

  /*
   * Does this represent a non-strict subset of specialized Type `t2'?
   *
   * @requires: (m_bits & t2.m_bits) == m_bits
   *            !t2.m_hasConstVal
   *            t2.isSpecialized()
   */
  bool subtypeOfSpecialized(Type t2) const;

  /*
   * Is this a subtype of any among a variadic list of Types?
   */
  template<typename... Types>
  bool subtypeOfAny(Type t2, Types... ts) const;
  bool subtypeOfAny() const;

  /*
   * Is this is a strict subset of `t2'?
   */
  bool strictSubtypeOf(Type t2) const;

  /*
   * Return true if any subtype of this is a subtype of t2, i.e., if the
   * intersection of the two is nontrivial.
   */
  bool maybe(Type t2) const;

  /*
   * Return true if no subtypes of this are subtypes of t2, i.e., if the
   * intersection of the two is trivial.
   *
   * @returns: !maybe(t2)
   */
  bool not(Type t2) const;

  /*
   * Return true if this is exactly equal to t2.
   *
   * Be careful: you probably mean subtypeOf().
   */
  bool equals(Type t2) const;

  bool operator==(Type t2) const;
  bool operator!=(Type t2) const;


  /////////////////////////////////////////////////////////////////////////////
  // Combinators.

  /*
   * Set operations: union, intersection, and difference.
   *
   * These operations may all return larger types than the "true" union,
   * intersection, or difference. (They must be conservative in that direction
   * for types that are too hard for us to represent, or we could generate
   * incorrect code by assuming certain possible values are impossible.)
   *
   * Note: operator| and operator& guarantee commutativity.
   */
  Type operator|(Type other) const;
  Type& operator|=(Type other) { return *this = *this | other; }

  Type operator&(Type other) const;
  Type& operator&=(Type other) { return *this = *this & other; }

  Type operator-(Type other) const;
  Type& operator-=(Type other) { return *this = *this - other; }

  /*
   * Return the least common predefined supertype of `t1' and `t2', i.e., the
   * most refined Type `t3' such that t1 <= t3 and t2 <= t3.
   *
   * Note that arbitrary union types are possible using operator| but this
   * function always returns one of the predefined types.
   */
  static Type unionOf(Type t1, Type t2);


  /////////////////////////////////////////////////////////////////////////////
  // Is-a methods.                                                      [const]

  /*
   * Is this a PHP null type, or a null pointer?
   */
  bool isZeroValType() const;

  /*
   * Is this (maybe) a boxed PHP type?
   */
  bool isBoxed() const;
  bool maybeBoxed() const;

  /*
   * Is this an unboxed PHP type?
   *
   * Note that this is different from !isBoxed().
   *
   * @requires: subtypeOf(Type::Gen)
   */
  bool notBoxed() const;

  /*
   * Is this (not) a pointer to a PHP type?
   */
  bool isPtr() const;
  bool notPtr() const;

  /*
   * Is this (maybe/not) a refcounted PHP type?
   */
  bool isCounted() const;
  bool maybeCounted() const;
  bool notCounted() const;

  /*
   * Is this a union type?
   *
   * Note that this is the plain old set definition of union, so Type::Str,
   * Type::Arr, and Type::Null will all return true.
   */
  bool isUnion() const;

  /*
   * Return true iff there exists a DataType in the range [KindOfUninit,
   * KindOfRef] that represents a non-strict supertype of this type.
   *
   * @requires: subtypeOf(StackElem)
   */
  bool isKnownDataType() const;

  /*
   * @returns: isKnownDataType() && notBoxed()
   */
  bool isKnownUnboxedDataType() const;

  /*
   * Does this require a register to hold a DataType or value at runtime?
   */
  bool needsReg() const;
  bool needsValueReg() const;

  /*
   * Might this be a type that has a static variant (i.e., StaticStr,
   * StaticArr)?
   */
  bool needsStaticBitCheck() const;

  /*
   * Might this be a type which can run destructors (i.e., an (optionally
   * boxed) array, object, or resource)?
   */
  bool canRunDtor() const;

  /*
   * Return true if this corresponds to a type that is passed by (value/
   * reference) in C++.
   */
  bool isSimpleType() const;
  bool isReferenceType() const;


  /////////////////////////////////////////////////////////////////////////////
  // Constants types.                                            [const/static]

private:
  /*
   * Return the Type to use for a given C++ value.
   *
   * The only interesting case is int/bool disambiguation.  Enums are treated
   * as ints.
   */
  template<class T>
  static typename std::enable_if<
    std::is_integral<T>::value || std::is_enum<T>::value,
    Type
  >::type forConst(T) {
    return std::is_same<T,bool>::value ? Type::Bool : Type::Int;
  }
  static Type forConst(double)            { return Dbl; }
  static Type forConst(const StringData* sd);
  static Type forConst(const ArrayData* ad);
  static Type forConst(const HPHP::Func*) { return Func; }
  static Type forConst(const Class*)      { return Cls; }
  static Type forConst(ConstCctx)         { return Cctx; }
  static Type forConst(jit::TCA)          { return TCA; }

public:
  /*
   * Does this Type represent a known value?
   */
  bool isConst() const;

  /*
   * Does this Type represent a known value subtyping `t'?
   *
   * @returns: isConst() && subtypeOf(t)
   */
  bool isConst(Type t) const;

  /*
   * Does this Type represent the constant val `val'?
   *
   * @returns: subtypeOf(cns(val))
   */
  template<typename T>
  bool isConst(T val) const;

  /*
   * Return the const value for a const Type as a uint64_t.
   *
   * @requires: isConst()
   */
  uint64_t rawVal() const;

  /*
   * Return the const value for a const Type.
   *
   * @requires: subtypeOf(Type::T) && m_hasConstVal
   */
  bool boolVal() const;
  int64_t intVal() const;
  double dblVal() const;
  const StringData* strVal() const;
  const ArrayData* arrVal() const;
  const HPHP::Func* funcVal() const;
  const Class* clsVal() const;
  ConstCctx cctxVal() const;
  RDS::Handle rdsHandleVal() const;
  jit::TCA tcaVal() const;

  /*
   * If this represents a constant value, return the most specific strict
   * supertype of this we can represent, else return *this.
   *
   * In most cases this just erases the constant value:
   *    Int<4> -> Int
   *    Dbl<2.5> -> Dbl
   *
   * Arrays are special since they can be both constant and specialized, so
   * keep the array's kind in the resulting type.
   */
  Type dropConstVal() const;

  /*
   * Return a const copy of `ret' with constant value `val'.
   */
  template<typename T>
  static Type cns(T val, Type ret);

  /*
   * Return a const type corresponding to `val'.
   *
   * @returns: cns(val, forConst(val))
   */
  template<typename T>
  static Type cns(T val);

  /*
   * @returns: Type::Nullptr
   */
  static Type cns(std::nullptr_t);

  /*
   * Return a const type for `tv'.
   */
  static Type cns(const TypedValue& tv);


  /////////////////////////////////////////////////////////////////////////////
  // Specialized types.                                                 [const]

  /*
   * Can this Type have a specialization for a class, array, or either?
   */
  bool canSpecializeClass() const;
  bool canSpecializeArray() const;
  bool canSpecializeAny() const;

  /*
   * Return a copy of this Type specialized with `klass'.
   *
   * Pre: canSpecializeClass() && getClass() == nullptr
   *      `klass' != nullptr
   */
  Type specialize(const Class* klass) const;
  Type specializeExact(const Class* klass) const;

  /*
   * Return a copy of this Type specialized with array information.
   *
   * @requires: canSpecializeArray()
   */
  Type specialize(ArrayData::ArrayKind arrayKind) const;
  Type specialize(const RepoAuthType::Array* array) const;

  /*
   * Return a copy of this Type with the specialization dropped.
   *
   * @returns: Type(m_bits)
   */
  Type unspecialize() const;

  /*
   * Does this Type have a specialization?
   */
  bool isSpecialized() const;

  /*
   * Return the Type's Class specialization.
   *
   * @requires: canSpecializeClass()
   */
  const Class* getClass() const;

  /*
   * Return the Type's exact Class specialization, or nullptr if the
   * specialization is non-exact.
   *
   * @requirest: canSpecializeClass() || subtypeOf(Type::Cls)
   */
  const Class* getExactClass() const;

  /*
   * Whether the Type has an array kind specialization.
   */
  bool hasArrayKind() const;

  /*
   * Return the Type's array kind specialization.
   *
   * getArrayKind: @requires: hasArrayKind()
   * getOptArrayKind: Return folly::none if !hasArrayKind()
   */
  ArrayData::ArrayKind getArrayKind() const;
  folly::Optional<ArrayData::ArrayKind> getOptArrayKind() const;

  /*
   * Return the Type's array type specialization.
   */
  const RepoAuthType::Array* getArrayType() const;

  /*
   * Project the Type onto those types which can be specialized, e.g.:
   *
   *  {Int|Str|Obj<C>|BoxedObj<C>}.specializedType() == {Obj<C>|BoxedObj<C>}
   */
  Type specializedType() const;


  /////////////////////////////////////////////////////////////////////////////
  // Inner types.                                                       [const]

  /*
   * Box or unbox a Type.
   *
   * @requires:
   *    box:    subtypeOf(Cell)
   *            not(Uninit) || equals(Cell)
   *    unbox:  subtypeOf(Gen)
   */
  Type box() const;
  Type unbox() const;

  /*
   * Get the inner Type of a boxed Type.
   *
   * @requires: isBoxed()
   */
  Type innerType() const;

  /*
   * Get a pointer to, or dereference, a Type.
   *
   * @requires:
   *    ptr:        !isPtr() && subtypeOf(Gen)
   *    deref:      isPtr()
   *    derefIfPtr: subtypeOf(Gen | PtrToGen)
   */
  Type ptr(Ptr kind) const;
  Type deref() const;
  Type derefIfPtr() const;

  /*
   * Return a Type stripped of boxing and pointerness.
   */
  Type strip() const;

  /*
   * Return the pointer category of a possibly-pointer type.
   *
   * Pre: maybe(Type::PtrToGen)
   */
  Ptr ptrKind() const;

  /////////////////////////////////////////////////////////////////////////////
  // Other methods.                                                     [const]

  /*
   * Return the most specific DataType that is a supertype of this Type.
   *
   * @requires: isKnownDataType()
   */
  DataType toDataType() const;

  /*
   * Relax the Type to one that we can check in codegen.
   */
  Type relaxToGuardable() const;

  /////////////////////////////////////////////////////////////////////////////

private:
  struct Union;
  struct Intersect;
  struct ArrayOps;
  struct ClassOps;

private:
  /*
   * Raw constructors.
   */
  explicit Type(bits_t bits, Ptr kind, uintptr_t extra = 0);
  explicit Type(bits_t bits, Ptr kind, ClassInfo classInfo);
  explicit Type(bits_t bits, Ptr kind, ArrayInfo arrayInfo);

  explicit Type(bits_t bits, ArrayData::ArrayKind) = delete;

  /*
   * Return false if a specialized type has a mismatching tag, else true.
   */
  bool checkValid() const;

  /*
   * Bit-pack an `outer' and an `inner' DataType for a Type.
   */
  static bits_t bitsFromDataType(DataType outer, DataType inner);

  /*
   * Methods and types used for operator| and operator&.  See type.cpp for
   * details.
   */
  template<typename Oper>
  static Type combine(bits_t newBits, Ptr newPtrKind, Type a, Type b);

  Ptr rawPtrKind() const;

  /////////////////////////////////////////////////////////////////////////////
  // ArrayInfo helpers.                                                [static]

private:
  /*
   * Pack an ArrayInfo from a kind and a RAT.
   */
  static ArrayInfo makeArrayInfo(folly::Optional<ArrayData::ArrayKind> kind,
                                 const RepoAuthType::Array* arrTy);

  /*
   * ArrayInfo accessors for the valid bit, kind, and RAT.
   *
   * arrayKind: @requires: arrayKindValid(info)
   * arrayType: May return nullptr.
   */
  static bool arrayKindValid(ArrayInfo info);
  static ArrayData::ArrayKind arrayKind(ArrayInfo info);
  static const RepoAuthType::Array* arrayType(ArrayInfo info);


  /////////////////////////////////////////////////////////////////////////////
  // Data members.

private:
  union {
    struct {
      bits_t m_bits : 58;
      bits_t m_ptrKind : 5;
      bool m_hasConstVal : 1;
    };
    uint64_t m_rawInt;
  };

  union {
    uintptr_t m_extra;

    // Constant values. Validity determined by m_hasConstVal and m_bits.
    bool m_boolVal;
    int64_t m_intVal;
    double m_dblVal;
    const StringData* m_strVal;
    const ArrayData* m_arrVal;
    const HPHP::Func* m_funcVal;
    const Class* m_clsVal;
    ConstCctx m_cctxVal;
    jit::TCA m_tcaVal;
    RDS::Handle m_rdsHandleVal;
    TypedValue* m_ptrVal;

    // Specialization for object classes and arrays.
    ClassInfo m_class;
    ArrayInfo m_arrayInfo;
  };
};

typedef folly::Optional<Type> OptType;

inline bool operator<(Type a, Type b) { return a.strictSubtypeOf(b); }
inline bool operator>(Type a, Type b) { return b.strictSubtypeOf(a); }
inline bool operator<=(Type a, Type b) { return a.subtypeOf(b); }
inline bool operator>=(Type a, Type b) { return b.subtypeOf(a); }

/*
 * jit::Type must be small enough for efficient pass-by-value.
 */
static_assert(sizeof(Type) <= 2 * sizeof(uint64_t),
              "jit::Type should fit in (2 * sizeof(uint64_t))");


///////////////////////////////////////////////////////////////////////////////

/*
 * Return the most refined type that can be used to represent the type in a
 * live TypedValue.
 */
Type liveTVType(const TypedValue* tv);

/*
 * Return the boxed version of the input type, taking into account PHP
 * semantics and subtle implementation details.
 */
Type boxType(Type);

/*
 * Create a Type from a RepoAuthType.
 */
Type convertToType(RepoAuthType ty);

/*
 * Return the type resulting from refining oldType with the fact that it also
 * belongs to newType.
 *
 * This essentially intersects the two types, except that it has special logic
 * for boxed types.  This function always_asserts that the resulting type isn't
 * Bottom.
 */
Type refineType(Type oldType, Type newType);

/*
 * Similar to refineType above, but this one doesn't get angry if the resulting
 * type is Bottom.
 */
Type refineTypeNoCheck(Type oldType, Type newType);

/*
 * Return the dest type for a LdRef with the given typeParam.
 *
 * @requires: typeParam.notBoxed()
 */
Type ldRefReturn(Type typeParam);

///////////////////////////////////////////////////////////////////////////////

/*
 * Type information used by guard relaxation code to track the properties of a
 * type that consumers care about.
 */
struct TypeConstraint {

  /*
   * Constructors.
   */
  /* implicit */ TypeConstraint(DataTypeCategory cat = DataTypeGeneric);
  explicit TypeConstraint(const Class* cls);

  /*
   * Stringify the TypeConstraint.
   */
  std::string toString() const;


  /////////////////////////////////////////////////////////////////////////////
  // Basic info.

  /*
   * Mark the TypeConstraint as weak; see documentation for `weak'.
   */
  TypeConstraint& setWeak(bool w = true);

  /*
   * Is this a trivial constraint?
   */
  bool empty() const;

  /*
   * Comparison.
   */
  bool operator==(TypeConstraint tc2) const;
  bool operator!=(TypeConstraint tc2) const;


  /////////////////////////////////////////////////////////////////////////////
  // Specialization.

  static constexpr uint8_t kWantArrayKind = 0x1;

  /*
   * Is this TypeConstraint for a specialized type?
   */
  bool isSpecialized() const;

  /*
   * Set or check the kWantArrayKind bit in `m_specialized'.
   *
   * @requires: isSpecialized()
   */
  TypeConstraint& setWantArrayKind();
  bool wantArrayKind() const;

  /*
   * Set, check, or return the specialized Class.
   *
   * @requires:
   *    setDesiredClass: isSpecialized()
   *                     desiredClass() is either nullptr, a parent of `cls',
   *                     or a child of `cls'
   *    desiredClass:    wantClass()
   */
  TypeConstraint& setDesiredClass(const Class* cls);
  bool wantClass() const;
  const Class* desiredClass() const;


  /////////////////////////////////////////////////////////////////////////////
  // Data members.

  /*
   * `category' starts as DataTypeGeneric and is refined to more specific
   * values by consumers of the type.
   */
  DataTypeCategory category;

  /*
   * If weak is true, the consumer of the value being constrained doesn't
   * actually want to constrain the guard (if found).
   *
   * Most often used to figure out if a type can be used without further
   * constraining guards.
   */
  bool weak;

private:
  /*
   * `m_specialized' either holds a Class* or a 1 in its low bit, indicating
   * that for a DataTypeSpecialized constraint, we require the specified class
   * or an array kind, respectively.
   */
  uintptr_t m_specialized;
};

///////////////////////////////////////////////////////////////////////////////

const int kTypeWordOffset = offsetof(TypedValue, m_type) % 8;
const int kTypeShiftBits = kTypeWordOffset * CHAR_BIT;

// left shift an immediate DataType, for type, to the correct position
// within one of the registers used to pass a TypedValue by value.
inline uint64_t toDataTypeForCall(Type type) {
  return uint64_t(type.toDataType()) << kTypeShiftBits;
}

///////////////////////////////////////////////////////////////////////////////
}}

#define incl_HPHP_JIT_TYPE_INL_H_
#include "hphp/runtime/vm/jit/type-inl.h"
#undef incl_HPHP_JIT_TYPE_INL_H_

#endif
