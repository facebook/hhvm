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

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/repo-auth-type.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/type-specialization.h"

#include <folly/Optional.h>

#include <cstdint>
#include <type_traits>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct ArrayData;
struct Class;
struct Func;
struct Shape;
struct StringData;
struct TypedValue;

namespace jit {
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
 * locations things can point after a fully generic member operation (see Memb
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
   * Memb is a number of possible locations that result from the more generic
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

constexpr auto kPtrRefBit = static_cast<uint32_t>(Ptr::Ref);

Ptr add_ref(Ptr);
Ptr remove_ref(Ptr);

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
  c(Static,        kStaticStr|kStaticArr)                               \
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
  IRT(ABC,         1ULL << 53) /* AsioBlockableChain */                 \
  IRT(RDSHandle,   1ULL << 54) /* rds::Handle */                        \
  IRT(Nullptr,     1ULL << 55)                                          \
  /* bits 58-62 are pointer kind */

#define IRT_UNIONS                                                      \
  IRT(Ctx,         kObj|kCctx|kNullptr)

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
  IRT(StkElem,      kGen|kCls)                                    \
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
  IRTP(PtrToMembGen,      Memb, kGen << kPtrShift)                \
  IRTP(PtrToMembInit,     Memb, kInit << kPtrShift)               \
  IRTP(PtrToClsInitGen,ClsInit, kGen << kPtrShift)                \
  IRTP(PtrToClsCnsGen,  ClsCns, kGen << kPtrShift)                \
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

///////////////////////////////////////////////////////////////////////////////

/*
 * Type is used to represent the types of values in the jit.  Every Type
 * represents a set of types, with TTop being a superset of all Types and
 * TBottom being a subset of all Types.  The elements forming these sets
 * of types come from the types of PHP-visible values (Str, Obj, Int, ...) and
 * runtime-internal types (Func, TCA, ...).
 *
 * Types can be constructed from the predefined constants or by composing
 * existing Types in various ways.  Unions, intersections, and subtractions are
 * all supported, though for implementation-specific reasons certain
 * combinations of specialized types cannot be represented.  A type is
 * considered specialized if it refers to a specific Class or a
 * ArrayData::ArrayKind.  As an example, if A and B are unrelated Classes,
 * Obj<A> | Obj<B> is impossible to represent.  However, if B is a subclass of
 * A, Obj<A> | Obj<B> == Obj<B>, which can be represented as a Type.
 */
struct Type {
  /////////////////////////////////////////////////////////////////////////////
  // Type tags.

private:
  using bits_t = uint64_t;

  static constexpr size_t kBoxShift     = 11;
  static constexpr size_t kPtrShift     = kBoxShift + kBoxShift;
  static constexpr size_t kPtrBoxShift  = kBoxShift + kPtrShift;

public:
  enum TypedBits {
#define IRT(name, bits)       k##name = (bits),
#define IRTP(name, ptr, bits) k##name = (bits),
    IR_TYPES
#undef IRT
#undef IRTP
  };


  /////////////////////////////////////////////////////////////////////////////
  // Basic methods.

public:
  /*
   * Default bottom constructor.
   */
  Type();

  /*
   * Construct from a predefined set of bits & pointer kind.
   */
  constexpr Type(bits_t bits, Ptr kind);

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
   * constValString: @requires: hasConstVal() ||
   *                            subtypeOfAny(Uninit, InitNull, Nullptr)
   */
  std::string toString() const;
  std::string constValString() const;
  static std::string debugString(Type t);


  /////////////////////////////////////////////////////////////////////////////
  // DataType.

  /*
   * Construct from a DataType.
   */
  explicit Type(DataType outer, DataType inner = KindOfUninit);
  explicit Type(DataType outer, KindOfAny);

  /*
   * Return true iff there exists a DataType in the range [KindOfUninit,
   * KindOfRef] that represents a non-strict supertype of this type.
   *
   * @requires: *this <= StkElem
   */
  bool isKnownDataType() const;

  /*
   * Return the most specific DataType that is a supertype of this Type.
   *
   * @requires: isKnownDataType()
   */
  DataType toDataType() const;


  /////////////////////////////////////////////////////////////////////////////
  // Comparisons.                                                       [const]

  /*
   * Return true if this is exactly equal to `rhs'.
   *
   * Be careful---you probably mean `<='.
   */
  bool operator==(Type rhs) const;
  bool operator!=(Type rhs) const;

  /*
   * Does this represent a subset (or superset) of `t2'?
   *
   * All operators are implemented in terms of operator==() and operator<=().
   */
  bool operator<=(Type rhs) const;
  bool operator>=(Type rhs) const;
  bool operator<(Type rhs) const;
  bool operator>(Type rhs) const;

  /*
   * Is this a non-strict subtype of any among a variadic list of Types?
   */
  template<typename... Types>
  bool subtypeOfAny(Type t2, Types... ts) const;
  bool subtypeOfAny() const;

  /*
   * Return true if this has nontrivial intersection with `t2'.
   */
  bool maybe(Type t2) const;


  /////////////////////////////////////////////////////////////////////////////
  // Combinators.

  /*
   * Set operations: union, intersection, and difference.
   *
   * These operations may all return larger types than the "true" union,
   * intersection, or difference.  (They must be conservative in that direction
   * for types that are too hard for us to represent, or we could generate
   * incorrect code by assuming certain possible values are impossible.)
   *
   * Note: operator| and operator& guarantee commutativity; operator- guarantees
   * (a - b) <= a.
   */
  Type operator|(Type other) const;
  Type& operator|=(Type other) { return *this = *this | other; }

  Type operator&(Type other) const;
  Type& operator&=(Type other) { return *this = *this & other; }

  Type operator-(Type other) const;
  Type& operator-=(Type other) { return *this = *this - other; }


  /////////////////////////////////////////////////////////////////////////////
  // Is-a methods.                                                      [const]

  /*
   * Is this a union type?
   *
   * Note that this is the plain old set definition of union, so TStr,
   * TArr, and TNull will all return true.
   */
  bool isUnion() const;

  /*
   * Does this require a register to hold a DataType at runtime?
   */
  bool needsReg() const;

  /*
   * Return true if this corresponds to a type that is passed by (value/
   * reference) in C++.
   */
  bool isSimpleType() const;
  bool isReferenceType() const;


  /////////////////////////////////////////////////////////////////////////////
  // Constant type creation.                                     [const/static]

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
   * @returns: TNullptr
   */
  static Type cns(std::nullptr_t);

  /*
   * Return a const type for `tv'.
   */
  static Type cns(const TypedValue& tv);

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


  /////////////////////////////////////////////////////////////////////////////
  // Constant introspection.                                            [const]

  /*
   * Does this Type have a constant value? If true, we can call xxVal().
   *
   * Note: Bottom is a type with no value, and Uninit/InitNull/Nullptr are
   * considered types with a single unique value, so this function returns false
   * for those types. You may want to explicitly check for them as needed.
   *
   */
  bool hasConstVal() const;

  /*
   * @return hasConstVal() && *this <= t.
   */
  bool hasConstVal(Type t) const;

  /*
   * Does this Type represent the constant val `val'?
   *
   * @returns: hasConstVal(cns(val))
   */
  template<typename T>
  bool hasConstVal(T val) const;

  /*
   * Return the const value for a const Type as a uint64_t.
   *
   * @requires: hasConstVal()
   */
  uint64_t rawVal() const;

  /*
   * Return the const value for a const Type.
   *
   * @requires: hasConstVal(Type::T)
   */
  bool boolVal() const;
  int64_t intVal() const;
  double dblVal() const;
  const StringData* strVal() const;
  const ArrayData* arrVal() const;
  const HPHP::Func* funcVal() const;
  const Class* clsVal() const;
  ConstCctx cctxVal() const;
  rds::Handle rdsHandleVal() const;
  jit::TCA tcaVal() const;


  /////////////////////////////////////////////////////////////////////////////
  // Specialized type creation.                                  [const/static]

  /*
   * Return a specialized TArr.
   */
  static Type Array(ArrayData::ArrayKind kind);
  static Type Array(const RepoAuthType::Array* rat);
  static Type Array(const Shape* shape);

  /*
   * Return a specialized TStaticArr.
   */
  static Type StaticArray(ArrayData::ArrayKind kind);
  static Type StaticArray(const RepoAuthType::Array* rat);
  static Type StaticArray(const Shape* shape);

  /*
   * Return a specialized TObj.
   */
  static Type SubObj(const Class* cls);
  static Type ExactObj(const Class* cls);

  /*
   * Return a copy of this Type with the specialization dropped.
   *
   * @returns: Type(m_bits)
   */
  Type unspecialize() const;


  /////////////////////////////////////////////////////////////////////////////
  // Specialization introspection.                                      [const]

  /*
   * Does this Type have a specialization?
   */
  bool isSpecialized() const;

  /*
   * Whether this type can meaningfully specialize along `kind'.
   *
   * For example, a Type only supports SpecKind::Class if its bits intersect
   * nontrivially with kAnyObj.
   */
  bool supports(SpecKind kind) const;

  /*
   * Return the corresponding type specialization.
   *
   * If the Type is able to support the specialization (i.e., supports()
   * returns true for the corresponding SpecKind), but no specialization is
   * present, Spec::Top will be returned.
   *
   * If supports() would return false for the corresponding SpecKind,
   * Spec::Bottom is returned.
   *
   * The Spec objects cast (explicitly) to true iff they are neither Spec::Top
   * nor Spec::Bottom, so these functions also answer the question, "Is this
   * Type nontrivially specialized along the respective kind?"
   */
  ArraySpec arrSpec() const;
  ClassSpec clsSpec() const;

  /*
   * Return a discriminated TypeSpec for this Type's specialization.
   */
  TypeSpec spec() const;


  /////////////////////////////////////////////////////////////////////////////
  // Inner types.                                                       [const]

  /*
   * Box or unbox a Type.
   *
   * The box() and inner() methods are inverses---they (respectively) take the
   * the {Cell, BoxedCell} bits of the Type and coerce them into the
   * {BoxedCell, Cell} sides of the lattice, replacing whatever was there
   * before; e.g.,
   *
   *    box(Int|BoxedDbl)   -> BoxedInt
   *    inner(BoxedInt|Dbl) -> Int
   *
   * Meanwhile, unbox() is like inner(), but rather than replacing the Cell
   * component of the Type, it unions it with the shifted BoxedCell bits, e.g.,
   *
   *    unbox(BoxedInt|Dbl) -> Int|Dbl
   *
   * @requires:
   *    box:    *this <= Cell
   *            !maybe(Uninit) || *this == Cell
   *    inner:  *this <= BoxedCell
   *    unbox:  *this <= Gen
   */
  Type box() const;
  Type inner() const;
  Type unbox() const;

  /*
   * Get a pointer to, or dereference, a Type.
   *
   * @requires:
   *    ptr:        *this <= Gen
   *    deref:      *this <= PtrToGen
   *    derefIfPtr: *this <= (Gen | PtrToGen)
   */
  Type ptr(Ptr kind) const;
  Type deref() const;
  Type derefIfPtr() const;

  /*
   * Return a Type stripped of boxing and pointerness.
   */
  Type strip() const;

  /*
   * Return the pointer category of a Type.
   */
  Ptr ptrKind() const;


  /////////////////////////////////////////////////////////////////////////////
  // Internal methods.

private:
  /*
   * Internal constructors.
   */
  Type(bits_t bits, Ptr kind, uintptr_t extra);
  Type(Type t, ArraySpec arraySpec);
  Type(Type t, ClassSpec classSpec);

  /*
   * Bit-pack an `outer' and an `inner' DataType for a Type.
   */
  static bits_t bitsFromDataType(DataType outer, DataType inner);

  /*
   * Check invariants and return false if the type is malformed.
   */
  bool checkValid() const;

  /*
   * Add specialization to a generic type via a TypeSpec.
   *
   * Used as the finalization step for union and intersect.  The `killable'
   * bits are the components of the Type which can be killed by components of
   * `spec' that are Bottom.
   */
  Type specialize(TypeSpec spec, bits_t killable = kTop) const;


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

    // Constant values.  Validity determined by m_hasConstVal and m_bits.
    bool m_boolVal;
    int64_t m_intVal;
    double m_dblVal;
    const StringData* m_strVal;
    const ArrayData* m_arrVal;
    const HPHP::Func* m_funcVal;
    const Class* m_clsVal;
    ConstCctx m_cctxVal;
    jit::TCA m_tcaVal;
    rds::Handle m_rdsHandleVal;
    TypedValue* m_ptrVal;

    // Specializations for object classes and arrays.
    ClassSpec m_clsSpec;
    ArraySpec m_arrSpec;
  };
};

typedef folly::Optional<Type> OptType;

/*
 * jit::Type must be small enough for efficient pass-by-value.
 */
static_assert(sizeof(Type) <= 2 * sizeof(uint64_t),
              "jit::Type should fit in (2 * sizeof(uint64_t))");


/////////////////////////////////////////////////////////////////////////////

/*
 * Return the most refined Type that can be used to represent the type of a
 * live TypedValue or a RepoAuthType.
 */
Type typeFromTV(const TypedValue* tv);
Type typeFromRAT(RepoAuthType ty);


///////////////////////////////////////////////////////////////////////////////

/*
 * Return the boxed version of the input type, taking into account PHP
 * semantics and subtle implementation details.
 */
Type boxType(Type);

/*
 * Return the dest type for a LdRef with the given typeParam.
 *
 * @requires: typeParam <= TCell
 */
Type ldRefReturn(Type typeParam);

/*
 * Returns the type that a value may have if it had type `srcType' and failed a
 * CheckType with `typeParam'.  Not all typeParams for CheckTypes are precise,
 * so the return value may even be `srcType' itself in some situations.
 */
Type negativeCheckType(Type typeParam, Type srcType);

///////////////////////////////////////////////////////////////////////////////

}}

#define incl_HPHP_JIT_TYPE_INL_H_
#include "hphp/runtime/vm/jit/type-inl.h"
#undef incl_HPHP_JIT_TYPE_INL_H_

#endif
