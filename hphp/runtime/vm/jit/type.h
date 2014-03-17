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
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/type-array.h"

#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/jit/types.h"

#include "hphp/util/data-block.h"

#include "folly/Optional.h"

#include <cstdint>
#include <cstring>

namespace HPHP {
struct Func;

namespace JIT {
struct DynLocation;
struct RuntimeType;
}
namespace JIT {

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
  IRT(Cctx,        1ULL << 48) /* Class* with the lowest bit set,  */   \
                               /* as stored in ActRec.m_cls field  */   \
  IRT(RetAddr,     1ULL << 49) /* Return address */                     \
  IRT(StkPtr,      1ULL << 50) /* stack pointer */                      \
  IRT(FramePtr,    1ULL << 51) /* frame pointer */                      \
  IRT(TCA,         1ULL << 52)                                          \
  IRT(ActRec,      1ULL << 53)                                          \
  IRT(RDSHandle,   1ULL << 54) /* RDS::Handle */                        \
  IRT(Nullptr,     1ULL << 55)

// The definitions for these are in ir.cpp
#define IRT_UNIONS                                                      \
  IRT(Ctx,         kObj|kCctx)                                          \

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

  bits_t m_bits:63;
  bool m_hasConstVal:1;

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
    JIT::TCA m_tcaVal;
    RDS::Handle m_rdsHandleVal;
    TypedValue* m_ptrVal;

    // Specialization for object classes and array kinds.
    const Class* m_class;
    struct {
      bool m_arrayKindValid;
      ArrayData::ArrayKind m_arrayKind;
      uint64_t m_padding:48; // We want all 8 bytes of m_extra to be defined.
    };
  };

  bool checkValid() const;

  explicit Type(bits_t bits, uintptr_t extra = 0)
    : m_bits(bits)
    , m_hasConstVal(false)
    , m_extra(extra)
  {
    assert(checkValid());
  }

  explicit Type(bits_t bits, const Class* klass)
    : m_bits(bits)
    , m_hasConstVal(false)
    , m_class(klass)
  {
    assert(checkValid());
  }

  explicit Type(bits_t bits, ArrayData::ArrayKind arrayKind)
    : m_bits(bits)
    , m_hasConstVal(false)
    , m_arrayKindValid(true)
    , m_arrayKind(arrayKind)
    , m_padding(0)
  {
    assert(checkValid());
  }

  static bits_t bitsFromDataType(DataType outer, DataType inner);

  // combine, Union, and Intersect are used for operator| and operator&. See
  // .cpp for details.
  template<typename Oper>
  static Type combine(bits_t newBits, Type a, Type b);

  struct Union;
  struct Intersect;

public:
# define IRT(name, ...) static const Type name;
  IR_TYPES
# undef IRT

  Type()
    : m_bits(kBottom)
    , m_hasConstVal(false)
    , m_extra(0)
  {}

  explicit Type(DataType outerType, DataType innerType = KindOfInvalid)
    : m_bits(bitsFromDataType(outerType, innerType))
    , m_hasConstVal(false)
    , m_extra(0)
  {}

  explicit Type(const RuntimeType& rtt);
  explicit Type(const DynLocation* dl);

  size_t hash() const {
    return hash_int64_pair(m_bits | (bits_t(m_hasConstVal) << 63), m_extra);
  }

  Type& operator=(Type b) {
    m_bits = b.m_bits;
    m_hasConstVal = b.m_hasConstVal;
    m_extra = b.m_extra;
    return *this;
  }

  std::string constValString() const;
  std::string toString() const;
  static std::string debugString(Type t);
  static Type fromString(const std::string& str);

  ////////// Support for constants //////////

 private:
  // forConst returns the Type to use for a given C++ value. The only
  // interesting case is int/bool disambiguation.  Enums are treated as ints.
  template<class T>
  static typename std::enable_if<
    std::is_integral<T>::value || std::is_enum<T>::value,
    Type
    >::type forConst(T) {
    return std::is_same<T,bool>::value ? Type::Bool : Type::Int;
  }
  static Type forConst(const HPHP::Func*)        { return Func; }
  static Type forConst(const Class*)             { return Cls; }
  static Type forConst(JIT::TCA)                 { return TCA; }
  static Type forConst(double)                   { return Dbl; }
  static Type forConst(const StringData* sd) {
    assert(sd->isStatic());
    return StaticStr;
  }
  static Type forConst(const ArrayData* ad) {
    assert(ad->isStatic());
    return StaticArr.specialize(ad->kind());
  }

