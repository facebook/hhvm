/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include "hphp/runtime/vm/translator/runtime-type.h"

namespace HPHP {
namespace Transl {
struct DynLocation;
}
namespace JIT {

#define IRT_BOXES(name, bit)                                            \
  IRT(name,             (bit))                                          \
  IRT(Boxed##name,      (bit) << kBoxShift)                             \
  IRT(PtrTo##name,      (bit) << kPtrShift)                             \
  IRT(PtrToBoxed##name, (bit) << kPtrBoxShift)

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
  c(Obj,          1ULL << 9)
// Boxed*:       10-19
// PtrTo*:       20-29
// PtrToBoxed*:  30-39

// This list should be in non-decreasing order of specificity
#define IRT_PHP_UNIONS(c)                                               \
  c(Null,          kUninit|kInitNull)                                   \
  c(Str,           kStaticStr|kCountedStr)                              \
  c(Arr,           kStaticArr|kCountedArr)                              \
  c(UncountedInit, kInitNull|kBool|kInt|kDbl|kStaticStr|kStaticArr)     \
  c(Uncounted,     kUncountedInit|kUninit)                              \
  c(Cell,          kUncounted|kStr|kArr|kObj)

#define IRT_RUNTIME                                                     \
  IRT(Cls,         1ULL << 40)                                          \
  IRT(Func,        1ULL << 41)                                          \
  IRT(VarEnv,      1ULL << 42)                                          \
  IRT(NamedEntity, 1ULL << 43)                                          \
  IRT(FuncCls,     1ULL << 44) /* {Func*, Cctx} */                      \
  IRT(FuncObj,     1ULL << 45) /* {Func*, Obj} */                       \
  IRT(Cctx,        1ULL << 46) /* Class* with the lowest bit set,  */   \
                               /* as stored in ActRec.m_cls field  */   \
  IRT(RetAddr,     1ULL << 47) /* Return address */                     \
  IRT(StkPtr,      1ULL << 48) /* stack pointer */                      \
  IRT(FramePtr,    1ULL << 49) /* frame pointer */                      \
  IRT(TCA,         1ULL << 50)                                          \
  IRT(ActRec,      1ULL << 51)                                          \
  IRT(None,        1ULL << 52)                                          \
  IRT(CacheHandle, 1ULL << 53) /* TargetCache::CacheHandle */

// The definitions for these are in ir.cpp
#define IRT_UNIONS                                                      \
  IRT(Ctx,         kObj|kCctx)                                          \
  IRT(FuncCtx,     kFuncCls|kFuncObj)

// Gen, Counted, PtrToGen, and PtrToCounted are here instead of
// IRT_PHP_UNIONS because boxing them (e.g., BoxedGen, PtrToBoxedGen)
// would be nonsense types.
#define IRT_SPECIAL                                                 \
  IRT(Bottom,       0)                                              \
  IRT(Counted,      kCountedStr|kCountedArr|kObj|kBoxedCell)        \
  IRT(PtrToCounted, kCounted << kPtrShift)                          \
  IRT(Gen,          kCell|kBoxedCell)                               \
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
#define IR_TYPES IRT_USERLAND(IRT_BOXES) IRT_RUNTIME IRT_UNIONS IRT_SPECIAL

class Type {
  typedef uint64_t bits_t;

  static const size_t kBoxShift = 10;
  static const size_t kPtrShift = kBoxShift * 2;
  static const size_t kPtrBoxShift = kBoxShift + kPtrShift;

  enum TypeBits {
#define IRT(name, bits) k##name = (bits),
  IR_TYPES
#undef IRT
  };

  union {
    bits_t m_bits;
    TypeBits m_typedBits;
  };
  const Class* m_class;

  // private ctor to build a specialized type
  explicit Type(bits_t bits, const Class* klass)
    : m_bits(bits), m_class(klass)
  {}

public:
# define IRT(name, ...) static const Type name;
  IR_TYPES
# undef IRT

  explicit Type(bits_t bits = kNone)
    : m_bits(bits), m_class(nullptr)
  {}

  size_t hash() const {
    return hash_int64_pair(m_bits, reinterpret_cast<uintptr_t>(m_class));
  }

  bool operator==(Type other) const {
    return equals(other);
  }

  bool operator!=(Type other) const {
    return !operator==(other);
  }

  Type operator|(Type other) const {
    assert(m_class == nullptr && other.m_class == nullptr);
    return Type(m_bits | other.m_bits);
  }

  Type operator&(Type other) const {
    if (m_class != nullptr && other.m_class != nullptr) {
      if (m_class->classof(other.m_class)) {
        return Type(m_bits & other.m_bits).specialize(other.m_class);
      } else if (other.m_class->classof(m_class)) {
        return Type(m_bits & other.m_bits).specialize(m_class);
      }
    }
    return Type(m_bits & other.m_bits);
  }

  Type operator-(Type other) const {
    assert(m_class == nullptr && other.m_class == nullptr);
    return Type(m_bits & ~other.m_bits);
  }

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
   * value, which allows us to avoid several checks.
   */
  bool isKnownDataType() const {
    // Calling this function with a type that can't be in a TypedValue isn't
    // meaningful
    assert(subtypeOf(Gen | Cls));
    // Str, Arr and Null are technically unions but can each be
    // represented by one data type. Same for a union that consists of
    // nothing but boxed types.
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
   * Returns true if this is same type or a subtype of any of the arguments.
   */
  bool subtypeOf(Type t2) const {
    return (m_bits & t2.m_bits) == m_bits
           && (t2.m_class == nullptr
               || (m_class != nullptr
                   && m_class->classof(t2.m_class)));
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
    return m_bits == t2.m_bits && m_class == t2.m_class;
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

  bool isString() const {
    return subtypeOf(Str);
  }

  const Class* getClass() const {
    assert(isObj());
    return m_class;
  }

  Type innerType() const {
    assert(isBoxed());
    return Type(m_bits >> kBoxShift);
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
    return Type(m_bits << kBoxShift);
  }

  // This computes the type effects of the Unbox opcode.
  Type unbox() const {
    assert(subtypeOf(Gen));
    return (*this & Cell) | (*this & BoxedCell).innerType();
  }

  Type deref() const {
    assert(isPtr());
    return Type(m_bits >> kPtrShift);
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
    return Type(m_bits << kPtrShift);
  }

  Type specialize(const Class* klass) const {
    assert(isObj() && m_class == nullptr);
    return Type(m_bits, klass);
  }

  bool canRunDtor() const {
    return
      (*this & (Obj | CountedArr | BoxedObj | BoxedCountedArr))
      != Type::Bottom;
  }

  // translates a compiler Type to an HPHP::DataType
  DataType toDataType() const {
    assert(!isPtr());
    if (isBoxed()) {
      return KindOfRef;
    }

    // Order is important here: types must progress from more specific
    // to less specific to return the most specific DataType.
    if (subtypeOf(None))          return KindOfInvalid;
    if (subtypeOf(Uninit))        return KindOfUninit;
    if (subtypeOf(Null))          return KindOfNull;
    if (subtypeOf(Bool))          return KindOfBoolean;
    if (subtypeOf(Int))           return KindOfInt64;
    if (subtypeOf(Dbl))           return KindOfDouble;
    if (subtypeOf(StaticStr))     return KindOfStaticString;
    if (subtypeOf(Str))           return KindOfString;
    if (subtypeOf(Arr))           return KindOfArray;
    if (subtypeOf(Obj))           return KindOfObject;
    if (subtypeOf(Cls))           return KindOfClass;
    if (subtypeOf(UncountedInit)) return KindOfUncountedInit;
    if (subtypeOf(Uncounted))     return KindOfUncounted;
    if (subtypeOf(Gen))           return KindOfAny;
    not_reached();
  }

  static Type fromDataType(DataType outerType,
                           DataType innerType = KindOfInvalid,
                           const Class* klass = nullptr) {
    assert(innerType != KindOfRef);

    switch (outerType) {
      case KindOfInvalid       : return None;
      case KindOfUninit        : return Uninit;
      case KindOfNull          : return InitNull;
      case KindOfBoolean       : return Bool;
      case KindOfInt64         : return Int;
      case KindOfDouble        : return Dbl;
      case KindOfStaticString  : return StaticStr;
      case KindOfString        : return Str;
      case KindOfArray         : return Arr;
      case KindOfObject        : {
        if (klass != nullptr) {
          return Obj.specialize(klass);
        } else {
          return Obj;
        }
      }
      case KindOfClass         : return Cls;
      case KindOfUncountedInit : return UncountedInit;
      case KindOfUncounted     : return Uncounted;
      case KindOfAny           : return Gen;
      case KindOfRef: {
        if (innerType == KindOfInvalid) {
          return BoxedCell;
        } else {
          return fromDataType(innerType).box();
        }
      }
      default                  : not_reached();
    }
  }

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
           || subtypeOf(Type::Obj);
  }

  // In tx64, KindOfUnknown is used to represent Variants (Type::Cell).
  // fromDataType() maps this to Type::None, which must be mapped
  // back to Type::Cell. This is not the best place to handle this.
  // See task #208726.
  static Type fromDataTypeWithCell(DataType type) {
    Type t = fromDataType(type);
    return t.equals(Type::None) ? Type::Cell : t;
  }

  static Type fromDataTypeWithRef(DataType outerType, bool isRef) {
    Type t = fromDataTypeWithCell(outerType);
    return isRef ? t.box() : t;
  }

  static Type fromRuntimeType(const Transl::RuntimeType& rtt) {
    return fromDataType(rtt.outerType(), rtt.innerType(), rtt.knownClass());
  }

  static Type fromDynLocation(const Transl::DynLocation* dynLoc);
};

static_assert(sizeof(Type) <= 2 * sizeof(uint64_t),
              "JIT::Type should fit in (2 * sizeof(uint64_t))");

}}

#endif
