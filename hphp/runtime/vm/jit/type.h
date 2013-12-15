/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include <cstdint>
#include <cstring>
#include <boost/optional/optional.hpp>

#include "hphp/util/data-block.h"

#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/vm/class.h"

namespace HPHP {
namespace JIT {
struct DynLocation;
struct RuntimeType;
}
namespace JIT {


#define IRT_BOXES(name, bits)                                           \
  IRT(name,             (bits))                                         \
  IRT(Boxed##name,      (bits) << kBoxShift)                            \
  IRT(PtrTo##name,      (bits) << kPtrShift)                            \
  IRT(PtrToBoxed##name, (bits) << kPtrBoxShift)

#define IRT_BOXES_WITH_ANY(name, bits)                                  \
  IRT_BOXES(name, bits)                                                 \
  IRT(Any##name, k##name | kBoxed##name | kPtrTo##name |                \
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
// Boxed*:       11-21
// PtrTo*:       22-32
// PtrToBoxed*:  33-43

// This list should be in non-decreasing order of specificity
#define IRT_PHP_UNIONS(c)                                               \
  c(Null,          kUninit|kInitNull)                                   \
  c(Str,           kStaticStr|kCountedStr)                              \
  c(Arr,           kStaticArr|kCountedArr)                              \
  c(UncountedInit, kInitNull|kBool|kInt|kDbl|kStaticStr|kStaticArr)     \
  c(Uncounted,     kUncountedInit|kUninit)                              \
  c(InitCell,      kUncountedInit|kStr|kArr|kObj|kRes)                  \
  c(Cell,          kInitCell|kUninit)

#define IRT_RUNTIME                                                     \
  IRT(Cls,         1ULL << 44)                                          \
  IRT(Func,        1ULL << 45)                                          \
  IRT(VarEnv,      1ULL << 46)                                          \
  IRT(NamedEntity, 1ULL << 47)                                          \
  IRT(FuncCls,     1ULL << 48) /* {Func*, Cctx} */                      \
  IRT(FuncObj,     1ULL << 49) /* {Func*, Obj} */                       \
  IRT(Cctx,        1ULL << 50) /* Class* with the lowest bit set,  */   \
                               /* as stored in ActRec.m_cls field  */   \
  IRT(RetAddr,     1ULL << 51) /* Return address */                     \
  IRT(StkPtr,      1ULL << 52) /* stack pointer */                      \
  IRT(FramePtr,    1ULL << 53) /* frame pointer */                      \
  IRT(TCA,         1ULL << 54)                                          \
  IRT(ActRec,      1ULL << 55)                                          \
  IRT(None,        1ULL << 56)                                          \
  IRT(RDSHandle,   1ULL << 57) /* RDS::Handle */                        \
  IRT(Nullptr,     1ULL << 58)

// The definitions for these are in ir.cpp
#define IRT_UNIONS                                                      \
  IRT(Ctx,         kObj|kCctx)                                          \
  IRT(FuncCtx,     kFuncCls|kFuncObj)

// Gen, Counted, PtrToGen, and PtrToCounted are here instead of
// IRT_PHP_UNIONS because boxing them (e.g., BoxedGen, PtrToBoxedGen)
// would yield nonsense types.
#define IRT_SPECIAL                                                 \
  IRT(Bottom,       0)                                              \
  IRT(Top,          0xffffffffffffffffULL)                          \
  IRT(Counted,      kCountedStr|kCountedArr|kObj|kRes|kBoxedCell)   \
  IRT(PtrToCounted, kCounted << kPtrShift)                          \
  IRT(Gen,          kCell|kBoxedCell)                               \
  IRT(StackElem,    kGen|kCls)                                      \
  IRT(Init,         kGen & ~kUninit)                                \
  IRT(PtrToGen,     kGen << kPtrShift)                              \
  IRT(PtrToInit,    kInit << kPtrShift)

// All types (including union types) that represent program values,
// except Gen (which is special). Boxed*, PtrTo*, and PtrToBoxed* only
// exist for these types.
#define IRT_USERLAND(c) IRT_PHP(c) IRT_PHP_UNIONS(c)

// All types with just a single bit set
#define IRT_PRIMITIVE IRT_PHP(IRT_BOXES) IRT_RUNTIME

// All types
#define IR_TYPES IRT_USERLAND(IRT_BOXES_WITH_ANY) IRT_RUNTIME IRT_UNIONS \
  IRT_SPECIAL

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
class Type {
  typedef uint64_t bits_t;

  static const size_t kBoxShift = 11;
  static const size_t kPtrShift = kBoxShift * 2;
  static const size_t kPtrBoxShift = kBoxShift + kPtrShift;

  enum TypedBits {
#define IRT(name, bits) k##name = (bits),
  IR_TYPES
#undef IRT
  };

  union {
    bits_t m_bits;
    TypedBits m_typedBits;
  };

  union {
    uintptr_t m_extra;
    const Class* m_class;
    struct {
      bool m_arrayKindValid;
      ArrayData::ArrayKind m_arrayKind;
      char padding[6];
    };
  };

  bool checkValid() const;

  explicit Type(bits_t bits, uintptr_t extra = 0)
    : m_bits(bits), m_extra(extra)
  {
    assert(checkValid());
  }

  explicit Type(bits_t bits, const Class* klass)
    : m_bits(bits), m_class(klass)
  {
    assert(checkValid());
  }

  explicit Type(bits_t bits, ArrayData::ArrayKind arrayKind)
    : m_bits(bits)
    , m_arrayKindValid(true)
    , m_arrayKind(arrayKind)
    , padding{0} // Keeping all 8 bytes of m_extra valid makes some comparisons
                 // simpler
  {
    assert(checkValid());
  }

  static bits_t bitsFromDataType(DataType outer, DataType inner);

  // combine, Union, and Intersect are used for operator| and operator&. See
  // .cpp for details.
  template<typename Oper>
  Type combine(bits_t newBits, Type b) const;

  struct Union;
  struct Intersect;

public:
# define IRT(name, ...) static const Type name;
  IR_TYPES
# undef IRT

  Type()
    : m_bits(kNone)
    , m_extra(0)
  {}

  explicit Type(DataType outerType, DataType innerType = KindOfInvalid)
    : m_bits(bitsFromDataType(outerType, innerType))
    , m_extra(0)
  {}

  explicit Type(const RuntimeType& rtt);
  explicit Type(const DynLocation* dl);

  size_t hash() const {
    return hash_int64_pair(m_bits, reinterpret_cast<uintptr_t>(m_class));
  }

  bool operator==(Type other) const { return equals(other); }
  bool operator!=(Type other) const { return !operator==(other); }

  Type operator|(Type other) const;
  Type& operator|=(Type other) { return *this = *this | other; }

  Type operator&(Type other) const;
  Type& operator&=(Type other) { return *this = *this & other; }

  Type operator-(Type other) const;
  Type& operator-=(Type other) { return *this = *this - other; }

  std::string toString() const;
  static std::string debugString(Type t);
  static Type fromString(const std::string& str);

  bool isBoxed() const {
    return subtypeOf(BoxedCell);
  }

  bool notBoxed() const {
    return subtypeOf(Cell);
  }

  bool maybeBoxed() const {
    return (*this & BoxedCell) != Bottom;
  }

  bool isPtr() const {
    return subtypeOf(PtrToGen);
  }

  bool notPtr() const {
    return (*this & PtrToGen) == Bottom;
  }

  bool isCounted() const {
    return subtypeOf(Counted);
  }

  bool maybeCounted() const {
    return (*this & Counted) != Bottom;
  }

  bool notCounted() const {
    return !maybeCounted();
  }

  /*
   * Returns true iff this is a union type. Note that this is the
   * plain old set definition of union, so Type::Str, Type::Arr, and
   * Type::Null will all return true.
   */
  bool isUnion() const {
    // This will return true iff more than 1 bit is set in
    // m_bits.
    return (m_bits & (m_bits - 1)) != 0;
  }

  /*
   * Returns true if this value has a known constant DataType enum
   * value, except if the type is exactly Type::Str or Type::Null, it
   * returns true anyway, even though it could be either
   * KindOfStaticString or KindOfString, or KindOfUninit or
   * KindOfNull, respectively.
   *
   * TODO(#3390819): this function should return false for Str and
   * Null.
   *
   * Pre: subtypeOf(Gen | Cls) || equals(None)
   */
  bool isKnownDataType() const {
    if (subtypeOf(Type::None)) return false;
    assert(subtypeOf(Gen | Cls));

    // Some unions that correspond to single KindOfs.  And Type::Str
    // and Type::Null for now for historical reasons.
    if (isString() || isArray() || isNull() || isBoxed()) {
      return true;
    }

    return !isUnion();
  }

  /*
   * Similar to isKnownDataType, with the added restriction that the
   * type not be Boxed.
   */
  bool isKnownUnboxedDataType() const {
    return isKnownDataType() && notBoxed();
  }

  /*
   * Returns true if this requires a register to hold a DataType at
   * runtime.
   */
  bool needsReg() const {
    return subtypeOf(Gen) && !isKnownDataType();
  }

  bool needsStaticBitCheck() const {
    return (*this & (StaticStr | StaticArr)) != Bottom;
  }

  // returns true if definitely not uninitialized
  bool isInit() const {
    return !Uninit.subtypeOf(*this);
  }

  bool maybeUninit() const {
    return !isInit();
  }

  /*
   * Returns true if this is a strict subtype of t2.
   */
  bool strictSubtypeOf(Type t2) const {
    return *this != t2 && subtypeOf(t2);
  }

  /*
   * True if type can have a specialized class.
   */
  bool canSpecializeClass() const {
    return (m_bits & kAnyObj) && !(m_bits & kAnyArr);
  }

  /*
   * True if type can have a specialized array kind.
   */
  bool canSpecializeArrayKind() const {
    return (m_bits & kAnyArr) && !(m_bits & kAnyObj);
  }

  bool canSpecializeAny() const {
    return canSpecializeClass() || canSpecializeArrayKind();
  }

  /*
   * Returns true if this is same type or a subtype of any of the arguments.
   */
  bool subtypeOf(Type t2) const {
    if ((m_bits & t2.m_bits) != m_bits) return false;

    if (canSpecializeClass()) {
      return t2.m_class == nullptr ||
        (m_class != nullptr && m_class->classof(t2.m_class));
    }
    if (canSpecializeArrayKind()) {
      return !t2.m_arrayKindValid ||
        (m_arrayKindValid && m_arrayKind == t2.m_arrayKind);
    }
    return true;
  }

  /*
   * Returns true if and only if this is the same as or a subtype of t2.
   */
  template<typename... Types>
  bool subtypeOfAny(Type t2, Types... ts) const {
    return subtypeOf(t2) || subtypeOfAny(ts...);
  }

  bool subtypeOfAny() const {
    return false;
  }

  /*
   * Returns true if any subtype of this is a subtype of t2.
   */
  bool maybe(Type t2) const {
    return (*this & t2) != Bottom;
  }

  /*
   * Returns true if no subtypes of this are subtypes of t2.
   */
  bool not(Type t2) const {
    return !maybe(t2);
  }

  /*
   * Returns true if this is exactly equal to t2. Be careful: you
   * probably mean subtypeOf.
   */
  bool equals(Type t2) const {
    return m_bits == t2.m_bits && m_extra == t2.m_extra;
  }

  /*
   * Returns the most refined of two types.
   *
   * Pre: the types must not be completely unrelated.
   */
  static Type mostRefined(Type t1, Type t2) {
    assert(t1.subtypeOf(t2) || t2.subtypeOf(t1));
    return t1.subtypeOf(t2) ? t1 : t2;
  }

  bool isArray() const {
    return subtypeOf(Arr);
  }

  bool isBool() const {
    return subtypeOf(Bool);
  }

  bool isDbl() const {
    return subtypeOf(Dbl);
  }

  bool isInt() const {
    return subtypeOf(Int);
  }

  bool isNull() const {
    return subtypeOf(Null);
  }

  bool isObj() const {
    return subtypeOf(Obj);
  }

  bool isRes() const {
    return subtypeOf(Res);
  }

  bool isString() const {
    return subtypeOf(Str);
  }

  bool isCls() const {
    return subtypeOf(Cls);
  }

  const Class* getClass() const {
    assert(canSpecializeClass());
    return m_class;
  }

  bool hasArrayKind() const {
    assert(canSpecializeArrayKind());
    return m_arrayKindValid;
  }

  ArrayData::ArrayKind getArrayKind() const {
    assert(canSpecializeArrayKind() && hasArrayKind());
    return m_arrayKind;
  }

  // Returns a subset of *this containing only the members relating to its
  // specialization.
  //
  // {Int|Str|Obj<C>|BoxedObj<C>}.specializedType() == {Obj<C>|BoxedObj<C>}
  Type specializedType() const {
    assert(isSpecialized());
    if (canSpecializeClass()) return *this & AnyObj;
    if (canSpecializeArrayKind()) return *this & AnyArr;
    not_reached();
  }

  Type innerType() const {
    assert(isBoxed());
    return Type(m_bits >> kBoxShift, m_extra);
  }

  /*
   * unionOf: return the least common predefined supertype of t1 and
   * t2, i.e.. the most refined type t3 such that t1 <: t3 and t2 <:
   * t3. Note that arbitrary union types are possible, but this
   * function always returns one of the predefined types.
   */
  static Type unionOf(Type t1, Type t2) {
    assert(t1 != None && t2 != None);
    if (t1 == t2) return t1;
    if (t1.subtypeOf(t2)) return t2;
    if (t2.subtypeOf(t1)) return t1;
    static const Type union_types[] = {
#   define IRT(name, ...) name,
      IRT_PHP_UNIONS(IRT_BOXES)
#   undef IRT
      Gen,
      PtrToGen,
    };
    Type t12 = t1 | t2;
    for (auto u : union_types) {
      if (t12.subtypeOf(u)) return u;
    }
    not_reached();
  }

  Type box() const {
    assert(subtypeOf(Cell));
    // Boxing Uninit returns InitNull but that logic doesn't belong
    // here.
    assert(not(Uninit) || equals(Cell));
    auto t = Type(m_bits << kBoxShift);
    return
      canSpecializeClass() ? t.specialize(m_class) :
      canSpecializeArrayKind() && hasArrayKind() ? t.specialize(m_arrayKind) :
      t;
  }

  // This computes the type effects of the Unbox opcode.
  Type unbox() const {
    assert(subtypeOf(Gen));
    return (*this & Cell) | (*this & BoxedCell).innerType();
  }

  Type deref() const {
    assert(isPtr());
    auto t = Type(m_bits >> kPtrShift);
    return
      canSpecializeClass() && m_class ? t.specialize(m_class) :
      canSpecializeArrayKind() && hasArrayKind() ? t.specialize(m_arrayKind) :
      t;
  }

  Type derefIfPtr() const {
    assert(subtypeOf(Gen | PtrToGen));
    return isPtr() ? deref() : *this;
  }

  // Returns the "stripped" version of this: dereferenced and unboxed,
  // if applicable.
  Type strip() const {
    return derefIfPtr().unbox();
  }

  Type ptr() const {
    assert(!isPtr());
    assert(subtypeOf(Gen));
    auto t = Type(m_bits << kPtrShift);
    return
      canSpecializeClass() ? t.specialize(m_class) :
      canSpecializeArrayKind() && hasArrayKind() ? t.specialize(m_arrayKind) :
      t;

  }

  Type specialize(const Class* klass) const {
    assert(canSpecializeClass() && m_class == nullptr);
    return Type(m_bits, klass);
  }

  bool isSpecialized() const {
    return (canSpecializeClass() && m_class) ||
      (canSpecializeArrayKind() && m_arrayKindValid);
  }

  Type unspecialize() const {
    return Type(m_bits);
  }

  Type specialize(ArrayData::ArrayKind arrayKind) const {
    assert(canSpecializeArrayKind());
    return Type(m_bits, arrayKind);
  }

  bool canRunDtor() const {
    return
      (*this & (CountedArr | BoxedCountedArr | Obj | BoxedObj |
                Res | BoxedRes))
      != Type::Bottom;
  }

  /*
   * TODO(#3390819): this function does not exactly convert this type
   * into a DataType in cases where a type does not exactly map to a
   * DataType.  For example, Null.toDataType() returns KindOfNull,
   * even though it could be KindOfUninit.
   *
   * Try not to use this function in new code.
   */
  DataType toDataType() const;

  RuntimeType toRuntimeType() const;

  // return true if this corresponds to a type that
  // is passed by value in C++
  bool isSimpleType() const {
    return subtypeOf(Type::Bool)
           || subtypeOf(Type::Int)
           || subtypeOf(Type::Dbl)
           || subtypeOf(Type::Null);
  }

  // return true if this corresponds to a type that
  // is passed by reference in C++
  bool isReferenceType() const {
    return subtypeOf(Type::Str)
           || subtypeOf(Type::Arr)
           || subtypeOf(Type::Obj)
           || subtypeOf(Type::Res);
  }

  int nativeSize() const {
    if (subtypeOf(Type::Int | Type::Func)) return sz::qword;
    if (subtypeOf(Type::Bool))             return sz::byte;
    not_implemented();
  }
};

inline bool operator<(Type a, Type b) { return a.strictSubtypeOf(b); }
inline bool operator>(Type a, Type b) { return b.strictSubtypeOf(a); }
inline bool operator<=(Type a, Type b) { return a.subtypeOf(b); }
inline bool operator>=(Type a, Type b) { return b.subtypeOf(a); }

/*
 * JIT::Type must be small enough for efficient pass-by-value.
 */
static_assert(sizeof(Type) <= 2 * sizeof(uint64_t),
              "JIT::Type should fit in (2 * sizeof(uint64_t))");

/*
 * Return the most refined type that can be used to represent the type
 * in a live TypedValue.
 */
Type liveTVType(const TypedValue* tv);

/*
 * Return the boxed version of the input type, taking into account php
 * semantics and subtle implementation details.
 */
Type boxType(Type);

struct TypeConstraint {
  /* implicit */ TypeConstraint(DataTypeCategory cat = DataTypeGeneric,
                                Type type = Type::Gen)
    : category(cat)
    , weak(false)
    , knownType(type)
  {}

  std::string toString() const;

  TypeConstraint& setWeak(bool w = true) {
    weak = w;
    return *this;
  }

  // category starts as DataTypeGeneric and is refined to more specific values
  // by consumers of the type.
  DataTypeCategory category;

  // if innerCat is set, this object represents a constraint on the inner type
  // of the current value. It is set and cleared when the constraint code
  // traces through an operation that unboxes or boxes an operand,
  // respectively.
  boost::optional<DataTypeCategory> innerCat;

  // If weak is true, the consumer of the value being constrained doesn't
  // actually want to constrain the guard (if found). Most often used to figure
  // out if a type can be used without further constraining guards.
  bool weak;

  // knownType represents an upper bound for the type of the guard, which is
  // known from static analysis or other guards that have been appropriately
  // constrained. It can be used to convert some guards into asserts.
  Type knownType;
};

}}

#endif