 public:
  // Returns true iff this Type represents a known value.
  bool isConst() const {
    return m_hasConstVal || subtypeOfAny(Uninit, InitNull, Nullptr);
  }

  // Returns true iff this Type is a constant value of type t.
  bool isConst(Type t) const {
    return subtypeOf(t) && isConst();
  }

  // Returns true iff this Type represents the constant val, using the same C++
  // type -> Type mapping as Type::cns().
  template<typename T>
  bool isConst(T val) const {
    return subtypeOf(cns(val));
  }

  template<typename T>
  static Type cns(T val, Type ret) {
    assert(!ret.m_hasConstVal);
    ret.m_hasConstVal = true;

    static_assert(sizeof(T) <= sizeof ret,
                  "Constant data was larger than supported");
    static_assert(std::is_pod<T>::value,
                  "Constant data wasn't a pod");
    const auto toCopy = constToBits_detail::promoteIfNeeded(val);
    static_assert(sizeof toCopy == 8,
                  "Unexpected size for toCopy");
    std::memcpy(&ret.m_extra, &toCopy, sizeof toCopy);
    return ret;
  }

  template<typename T>
  static Type cns(T val) {
    return cns(val, forConst(val));
  }

  static Type cns(std::nullptr_t) {
    return Type::Nullptr;
  }

  static Type cns(const TypedValue tv) {
    auto ret = [&] {
      switch (tv.m_type) {
        case KindOfClass:
        case KindOfUninit:
        case KindOfNull:
        case KindOfBoolean:
        case KindOfInt64:
        case KindOfDouble:
        case KindOfStaticString:
          return Type(tv.m_type);

        case KindOfString:
          return forConst(tv.m_data.pstr);

        case KindOfArray:
          return forConst(tv.m_data.parr);

        default:
          always_assert(false && "Invalid KindOf for constant TypedValue");
      }
    }();
    ret.m_hasConstVal = true;
    ret.m_extra = tv.m_data.num;
    return ret;
  }

  // If this represents a constant value, return the most specific strict
  // supertype of this we can represent. In most cases this just erases the
  // constant value: Int<4> -> Int, Dbl<2.5> -> Dbl. Arrays are special since
  // they can be both constant and specialized, so keep the array's kind in the
  // resulting type.
  Type dropConstVal() const {
    if (!m_hasConstVal) return *this;
    assert(!isUnion());

    if (subtypeOf(StaticArr)) {
      return Type::StaticArr.specialize(arrVal()->kind());
    }
    return Type(m_bits);
  }

  bool hasRawVal() const {
    return m_hasConstVal;
  }

  uint64_t rawVal() const {
    assert(m_hasConstVal);
    return m_intVal;
  }

  bool boolVal() const {
    assert(subtypeOf(Bool) && m_hasConstVal);
    assert(m_boolVal <= 1);
    return m_boolVal;
  }

  int64_t intVal() const {
    assert(subtypeOf(Int) && m_hasConstVal);
    return m_intVal;
  }

  double dblVal() const {
    assert(subtypeOf(Dbl) && m_hasConstVal);
    return m_dblVal;
  }

  const StringData* strVal() const {
    assert(subtypeOf(StaticStr) && m_hasConstVal);
    return m_strVal;
  }

  const ArrayData* arrVal() const {
    assert(subtypeOf(StaticArr) && m_hasConstVal);
    return m_arrVal;
  }

  const HPHP::Func* funcVal() const {
    assert(subtypeOf(Func) && m_hasConstVal);
    return m_funcVal;
  }

  const Class* clsVal() const {
    assert(subtypeOf(Cls) && m_hasConstVal);
    return m_clsVal;
  }

  RDS::Handle rdsHandleVal() const {
    assert(subtypeOf(RDSHandle) && m_hasConstVal);
    return m_rdsHandleVal;
  }

  JIT::TCA tcaVal() const {
    assert(subtypeOf(TCA) && m_hasConstVal);
    return m_tcaVal;
  }

  ////////// Methods to query properties of the type //////////

  bool isBoxed() const {
    return subtypeOf(BoxedCell);
  }

  bool notBoxed() const {
    assert(subtypeOf(Gen));
    return subtypeOf(Cell);
  }

  bool maybeBoxed() const {
    return maybe(BoxedCell);
  }

  bool isPtr() const {
    return subtypeOf(PtrToGen);
  }

  bool notPtr() const {
    return not(PtrToGen);
  }

  bool isCounted() const {
    return subtypeOf(Counted);
  }

  bool maybeCounted() const {
    return maybe(Counted);
  }

