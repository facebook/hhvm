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
 * The Ptr enum is a lattice that represents the "pointerness" of a type:
 * whether it's a pointer at all, and what kind of location it may point to.
 *
 * We have a pointer kind for each of the major segregated locations in which
 * php values can live (eval stack, frame slots, properties, etc...). For most
 * of the primitive kinds, we have a predefined union of the kind and "inside a
 * Ref", so PtrToRStkGen is exactly the same as PtrTo{Ref|Stk}Gen.  These
 * classify PtrTo* types into some categories that cannot possibly alias,
 * without any smarter analysis needed to prove it.  There is also a union for
 * the various locations things can point after a fully generic member
 * operation (see Memb below).
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
 * for example, a PtrToStkGen or a PtrToGblGen (although it could generally
 * alias a PtrToRGblGen because both could be inside the same RefData.). Note
 * that PtrToRFooGen is just shorthand for PtrTo{Ref|Foo}Gen.
 *
 * Memb is a number of different locations that result from the more generic
 * types of member operations: Prop, Elem, MIS, MMisc, and Other. MMisc
 * contains something living in a collection instance or object's dynamic
 * property array. Other contains init_null_variant, null_variant, or the
 * lvalBlackHole.
 *
 * ClsInit is a pointer to class property initializer data.  These can never be
 * refs, so we don't have a RClsInit type.
 *
 * ClsCns is a pointer to class constant values in RDS.
 *
 * Ptr is a supertype of all Ptr types, Foo is a subtype of RFoo, and Ref is a
 * subtype of RFoo. NotPtr is unrelated to all other types. The hierarchy looks
 * something like this:
 *
 *                            Ptr                            NotPtr
 *                             |
 *         +-------------------+----+--------+-------+
 *         |                        |        |       |
 *       RMemb                      |     ClsInit  ClsCns
 *         |                        |
 *  +------+---------+              |
 *  |      |         |              |
 *  |      |         |              |
 *  |      |         |              |
 *  |      |    +----+-----+        +--------+----- ... etc
 *  |      |    |    |     |        |        |
 *  |    Memb  RMIS RProp RElem   RFrame    RStk
 *  |      |   /  | /   | /|        |  \      | \
 *  |   +--+-+/---|/+   |/ |        |  Frame  |  Stk
 *  |   |    /    / |   /  |        |         |
 *  |   |   /|   /| |  /|  |        |         |
 *  |   |  / |  / | | / |  |        |         |
 *  |   MIS  Prop | Elem|  |        |         |
 *  |             |     |  |        |         |
 *  +-------------+--+--+--+--------+---------+
 *                   |
 *                  Ref
 *
 * Note: if you add a new pointer type, you very likely need to update
 * pointee() in memory-effects.cpp for it to remain correct.
 *
 */

