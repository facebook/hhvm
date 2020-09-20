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

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/bespoke-layout.h"
#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/repo-auth-type.h"
#include "hphp/runtime/vm/class-meth-data-ref.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/type-specialization.h"
#include "hphp/util/bitset.h"
#include "hphp/util/low-ptr.h"

#include <folly/Optional.h>

#include <cstdint>
#include <type_traits>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct ArrayData;
struct Class;
struct Func;
struct RecordDesc;
struct StringData;
struct TypeConstraint;
struct TypedValue;

namespace jit {
///////////////////////////////////////////////////////////////////////////////

struct GuardConstraint;
struct ProfDataSerializer;
struct ProfDataDeserializer;

/*
 * The Ptr enum is a lattice that represents the "pointerness" of a type:
 * whether it's a pointer at all, and what kind of location it may point to.
 *
 * We have a pointer kind for each of the major segregated locations in which
 * php values can live (eval stack, frame slots, properties, etc...).  These
 * classify PtrTo* types into some categories that cannot possibly alias,
 * without any smarter analysis needed to prove it.  There is also a union for
 * the various locations things can point after a fully generic member
 * operation (see Memb below).
 *
 * Memb is a number of different locations that result from the more generic
 * types of member operations: Prop, Elem, MIS, MMisc, and Other. MMisc
 * contains something living in a collection instance or object's dynamic
 * property array. Other contains init_null_variant, uninit_variant, or the
 * lvalBlackHole.
 *
 * ClsInit is a pointer to class property initializer data.
 *
 * ClsCns is a pointer to class constant values in RDS.
 *
 * The hierarchy looks something like this:
 *
 *                            Ptr                            NotPtr
 *                             |
 *         +-------------------+----+--------+-------+
 *         |                        |        |       |
 *        Memb                      |     ClsInit  ClsCns
 *         |                        |
 *         |                        +--------+----- ... etc
 *         |                        |        |
 *         |                      Frame     Stk
 *      +--+-+------+----+
 *      |    |      |    |
 *     MIS  Prop   Elem Field
 *
 * Note: if you add a new pointer type, you very likely need to update
 * pointee() in memory-effects.cpp for it to remain correct.
 *
 */

#define PTR_PRIMITIVE(f,...)                             \
  f(ClsInit,  1U << 0, __VA_ARGS__)                      \
  f(ClsCns,   1U << 1, __VA_ARGS__)                      \
  f(Frame, 1U << 2, __VA_ARGS__)                         \
  f(Stk,   1U << 3, __VA_ARGS__)                         \
  f(Gbl,   1U << 4, __VA_ARGS__)                         \
  f(Prop,  1U << 5, __VA_ARGS__)                         \
  f(Elem,  1U << 6, __VA_ARGS__)                         \
  f(SProp, 1U << 7, __VA_ARGS__)                         \
  f(MIS,   1U << 8, __VA_ARGS__)                         \
  f(MMisc, 1U << 9, __VA_ARGS__)                         \
  f(Other, 1U << 10, __VA_ARGS__)                        \
  f(Field, 1U << 11, __VA_ARGS__)                        \
  /* NotPtr,  1U << 12, declared below */

#define PTR_TYPES(f, ...)                                \
  PTR_PRIMITIVE(f, __VA_ARGS__)                          \
  f(Memb, Prop | Elem | MIS | MMisc | Other | Field, __VA_ARGS__)

enum class Ptr : uint16_t {
  /*
   * The Ptr kinds here are kept out of PTR_TYPES to avoid generating names like
   * TPtrToNotPtrCell or TPtrToPtrCell. Note that those types do exist, just
   * with less ridiculous names: TCell and TPtrToCell, respectively.
   */
  Bottom = 0,
  Top    = 0x1fffU, // Keep this in sync with the number of bits used in
                    // PTR_PRIMITIVE, to keep pretty-printing cleaner.
  NotPtr = 1U << 12,
  Ptr    = Top & ~NotPtr,

#define PTRT(name, bits, ...) name = (bits),
  PTR_TYPES(PTRT)
#undef PTRT
};

using ptr_t = std::underlying_type<Ptr>::type;

constexpr Ptr operator~(Ptr p) {
  return static_cast<Ptr>(~static_cast<ptr_t>(p));
}
constexpr Ptr operator|(Ptr a, Ptr b) {
  return static_cast<Ptr>(static_cast<ptr_t>(a) | static_cast<ptr_t>(b));
}
inline Ptr& operator|=(Ptr& a, Ptr b) {
  return a = a | b;
}
constexpr Ptr operator&(Ptr a, Ptr b) {
  return static_cast<Ptr>(static_cast<ptr_t>(a) & static_cast<ptr_t>(b));
}
inline Ptr& operator&=(Ptr& a, Ptr b) {
  return a = a & b;
}
constexpr Ptr operator-(Ptr a, Ptr b) {
  return static_cast<Ptr>(static_cast<ptr_t>(a) & ~static_cast<ptr_t>(b));
}

constexpr bool operator<=(Ptr a, Ptr b) {
  return (a & b) == a;
}
constexpr bool operator>=(Ptr a, Ptr b) {
  return b <= a;
}
constexpr bool operator<(Ptr a, Ptr b) {
  return a <= b && a != b;
}
constexpr bool operator>(Ptr a, Ptr b) {
  return a >= b && a != b;
}

///////////////////////////////////////////////////////////////////////////////

/*
 * Mem is a lattice which supervenes on Ptr describing how to interpret a
 * memory address.
 *
 * Whereas Ptr describes what we know about the memory location, Mem describes
 * what we know about the memory address itself.
 */
enum class Mem : uint8_t {
  /* Bottom: No other components of the type are compatible with Mem: TCls,
   *         TRDSHandle, etc... */
  Bottom = 0,
  /* NotMem: Normal values like TInt or TCell. */
  NotMem = 1U << 0,
  /* Ptr: TypedValue*: TPtrToInt, TPtrToCell, etc... */
  Ptr    = 1U << 1,
  /* Lval: tv_lval: TLvalToInt, TLvalToCell, etc... */
  Lval   = 1U << 2,
  /* Mem: Either Ptr or Lval. No concrete values can have this type because
   * there is no way to distinguish between TPtrToFoo and TLvalToFoo at
   * runtime. */
  Mem    = Ptr | Lval,
  /* Top: Only used in TTop. */
  Top    = NotMem | Mem,
};

using mem_t = std::underlying_type<Mem>::type;

constexpr Mem operator~(Mem m) {
  return static_cast<Mem>(~static_cast<mem_t>(m));
}
constexpr Mem operator|(Mem a, Mem b) {
  return static_cast<Mem>(static_cast<mem_t>(a) | static_cast<mem_t>(b));
}
inline Mem& operator|=(Mem& a, Mem b) {
  return a = a | b;
}
constexpr Mem operator&(Mem a, Mem b) {
  return static_cast<Mem>(static_cast<mem_t>(a) & static_cast<mem_t>(b));
}
inline Mem& operator&=(Mem& a, Mem b) {
  return a = a & b;
}
constexpr Mem operator-(Mem a, Mem b) {
  return static_cast<Mem>(static_cast<mem_t>(a) & ~static_cast<mem_t>(b));
}

constexpr bool operator<=(Mem a, Mem b) {
  return (a & b) == a;
}
constexpr bool operator>=(Mem a, Mem b) {
  return b <= a;
}
constexpr bool operator<(Mem a, Mem b) {
  return a <= b && a != b;
}
constexpr bool operator>(Mem a, Mem b) {
  return a >= b && a != b;
}

///////////////////////////////////////////////////////////////////////////////

/*
 * This section defines a number of macros used to stamp out code and data for
 * all predefined types (types not represented here can be constructed by
 * combining different types). It can be viewed as one big X macro, with
 * different values of X:
 *
 * IRT(name, bits): A plain type, like Bool or Obj.
 * IRTP(name, ptr, bits): A Ptr type, with Mem::Ptr and a custom Ptr.
 * IRTL(name, ptr, bits): An Lval type, with Mem::Lval and a custom Ptr.
 * IRTM(name, ptr, bits): A Mem type, with Mem::Mem and a custom Ptr.
 * IRTX(name, x, bits): A custom memory type, with Mem::x and Ptr::x.
 */

#define IRTP_FROM_PTR(ptr, ptr_bits, name)                    \
  IRTP(PtrTo##ptr##name, ptr, k##name)

#define IRTL_FROM_PTR(ptr, ptr_bits, name)                    \
  IRTL(LvalTo##ptr##name, ptr, k##name)

#define IRTM_FROM_PTR(ptr, ptr_bits, name)                    \
  IRTM(MemTo##ptr##name, ptr, k##name)

#define IRT_PTRS_LVALS(name, bits)                            \
  IRT(name,               (bits))                             \
  IRTP(PtrTo##name,       Ptr, k##name)                       \
  PTR_TYPES(IRTP_FROM_PTR, name)                              \
  IRTL(LvalTo##name,      Ptr, k##name)                       \
  PTR_TYPES(IRTL_FROM_PTR, name)                              \
  IRTM(MemTo##name,       Ptr, k##name)                       \
  PTR_TYPES(IRTM_FROM_PTR, name)                              \
/**/

#define IRT_PHP(c)                                                      \
  c(Uninit,          bits_t::bit<0>())                                  \
  c(InitNull,        bits_t::bit<1>())                                  \
  c(Bool,            bits_t::bit<2>())                                  \
  c(Int,             bits_t::bit<3>())                                  \
  c(Dbl,             bits_t::bit<4>())                                  \
  c(StaticStr,       bits_t::bit<5>())                                  \
  c(UncountedStr,    bits_t::bit<6>())                                  \
  c(CountedStr,      bits_t::bit<7>())                                  \
  c(StaticVec,       bits_t::bit<8>())                                  \
  c(UncountedVec,    bits_t::bit<9>())                                  \
  c(CountedVec,      bits_t::bit<10>())                                 \
  c(StaticDict,      bits_t::bit<11>())                                 \
  c(UncountedDict,   bits_t::bit<12>())                                 \
  c(CountedDict,     bits_t::bit<13>())                                 \
  c(StaticKeyset,    bits_t::bit<14>())                                 \
  c(UncountedKeyset, bits_t::bit<15>())                                 \
  c(CountedKeyset,   bits_t::bit<16>())                                 \
  c(Obj,             bits_t::bit<17>())                                 \
  c(Res,             bits_t::bit<18>())                                 \
  c(Func,            bits_t::bit<19>())                                 \
  c(Cls,             bits_t::bit<20>())                                 \
  c(ClsMeth,         bits_t::bit<21>())                                 \
  c(Record,          bits_t::bit<22>())                                 \
  c(RecDesc,         bits_t::bit<23>())                                 \
  c(RFunc,           bits_t::bit<24>())                                 \
  c(StaticVArr,      bits_t::bit<25>())                                 \
  c(UncountedVArr,   bits_t::bit<26>())                                 \
  c(CountedVArr,     bits_t::bit<27>())                                 \
  c(StaticDArr,      bits_t::bit<28>())                                 \
  c(UncountedDArr,   bits_t::bit<29>())                                 \
  c(CountedDArr,     bits_t::bit<30>())                                 \
  c(RClsMeth,        bits_t::bit<31>())                                 \
  c(LazyCls,         bits_t::bit<32>())                                 \
/**/

/*
 * This list should be in non-decreasing order of specificity.
 */
#ifdef USE_LOWPTR
#define UNCCOUNTED_INIT_UNION \
        kInitNull|kBool|kInt|kDbl|kPersistent|kFunc|kCls|kRecDesc|kLazyCls| \
        kClsMeth
#else
#define UNCCOUNTED_INIT_UNION \
        kInitNull|kBool|kInt|kDbl|kPersistent|kFunc|kCls|kRecDesc|kLazyCls
#endif

#ifdef USE_LOWPTR
#define INIT_CELL_UNION kUncountedInit|kStr|kArrLike|kObj|kRes|kRecord| \
                        kRFunc|kRClsMeth
#else
#define INIT_CELL_UNION kUncountedInit|kStr|kArrLike|kObj|kRes|kRecord| \
                        kClsMeth|kRFunc|kRClsMeth
#endif

#define IRT_PHP_UNIONS(c)                                               \
  c(Null,                kUninit|kInitNull)                             \
  c(PersistentStr,       kStaticStr|kUncountedStr)                      \
  c(Str,                 kPersistentStr|kCountedStr)                    \
  c(PersistentVArr,      kStaticVArr|kUncountedVArr)                    \
  c(VArr,                kPersistentVArr|kCountedVArr)                  \
  c(PersistentDArr,      kStaticDArr|kUncountedDArr)                    \
  c(DArr,                kPersistentDArr|kCountedDArr)                  \
  c(StaticArr,           kStaticVArr|kStaticDArr)                       \
  c(UncountedArr,        kUncountedVArr|kUncountedDArr)                 \
  c(CountedArr,          kCountedVArr|kCountedDArr)                     \
  c(PersistentArr,       kStaticArr|kUncountedArr)                      \
  c(Arr,                 kPersistentArr|kCountedArr)                    \
  c(PersistentVec,       kStaticVec|kUncountedVec)                      \
  c(Vec,                 kPersistentVec|kCountedVec)                    \
  c(PersistentDict,      kStaticDict|kUncountedDict)                    \
  c(Dict,                kPersistentDict|kCountedDict)                  \
  c(PersistentKeyset,    kStaticKeyset|kUncountedKeyset)                \
  c(Keyset,              kPersistentKeyset|kCountedKeyset)              \
  c(PersistentArrLike,   kPersistentArr|kPersistentVec|kPersistentDict|kPersistentKeyset) \
  c(ArrLike,             kArr|kVec|kDict|kKeyset)                       \
  c(NullableObj,         kObj|kInitNull|kUninit)                        \
  c(Persistent,          kPersistentStr|kPersistentArrLike)             \
  c(UncountedInit,       UNCCOUNTED_INIT_UNION)                         \
  c(Uncounted,           kUninit|kUncountedInit)                        \
  c(InitCell,            INIT_CELL_UNION)                               \
  c(Cell,                kUninit|kInitCell)                             \
  c(FuncLike,            kFunc|kRFunc)                                  \
  c(ClsMethLike,         kClsMeth|kRClsMeth)

/*
 * Adding a new runtime type needs updating numRuntime variable.
 */
#define IRT_RUNTIME                                                     \
  IRT(NamedEntity, bits_t::bit<kRuntime>())                             \
  IRT(RetAddr,     bits_t::bit<kRuntime+1>()) /* Return address */      \
  IRT(StkPtr,      bits_t::bit<kRuntime+2>()) /* Stack pointer */       \
  IRT(FramePtr,    bits_t::bit<kRuntime+3>()) /* Frame pointer */       \
  IRT(TCA,         bits_t::bit<kRuntime+4>())                           \
  IRT(ABC,         bits_t::bit<kRuntime+5>()) /* AsioBlockableChain */  \
  IRT(RDSHandle,   bits_t::bit<kRuntime+6>()) /* rds::Handle */         \
  IRT(Nullptr,     bits_t::bit<kRuntime+7>())                           \
  IRT(Smashable,   bits_t::bit<kRuntime+8>()) /* Smashable uint64_t */  \
  /* bits above this are unused */

/*
 * Cell, Counted, Init, PtrToCell, etc...
 */
#ifdef USE_LOWPTR
#define COUNTED_INIT_UNION \
  kCountedStr|kCountedArr|kCountedVec|kCountedDict|kCountedKeyset|kObj|kRes| \
  kRecord|kRFunc|kRClsMeth
#else
#define COUNTED_INIT_UNION \
  kCountedStr|kCountedArr|kCountedVec|kCountedDict|kCountedKeyset|kObj|kRes| \
  kRecord|kClsMeth|kRFunc|kRClsMeth
#endif

#define IRT_SPECIAL                                           \
  /* Bottom and Top use IRTX to specify a custom Ptr kind */  \
  IRTX(Bottom,         Bottom, kBottom)                       \
  IRTX(Top,            Top,    kTop)                          \
  IRT(Counted,                 COUNTED_INIT_UNION)            \
  IRTP(PtrToCounted,   Ptr,    kCounted)                      \
  IRTL(LvalToCounted,  Ptr,    kCounted)                      \
  IRTM(MemToCounted,   Ptr,    kCounted)                      \
/**/

/*
 * All types that represent a non-union type.
 */
#define IRT_PRIMITIVE IRT_PHP(IRT_PTRS_LVALS) IRT_RUNTIME

/*
 * All types.
 */
#define IR_TYPES                  \
  IRT_PHP(IRT_PTRS_LVALS)         \
  IRT_PHP_UNIONS(IRT_PTRS_LVALS)  \
  IRT_RUNTIME                     \
  IRT_SPECIAL

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
  static constexpr size_t kRuntime = 33;
  static constexpr size_t numRuntime = 9;
  using bits_t = BitSet<kRuntime + numRuntime>;

public:
  static constexpr bits_t kBottom{};
  static constexpr bits_t kTop = ~kBottom;

#define IRT(name, bits)       static constexpr bits_t k##name = (bits);
#define IRTP(name, ptr, bits)
#define IRTL(name, ptr, bits)
#define IRTM(name, ptr, bits)
#define IRTX(name, ptr, bits)
    IR_TYPES
#undef IRT
#undef IRTP
#undef IRTL
#undef IRTM
#undef IRTX

  static constexpr bits_t kArrSpecBits  = kArrLike;
  static constexpr bits_t kClsSpecBits  = kObj | kCls;
  static constexpr bits_t kRecSpecBits  = kRecord | kRecDesc;

  /////////////////////////////////////////////////////////////////////////////
  // Basic methods.

public:
  /*
   * Default bottom constructor.
   */
  Type();

  /*
   * Construct from a predefined set of bits, pointer kind, and mem kind.
   */
  constexpr Type(bits_t bits, Ptr ptr, Mem mem);

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

  /*
   * Serialization/deserialization.
   */
  void serialize(ProfDataSerializer&) const;
  static Type deserialize(ProfDataDeserializer&);

  /////////////////////////////////////////////////////////////////////////////
  // DataType.

  /*
   * Construct from a DataType.
   */
  explicit Type(DataType outer);

  /*
   * Return true iff there exists a DataType that represents a non-strict
   * supertype of this type.
   *
   * @requires: *this <= Cell
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
   * Is this a non-strict subtype of `t2' and is not Bottom?
   */
  bool nonTrivialSubtypeOf(Type t2) const;

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

  template<typename... Types>
  static Type unionAll(Type t, Types... ts);
  static Type unionAll();


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
   * Does this Type have a constant value?  If true, we can call xxVal().
   *
   * NOTE: Bottom is a type with no value, and Uninit/InitNull/Nullptr are
   * considered types with a single unique value, so this function returns false
   * for those types.  You may want to explicitly check for them as needed.
   *
   * NOTE: Constant pointer values differ from other constants in a few ways:
   *  1. They may be a "union" - i.e. m_bits may have multiple bits set.
   *  2. The ptrVal result does not necessary point to a valid value.
   *  3. Constant pointer types never have specializations.
   *
   * The motivation for these changes is to provide types for a specialized
   * pointer iterator's `pos` and `end` fields. Consider the `end` pointer for
   * some static vec with int and dict values. The type of this pointer is a
   * constant subtype of the type "PtrToElem{Int|Dict}" (1). It points one
   * element past the end of the vec, so its target is invalid (2).
   *
   * (3) is a consequence of (2). We use the same union field to store constant
   * types and type specializations. For non-pointer types, we can recover the
   * specialization from the constant, so this usage is okay, but for pointer
   * types, we can't, so constant pointers lose the specialization.
   */
  bool hasConstVal() const;

  /*
   * @returns: hasConstVal() && *this <= t
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
   * Whether this Type represents a single possible value.
   *
   * @returns: hasConstVal() || subtypeOfAny(TNullptr, TInitNull, TUninit)
   */
  bool admitsSingleVal() const;

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
  const ArrayData* vecVal() const;
  const ArrayData* dictVal() const;
  const ArrayData* shapeVal() const;
  const ArrayData* keysetVal() const;
  const ArrayData* arrLikeVal() const;
  const HPHP::Func* funcVal() const;
  const Class* clsVal() const;
  LazyClassData lclsVal() const;
  const RecordDesc* recVal() const;
  ClsMethDataRef clsmethVal() const;
  rds::Handle rdsHandleVal() const;
  jit::TCA tcaVal() const;
  const TypedValue* ptrVal() const;


  /////////////////////////////////////////////////////////////////////////////
  // Specialized type creation.                                  [const/static]

  /*
   * Return a specialized TArr/TVec/TDict/TKeyset.
   * These types are always subtypes of TVanillaArrLike.
   */
  static Type Array(const RepoAuthType::Array*);
  static Type VArr(const RepoAuthType::Array*);
  static Type DArr(const RepoAuthType::Array*);
  static Type Vec(const RepoAuthType::Array*);
  static Type Dict(const RepoAuthType::Array*);
  static Type Keyset(const RepoAuthType::Array*);

  /*
   * Return a specialized TStaticArr/TStaticVec/TStaticDict/TStaticKeyset.
   * These types are always subtypes of TVanillaArrLike.
   */
  static Type StaticArray(const RepoAuthType::Array*);
  static Type StaticVArr(const RepoAuthType::Array*);
  static Type StaticDArr(const RepoAuthType::Array*);
  static Type StaticVec(const RepoAuthType::Array*);
  static Type StaticDict(const RepoAuthType::Array*);
  static Type StaticKeyset(const RepoAuthType::Array*);

  /*
   * Return a specialized TCountedArr/TCountedVec/TCountedDict.
   * These types are always subtypes of TVanillaArrLike.
   */
  static Type CountedArray(const RepoAuthType::Array*);
  static Type CountedVArr(const RepoAuthType::Array*);
  static Type CountedDArr(const RepoAuthType::Array*);
  static Type CountedVec(const RepoAuthType::Array*);
  static Type CountedDict(const RepoAuthType::Array*);
  static Type CountedKeyset(const RepoAuthType::Array*);

  /*
   * Return a specialized TObj.
   */
  static Type SubObj(const Class* cls);
  static Type ExactObj(const Class* cls);

  /*
   * Return a specialized TRecord.
   */
  static Type SubRecord(const RecordDesc*);
  static Type ExactRecord(const RecordDesc*);

  static Type ExactCls(const Class* cls);
  static Type SubCls(const Class* cls);

  /*
   * Return a copy of this type with a vanilla ArraySpec. Examples:
   *   TArr.narrowToVanilla()        == TVanillaArr
   *   (TVec|TInt).narrowToVanilla() == TVanillaVec|TInt
   *   (TVec|TObj).narrowToVanilla() == TVec|TObj
   *   TPackedArr.narrowToVanilla()  == TPackedArr
   */
  Type narrowToVanilla() const;

  /*
   * Return a copy of this type with an ArraySpec for the given bespoke index
   *   TVanillaArr.narrowToBespokeLayout(<some layout>) == TBottom
   *   TVec.narrowToBespokeLayout(<some layout>) == TVec=Bespoke(<some layout>)
   *   (TVec|TInt).narrowToBespokeLayout(<some layout>) == TVec=Bespoke(<some layout>)|TInt
   */
  Type narrowToBespokeLayout(BespokeLayout) const;

  /*
   * Return a copy of this Type with the specialization dropped.
   *
   * @returns: Type(m_bits)
   */
  Type unspecialize() const;

  /*
   * Return a copy of this Type with the specialization and staticness
   * dropped.
   *
   * @requires *this <= TInitCell
   */
  Type modified() const;

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
   * nontrivially with kObj.
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
  RecordSpec recSpec() const;

  /*
   * Return a discriminated TypeSpec for this Type's specialization.
   */
  TypeSpec spec() const;


  /////////////////////////////////////////////////////////////////////////////
  // Inner types.                                                       [const]

  /*
   * Get a pointer to, or dereference, a Type.
   *
   * @requires:
   *    ptr, lval:  *this <= Cell && kind <= Ptr::Ptr
   *    mem:        *this <= Cell && kind <= Ptr::Ptr && mem <= Mem::Mem
   *    deref:      *this <= MemToCell
   *    derefIfPtr: *this <= (Cell | MemToCell)
   */
  Type ptr(Ptr kind) const;
  Type lval(Ptr kind) const;
  Type mem(Mem mem, Ptr kind) const;
  Type deref() const;
  Type derefIfPtr() const;

  /*
   * Return the pointer or memory category of a Type.
   */
  Ptr ptrKind() const;
  Mem memKind() const;

  /////////////////////////////////////////////////////////////////////////////
  // Internal methods.

private:
  /*
   * Internal constructors.
   */
  Type(bits_t bits, Ptr ptr, Mem mem, bool hasConstVal, uintptr_t extra);
  Type(Type t, ArraySpec arraySpec);
  Type(Type t, ClassSpec classSpec);
  Type(Type t, RecordSpec recSpec);

  /*
   * Bit-pack a DataType
   */
  static bits_t bitsFromDataType(DataType outer);

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
  bits_t m_bits;
  Ptr m_ptr;
  Mem m_mem;
  bool m_hasConstVal;

  union {
    uintptr_t m_extra;

    // Constant values.  Validity determined by m_hasConstVal and m_bits.
    bool m_boolVal;
    int64_t m_intVal;
    double m_dblVal;
    const StringData* m_strVal;
    const ArrayData* m_arrVal;
    const ArrayData* m_vecVal;
    const ArrayData* m_dictVal;
    const ArrayData* m_keysetVal;
    const HPHP::Func* m_funcVal;
    const Class* m_clsVal;
    const RecordDesc* m_recVal;
    ClsMethDataRef m_clsmethVal;
    LazyClassData m_lclsVal;
    jit::TCA m_tcaVal;
    rds::Handle m_rdsHandleVal;
    TypedValue* m_ptrVal;

    // Specializations for object classes, records and arrays.
    ClassSpec m_clsSpec;
    ArraySpec m_arrSpec;
    RecordSpec m_recSpec;
  };
};

using OptType = folly::Optional<Type>;

/////////////////////////////////////////////////////////////////////////////

/*
 * Return the most refined Type that can be used to represent the type of a
 * live TypedValue or a RepoAuthType.
 *
 * For these methods, ctx may be null in general. propCls may be null during
 * Class initialization, but only if the type-hints involved are not "this",
 * "self", or other hints that use propCls. We always check this constraint.
 */
Type typeFromTV(tv_rval tv, const Class* ctx);
Type typeFromRAT(RepoAuthType ty, const Class* ctx);
Type typeFromPropTC(const HPHP::TypeConstraint& tc,
                    const Class* propCls,
                    const Class* ctx,
                    bool isSProp);

///////////////////////////////////////////////////////////////////////////////

/*
 * Returns the type that a value may have if it had type `srcType' and failed a
 * CheckType with `typeParam'.  Not all typeParams for CheckTypes are precise,
 * so the return value may even be `srcType' itself in some situations.
 */
Type negativeCheckType(Type typeParam, Type srcType);

/*
 * Returns the least specific supertype of `t' that maintains the properties
 * required by `cat' or by `gc'. The latter should be used where possible;
 * we have constraints when forming tracelet regions, but we only keep around
 * DataTypeCategory when we save them to the profile data.
 */
Type relaxType(Type t, DataTypeCategory cat);
Type relaxToConstraint(Type t, const GuardConstraint& gc);

/*
 * Returns the smallest supertype of ty that we can reasonably guard on. Used
 * for checking inner ref cells and locals in pseudomains.
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