  bool notCounted() const {
    return not(Counted);
  }

  /*
   * Returns true iff this is a union type. Note that this is the plain old set
   * definition of union, so Type::Str, Type::Arr, and Type::Null will all
   * return true.
   */
  bool isUnion() const {
    // This will return true iff more than 1 bit is set in m_bits.
    return (m_bits & (m_bits - 1)) != 0;
  }

  /*
   * Returns true if this value has a known constant DataType enum value.  If
   * the type is exactly Type::Str or Type::Null it returns true anyway, even
   * though it could be either KindOfStaticString or KindOfString, or
   * KindOfUninit or KindOfNull, respectively.
   *
   * TODO(#3390819): this function should return false for Str and Null.
   *
   * Pre: subtypeOf(StackElem)
   */
  bool isKnownDataType() const {
    assert(subtypeOf(StackElem));

    // Some unions that correspond to single KindOfs.  And Type::Str
    // and Type::Null for now for historical reasons.
    if (subtypeOfAny(Str, Arr, Null, BoxedCell)) {
      return true;
    }

    return !isUnion();
  }

  /*
   * Similar to isKnownDataType, with the added restriction that the type not
   * be Boxed.
   */
  bool isKnownUnboxedDataType() const {
    return isKnownDataType() && notBoxed();
  }

  /*
   * Returns true if this requires a register to hold a DataType at runtime.
   */
  bool needsReg() const {
    return subtypeOf(StackElem) && !isKnownDataType();
  }

  bool needsValueReg() const {
    return !subtypeOfAny(Uninit, InitNull, Nullptr);
  }

  bool needsStaticBitCheck() const {
    return maybe(StaticStr | StaticArr);
  }

  bool canRunDtor() const {
    return maybe(CountedArr | BoxedCountedArr | Obj | BoxedObj |
                 Res | BoxedRes);
  }

  // return true if this corresponds to a type that is passed by value in C++
  bool isSimpleType() const {
    return subtypeOfAny(Bool, Int, Dbl, Null);
  }

  // return true if this corresponds to a type that is passed by reference in
  // C++
  bool isReferenceType() const {
    return subtypeOfAny(Str, Arr, Obj, Res);
  }

  int nativeSize() const {
    if (subtypeOf(Type::Int | Type::Func)) return sz::qword;
    if (subtypeOf(Type::Bool))             return sz::byte;
    not_implemented();
  }

  ////////// Support for specialized types: Obj classes and Arr kinds //////////

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

  Type specialize(const Class* klass) const {
    assert(canSpecializeClass() && m_class == nullptr);
    return Type(m_bits, klass);
  }

  Type specialize(ArrayData::ArrayKind arrayKind) const {
    assert(canSpecializeArrayKind());
    return Type(m_bits, arrayKind);
  }

  bool isSpecialized() const {
    return (canSpecializeClass() && getClass()) ||
      (canSpecializeArrayKind() && hasArrayKind());
  }

  Type unspecialize() const {
    return Type(m_bits);
  }

  const Class* getClass() const {
    assert(canSpecializeClass());
    return m_class;
  }

  bool hasArrayKind() const {
    assert(canSpecializeArrayKind());
    return m_hasConstVal || m_arrayKindValid;
  }