#define PTR_R(f, name, bits, ...)                    \
  f(name, bits, __VA_ARGS__)                         \
  f(R##name, (bits) | Ref, __VA_ARGS__)

#define PTR_NO_R(f, name, bits, ...)            \
  f(name, bits, __VA_ARGS__)

/*
 * Types that can never be refs directly call f; types that can call r with f
 * as an argument. Callers of PTR_TYPES may control whether the Ref cases are
 * actually expanded by passing PTR_R or PTR_NO_R for the r argument. Any
 * arguments passed beyond f and r will be forwarded to f.
 */
#define PTR_PRIMITIVE(f, r, ...)                         \
  f(Ref,      1U << 0, __VA_ARGS__)                      \
  f(ClsInit,  1U << 1, __VA_ARGS__)                      \
  f(ClsCns,   1U << 2, __VA_ARGS__)                      \
  r(f, Frame, 1U << 3, __VA_ARGS__)                      \
  r(f, Stk,   1U << 4, __VA_ARGS__)                      \
  r(f, Gbl,   1U << 5, __VA_ARGS__)                      \
  r(f, Prop,  1U << 6, __VA_ARGS__)                      \
  r(f, Elem,  1U << 7, __VA_ARGS__)                      \
  r(f, SProp, 1U << 8, __VA_ARGS__)                      \
  r(f, MIS,   1U << 9, __VA_ARGS__)                      \
  r(f, MMisc, 1U << 10, __VA_ARGS__)                     \
  r(f, Other, 1U << 11, __VA_ARGS__)                    \
  /* NotPtr,  1U << 12, declared below */

#define PTR_TYPES(f, r, ...)                             \
  PTR_PRIMITIVE(f, r, __VA_ARGS__)                       \
  r(f, Memb, Prop | Elem | MIS | MMisc | Other, __VA_ARGS__)

enum class Ptr : uint16_t {
  /*
   * The Ptr kinds here are kept out of PTR_TYPES to avoid generating names
   * like TPtrToNotPtrGen or TPtrToPtrGen. Note that those types do exist, just
   * with less ridiculous names: TGen and TPtrToGen, respectively.
   */
  Bottom = 0,
  Top    = 0x1fffU, // Keep this in sync with the number of bits used in
                    // PTR_PRIMITIVE, to keep pretty-printing cleaner.
  NotPtr = 1U << 12,
  Ptr    = Top & ~NotPtr,

#define PTRT(name, bits, ...) name = (bits),
  PTR_TYPES(PTRT, PTR_R)
#undef PTRT
};

using ptr_t = std::underlying_type<Ptr>::type;
constexpr auto kPtrRefBit = static_cast<ptr_t>(Ptr::Ref);

constexpr Ptr operator|(Ptr a, Ptr b) {
  return static_cast<Ptr>(static_cast<ptr_t>(a) | static_cast<ptr_t>(b));
}
constexpr Ptr operator&(Ptr a, Ptr b) {
  return static_cast<Ptr>(static_cast<ptr_t>(a) & static_cast<ptr_t>(b));
}
constexpr Ptr operator-(Ptr a, Ptr b) {
  return static_cast<Ptr>(static_cast<ptr_t>(a) & ~static_cast<ptr_t>(b));
}
constexpr bool operator<=(Ptr a, Ptr b) {
  return (a & b) == a;
}
bool operator>=(Ptr, Ptr) = delete;
bool operator<(Ptr, Ptr) = delete;
bool operator>(Ptr, Ptr) = delete;

///////////////////////////////////////////////////////////////////////////////

#define IRTP_FROM_PTR(ptr, ptr_bits, name)                    \
  IRTP(PtrTo##ptr##name, ptr, k##name)                        \

#define IRT_BOXES_AND_PTRS(name, bits)                        \
  IRT(name,              (bits))                              \
  IRT(Boxed##name,       (bits) << kBoxShift)                 \
  IRTP(PtrTo##name,      Ptr, k##name)                        \
  IRTP(PtrToBoxed##name, Ptr, kBoxed##name)                   \
  PTR_TYPES(IRTP_FROM_PTR, PTR_R, name)                       \
  PTR_TYPES(IRTP_FROM_PTR, PTR_NO_R, Boxed##name)

#define IRT_PHP(c)                                                      \
  c(Uninit,       1ULL << 0)                                            \
  c(InitNull,     1ULL << 1)                                            \
  c(Bool,         1ULL << 2)                                            \
  c(Int,          1ULL << 3)                                            \
  c(Dbl,          1ULL << 4)                                            \
  c(StaticStr,    1ULL << 5)                                            \
  c(UncountedStr, 1ULL << 6)                                            \
  c(CountedStr,   1ULL << 7)                                            \
  c(StaticArr,    1ULL << 8)                                            \
  c(UncountedArr, 1ULL << 9)                                            \
  c(CountedArr,   1ULL << 10)                                           \
  c(Obj,          1ULL << 11)                                           \
  c(Res,          1ULL << 12)
// Boxed*:        13-25

/*
 * This list should be in non-decreasing order of specificity.
 */
#define IRT_PHP_UNIONS(c)                                               \
  c(Null,          kUninit|kInitNull)                                   \
  c(PersistentStr, kStaticStr|kUncountedStr)                            \
  c(Str,           kPersistentStr|kCountedStr)                          \
  c(PersistentArr, kStaticArr|kUncountedArr)                            \
  c(Arr,           kPersistentArr|kCountedArr)                          \
  c(NullableObj,   kObj|kInitNull|kUninit)                              \
  c(Persistent,    kPersistentStr|kPersistentArr)                       \
  c(UncountedInit, kInitNull|kBool|kInt|kDbl|kPersistent)               \
  c(Uncounted,     kUninit|kUncountedInit)                              \
  c(InitCell,      kUncountedInit|kStr|kArr|kObj|kRes)                  \
  c(Cell,          kUninit|kInitCell)

#define IRT_RUNTIME                                                     \
  IRT(Cls,         1ULL << 26)                                          \
  IRT(Func,        1ULL << 27)                                          \
  IRT(VarEnv,      1ULL << 28)                                          \
  IRT(NamedEntity, 1ULL << 29)                                          \
  IRT(Cctx,        1ULL << 30) /* Class* with the lowest bit set,  */   \
                               /* as stored in ActRec.m_cls field  */   \
  IRT(RetAddr,     1ULL << 31) /* Return address */                     \
  IRT(StkPtr,      1ULL << 32) /* Stack pointer */                      \
  IRT(FramePtr,    1ULL << 33) /* Frame pointer */                      \
  IRT(TCA,         1ULL << 34)                                          \
  IRT(ABC,         1ULL << 35) /* AsioBlockableChain */                 \
  IRT(RDSHandle,   1ULL << 36) /* rds::Handle */                        \
  IRT(Nullptr,     1ULL << 37)                                          \
  /* bits 38-49 are padding, 50-62 are pointer kind, 63 is hasConstVal */

/*
 * Gen, Counted, Init, PtrToGen, etc... are here instead of IRT_PHP_UNIONS
 * because boxing them (e.g., BoxedGen, PtrToBoxedGen) would yield nonsense
 * types.
 */
#define IRT_SPECIAL                                                     \
  /* Bottom and Top use IRTP to specify a custom Ptr kind */            \
  IRTP(Bottom,       Bottom, kBottom)                                   \
  IRTP(Top,          Top,    kTop)                                      \
  IRT(Ctx,                   kObj|kCctx|kNullptr)                       \
  IRTP(AnyObj,       Top,    kAnyObj)                                   \
  IRTP(AnyArr,       Top,    kAnyArr)                                   \
  IRT(Counted,               kCountedStr|kCountedArr|kObj|kRes|kBoxedCell) \
  IRTP(PtrToCounted, Ptr,    kCounted)                                  \
  IRT(Gen,                   kCell|kBoxedCell)                          \
  IRT(InitGen,               kGen & ~kUninit)                           \
  IRT(StkElem,               kGen|kCls)                                 \
  IRTP(PtrToGen,     Ptr,    kGen)                                      \
  IRTP(PtrToInitGen, Ptr,    kInitGen)                                  \
  PTR_TYPES(IRTP_FROM_PTR, PTR_R, Gen)                                  \
  PTR_TYPES(IRTP_FROM_PTR, PTR_R, InitGen)

/*
 * All types that represent a non-union type.
 */
#define IRT_PRIMITIVE IRT_PHP(IRT_BOXES_AND_PTRS) IRT_RUNTIME

/*
 * All types.
 */
#define IR_TYPES                                \
  IRT_PHP(IRT_BOXES_AND_PTRS)                   \
  IRT_PHP_UNIONS(IRT_BOXES_AND_PTRS)            \
  IRT_RUNTIME                                   \
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
 * TBottom being a subset of all Types.  The elements forming these sets of
 * types come from the types of PHP-visible values (Str, Obj, Int, ...) and
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
private:
  using bits_t = uint64_t;
  static constexpr size_t kBoxShift = 13;

public:
  enum Bits : bits_t {
    kBottom = 0ULL,
    kTop    = 0xffffffffffffffffULL,

#define IRT(name, bits)       k##name = (bits),
#define IRTP(name, ptr, bits)
    IR_TYPES
#undef IRT
#undef IRTP

    kAnyArr      = kArr | kBoxedArr,
    kArrSpecBits = kAnyArr,
    kAnyObj      = kObj | kBoxedObj,
    kClsSpecBits = kAnyObj | kCls,
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

  static Type ExactCls(const Class* cls);
  static Type SubCls(const Class* cls);

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
   *    ptr:        *this <= Gen && kind <= Ptr::Ptr
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
   * Return a version of *this with the given specialization. If TypeSpec
   * contains both kinds of specialization or *this supports both types of
   * specialization, the resulting type will not be specialized at all. Any
   * constant value in *this will be dropped if the specialization is
   * applied. *this must support any specializations present in the given
   * TypeSpec.
   */
  Type specialize(TypeSpec) const;

  /*
   * Do the given bits support a specific kind of specialization?
   */
  static bool supports(bits_t, SpecKind kind);

  /////////////////////////////////////////////////////////////////////////////
  // Data members.

private:
  union {
    struct {
      // Only 38 of the 51 bits are actively in use but we make sure these
      // three bit fields fill up all 64 bits to ensure they're all
      // initialized.
      bits_t m_bits : 50;
      bits_t m_ptrKind : 13;
      bits_t m_hasConstVal : 1;
    };
    uint64_t m_raw;
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

using OptType = folly::Optional<Type>;

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

/*
 * Returns the least specific supertype of `t' that maintains the properties
 * required by `cat'.
 */
Type relaxType(Type t, DataTypeCategory cat);

/*
 * Returns the smallest supertype of ty that we can reasonably guard on. Used
 * for checking inner ref cells and locals in pesudomains.
 */
Type relaxToGuardable(Type ty);

///////////////////////////////////////////////////////////////////////////////

}}

///////////////////////////////////////////////////////////////////////////////

namespace std {
  template<> struct hash<HPHP::jit::Type> {
    size_t operator()(HPHP::jit::Type t) const { return t.hash(); }
  };
}

///////////////////////////////////////////////////////////////////////////////

#define incl_HPHP_JIT_TYPE_INL_H_
#include "hphp/runtime/vm/jit/type-inl.h"
#undef incl_HPHP_JIT_TYPE_INL_H_

#endif