  ArrayData::ArrayKind getArrayKind() const {
    assert(hasArrayKind());
    return m_hasConstVal ? m_arrVal->kind() : m_arrayKind;
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

  ////////// Methods for comparing types //////////

  /*
   * Returns true iff this represents a non-strict subset of t2.
   */
  bool subtypeOf(Type t2) const {
    // First, check for any members in m_bits that aren't in t2.m_bits.
    if ((m_bits & t2.m_bits) != m_bits) return false;

    // If t2 is a constant, we must be the same constant or Bottom.
    if (t2.m_hasConstVal) {
      assert(!t2.isUnion());
      return m_bits == kBottom || (m_hasConstVal && m_extra == t2.m_extra);
    }

    // If t2 is specialized, we must either not be eligible for the same kind
    // of specialization (Int <= {Int|Arr<Packed>}) or have a specialization
    // that is a subtype of t2's specialization.
    if (t2.isSpecialized()) {
      if (t2.canSpecializeClass()) {
        return !canSpecializeClass() ||
          (m_class != nullptr && m_class->classof(t2.m_class));
      }

      return !canSpecializeArrayKind() ||
        (m_arrayKindValid && getArrayKind() == t2.getArrayKind());
    }

    return true;
  }

  template<typename... Types>
  bool subtypeOfAny(Type t2, Types... ts) const {
    return subtypeOf(t2) || subtypeOfAny(ts...);
  }

  bool subtypeOfAny() const {
    return false;
  }

  /*
   * Returns true if this is a strict subtype of t2.
   */
  bool strictSubtypeOf(Type t2) const {
    return *this != t2 && subtypeOf(t2);
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
    return m_bits == t2.m_bits && m_hasConstVal == t2.m_hasConstVal &&
      m_extra == t2.m_extra;
  }

  bool operator==(Type t2) const { return equals(t2); }
  bool operator!=(Type t2) const { return !operator==(t2); }

  ////////// Methods for combining Types in various ways //////////

  /*
   * Standard set operations: union, intersection, and difference.
   */
  Type operator|(Type other) const;
  Type& operator|=(Type other) { return *this = *this | other; }

  Type operator&(Type other) const;
  Type& operator&=(Type other) { return *this = *this & other; }

  Type operator-(Type other) const;
  Type& operator-=(Type other) { return *this = *this - other; }

  /*
   * unionOf: return the least common predefined supertype of t1 and t2,
   * i.e.. the most refined type t3 such that t1 <= t3 and t2 <= t3. Note that
   * arbitrary union types are possible using operator| but this function
   * always returns one of the predefined types.
   */
  static Type unionOf(Type t1, Type t2);

  ////////// Support for inner types of boxed and pointer types //////////

  Type box() const {
    assert(subtypeOf(Cell));
    // Boxing Uninit returns InitNull but that logic doesn't belong
    // here.
    assert(not(Uninit) || equals(Cell));
    return Type(m_bits << kBoxShift,
                isSpecialized() && !m_hasConstVal ? m_extra : 0);
  }

  // This computes the type effects of the Unbox opcode.
  Type unbox() const {
    assert(subtypeOf(Gen));
    return (*this & Cell) | (*this & BoxedCell).innerType();
  }

  Type innerType() const {
    assert(isBoxed() || equals(Bottom));
    return Type(m_bits >> kBoxShift, m_extra);
  }

  Type deref() const {
    assert(isPtr());
    return Type(m_bits >> kPtrShift, isSpecialized() ? m_extra : 0);
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
    return Type(m_bits << kPtrShift,
                isSpecialized() && !m_hasConstVal ? m_extra : 0);
  }

  ////////// Methods for talking to other type systems in the VM //////////

  /*
   * TODO(#3390819): this function does not exactly convert this type into a
   * DataType in cases where a type does not exactly map to a DataType.  For
   * example, Null.toDataType() returns KindOfNull, even though it could be
   * KindOfUninit.
   *
   * Try not to use this function in new code.
   */
  DataType toDataType() const;

  RuntimeType toRuntimeType() const;
};

typedef folly::Optional<Type> OptType;

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

/*
 * Create a Type from a RepoAuthType.
 */
Type convertToType(RepoAuthType ty);

//////////////////////////////////////////////////////////////////////

struct TypeConstraint {
  /* implicit */ TypeConstraint(DataTypeCategory cat = DataTypeGeneric,
                                Type aType = Type::Gen,
                                DataTypeCategory inner = DataTypeGeneric)
    : category(cat)
    , innerCat(inner)
    , weak(false)
    , assertedType(aType)
  {}

  std::string toString() const;

  TypeConstraint& setWeak(bool w = true) {
    weak = w;
    return *this;
  }

  // category starts as DataTypeGeneric and is refined to more specific values
  // by consumers of the type.
  DataTypeCategory category;

  // When a value is boxed, innerCat is used to determine how we can relax the
  // inner type.
  DataTypeCategory innerCat;

  // If weak is true, the consumer of the value being constrained doesn't
  // actually want to constrain the guard (if found). Most often used to figure
  // out if a type can be used without further constraining guards.
  bool weak;

  // It's fairly common to emit an AssertType op with a type that is less
  // specific than the current guard type, but more specific than the type the
  // guard will eventually be relaxed to. We want to simplify these
  // instructions away, and when we do, we remember their type in assertedType.
  Type assertedType;
};

const int kTypeWordOffset = offsetof(TypedValue, m_type) % 8;
const int kTypeShiftBits = kTypeWordOffset * CHAR_BIT;

// left shift an immediate DataType, for type, to the correct position
// within one of the registers used to pass a TypedValue by value.
inline uint64_t toDataTypeForCall(Type type) {
  return uint64_t(type.toDataType()) << kTypeShiftBits;
}


}}

#endif
